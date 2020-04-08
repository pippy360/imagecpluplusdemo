
#include <vector>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <math.h>       /* pow, atan2 */


#include "annoylib.h"
#include "annoymodule.cc"


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
        int rotationJump,
        trans::matrix_transformer<double, 2, 2> transMat = trans::matrix_transformer<double, 2, 2>(),
        bool applyTransMat = false)
{
    vector<tuple<ring_t, uint64_t, int>> ret;

//#pragma omp parallel for
    for (int i = 0; i < shapes.size(); i++)
    {
        auto shape = shapes[i];
        auto hashes = getHashesForShape(imgdata, shape, rotations, rotationJump, 32, 0, transMat, applyTransMat);
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

void findContoursWrapper(const Mat &canny_output, vector<vector<Point>> &contours, double epsilon, bool smooth)
{
    vector<Vec4i> hierarchy;
    if (smooth) {
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


typedef std::vector< double > state_type;
struct push_back_state_and_time
{
    std::vector< state_type >& m_states;
    std::vector< double >& m_times;

    push_back_state_and_time( std::vector< state_type > &states , std::vector< double > &times )
            : m_states( states ) , m_times( times ) { }

    void operator()( const state_type &x , double t )
    {
        m_states.push_back( x );
        m_times.push_back( t );
    }
};

vector<double> getMaximumPointsFromCurvature(linestring_t contour)
{
    vector<double> xs;
    vector<double> ys;

    int iter = (contour.size() < 8 )? 8 : contour.size();
    for (int i = 0; i < iter; i++) {
        //repeat the last point if < 8 because boost splines need 8 or more points
        int index = (contour.size()-1 < i)? contour.size()-1 : i;
        auto p = contour[index];
        xs.push_back(p.get<0>());
        ys.push_back(p.get<1>());
    }

    cardinal_quintic_b_spline<double> spline_xs(xs, 0, 1);
    cardinal_quintic_b_spline<double> spline_ys(ys, 0, 1);

    vector<double> curvatures;
    for (int i = 0; i < xs.size(); i++) {
      curvatures.push_back(calcCurvature(i, spline_xs, spline_ys));
    }

    return curvatures;
}


vector<ring_t> extractShapes(
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        Mat &grayImg,
        bool useDilate,
        bool useErodeBefore,
        bool useErodeAfter,
        int erosion_before_size,
        int dilate_size,
        int erosion_after_size
        )
{
    Mat canny_output = applyCanny(grayImg, thresh, ratio, kernel_size, blur_width, useDilate,
                                  useErodeBefore,
                                  useErodeAfter,
                                  erosion_before_size,
                                  dilate_size,
                                  erosion_after_size);

    vector<vector<Point>> contours;
    findContoursWrapper(canny_output, contours);
    return extractShapesFromContours(contours, areaThresh);
}

bool g_useRotatedImageForHashes = true;//FIXME:


trans::matrix_transformer<double, 2, 2> convertInvMatrixToBoost(cv::Mat inmat) {
    vector<vector<double> > mat = {
            {inmat.at<double>(0,0), inmat.at<double>(0,1), inmat.at<double>(0,2)},
            {inmat.at<double>(1,0), inmat.at<double>(1,1), inmat.at<double>(1,2)},
            {0, 0, 1}
    };

    return trans::matrix_transformer<double, 2, 2> (
            mat[0][0], mat[0][1], mat[0][2],
            mat[1][0], mat[1][1], mat[1][2],
            mat[2][0], mat[2][1], mat[2][2]);
}

vector<ring_t> applyInvMatrixToPoints(vector<ring_t> shapes, trans::matrix_transformer<double, 2, 2> transMat) {
    vector<ring_t> result;

    for (auto shape : shapes) {
        ring_t outPoly;
        boost::geometry::transform(shape, outPoly, transMat);
        result.push_back(outPoly);
    }

    return shapes;
}

//FIXME: remove
static RNG rng(12345);

vector<tuple<ring_t, uint64_t, int>> getAllTheHashesForImage(
        Mat img_in,
        int rotations,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        bool simplify,
        bool useDilate,
        bool useErodeBefore,
        bool useErodeAfter,
        int erosion_before_size,
        int dilate_size,
        int erosion_after_size
        )
{
    vector<tuple<ring_t, uint64_t, int>> v;

    Mat grayImg = convertToGrey(img_in);
    for (int i = 0; i < rotations; i += 1) {
        //rotate it and continue
        cout << "Doing for rotation: " << i << endl;
        vector<tuple<ring_t, uint64_t, int>> v_prime;

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
                                              dst,
                                              useDilate,
                                              useErodeBefore,
                                              useErodeAfter,
                                              erosion_before_size,
                                              dilate_size,
                                              erosion_after_size
                                              );
        Mat canny_output = applyCanny(dst, thresh, ratio, kernel_size, blur_width, useDilate,
                                      useErodeBefore,
                                      useErodeAfter,
                                      erosion_before_size,
                                      dilate_size,
                                      erosion_after_size);

//        if (i == 150) {
//            Mat imageCannyOut;
//            cvtColor(canny_output, imageCannyOut, COLOR_GRAY2RGBA);
//
//            vector<vector<Point> > contours;
//            vector<Vec4i> hierarchy;
//            findContoursWrapper(canny_output, contours);
//
//            //Draw contours------
//            Mat contours_img = imageCannyOut.clone();
//            for (int i = 0; i < contours.size(); i++) {
//                Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255), 255);
//                drawContours(contours_img, contours, i, color, 1, 8, hierarchy, 0, Point());
//            }
//            cv::imshow("dist", contours_img);
//            cv::waitKey(0);
//        }

        Mat outRot;
        cv::invertAffineTransform(rot, outRot);
        auto invmat = convertInvMatrixToBoost(outRot);
        g_useRotatedImageForHashes = true;
        if (g_useRotatedImageForHashes) {
            //pass in inv matrix
            v_prime = getAllTheHashesForImageAndShapes(dst, shapes, 1, 1, invmat, true);
        } else {

            //apply inv transformation matrix to all shapes
            auto newShapes = applyInvMatrixToPoints(shapes, invmat);
            //pass in identity matrix
            //WRONG, shape is used wrong here...
            v_prime = getAllTheHashesForImageAndShapes(grayImg, newShapes, 1, 1);
        }

        v.reserve(v.size() + distance(v_prime.begin(),v_prime.end()));
        v.insert(v.end(),v_prime.begin(),v_prime.end());
    }
    return v;
}

using namespace std::chrono;

vector<tuple<ring_t, uint64_t, int>> g_imghashes;
HammingWrapper *tree;

vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>> findMatches(
        Mat img_in,
        Mat img_in2,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        int areaThresh,
        bool flushCache,
        bool useDilate,
        bool useErodeBefore,
        bool useErodeAfter,
        int erosion_before_size,
        int dilate_size,
        int erosion_after_size
        )
{

    //FIXME: this doesn't check if the data changed!!
    // use cached results
    if (flushCache) {
        cout << "Recomping cache" << endl;
//        prevImg = img_in.data;
        g_imghashes = getAllTheHashesForImage(
                img_in,
                360,
                thresh,
                ratio,
                kernel_size,
                blur_width,
                areaThresh,
                false,
                useDilate,
                useErodeBefore,
                useErodeAfter,
                erosion_before_size,
                dilate_size,
                erosion_after_size);

        if (tree != nullptr)
            delete tree;

        tree = new HammingWrapper(64);
        int count = 0;
        for (auto h : g_imghashes) {
            auto [shape1, hash1, rotation1] = h;
            vector<float> unpacked(64, 0);
            tree->_unpack(&hash1, &unpacked[0]);
            tree->add_item(count++, &unpacked[0], nullptr);
        }

        tree->build(20, nullptr);
    }

    auto img2hashes = getAllTheHashesForImage(
                img_in2,
                1,
                thresh,
                ratio,
                kernel_size,
                blur_width,
                areaThresh,
                false,
                useDilate,
                useErodeBefore,
                useErodeAfter,
                erosion_before_size,
                dilate_size,
                erosion_after_size);

    vector<tuple<ring_t, ring_t, uint64_t, uint64_t, int>> res;

    for (auto h2 : img2hashes) {
        vector<int32_t> result;
        vector<float> distances;
        auto [shape2, hash2, rotation2] = h2;
        vector<float> unpacked(64, 0);
        tree->_unpack(&hash2, &unpacked[0]);
        tree->get_nns_by_vector(&unpacked[0], 6, -1, &result, &distances);
        for (int i = 0; i < result.size(); i++) {
            if (distances[i] < 8) {
                auto [shape1, hash1, rotation1] = g_imghashes[result[i]];
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

void handleForRotation(const Mat &input_image, ring_t shape, int output_width,
                       vector<tuple<ring_t, uint64_t, int>> &ret, const point_t centroid, double a,
                       double b, double area, unsigned int _rotation_in,
                       trans::matrix_transformer<double, 2, 2> transMat, bool applyTransMat)
{
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

vector<tuple<ring_t, uint64_t, int>> getHashesForShape(const cv::Mat& input_image,
                                                         ring_t shape,
                                                         int numRotations,
                                                         int rotationJump,
                                                         int output_width,
                                                         int start_rotation,
                                                         trans::matrix_transformer<double, 2, 2> transMat,
                                                         bool applyTransMat
                                                         )
{
    point_t p;
    auto ret = vector<tuple<ring_t, uint64_t, int>>();

    //FIXME: test
    if (applyTransMat) {
        ring_t outPoly;
        boost::geometry::transform(shape, outPoly, transMat);
        shape = outPoly;
    }

    bg::centroid(shape, p);
    auto [a, b] = getAandBWrapper(shape, p);
    double area = bg::area(shape);
    for (unsigned int i = start_rotation; i < start_rotation + numRotations; i += rotationJump)
    {
        handleForRotation(input_image, shape, output_width, ret, p, a, b, area, i, transMat, applyTransMat);
    }
    return ret;
}

tuple<ring_t, uint64_t, int> getHashesForShape_singleRotation(const cv::Mat& input_image,
                                                              const ring_t& shape,
                                                              int rotation)
{
    trans::matrix_transformer<double, 2, 2> transMat;
    return getHashesForShape(input_image, shape, 1, 1, 32, rotation)[0];
}

Mat applyCanny(
        Mat &src_gray,
        int thresh,
        int ratio,
        int kernel_size,
        int blur_width,
        bool useDilate,
        bool useErodeBefore,
        bool useErodeAfter,
        int erosion_before_size,
        int dilate_size,
        int erosion_after_size
        )
{
    Mat canny_output;
    Mat src_gray_blur;
    Mat erosion_dst;
    Mat erosion_before;
    Mat dilation_output;

    assert(src_gray.type() == CV_8U);
    /// Convert image to gray and blur it
    blur( src_gray, src_gray_blur, Size(blur_width, blur_width) );

    /// Detect edges using canny
    Canny( src_gray_blur, canny_output, thresh, thresh*ratio, kernel_size );
    return canny_output;
//    if (useErodeBefore) {
//        int erosion_type;
//        int erosion_size = erosion_before_size;
//        int erosion_elem = 0;
//        if( erosion_elem == 0 ){ erosion_type = MORPH_RECT; }
//        else if( erosion_elem == 1 ){ erosion_type = MORPH_CROSS; }
//        else if( erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }
//
//        Mat element = getStructuringElement( erosion_type, Size( 2*erosion_size + 1, 2*erosion_size+1 ), Point( erosion_size, erosion_size ) );
//
//        erode( canny_output, erosion_before, element );
//    } else {
//        erosion_before = canny_output;
//    }
//
//    if (useDilate) {
//        int dilate_type;
//        int dilate_elem = 0;
//        if( dilate_elem == 0 ){ dilate_type = MORPH_RECT; }
//        else if( dilate_elem == 1 ){ dilate_type = MORPH_CROSS; }
//        else if( dilate_elem == 2) { dilate_type = MORPH_ELLIPSE; }
//
//        Mat element = getStructuringElement( dilate_type, Size( 2*dilate_size + 1, 2*dilate_size+1 ), Point( dilate_size, dilate_size ) );
//
//        dilate( erosion_before, dilation_output, element );
//    } else {
//        dilation_output = erosion_before;
//    }
//
//    if (useErodeAfter) {
//        int erosion_type;
//        int erosion_size = erosion_after_size;
//        int erosion_elem = 0;
//        if( erosion_elem == 0 ){ erosion_type = MORPH_RECT; }
//        else if( erosion_elem == 1 ){ erosion_type = MORPH_CROSS; }
//        else if( erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }
//
//        Mat element = getStructuringElement( erosion_type, Size( 2*erosion_size + 1, 2*erosion_size+1 ), Point( erosion_size, erosion_size ) );
//
//        erode( dilation_output, erosion_dst, element );
//    } else {
//        erosion_dst = dilation_output;
//    }
//
//    return erosion_dst;
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


