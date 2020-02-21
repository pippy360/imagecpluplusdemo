#include <stdlib.h> // required for malloc definition
#include <opencv2/opencv.hpp>
#include <sstream>

#include <map>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "boostGeometryTypes.hpp"
#include "mainImageProcessingFunctions.hpp"

#include "PerceptualHash.hpp"
#include "PerceptualHash_Fast.hpp"

#include "shapeNormalise.hpp"

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
            outputImage2(size, (unsigned char *) malloc(size))
    {
    }


    string shapeStr;
    ValWrapper edgeImage;
    ValWrapper outputImage1;
    ValWrapper outputImage2;
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
    Mat image(cv::Size(400, 400), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);
    auto vec = getAllTheHashesForImage<hashes::PerceptualHash>(
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
////    for (auto &v: vec) {
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

inline uchar reduceVal(const uchar val)
{
//    if (val < 64) return 0;
    if (val < 128) return 0;
    return 255;
}

void processColors(Mat& img)
{
    uchar* pixelPtr = img.data;
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            const int pi = i*img.cols*4 + j*4;
            pixelPtr[pi + 0] = reduceVal(pixelPtr[pi + 0]); // B
            pixelPtr[pi + 1] = reduceVal(pixelPtr[pi + 1]); // G
            pixelPtr[pi + 2] = reduceVal(pixelPtr[pi + 2]); // R
            pixelPtr[pi + 3] = reduceVal(pixelPtr[pi + 3]); // A
        }
    }
}

void encode(
        uintptr_t img_in,
        ValHolder *valsOut,
        int width,
        int height,
        int thresh = 100,
        int ratio=3,
        int kernel_size=3,
        int blurSize=3,
        int areaThresh=200
                )
{
    int size = width * height * 4 * sizeof(uint8_t);
    cout << "size: " << size << endl;
    vector<vector<Point> > contours;
    std::stringstream polygonString;
    vector<Vec4i> hierarchy;
    Mat canny_output;
    Mat src_gray;

    Mat image(cv::Size(width, height), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);

    /// Convert image to gray and blur it
    processColors(image);
    cvtColor( image, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray, Size(blurSize, blurSize) );

    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*ratio, kernel_size );
    /// Find contours
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

    Mat imageCannyOut;
    Mat imageCannyOut2;
    Mat grayOut;

    cvtColor(src_gray, grayOut, COLOR_GRAY2RGBA);
    cvtColor(canny_output, imageCannyOut, COLOR_GRAY2RGBA);
    cvtColor(canny_output, imageCannyOut2, COLOR_GRAY2RGBA);

    for( int i = 0; i< contours.size(); i++ )
    {
        vector<Point> hull;
        convexHull( contours[i], hull );
        ring_t outPoly;

        if(!convert_to_boost(hull, outPoly) || bg::area(outPoly) <= areaThresh ){
            continue;
        }
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255), 255 );
        //drawContours( image, contours, i, color, 2, 8, hierarchy, 0, Point() );
        drawContours( imageCannyOut, contours, i, color, 2, 8, hierarchy, 0, Point() );
    }

    vector<ring_t> result;
    extractShapes(image, result, thresh, ratio, kernel_size);

    cout << "number of fragments: " << result.size() << endl;

    for (auto &shape : result) {
        polygonString << bg::wkt(shape) << endl;
    }

    cout << "number of string: " << polygonString.str().length() << endl;
    valsOut->shapeStr = polygonString.str();
    memcpy(valsOut->outputImage1.ptr_, grayOut.data, size);

    memcpy(valsOut->outputImage2.ptr_, imageCannyOut2.data, size);

    memcpy(valsOut->edgeImage.ptr_, imageCannyOut.data, size);
    cout << "sizeout of size of: " << sizeof(size_t) << endl;

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
        ;

    emscripten::function("encode", &encode, allow_raw_pointers());
    emscripten::function("getHashesForShape2", &getHashesForShape2, allow_raw_pointers());
    emscripten::function("calcMatrixFromString", &calcMatrixFromString);
    emscripten::function("getAllTheHashesForImageFromCanvas", &getAllTheHashesForImageFromCanvas);
}
