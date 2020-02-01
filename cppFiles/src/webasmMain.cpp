#include <stdlib.h> // required for malloc definition
#include <opencv2/opencv.hpp>
#include <sstream>

#include <emscripten.h>

#include "boostGeometryTypes.hpp"
#include "mainImageProcessingFunctions.hpp"

using namespace std;

extern "C" {
EMSCRIPTEN_KEEPALIVE
uint8_t *create_buffer(int width, int height) {
    void *res = malloc(width * height * 4 * sizeof(uint8_t));
    cout << "ptr: " << res << " of size width: " << width << " : " << height << endl;
    return (uint8_t *) res;
}

EMSCRIPTEN_KEEPALIVE
uint8_t *create_buffer_zero(int width, int height) {
    void *res = malloc(width * height * 4 * sizeof(uint8_t));
    memset(res, 0, width * height * 4 * sizeof(uint8_t));
    cout << "ptr: " << res << " of size width: " << width << " : " << height << endl;
    return (uint8_t *) res;
}

EMSCRIPTEN_KEEPALIVE
void destroy_buffer(uint8_t* p) {
    free(p);
}
RNG rng(12345);

EMSCRIPTEN_KEEPALIVE
const char *encode(uint8_t* img_in, uint8_t* outputBuffer, size_t* sizeout, int width, int height) {
    int size = width * height * 4 * sizeof(uint8_t);

    cout << "input image: " << (void *) img_in << endl;
    cout << "output buffer: " << (void *) outputBuffer << endl;
    cout << "sizeout buffer: " << (void *) sizeout << endl;


    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat src_gray;
    int thresh = 50;

    cv::Mat image = cv::Mat(cv::Size(width, height), CV_8UC4, img_in, cv::Mat::AUTO_STEP);

    /// Convert image to gray and blur it
    cvtColor( image, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray, Size(6,6) );

    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255), 255 );
        drawContours( image, contours, i, color, 2, 8, hierarchy, 0, Point() );
    }

    vector<ring_t> result;
    extractShapes(image, result);

    cout << "number of fragments: " << result.size() << endl;

    stringstream *ss = new stringstream();
    for (auto &shape : result) {
        *ss << bg::wkt(shape) << endl;
    }

    cout << "number of string: " << ss->str().length() << endl;


    memcpy(outputBuffer, image.data, size);
    //memcpy(outputBuffer, converted.data, size);
    *sizeout = ss->str().length();
    cout << "sizeout: " << *sizeout << endl;

    char * outString = (char *) malloc(ss->str().length()+1);
    memcpy(outString, ss->str().c_str(), ss->str().length()+1);

    cout << "sizeout of size of: " << sizeof(size_t) << endl;

    return outString;
}
}

