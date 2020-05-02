
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

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include <algorithm>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;

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

vector<tuple<ring_t, vector<uint64_t>>> getAllTheHashesForImageAndShapes(Mat &imgdata,
        vector<ring_t> shapes,
        int rotations,
        int rotationJump,
        trans::matrix_transformer<double, 2, 2> transMat = trans::matrix_transformer<double, 2, 2>(),
        bool applyTransMat = false)
{
    vector<tuple<ring_t, vector<uint64_t>>> ret;

//#pragma omp parallel for
    for (int i = 0; i < shapes.size(); i++)
    {
        ring_t shape = shapes[i];
        if (applyTransMat) {
            ring_t outPoly;
            boost::geometry::transform(shape, outPoly, transMat);
            shape = outPoly;
        }

        vector<uint64_t> hashes = getHashesForShape(imgdata, shape, rotations, rotationJump, 32, 0);
        ret.push_back(make_tuple(shape, hashes));
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

void findContoursWrapper(const Mat &canny_output, vector<vector<Point>> &contours, double epsilon, bool smooth)
{
    vector<Vec4i> hierarchy;
    if (false) {
        vector<vector<Point>> contours0;
        findContours(canny_output, contours0, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0));
        contours.resize(contours0.size());
        for( size_t k = 0; k < contours0.size(); k++ ) {
            approxPolyDP(Mat(contours0[k]), contours[k], epsilon, false);//FIXME: are we sure this should be false?
        }
    } else {
        findContours(canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0));
    }

}

vector<Point> toCVPoints(ring_t in) {
    vector<Point> v;
    for (auto pt: in) {
        v.push_back(Point2d(pt.get<0>(), pt.get<1>()));
    }
    return v;
}

vector<vector<Point>> allToCVPoints(vector<ring_t> in) {
    vector<vector<Point>> v;
    for (auto pt: in) {
        v.push_back(toCVPoints(pt));
    }
    return v;
}

void drawContours(const Mat &img, vector<vector<Point>> pts) {
    int idx = 0;
    for( ; idx < pts.size(); idx++ )
    {
        Scalar color( rand()&255, rand()&255, rand()&255 );
        drawContours(img, pts, idx, color, 4, 8);
    }
}

void drawContoursWithRing(const Mat &img, vector<ring_t> _pts) {
    auto pts = allToCVPoints(_pts);
    drawContours(img, pts);
}

vector<ring_t> extractShapes(
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        Mat &grayImg
        )
{
    //re-run the image at different sizes to get better results
    vector<ring_t> result;
    double resizePerc = 1;

    //FIXME: WE DON't need to resize for query image
    for (int i = 0; i < NUMBER_OF_IMAGE_RESIZES; i++, resizePerc *= PERCENTAGE_IMAGE_RESIZE) {
        Mat dst;
        if (i > 0) {
            Size size(grayImg.cols * resizePerc, grayImg.rows * resizePerc);
            resize(grayImg,dst,size);//resize image
        } else {
            dst = grayImg;
        }

        Mat canny_output = applyCanny(dst, thresh, ratio, kernel_size, blur_width);

        vector<vector<Point>> contours;
        findContoursWrapper(canny_output, contours);
        auto v_prime = extractShapesFromContours(contours, areaThresh);

        if (i > 0) {
            vector<ring_t> scaleFixedShapes;
            trans::scale_transformer<double, 2, 2> scale(1.0/resizePerc);
            for (auto s :  v_prime) {
                ring_t outPoly;
                boost::geometry::transform(s, outPoly, scale);
                scaleFixedShapes.push_back(outPoly);
            }
            v_prime = scaleFixedShapes;
        }

        result.reserve(result.size() + distance(v_prime.begin(),v_prime.end()));
        result.insert(result.end(),v_prime.begin(),v_prime.end());
    }

    return result;
}

bool g_useRotatedImageForHashes = true;//FIXME:


trans::matrix_transformer<double, 2, 2> convertCVMatrixToBoost(cv::Mat inmat)
{
    return trans::matrix_transformer<double, 2, 2> (
            inmat.at<double>(0,0), inmat.at<double>(0,1), inmat.at<double>(0,2),
            inmat.at<double>(1,0), inmat.at<double>(1,1), inmat.at<double>(1,2),
            0, 0, 1);
}

vector<ring_t> applyMatrixToPoints(vector<ring_t> shapes, trans::matrix_transformer<double, 2, 2> transMat) {
    vector<ring_t> result;

    for (auto shape : shapes) {
        ring_t outPoly;
        boost::geometry::transform(shape, outPoly, transMat);
        result.push_back(outPoly);
    }

    return shapes;
}

