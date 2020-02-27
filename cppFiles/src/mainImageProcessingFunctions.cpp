
#include <vector>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iomanip>      // std::setw
#include <math.h>       /* pow, atan2 */

#include "FragmentHash.h"
#include "ShapeAndPositionInvariantImage.hpp"
#include "boostGeometryTypes.hpp"
#include "miscUtils.hpp"
#include "shapeNormalise.hpp"

#define NUM_OF_ROTATIONS 1
#define HASH_SIZE 8
#define FRAGMENT_WIDTH 60*.86
#define FRAGMENT_HEIGHT 60

#define PI 3.14159265

using namespace cv;

Mat covertToDynamicallyAllocatedMatrix(const Matx33d transformation_matrix)
{
    cv::Mat m = cv::Mat::ones(2, 3, CV_64F);
    m.at<double>(0, 0) = transformation_matrix(0, 0);
    m.at<double>(0, 1) = transformation_matrix(0, 1);
    m.at<double>(0, 2) = transformation_matrix(0, 2);
    m.at<double>(1, 0) = transformation_matrix(1, 0);
    m.at<double>(1, 1) = transformation_matrix(1, 1);
    m.at<double>(1, 2) = transformation_matrix(1, 2);
    return m;
}

Mat _calcMatrix(
        double areaFix,
        double transx,
        double transy,
        double rotation,
        double output_width,
        double a,
        double b,
        double zoomin=1) {

    double cosval = cos( rotation*PI / 180.0 );
    double sinval = sin( rotation*PI / 180.0 );

    cv::Matx33d transpose_1(1.0, 0.0, transx,
                            0.0, 1.0, transy,
                            0.0, 0.0, 1.0);

    cv::Matx33d transpose_rot(cosval, -sinval, 0,
                              sinval, cosval, 0,
                              0.0, 0.0, 1.0);

    cv::Matx33d transpose_2(a, b, 0,
                            0.0, 1.0/a, 0,
                            0.0, 0.0, 1.0);

    //FIXME: explain all the variables here
    double scaleFix = zoomin*output_width;///(sqrt(areaFix))
    cv::Matx33d transpose_scale(
                            scaleFix/sqrt(areaFix), 0.0, 0.0,
                            0.0, scaleFix/sqrt(areaFix), 0.0,
                            0.0, 0.0, 1.0);

    cv::Matx33d transpose_3(1.0, 0.0, output_width/2,
                            0.0, 1.0, output_width/2,
                            0.0, 0.0, 1.0);

    return covertToDynamicallyAllocatedMatrix(transpose_3*transpose_scale*transpose_rot*transpose_2*transpose_1);
}

Mat calcMatrix(ring_t shape, double rotation, double output_width, double zoom=1) {
    point_t p;
    ring_t transformedPoly;
    bg::centroid(shape, p);
    bg::strategy::transform::translate_transformer<double, 2, 2> translate(-p.get<0>(), -p.get<1>());
    bg::transform(shape, transformedPoly, translate);

    auto [a, b] = getAandB(transformedPoly);
    double area = bg::area(transformedPoly);
    return _calcMatrix(area, -p.get<0>(), -p.get<1>(), rotation, output_width, a, b, zoom);
}

Mat applyCanny(
        Mat &imgdata,
        int thresh=100,
        int kernel_size=3,
        int ratio=3,
        int blur_width=6
                )
{
    Mat canny_output;
    Mat src_gray;
    Mat src_gray_blur;

    /// Convert image to gray and blur it
    cvtColor( imgdata, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray_blur, Size(blur_width, blur_width) );

    /// Detect edges using canny
//    Canny( src_gray_blur, canny_output, thresh, thresh*ratio, kernel_size );
    Canny( src_gray_blur, canny_output, 100, 100*2, 3 );
    return canny_output;
}

vector<ring_t> extractShapesFromContours(
        vector<vector<Point> > contours,
        int areaThresh=200
                )
{
    vector<ring_t> result;
    vector<vector<Point> > hull( contours.size() );

    for( size_t i = 0; i < contours.size(); i++ ) {
        convexHull( contours[i], hull[i] );
    }

    for( int i = 0; i < contours.size(); i++ ) {

        ring_t outPoly;
        if(!convert_to_boost(hull[i], outPoly) || bg::area(outPoly) <= areaThresh){
            continue;
        }

        result.push_back(outPoly);
    }

    return result;
}

inline uchar reduceVal(const uchar val)
{
//    if (val < 64) return 0;
    if (val < 128) return 0;
    return 255;
}

void simplifyColors(Mat& img)
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
//            pixelPtr[pi + 3] = reduceVal(pixelPtr[pi + 3]); // A
        }
    }
}


