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

#define NUM_OF_ROTATIONS 360
#define HASH_SIZE 8
#define FRAGMENT_WIDTH 60*.86
#define FRAGMENT_HEIGHT 60


namespace cv
{

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

    std::vector<ShapeAndPositionInvariantImage> normaliseScaleAndRotationForSingleFrag(ShapeAndPositionInvariantImage& fragment)
    {
        auto shape = fragment.getShape();
        auto ret = std::vector<ShapeAndPositionInvariantImage>();
        int outputTriangleSizeX = FRAGMENT_WIDTH;
        int outputTriangleSizeY = FRAGMENT_HEIGHT;
        for (unsigned int i = 0; i < NUM_OF_ROTATIONS; i++)
        {
            auto transformationMatrix = calcTransformationMatrixWithShapePreperation(shape, i);
            auto input_img = fragment.getImageData();
            auto newImageData = applyTransformationMatrixToImage(input_img, transformationMatrix, outputTriangleSizeX, outputTriangleSizeY);
            auto t = ShapeAndPositionInvariantImage(fragment.getImageName(), newImageData, shape, fragment.getImageFullPath());
            ret.push_back(t);
        }

        return ret;
    }

    ShapeAndPositionInvariantImage getFragment(const ShapeAndPositionInvariantImage& input_image, const ring_t& shape)
    {
        return ShapeAndPositionInvariantImage(input_image.getImageName(), input_image.getImageData(), shape, "");
    }

    template<typename T> std::vector<T> getHashesForFragments(std::vector<ShapeAndPositionInvariantImage>& normalisedFragments)
    {
        auto ret = std::vector<T>();
        for (auto frag : normalisedFragments)
        {
            auto calculatedHash = T(frag);
            ret.push_back(calculatedHash);
        }
        return ret;
    }

    template<typename T> static std::vector<T> getHashesForShape(ShapeAndPositionInvariantImage& input_image, const ring_t& tri)
    {
        auto fragment = getFragment(input_image, tri);
        auto normalisedFragments = normaliseScaleAndRotationForSingleFrag(fragment);
        auto hashes = getHashesForFragments<T>(normalisedFragments);

        return hashes;
    }

    template<typename T> static vector<pair<ring_t, T>> getAllTheHashesForImage(ShapeAndPositionInvariantImage inputImage,
            std::vector<ring_t> shapes)
    {
        vector<pair<ring_t, T>> ret(shapes.size()*NUM_OF_ROTATIONS);

#pragma omp parallel for
        for (unsigned int i = 0; i < shapes.size(); i++)
        {
            auto tri = shapes[i];
            auto hashes = getHashesForShape<T>(inputImage.getImageData(), tri);

            for (unsigned int j = 0; j < 3; j++)
            {
                ret[(i * 3) + j] = pair<ring_t, T>(tri, hashes[j]);
            }
        }
        return ret;
    }

}//namespace cv

#endif//mainImageProcessingFunctions_cpp
