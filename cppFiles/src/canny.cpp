#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "shapeNormalise.hpp"

using namespace cv;
using namespace std;

Mat src; Mat src_gray;
int thresh = 50;
int max_thresh = 255;
RNG rng(12345);

/// Function header
void thresh_callback(int, void* );

/** @function main */
int main( int argc, char** argv )
{
    /// Load source image and convert it to gray
    src = imread( "test.jpg", 1 );

    /// Convert image to gray and blur it
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    blur( src_gray, src_gray, Size(6,6) );

    /// Create Window
    char* source_window = "Source";
    namedWindow( source_window, WINDOW_AUTOSIZE );
    imshow( source_window, src_gray );

    createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );
    thresh_callback( 0, 0 );

    waitKey(0);
    return(0);
}

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

/** @function thresh_callback */
void thresh_callback(int, void* )
{
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

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
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        ring_t outPoly;
        std::string reason;
        if(!convert_to_boost(hull[i], outPoly, reason)){
            //std::cout << "Poly isn't valid" << reason << std::endl;
            continue;
        }

        ring_t transformedPoly;
        point_t p;
        boost::geometry::centroid(outPoly, p);
        bg::strategy::transform::translate_transformer<double, 2, 2> translate(-p.get<0>(), -p.get<1>());
        boost::geometry::transform(outPoly, transformedPoly, translate);

        drawContours( drawing, hull, i, color, 2, 8, hierarchy, 0, Point() );
        auto [a, b] = getAandB(transformedPoly);
        std::cout << a << " : " << b <<std::endl;

        cv::Matx33d transpose_m(1.0, 0.0, -p.get<0>(),
                                0.0, 1.0, -p.get<1>(),
                                0.0, 0.0, 1.0);
        cv::Matx33d transpose_2(a, b, 0,
                                0.0, 1.0/a, 0,
                                0.0, 0.0, 1.0);
        cv::Matx33d transpose_3(1.0, 1.0, 250,
                                0.0, 1.0, 250,
                                0.0, 0.0, 1.0);
        Mat m = covertToDynamicallyAllocatedMatrix(transpose_3*transpose_2*transpose_m);
        Mat outputImage(500, 500, CV_8UC3, Scalar(0, 0, 0));
        warpAffine(src, outputImage, m, outputImage.size());
        //imwrite("frag.jpg" , outputImage);
        imshow( "Contours", outputImage );
        waitKey(0);
    }

    //TODO: append point to the end
    /// Show in a window
    namedWindow( "Contours", WINDOW_AUTOSIZE );
    imshow( "Contours", drawing );
    imwrite("output.jpg", drawing);
}