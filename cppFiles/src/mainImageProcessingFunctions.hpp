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

Mat calcMatrix(ring_t shape, double rotation, double output_width, double zoom);

trans::matrix_transformer<double, 2, 2> convertCVMatrixToBoost(cv::Mat inmat);

vector<uint64_t> getHashesForShape(const cv::Mat& input_image,
                                   ring_t shape,
                                   int numRotations,
                                   int rotationJump,
                                   int output_width = 32,
                                   int start_rotation = 0,
                                   double zoom=1);

uint64_t getHashesForShape_singleRotation(const cv::Mat& input_image,
                                          ring_t shape,
                                          int rotation,
                                          double zoom);

Mat convertToGrey(Mat img_in);

vector<ring_t> extractShapes(
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        Mat &grayImg
                );

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

vector<tuple<ring_t, vector<uint64_t>>> getAllTheHashesForImage(
        Mat img_in,
        int rotations=360,
        int thresh=CANNY_THRESH,
        int ratio=CANNY_RATIO,
        int kernel_size=CANNY_KERNEL_SIZE,
        int blur_width=CANNY_BLUR_WIDTH,
        int areaThresh=CANNY_AREA_THRESH,
        int second_rotations=1,
        double zoom=HASH_ZOOM
            );

void drawContoursWithRing(const Mat &img, vector<ring_t> _pts);

//only in this header file for testing
std::tuple<double, double> getAandBWrapper(const ring_t& shape, point_t centroid);

void handleForRotation(const Mat &input_image, const ring_t &shape, int output_width,
                       vector<tuple<ring_t, uint64_t, int>> &ret, const point_t centroid, double a,
                       double b, double area, unsigned int _rotation_in);

void findContoursWrapper(const Mat &canny_output, vector<vector<Point>> &contours,
        double epsilon=SMOOTH_CONTOURS_EPSILON,
        bool smooth=SMOOTH_CONTOURS_BOOL);

#endif//mainImageProcessingFunctions_cpp
