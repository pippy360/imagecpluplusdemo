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

#define NUM_OF_ROTATIONS 360
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

Mat applyTransformationMatrixToImage(Mat inputImage, const Matx33d transformation_matrix, int outputTriangleSizeX, int outputTriangleSizeY)
{
    Mat m = covertToDynamicallyAllocatedMatrix(transformation_matrix);
    Mat outputImage(outputTriangleSizeY, outputTriangleSizeX, CV_8UC3, Scalar(0, 0, 0));
    warpAffine(inputImage, outputImage, m, outputImage.size());
    return outputImage;
}

Matx33d calcTransformationMatrixWithShapePreperation(const ring_t shape, double rotation)
{
    //get centroid, move it over and then compute the transformaiton matrix
    return {};
}

ShapeAndPositionInvariantImage getFragment(const cv::Mat& input_image, const ring_t& shape)
{
    return ShapeAndPositionInvariantImage("output.jpg", input_image, shape, "");
}

template<typename T> static vector<pair<ring_t, T>> getHashesForShape(const cv::Mat& input_image, const ring_t& shape)
{
    auto ret = vector<pair<ring_t, T>>();
    int outputTriangleSizeX = FRAGMENT_WIDTH;
    int outputTriangleSizeY = FRAGMENT_HEIGHT;
    for (unsigned int i = 0; i < NUM_OF_ROTATIONS; i++)
    {
        auto transformationMatrix = calcTransformationMatrixWithShapePreperation(shape, i);
        auto newImageData = applyTransformationMatrixToImage(input_image, transformationMatrix, outputTriangleSizeX, outputTriangleSizeY);
        point_t p;
        ring_t transformedPoly;
        bg::centroid(shape, p);
        bg::strategy::transform::translate_transformer<double, 2, 2> translate(-p.get<0>(), -p.get<1>());
        bg::transform(shape, transformedPoly, translate);

        auto [a, b] = getAandB(transformedPoly);
        cv::Matx33d transpose_m(1.0, 0.0, -p.get<0>(),
                                0.0, 1.0, -p.get<1>(),
                                0.0, 0.0, 1.0);
        double val = PI / 180.0;
        double cosval = cos( ((double)i)*val );
        double sinval = sin( ((double)i)*val );
        cv::Matx33d transpose_rot(cosval, -sinval, 0,
                                sinval, cosval, 0,
                                0.0, 0.0, 1.0);
        cv::Matx33d transpose_2(a, b, 0,
                                0.0, 1.0/a, 0,
                                0.0, 0.0, 1.0);
        cv::Matx33d transpose_3(1.0, 1.0, 250,
                                0.0, 1.0, 250,
                                0.0, 0.0, 1.0);

        Mat m = covertToDynamicallyAllocatedMatrix(transpose_3*transpose_2*transpose_rot*transpose_m);
        Mat outputImage(32, 32, CV_8UC3, Scalar(0, 0, 0));
        warpAffine(input_image, outputImage, m, outputImage.size());

        //imwrite("frag.jpg" , outputImage);
//        imshow( "Contours", outputImage );
//        waitKey(0);

        auto calculatedHash = T(outputImage);
//        cout << calculatedHash.toString() << endl;
        ret.push_back(std::make_pair(shape, calculatedHash));
    }
    return ret;
}

vector<ring_t> extractShapes(Mat &imgdata)
{
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat src_gray;
    int thresh = 50;

    /// Convert image to gray and blur it
    cvtColor( imgdata, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray, Size(6,6) );

    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

    vector<vector<Point> >hull( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
    {
        convexHull( contours[i], hull[i] );
    }

    /// Draw contours
    vector<ring_t> result;
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ ){
        ring_t outPoly;
        if(!convert_to_boost(hull[i], outPoly)){
            continue;
        }
        result.push_back(outPoly);
    }
    return result;
}

template<typename T> static vector<vector<pair<ring_t, T>>> getAllTheHashesForImage(cv::Mat &imgdata)
{
    auto shapes = extractShapes(imgdata);
    vector<vector<pair<ring_t, T>>> ret(shapes.size());

#pragma omp parallel for
    for (int i = 0; i < shapes.size(); i++)
    {
        auto shape = shapes[i];
        ret[i] = getHashesForShape<T>(imgdata, shape);
    }
    return ret;
}

#endif//mainImageProcessingFunctions_cpp
