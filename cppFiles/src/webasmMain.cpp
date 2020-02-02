#include <stdlib.h> // required for malloc definition
#include <opencv2/opencv.hpp>
#include <sstream>

#include <emscripten.h>

#include "boostGeometryTypes.hpp"
#include "mainImageProcessingFunctions.hpp"

using namespace std;
string type2str(int type) {
    string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}

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
const char *encode(
        uint8_t* img_in,
        uint8_t* outputBuffer,
        size_t* sizeout,
        uint8_t* outputBufferEdge,
        int width,
        int height,
        int thresh = 100,
        int ratio=3,
        int kernel_size=3)
{
    int size = width * height * 4 * sizeof(uint8_t);

    cout << "input image: " << (void *) img_in << endl;
    cout << "output buffer: " << (void *) outputBuffer << endl;
    cout << "sizeout buffer: " << (void *) sizeout << endl;


    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat src_gray;

    Mat image(cv::Size(width, height), CV_8UC4, img_in, cv::Mat::AUTO_STEP);

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

    stringstream *ss = new stringstream();
    for (auto &shape : result) {
        *ss << bg::wkt(shape) << endl;

    }

    cout << "number of string: " << ss->str().length() << endl;
    cout << type2str(imageCannyOut.type()) << endl;
    memcpy(outputBuffer, image.data, size);
//    memcpy(outputBuffer, imageCannyOut.data, size);
    memcpy(outputBufferEdge, imageCannyOut.data, size);
    *sizeout = ss->str().length();
    cout << "sizeout: " << *sizeout << endl;

    char * outString = (char *) malloc(ss->str().length()+1);
    memcpy(outString, ss->str().c_str(), ss->str().length()+1);

    cout << "sizeout of size of: " << sizeof(size_t) << endl;

    return outString;
}
}

