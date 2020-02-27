#ifndef mainImageProcessingFunctions_hpp
#define mainImageProcessingFunctions_hpp

#include <vector>
#include <opencv2/opencv.hpp>

#include "FragmentHash.h"
#include "ShapeAndPositionInvariantImage.hpp"
#include "boostGeometryTypes.hpp"
#include "shapeNormalise.hpp"

using namespace cv;

Mat covertToDynamicallyAllocatedMatrix(const Matx33d transformation_matrix);

//FIXME: move this from this header file and stop making everything static
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

template<typename T>
static vector<pair<ring_t, T > > getAllTheHashesForImageAndShapes(Mat &imgdata, vector<ring_t> shapes, int rotations=360)
{
    vector<pair<ring_t, T> > ret(shapes.size()*rotations);
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

void simplifyColors(Mat& img);

template<typename T>
static vector<pair<ring_t, T>> getAllTheHashesForImage(
        Mat &img_in,
        int rotations=360,
        int thresh=100,
        int ratio=3,
        int kernel_size=3,
        int blur_width=6,
        int areaThresh=200,
        bool simplify=true
)
{
    Mat img = img_in.clone();
    if (simplify) {
        simplifyColors(img);
    }

    Mat canny_output = applyCanny(img, thresh, kernel_size, ratio, blur_width);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0));

    vector<ring_t> shapes = extractShapesFromContours(contours, areaThresh);

    return getAllTheHashesForImageAndShapes<T>(img_in, shapes, rotations);
}


#endif//mainImageProcessingFunctions_cpp
