
#include <vector>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <math.h>       /* pow, atan2 */

#include "ImageHash.hpp"
#include "boostGeometryTypes.hpp"
#include "miscUtils.hpp"
#include "shapeNormalise.hpp"
#include "PerceptualHash.hpp"
#include "mainImageProcessingFunctions.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
using namespace std;
using namespace std::chrono;

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
        double zoomin) {

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

Mat calcMatrix(ring_t shape, double rotation, double output_width, double zoom) {
    point_t p;
    ring_t transformedPoly;
    bg::centroid(shape, p);
    bg::strategy::transform::translate_transformer<double, 2, 2> translate(-p.get<0>(), -p.get<1>());
    bg::transform(shape, transformedPoly, translate);

    auto [a, b] = getAandB(transformedPoly);
    double area = bg::area(transformedPoly);
    return _calcMatrix(area, -p.get<0>(), -p.get<1>(), rotation, output_width, a, b, zoom);
}

vector<tuple<ring_t, uint64_t, int>> getAllTheHashesForImageAndShapes(Mat &imgdata,
        vector<ring_t> shapes,
        int rotations,
        int rotationJump)
{
//    vector<pair<ring_t, uint64_t>> ret(shapes.size()*rotations*4);
    vector<tuple<ring_t, uint64_t, int>> ret;
//#pragma omp parallel for
    for (int i = 0; i < shapes.size(); i++)
    {
        auto shape = shapes[i];
//        cout << "entering for shape:" << endl;
        auto hashes = getHashesForShape(imgdata, shape, rotations, rotationJump);
        for (int j =0; j < hashes.size(); j++) {
            ret.push_back(hashes[j]);
        }
    }
    return ret;
}

Mat convertToGrey(Mat img_in) {
    Mat grayImg;
    if(img_in.type() == CV_8UC3)
    {
        cv::cvtColor(img_in, grayImg, COLOR_BGR2GRAY);
    }
    else if(img_in.type() == CV_8UC4)
    {
        cv::cvtColor(img_in, grayImg, COLOR_BGRA2GRAY);
    }
    else
    {
        grayImg = img_in;
    }
    return grayImg;
}

vector<ring_t> extractShapes(int thresh, int ratio, int kernel_size, int blur_width, int areaThresh, Mat &grayImg)
{
    Mat canny_output = applyCanny(grayImg, thresh, kernel_size, ratio, blur_width);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0));
    return extractShapesFromContours(contours, areaThresh);
}

vector<tuple<ring_t, uint64_t, int>> getAllTheHashesForImage(
        Mat img_in,
        int rotations,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        bool simplify)
{
    Mat grayImg = convertToGrey(img_in);
    vector<ring_t> shapes = extractShapes(thresh, ratio, kernel_size, blur_width, areaThresh, grayImg);

    std::cout << "Inside cpp code the extracted shapes: " << shapes.size() << std::endl;
    return getAllTheHashesForImageAndShapes(grayImg, shapes, rotations, 1);
}

using namespace std::chrono;

static void *prevImg;
vector<tuple<ring_t, uint64_t, int>> g_imghashes;

vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>> findMatches(
        Mat img_in,
        Mat img_in2,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        bool flushCache)
{

    //FIXME: this doesn't check if the data changed!!
    // use cached results
    if (flushCache) {
        prevImg = img_in.data;
        Mat grayImg = convertToGrey(img_in);
        vector<ring_t> shapes = extractShapes(thresh, ratio, kernel_size, blur_width, areaThresh, grayImg);

        g_imghashes = getAllTheHashesForImageAndShapes(grayImg, shapes, 360, 2);
    }

    Mat grayImg2 = convertToGrey(img_in2);
    vector<ring_t> shapes2 = extractShapes(thresh, ratio, kernel_size, blur_width, areaThresh, grayImg2);

    auto img2hashes = getAllTheHashesForImageAndShapes(grayImg2, shapes2, 1, 1);

    vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>> res;
    for (auto h : g_imghashes) {
        for (auto h2 : img2hashes) {
            auto [shape1, hash1, rotation1] = h;
            auto [shape2, hash2, rotation2] = h2;
            if( ImageHash::bitCount(hash1 ^ hash2) < 5 ) {
                res.push_back(std::tie(shape1, shape2, hash1, hash2, rotation1));
            }
        }
    }

    return res;
}

