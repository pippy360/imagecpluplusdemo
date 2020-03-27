#ifndef mainImageProcessingFunctions_hpp
#define mainImageProcessingFunctions_hpp

#include <vector>
#include <opencv2/opencv.hpp>

#include "ImageHash.hpp"
#include "boostGeometryTypes.hpp"
#include "shapeNormalise.hpp"

#include "defaultImageValues.h"

using namespace cv;

Mat covertToDynamicallyAllocatedMatrix(const Matx33d transformation_matrix);

Mat calcMatrix(ring_t shape, double rotation, double output_width, double zoom=1);

Mat _calcMatrix(
        double areaFix,
        double transx,
        double transy,
        double rotation,
        double output_width,
        double a,
        double b,
        double zoomin=1);

vector<tuple<ring_t, uint64_t, int>> getHashesForShape(const cv::Mat& input_image,
                                                const ring_t& shape,
                                                int numRotations,
                                                int rotationJump,
                                                int output_width=32,
                                                int start_rotation=0);

tuple<ring_t, uint64_t, int> getHashesForShape_singleRotation(const cv::Mat& input_image,
                                                              const ring_t& shape,
                                                              int rotation);

Mat convertToGrey(Mat img_in);

vector<ring_t> extractShapes(int thresh, int ratio, int kernel_size, int blur_width, int areaThresh, Mat &grayImg);

vector<tuple<ring_t, uint64_t, int>> getAllTheHashesForImageAndShapes(Mat &imgdata, vector<ring_t> shapes,
        int rotations);

Mat applyCanny(
        Mat &imgdata,
        int thresh=CANNY_THRESH,
        int ratio=CANNY_RATIO,
        int kernel_size=CANNY_KERNEL_SIZE,
        int blur_width=CANNY_BLUR_WIDTH
                );

vector<ring_t> extractShapesFromContours(
        vector<vector<Point> > contours,
        int areaThresh=CANNY_AREA_THRESH
                );

vector<tuple<ring_t, uint64_t, int>> getAllTheHashesForImage(
        Mat img_in,
        int rotations=360,
        int thresh=CANNY_THRESH,
        int ratio=CANNY_RATIO,
        int kernel_size=CANNY_KERNEL_SIZE,
        int blur_width=CANNY_BLUR_WIDTH,
        int areaThresh=CANNY_AREA_THRESH,
        bool simplify=true
);


//only in this header file for testing
std::tuple<double, double> getAandBWrapper(const ring_t& shape, point_t centroid);

void handleForRotation(const Mat &input_image, const ring_t &shape, int output_width,
                       vector<tuple<ring_t, uint64_t, int>> &ret, const point_t centroid, double a,
                       double b, double area, unsigned int _rotation_in);

void handleForRotation2(const Mat &input_image, const ring_t &shape, int output_width,
                       vector<tuple<ring_t, uint64_t, int>> &ret, const point_t centroid, double a,
                       double b, double area, unsigned int _rotation_in);

vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>> findMatches(
        Mat img_in,
        Mat img_in2,
        int thresh=CANNY_THRESH,
        int ratio=CANNY_RATIO,
        int kernel_size=CANNY_KERNEL_SIZE,
        int blur_width=CANNY_BLUR_WIDTH,
        int areaThresh=CANNY_AREA_THRESH,
        bool flushCache=true);

#endif//mainImageProcessingFunctions_cpp
