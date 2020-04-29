//
// Created by Tom Murphy on 2020-04-21.
//

#ifndef IMAGECPLUPLUSDEMO2_SEARCH_H
#define IMAGECPLUPLUSDEMO2_SEARCH_H

#include "annoylib.h"
#include "annoymodule.cc"

#include "boostGeometryTypes.hpp"
#include "defaultImageValues.h"

#include "mainImageProcessingFunctions.hpp"


class ImageHashDatabase {
public:
    vector<string> imagePaths;
    vector<tuple<ring_t, string, int>> shapesStrCache;
    vector<tuple<int, uint64_t, double>> database;
    HammingWrapper tree;

    ImageHashDatabase () :
            tree(64)
    {};
};

map<string, map<string, vector< tuple<uint64_t, uint64_t, int> >>> findMatchesBetweenTwoImages(
        cv::Mat img_in,
        cv::Mat img_in2,
        int thresh=CANNY_THRESH,
        int ratio=CANNY_RATIO,
        int kernel_size=CANNY_KERNEL_SIZE,
        int blur_width=CANNY_BLUR_WIDTH,
        int areaThresh=CANNY_AREA_THRESH,
        bool flushCache=true
                );

void addImageToSearchTree(
        ImageHashDatabase &database,
        string imageName,
        Mat img_in,
        int thresh=CANNY_THRESH,
        int ratio=CANNY_RATIO,
        int kernel_size=CANNY_KERNEL_SIZE,
        int blur_width=CANNY_BLUR_WIDTH,
        int areaThresh=CANNY_AREA_THRESH
        );

map<string, map<string, map<string, vector< tuple<uint64_t, uint64_t, int> >>>> findDetailedMatches(
        ImageHashDatabase &database,
        Mat img_in2,
        int thresh=CANNY_THRESH,
        int ratio=CANNY_RATIO,
        int kernel_size=CANNY_KERNEL_SIZE,
        int blur_width=CANNY_BLUR_WIDTH,
        int areaThresh=CANNY_AREA_THRESH
        );

#endif //IMAGECPLUPLUSDEMO2_SEARCH_H