std::tuple<double, double> getAandBWrapper(const ring_t& shape, point_t centroid) {
    bg::strategy::transform::translate_transformer<double, 2, 2> translate(-centroid.get<0>(), -centroid.get<1>());
    ring_t transformedPoly;
    bg::transform(shape, transformedPoly, translate);
    return getAandB(transformedPoly);
}

void handleForRotation(const Mat &input_image, const ring_t &shape, int output_width,
                       vector<tuple<ring_t, uint64_t, int>> &ret, const point_t centroid, double a,
                       double b, double area, unsigned int _rotation_in) {
    double rotation = _rotation_in;
//        std::cout << "rotation: " << rotation << std::endl;
    Mat m = _calcMatrix(area, -centroid.get<0>(), -centroid.get<1>(), rotation, output_width, a, b);

    Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
    warpAffine(input_image, outputImage, m, outputImage.size());

    uint64_t calculatedHash = img_hash::PHash::compute(outputImage);
//        imshow("image", outputImage);
//        waitKey(0);

    ret.push_back(tie(shape, calculatedHash, rotation));

}

void handleForRotation2(const Mat &input_image, const ring_t &shape, int output_width,
                              vector<tuple<ring_t, uint64_t, int>> &ret, const point_t centroid, double a,
                              double b, double area, unsigned int _rotation_in) {
//    for (unsigned int j = 0; j < 4; j++) {
//        double rotation = _rotation_in + (90*j);
////        std::cout << "rotation: " << rotation << std::endl;
//        Mat m = _calcMatrix(area, -centroid.get<0>(), -centroid.get<1>(), rotation, output_width, a, b);
//
//        Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
//        warpAffine(input_image, outputImage, m, outputImage.size());
//
//        uint64_t calculatedHash = img_hash::PHash::compute(outputImage);
////        imshow("image", outputImage);
////        waitKey(0);
//
//        ret.push_back(make_pair(shape, calculatedHash));
//    }

        double rotation = _rotation_in;
//        std::cout << "rotation: " << rotation << std::endl;
        Mat m = _calcMatrix(area, -centroid.get<0>(), -centroid.get<1>(), rotation, output_width, a, b);

        Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
        warpAffine(input_image, outputImage, m, outputImage.size());

    {
        vector<uint64_t> calculatedHashs = img_hash::PHash::compute_fast(outputImage);
//        imshow("image", outputImage);
//        waitKey(0);

        for (auto h : calculatedHashs)
            ret.push_back(tie(shape, h, rotation));
    }
}

vector<tuple<ring_t, uint64_t, int>> getHashesForShape(const cv::Mat& input_image,
                                                         const ring_t& shape,
                                                         int numRotations,
                                                         int rotationJump,
                                                         int output_width)
{
    point_t p;
    auto ret = vector<tuple<ring_t, uint64_t, int>>();
    bg::centroid(shape, p);
    auto [a, b] = getAandBWrapper(shape, p);
    double area = bg::area(shape);
    for (unsigned int i = 0; i < numRotations; i += rotationJump)
    {
        handleForRotation(input_image, shape, output_width, ret, p, a, b, area, i);
    }
    return ret;
}

Mat applyCanny(
        Mat &src_gray,
        int thresh,
        int kernel_size,
        int ratio,
        int blur_width
        )
{
    Mat canny_output;
    Mat src_gray_blur;
    assert(src_gray.type() == CV_8U);
    /// Convert image to gray and blur it
    blur( src_gray, src_gray_blur, Size(blur_width, blur_width) );

    /// Detect edges using canny
    Canny( src_gray_blur, canny_output, thresh, thresh*ratio, kernel_size );
    return canny_output;
}

vector<ring_t> extractShapesFromContours(
        vector<vector<Point>> contours,
        int areaThresh)
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

//inline uchar reduceVal(const uchar val)
//{
////    if (val < 64) return 0;
//    if (val < 128) return 0;
//    return 255;
//}

//FIXME: consider adding this back in
void simplifyColors(Mat& img)
{
    return;
//    uchar* pixelPtr = img.data;
//    for (int i = 0; i < img.rows; i++)
//    {
//        for (int j = 0; j < img.cols; j++)
//        {
//            const int pi = i*img.cols*4 + j*4;
//            pixelPtr[pi + 0] = reduceVal(pixelPtr[pi + 0]); // B
//            pixelPtr[pi + 1] = reduceVal(pixelPtr[pi + 1]); // G
//            pixelPtr[pi + 2] = reduceVal(pixelPtr[pi + 2]); // R
////            pixelPtr[pi + 3] = reduceVal(pixelPtr[pi + 3]); // A
//        }
//    }
}


