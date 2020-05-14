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

map<string, map<string, vector< tuple<uint64_t, uint64_t, int, int> >>> findMatchesBetweenTwoImages(
        cv::Mat img_in,
        cv::Mat img_in2,
        DrawingOptions d,
        bool flushCache=true
                );

void addImageToSearchTree(
        ImageHashDatabase &database,
        string imageName,
        Mat img_in,
        DrawingOptions d
            );

map<string, map<string, map<string, vector< tuple<uint64_t, uint64_t, int, int> >>>> findDetailedMatches(
        ImageHashDatabase &database,
        Mat img_in2,
        DrawingOptions d
            );


vector<map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >>> findDetailedSameImageMatches_prepopulatedDatabase(
        Mat queryImage,
        ImageHashDatabase &localDatabase,
        string databaseKey,
        Mat databaseToQuery_CVMat,
        DrawingOptions d
            );

vector<map<string, map<string, tuple<ring_t, ring_t, vector<tuple<uint64_t, uint64_t, int, int>>> >>> findDetailedSameImageMatches(
        Mat queryImage,
        Mat databaseImage,
        Mat databaseToQuery_CVMat,
        DrawingOptions d
            );


#endif //IMAGECPLUPLUSDEMO2_SEARCH_H
