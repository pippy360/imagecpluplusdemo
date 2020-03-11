#include <stdlib.h> // required for malloc definition
#include <opencv2/opencv.hpp>
#include <sstream>

#include <map>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "boostGeometryTypes.hpp"
#include "mainImageProcessingFunctions.hpp"

#include "shapeNormalise.hpp"
//#include "ImageHash.hpp"

using namespace cv;

class ValWrapper{
public:

    ValWrapper(size_t size, unsigned char *ptr)
        : ptr_(ptr),
        size_(size),
        val_(emscripten::typed_memory_view(size, ptr))
    {
//        std::cout << "ptr created with size " << size_ << std::endl;

    }

    ~ValWrapper()
    {
//        std::cout << "ptr free with size " << size_ << std::endl;
        free(ptr_);
    }
    unsigned char *ptr_;
    size_t size_;
    emscripten::val val_;
};

class ValHolder {
public:
    ValHolder(size_t size)
            :edgeImage(size, (unsigned char *) malloc(size)),
            outputImage1(size, (unsigned char *) malloc(size)),
            outputImage2(size, (unsigned char *) malloc(size)),
            outputImage3(size, (unsigned char *) malloc(size))
    {
    }


    string shapeStr;
    ValWrapper edgeImage;
    ValWrapper outputImage1;
    ValWrapper outputImage2;
    ValWrapper outputImage3;
};

RNG rng(12345);

string calcMatrixFromString(string shapeStr, int output_width=400, double zoom=1) {
    ring_t shape;
    bg::read_wkt(shapeStr, shape);
    Mat m = calcMatrix(shape, 0, output_width, zoom);

    std::stringstream polygonString;
    polygonString << "{ \"mat\" : [";
    double *data = (double *) m.data;
    for (int i = 0; i < 2; i++) {
        if (i > 0)
            polygonString << ",";

        polygonString << "[";
        for (int j = 0; j < 3; j++) {
            if (j > 0)
                polygonString << ",";

            polygonString << data[(3*i)+j];
        }
        polygonString << "]";
    }
    polygonString << ",[0,0,1]]}";
    return polygonString.str();
}

void getImageFragmentFromShape(uintptr_t img_in, int width, int height, ValHolder *valsOut, string shapeStr, int output_width=400, double zoom=1)
{
    point_t p;
    ring_t transformedPoly;
    ring_t shape;

    bg::read_wkt(shapeStr, shape);

    Mat m = calcMatrix(shape, 0, output_width, zoom);

    Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
    Mat image(cv::Size(width, height), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);

    warpAffine(image, outputImage, m, outputImage.size());
    memcpy(valsOut->outputImage2.ptr_, outputImage.data, output_width*output_width*4);
}

std::string findMatchesForImageFromCanvas(
        uintptr_t img_in, int img_in_width, int img_in_height,
        uintptr_t img_in2, int img_in2_width, int img_in2_height,
        int rotation,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        bool flush_cache)
{
    Mat image(cv::Size(img_in_width, img_in_height), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);
    Mat image2(cv::Size(img_in2_width, img_in2_height), CV_8UC4, (void *) img_in2, cv::Mat::AUTO_STEP);
    auto vec = findMatches(
            image,
            image2,
            thresh,
            ratio,
            kernel_size,
            blur_width,
            200,//int areaThresh=200
            flush_cache
    );
    std::cout << "findMatchesForImageFromCanvas called, with this many matches: " << vec.size() << std::endl;

    std::stringstream polygonString;
    polygonString << "{ ";

    for (int i = 0; i < vec.size(); i++) {
        auto v = vec[i];
        //FIXME: we need to check that no two hashes are the same, otherwise we can create invalid json
        auto [shape1, shape2, hash1, hash2] = v;
        if (i > 0) {
            polygonString << ",";
        }

        polygonString << "\"" << ImageHash::convertHashToString(hash1)
                << "\" : [\"" << bg::wkt(shape1) << "\", \"" << bg::wkt(shape2) << "\"]";
    }
    polygonString << "} ";
    return polygonString.str();
}

std::string getAllTheHashesForImageFromCanvas(
        uintptr_t img_in,
        int width,
        int height,
        int rotation,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width
)
{
    std::cout << "getAllTheHashesForImageFromCanvas called" << std::endl;

    Mat image(cv::Size(width, height), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);
    auto vec = getAllTheHashesForImage(
        image,
        rotation,
        thresh,
        ratio,
        kernel_size,
        blur_width
            );

    //FIXME: rotate the image here rather than doing it for each fragment later
    std::stringstream polygonString;
    polygonString << "{ ";
    for (int i = 0; i < vec.size(); i++) {
        auto v = vec[i];
        //FIXME: we need to check that no two hashes are the same, otherwise we can create invalid json
        auto [shape, hash] = v;
        if (i > 0) {
            polygonString << ",";
        }

        polygonString << "\"" << ImageHash::convertHashToString(hash) << "\" : \"" << bg::wkt(shape) << "\"" << endl;
    }
    polygonString << "} ";
    return polygonString.str();
}



