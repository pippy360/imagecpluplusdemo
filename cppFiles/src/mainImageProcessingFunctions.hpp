#ifndef mainImageProcessingFunctions_hpp
#define mainImageProcessingFunctions_hpp

#include <vector>
#include <opencv2/opencv.hpp>

#include "ImageHash.hpp"
#include "boostGeometryTypes.hpp"
#include "shapeNormalise.hpp"

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

vector<pair<ring_t, ImageHash>> getHashesForShape(const cv::Mat& input_image,
                                          const ring_t& shape,
                                          int numRotations=360,
                                          int output_width=32);

vector<pair<ring_t, ImageHash > > getAllTheHashesForImageAndShapes(Mat &imgdata, vector<ring_t> shapes,
        int rotations=360);

Mat applyCanny(
        Mat &imgdata,
        int thresh=100,
        int kernel_size=3,
        int ratio=3,
        int blur_width=6
                );

vector<ring_t> extractShapesFromContours(
        vector<vector<Point> > contours,
        int areaThresh=200
                );

//void simplifyColors(Mat& img);

/*
 *
 * //FIXME: force common config
 *
 *
 * let g_blurWidth = 3;
let g_kernelSize = 3;
let g_ratio = 3;
let g_thresh = 100;
let g_areaThresh = 200;
 *
 * */

vector<pair<ring_t, ImageHash>> getAllTheHashesForImage(
        Mat img_in,
        int rotations=360,
        int thresh=100,
        int ratio=3,
        int kernel_size=3,
        int blur_width=3,
        int areaThresh=200,
        bool simplify=true
);

#endif//mainImageProcessingFunctions_cpp