//FIXME: this code is such a mess
vector<tuple<ring_t, vector<uint64_t>>> getAllTheHashesForImage(
        Mat img_in,
        int rotations,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        int second_rotation)
{
    vector<tuple<ring_t, vector<uint64_t>>> v;
    Mat grayImg = convertToGrey(img_in);
    for (int i = 0; i < second_rotation; i += 1)
    {
        vector<tuple<ring_t, vector<uint64_t>>> v_prime;

        double angle = i;
        // get rotation matrix for rotating the image around its center in pixel coordinates
        cv::Point2f center((grayImg.cols-1)/2.0, (grayImg.rows-1)/2.0);
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), grayImg.size(), angle).boundingRect2f();
        rot.at<double>(0,2) += bbox.width/2.0 - grayImg.cols/2.0;
        rot.at<double>(1,2) += bbox.height/2.0 - grayImg.rows/2.0;
        cv::Mat dst;
        cv::warpAffine(grayImg, dst, rot, bbox.size());

        vector<ring_t> shapes = extractShapes(thresh,
                                              ratio,
                                              kernel_size,
                                              blur_width,
                                              areaThresh,
                                              dst);
        Mat canny_output = applyCanny(dst, thresh, ratio, kernel_size, blur_width);
        Mat outRot;
        cv::invertAffineTransform(rot, outRot);
        auto invmat = convertCVMatrixToBoost(outRot);

        g_useRotatedImageForHashes = true;
        if (g_useRotatedImageForHashes) {
            //pass in inv matrix
            v_prime = getAllTheHashesForImageAndShapes(dst, shapes, rotations, 1, invmat, true);
        } else {

            //apply inv transformation matrix to all shapes
            auto newShapes = applyMatrixToPoints(shapes, invmat);
            //pass in identity matrix
            //WRONG, shape is used wrong here...
            v_prime = getAllTheHashesForImageAndShapes(grayImg, newShapes, rotations, 1);
        }
//        for (auto v : v_prime) {
//            auto [outRing, p, b] = v;
//            drawContoursWithRing(img_in_c, vector<ring_t>(1, outRing));
//        }
//        cout << "v.size(): " << v_prime.size() << endl;
//        imshow("outagain", img_in_c);
//        waitKey(0);
        v.reserve(v.size() + distance(v_prime.begin(),v_prime.end()));
        v.insert(v.end(),v_prime.begin(),v_prime.end());
    }
    return v;
}

std::tuple<double, double> getAandBWrapper(const ring_t& shape, point_t centroid) {
    bg::strategy::transform::translate_transformer<double, 2, 2> translate(-centroid.get<0>(), -centroid.get<1>());
    ring_t transformedPoly;
    bg::transform(shape, transformedPoly, translate);
    return getAandB(transformedPoly);
}

uint64_t handleForRotation(const Mat &input_image, ring_t shape, int output_width,
                       const point_t centroid, double a,
                       double b, double area, unsigned int _rotation_in)
{
    double rotation = _rotation_in;
    Mat m = _calcMatrix(area, -centroid.get<0>(), -centroid.get<1>(), rotation, output_width, a, b);

    Mat outputImage(output_width, output_width, CV_8UC3, Scalar(0, 0, 0));
    warpAffine(input_image, outputImage, m, outputImage.size());

    return img_hash::PHash::compute(outputImage);
}

vector<uint64_t> getHashesForShape(const cv::Mat& input_image,
                                     ring_t shape,
                                     int numRotations,
                                     int rotationJump,
                                     int output_width,
                                     int start_rotation)
{
    point_t p;
    bg::centroid(shape, p);
    auto [a, b] = getAandBWrapper(shape, p);
    double area = bg::area(shape);

    auto ret = vector<uint64_t>();
    for (unsigned int i = start_rotation; i < start_rotation + numRotations; i += rotationJump)
    {
        ret.push_back(handleForRotation(input_image, shape, output_width, p, a, b, area, i));
    }
    return ret;
}

uint64_t getHashesForShape_singleRotation(const cv::Mat& input_image,
                                                              ring_t shape,
                                                              int rotation)
{
    return getHashesForShape(input_image, shape, 1, 1, 32, rotation)[0];
}

Mat applyCanny(
        Mat &src_gray,
        int thresh,
        int ratio,
        int kernel_size,
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

