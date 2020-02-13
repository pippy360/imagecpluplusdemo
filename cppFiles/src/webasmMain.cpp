#include <stdlib.h> // required for malloc definition
#include <opencv2/opencv.hpp>
#include <sstream>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "boostGeometryTypes.hpp"
#include "mainImageProcessingFunctions.hpp"

class ValWrapper{
public:

    ValWrapper(size_t size, unsigned char *ptr)
        : ptr_(ptr),
        size_(size),
        val_(emscripten::typed_memory_view(size, ptr))
    {
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

    ValWrapper edgeImage;
    ValWrapper outputImage1;
    ValWrapper outputImage2;
};

RNG rng(12345);

void encode(
        uintptr_t img_in,
        ValHolder *valsOut,
        int width,
        int height,
        int thresh = 100,
        int ratio=3,
        int kernel_size=3)
{

    cout << width << " <- width height -> " << height << endl;

    int size = width * height * 4 * sizeof(uint8_t);
    cout << "size: " << size << endl;
    vector<vector<Point> > contours;
    std::stringstream polygonString;
    vector<Vec4i> hierarchy;
    Mat canny_output;
    Mat src_gray;

    Mat image(cv::Size(width, height), CV_8UC4, (void *) img_in, cv::Mat::AUTO_STEP);

    /// Convert image to gray and blur it
    cvtColor( image, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray, Size(6,6) );

    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*ratio, kernel_size );
    /// Find contours
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

    Mat imageCannyOut;
    cvtColor(canny_output, imageCannyOut, COLOR_GRAY2RGBA);

    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255), 255 );
        drawContours( image, contours, i, color, 2, 8, hierarchy, 0, Point() );
        drawContours( imageCannyOut, contours, i, color, 2, 8, hierarchy, 0, Point() );
    }

    vector<ring_t> result;
    extractShapes(image, result);

    cout << "number of fragments: " << result.size() << endl;

    for (auto &shape : result) {
        polygonString << bg::wkt(shape) << endl;
    }

    cout << "number of string: " << polygonString.str().length() << endl;
    //shapestring = polygonString.str();
    memcpy(valsOut->outputImage1.ptr_, image.data, size);

    memcpy(valsOut->edgeImage.ptr_, imageCannyOut.data, size);
    cout << "sizeout of size of: " << sizeof(size_t) << endl;
}

using namespace emscripten;

EMSCRIPTEN_BINDINGS(my_value_example) {
    ;
    class_<ValWrapper>("ValWrapper")
            .property("val_", &ValWrapper::val_)
            ;

    class_<ValHolder>("ValHolder")
            .constructor<size_t>()
            .property("edgeImage", &ValHolder::edgeImage)
            .property("outputImage1", &ValHolder::outputImage1)
            .property("outputImage2", &ValHolder::outputImage2)
        ;
    emscripten::function("encode", &encode, emscripten::allow_raw_pointers());
}
