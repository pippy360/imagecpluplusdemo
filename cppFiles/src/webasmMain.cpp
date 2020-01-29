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
    cout << "ptr: " << res << endl;
    return (uint8_t *) res;
}

EMSCRIPTEN_KEEPALIVE
void destroy_buffer(uint8_t* p) {
    free(p);
}


EMSCRIPTEN_KEEPALIVE
const char *encode(uint8_t* img_in, uint8_t* outputBuffer, size_t* sizeout, int width, int height) {
    int size = width * height * 4 * sizeof(uint8_t);

    cv::Mat converted;
    cv::Mat image = cv::Mat(cv::Size(width, height), CV_8UC4, img_in, cv::Mat::AUTO_STEP);
    cv::cvtColor(image, converted, cv::COLOR_RGBA2BGRA);

    vector<ring_t> result;
    extractShapes(image, result);

    stringstream *ss = new stringstream();
    for (auto &shape : result) {
        *ss << bg::wkt(shape) << endl;
    }
    memcpy(outputBuffer, converted.data, size);
    *sizeout = (size_t) ss->str().length();
    char * outString = (char *) malloc(ss->str().length()+1);
    memcpy(outString, ss->str().c_str(), ss->str().length()+1);
    return outString;
}
}

