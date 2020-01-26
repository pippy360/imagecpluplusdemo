#include <stdlib.h> // required for malloc definition
#include <opencv2/opencv.hpp>

#include <emscripten.h>

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
size_t encode(uint8_t* img_in, uint8_t* outputBuffer, int width, int height) {
    int size = width * height * 4 * sizeof(uint8_t);

    cv::Mat converted;
    cv::Mat image = cv::Mat(cv::Size(width, height), CV_8UC4, img_in, cv::Mat::AUTO_STEP);
    cv::cvtColor(image, converted, cv::COLOR_RGBA2BGRA);

    extractShapes(image);

    std::memcpy(outputBuffer, converted.data, size);
    return size;
}
}

