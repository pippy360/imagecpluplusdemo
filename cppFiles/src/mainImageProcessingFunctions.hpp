#ifndef mainImageProcessingFunctions_hpp
#define mainImageProcessingFunctions_hpp

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

static Mat covertToDynamicallyAllocatedMatrix(const Matx33d transformation_matrix)
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

static Mat calcMatrix(
        double areaFix,
        double transx,
        double transy,
        double rotation,
        double output_width,
        double a,
        double b,
        double zoomin=1) {

    double cosval = cos( rotation );
    double sinval = sin( rotation );

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

    return covertToDynamicallyAllocatedMatrix(transpose_3*transpose_scale*transpose_2*transpose_rot*transpose_1);
}

template<typename T>
static vector<pair<ring_t, T>> getHashesForShape(const cv::Mat& input_image, const ring_t& shape, int output_width=32,
        int numRotations=360)
{
    auto ret = vector<pair<ring_t, T>>();
    ring_t transformedPoly;
    point_t p;
    bg::centroid(shape, p);
    bg::strategy::transform::translate_transformer<double, 2, 2> translate(-p.get<0>(), -p.get<1>());
    bg::transform(shape, transformedPoly, translate);
    auto [a, b] = getAandB(transformedPoly);
    double area = bg::area(transformedPoly);
    for (unsigned int i = 0; i < numRotations; i++)
    {

        double val = PI / 180.0;

        Mat m = calcMatrix(area, -p.get<0>(), -p.get<1>(), ((double) i)*val, output_width, a, b);

        Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
        warpAffine(input_image, outputImage, m, outputImage.size());

        auto calculatedHash = T(outputImage);
        ret.push_back(std::make_pair(shape, calculatedHash));
    }
    return ret;
}

static void extractShapes(Mat &imgdata, vector<ring_t> &result, int thresh = 100, int ratio=3, int kernel_size=3)
{
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat src_gray;

    /// Convert image to gray and blur it
    cvtColor( imgdata, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray, Size(6,6) );

    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*ratio, kernel_size );

    /// Find contours
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

    vector<vector<Point> >hull( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
    {
        convexHull( contours[i], hull[i] );
    }

    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ ){
        ring_t outPoly;
        if(!convert_to_boost(hull[i], outPoly)){
            continue;
        }
        result.push_back(outPoly);
    }
}

template<typename T> static vector<pair<ring_t, T>> getAllTheHashesForImage(cv::Mat &imgdata)
{
    vector<ring_t> shapes;
    extractShapes(imgdata, shapes);
    vector<pair<ring_t, T>> ret(shapes.size()*NUM_OF_ROTATIONS);

//#pragma omp parallel for
    for (int i = 0; i < shapes.size(); i++)
    {
        auto shape = shapes[i];
        auto hashes = getHashesForShape<T>(imgdata, shape);
        for (int j =0; j < hashes.size(); j++) {
            ret[(i*j) + j] = hashes[j];
        }
    }
    return ret;
}

#endif//mainImageProcessingFunctions_cpp
