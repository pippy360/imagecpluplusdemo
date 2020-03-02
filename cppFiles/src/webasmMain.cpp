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
        std::cout << "ptr created with size " << size_ << std::endl;

    }

    ~ValWrapper()
    {
        std::cout << "ptr free with size " << size_ << std::endl;
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

emscripten::val calcMatrixFromString(string shapeStr, int output_width=400, double zoom=1) {

    ring_t shape;
    bg::read_wkt(shapeStr, shape);
    Mat m = calcMatrix(shape, 0, output_width, zoom);

    return emscripten::val(emscripten::typed_memory_view(9, (double *)m.data));
}

void getHashesForShape2(uintptr_t img_in, ValHolder *valsOut, string shapeStr, int output_width=400, double zoom=1)
{
    point_t p;
    ring_t transformedPoly;

    ring_t shape;
    bg::read_wkt(shapeStr, shape);
    Mat m = calcMatrix(shape, 0, output_width, zoom);

    Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
    Mat image(cv::Size(400, 400), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);

    warpAffine(image, outputImage, m, outputImage.size());
    memcpy(valsOut->outputImage2.ptr_, outputImage.data, output_width*output_width*4);
}

std::string getAllTheHashesForImageFromCanvas(uintptr_t img_in, int rotation,
                                              int thresh,
                                              int ratio,
                                              int kernel_size,
                                              int blur_width
                                              )
{
    std::cout << "getAllTheHashesForImageFromCanvas called" << std::endl;

    Mat image(cv::Size(400, 400), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);
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
//    for (auto &v: vec) {
    for (int i = 0; i < vec.size(); i++) {
        auto v = vec[i];
        auto [shape, hash] = v;
        if (i > 0)
            polygonString << ",";
        polygonString << "\"";
        polygonString << hash.toString();
        polygonString << "\" : \"";
        polygonString << bg::wkt(shape);
        polygonString << "\"";
    }
    polygonString << "} ";
    return polygonString.str();
}



void getShapeWithPointInside(
        uintptr_t img_in_ptr,
        ValHolder *valsOut,
        int width,
        int height,
        double x,
        double y,
        int thresh = 100,
        int ratio=3,
        int kernel_size=3,
        int blur_width=3,
        int areaThresh=200,
        bool simplify=true
                )
{
    Mat img_in(cv::Size(width, height), CV_8UC4, (void *) img_in_ptr, cv::Mat::AUTO_STEP);
    Mat img = img_in.clone();
    if (simplify) {
        simplifyColors(img);
    }

    Mat canny_output = applyCanny(img, thresh, kernel_size, ratio, blur_width);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0));

    vector<ring_t> shapes = extractShapesFromContours(contours, areaThresh);

    std::stringstream polygonString;
    for (auto &shape : shapes) {
        if (bg::within(point_t(x, y), shape))
            polygonString << bg::wkt(shape) << endl;
    }
    valsOut->shapeStr = polygonString.str();
}

void encode(
        uintptr_t img_in_ptr,
        ValHolder *valsOut,
        int width,
        int height,
        int thresh = 100,
        int ratio=3,
        int kernel_size=3,
        int blur_width=3,
        int areaThresh=200,
        bool simplify=true
                )
{
    cout << "width: " << width << " height: " << height << endl;

    Mat img_in(cv::Size(width, height), CV_8UC4, (void *) img_in_ptr, cv::Mat::AUTO_STEP);
    Mat img = img_in.clone();
    if (simplify) {
        simplifyColors(img);
    }

    Mat _canny_output = applyCanny(img_in, thresh, kernel_size, ratio, blur_width);
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
        drawContours( hulls_img, vector<vector<Point> >(1,hull), -1, color );
    }

    Mat valid_hulls_img = imageCannyOut.clone();
    int  failed_area = 0;
    for ( int i = 0; i < contours.size(); i++ )
    {
        vector<Point> hull;
        convexHull( contours[i], hull );
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255), 255 );
        ring_t outPoly;
        cout << "convert_to_boost about to be called: " << endl;
        if (!convert_to_boost(hull, outPoly)) {
            continue;
        }

        if (bg::area(outPoly) <= areaThresh){
            cout << "passed area" << endl;
            failed_area++;
            continue;
        }

        drawContours( valid_hulls_img, vector<vector<Point> >(1,hull), -1, color );
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
    emscripten::function("getHashesForShape2", &getHashesForShape2, allow_raw_pointers());
    emscripten::function("calcMatrixFromString", &calcMatrixFromString);
    emscripten::function("getAllTheHashesForImageFromCanvas", &getAllTheHashesForImageFromCanvas);
}
