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

static Mat _calcMatrix(
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

//FIXME: move this from this header file and stop making everything static
static Mat calcMatrix(ring_t shape, double rotation, double output_width, double zoom=1) {
    point_t p;
    ring_t transformedPoly;
    bg::centroid(shape, p);
    bg::strategy::transform::translate_transformer<double, 2, 2> translate(-p.get<0>(), -p.get<1>());
    bg::transform(shape, transformedPoly, translate);

    auto [a, b] = getAandB(transformedPoly);
    double area = bg::area(transformedPoly);
    return _calcMatrix(area, -p.get<0>(), -p.get<1>(), rotation, output_width, a, b, zoom);
}

template<typename T>
static vector<pair<ring_t, T>> getHashesForShape(const cv::Mat& input_image,
        const ring_t& shape,
        int numRotations=360,
        int output_width=32
        )
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
        double rotation = ((double) i)*(360.0/(double)numRotations);
        Mat m = _calcMatrix(area, -p.get<0>(), -p.get<1>(), rotation, output_width, a, b);

        Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
        warpAffine(input_image, outputImage, m, outputImage.size());

        auto calculatedHash = T(outputImage);

//        imshow("image", outputImage);
//        waitKey(0);

        ret.push_back(std::make_pair(shape, calculatedHash));
    }
    return ret;
}

static void extractShapes(Mat &imgdata, vector<ring_t> &result, int thresh = 100, int ratio=3, int kernel_size=3, int blur_width=6)
{
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat src_gray;

    /// Convert image to gray and blur it
    cvtColor( imgdata, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray, Size(blur_width, blur_width) );

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

template<typename T>
static vector<pair<ring_t, T>> getAllTheHashesForImage(
        cv::Mat &imgdata,
        int rotations=360,
        int thresh=100,
        int ratio=3,
        int kernel_size=3,
        int blur_width=6
                )
{
    vector<ring_t> shapes;
    extractShapes(imgdata, shapes, thresh, ratio, kernel_size, blur_width);
    vector<pair<ring_t, T>> ret(shapes.size()*rotations);

//#pragma omp parallel for
    for (int i = 0; i < shapes.size(); i++)
    {
        auto shape = shapes[i];
        cout << "entering for shape:" << endl;
        auto hashes = getHashesForShape<T>(imgdata, shape, rotations);
        for (int j =0; j < hashes.size(); j++) {
            ret[(i*rotations) + j] = hashes[j];
        }
    }
    return ret;
}

#endif//mainImageProcessingFunctions_cpp