string getShapeWithPointInside(
        uintptr_t img_in_ptr,
        int width,
        int height,
        double x,
        double y,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh
                )
{
    Mat img_in(cv::Size(width, height), CV_8UC4, (void *) img_in_ptr, cv::Mat::AUTO_STEP);
    Mat img = img_in.clone();

    Mat src_gray;
    cvtColor( img, src_gray, COLOR_BGRA2GRAY );//FIXME: detect and assert
    Mat canny_output = applyCanny(src_gray, thresh, kernel_size, ratio, blur_width);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0));

    vector<ring_t> shapes = extractShapesFromContours(contours, areaThresh);

    std::stringstream polygonString;
    for (auto &shape : shapes) {
        if (bg::within(point_t(x, y), shape))
            polygonString << bg::wkt(shape) << endl;
    }
    return polygonString.str();
}

void encode(
        uintptr_t img_in_ptr,
        int width,
        int height,
        ValHolder *valsOut,
        int thresh = 100,
        int ratio=3,
        int kernel_size=3,
        int blur_width=3,
        int areaThresh=200
                )
{
    Mat img_in(cv::Size(width, height), CV_8UC4, (void *) img_in_ptr, cv::Mat::AUTO_STEP);
    Mat img = img_in.clone();

    Mat src_gray;
    cvtColor( img, src_gray, COLOR_BGRA2GRAY );//FIXME: detect and assert

    Mat _canny_output = applyCanny(src_gray, thresh, kernel_size, ratio, blur_width);
    Mat imageCannyOut;
    cvtColor(_canny_output, imageCannyOut, COLOR_GRAY2RGBA);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(_canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0));

    //Draw contours------
    Mat contours_img = imageCannyOut.clone();
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255), 255 );
        drawContours( contours_img, contours, i, color, 1, 8, hierarchy, 0, Point() );
    }
    //--------

    //Draw hulls------
    Mat hulls_img = imageCannyOut.clone();
    for ( int i = 0; i < contours.size(); i++ )
    {
        vector<Point> hull;
        convexHull( contours[i], hull );
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255), 255 );
        drawContours( hulls_img, vector<vector<Point> >(1,hull), -1, color, 2 );
    }

    Mat valid_hulls_img = imageCannyOut.clone();
    int  failed_area = 0;
    for ( int i = 0; i < contours.size(); i++ )
    {
        vector<Point> hull;
        convexHull( contours[i], hull );
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255), 255 );
        ring_t outPoly;
        if (!convert_to_boost(hull, outPoly)) {
            continue;
        }

        if (bg::area(outPoly) <= areaThresh){
            failed_area++;
            continue;
        }

        drawContours( valid_hulls_img, vector<vector<Point> >(1,hull), -1, color, 2 );
    }
    cout << "Number of shapes which failed area: " << failed_area << endl;

    //--------

    cout << "contours.size(): " << contours.size() << endl;
    vector<ring_t> shapes = extractShapesFromContours(contours, areaThresh);
    cout << "shapes.size(): " << shapes.size() << endl;

    std::stringstream polygonString;
    for (auto &shape : shapes) {
        polygonString << bg::wkt(shape) << endl;
    }

    size_t size = width * height * 4;

    valsOut->shapeStr = polygonString.str();
    memcpy(valsOut->outputImage1.ptr_, contours_img.data, size);
    memcpy(valsOut->outputImage2.ptr_, hulls_img.data, size);
    memcpy(valsOut->outputImage3.ptr_, valid_hulls_img.data, size);

    memcpy(valsOut->edgeImage.ptr_, imageCannyOut.data, size);
}

using namespace emscripten;

EMSCRIPTEN_BINDINGS(my_value_example) {

    class_<ValWrapper>("ValWrapper")
            .property("val_", &ValWrapper::val_)
        ;

    class_<ValHolder>("ValHolder")
            .constructor<size_t>()
            .property("shapeStr", &ValHolder::shapeStr)
            .property("edgeImage", &ValHolder::edgeImage)
            .property("outputImage1", &ValHolder::outputImage1)
            .property("outputImage2", &ValHolder::outputImage2)
            .property("outputImage3", &ValHolder::outputImage3)
        ;

    emscripten::function("encode", &encode, allow_raw_pointers());
    emscripten::function("getShapeWithPointInside", &getShapeWithPointInside, allow_raw_pointers());
    emscripten::function("getImageFragmentFromShape", &getImageFragmentFromShape, allow_raw_pointers());
    emscripten::function("calcMatrixFromString", &calcMatrixFromString);
    emscripten::function("getAllTheHashesForImageFromCanvas", &getAllTheHashesForImageFromCanvas);
    emscripten::function("findMatchesForImageFromCanvas", &findMatchesForImageFromCanvas);
}
