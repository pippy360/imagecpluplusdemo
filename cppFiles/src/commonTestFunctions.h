//
// Created by Tom Murphy on 2020-04-10.
//

#ifndef IMAGECPLUPLUSDEMO2_COMMONTESTFUNCTIONS_H
#define IMAGECPLUPLUSDEMO2_COMMONTESTFUNCTIONS_H

#include "annoylib.h"

#include <vector>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <math.h>

#include "ImageHash.hpp"
#include "boostGeometryTypes.hpp"
#include "miscUtils.hpp"
#include "shapeNormalise.hpp"
#include "PerceptualHash.hpp"
#include "mainImageProcessingFunctions.hpp"

#include "search.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include <algorithm>
#include <chrono>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>


namespace pt = boost::property_tree;


tuple<Mat, Mat> transfromImage_keepVisable(Mat img_in, cv::Matx33d transmat);

vector<tuple<ring_t, vector<tuple<ring_t, double, int>>>> compareImages(Mat img_in, Mat img_in2,
                   DrawingOptions d,
                   Mat transmat);

tuple<pair<int, int>, map<string, int>> getMatchesForTransformation(
                    ImageHashDatabase &database,
                    Mat databaseImg,
                    string databaseImgKey,
                    Matx33f m33,
                    DrawingOptions d);

pt::ptree getMatchesForTransformation_json(
        ImageHashDatabase &database,
        map<string, int> &imgMismatches,
        Mat databaseImg,
        string databaseImgKey,
        Matx33f m33,
        DrawingOptions d);

vector<vector<int>> getMatchesForTransformation_hashDistances(
        ImageHashDatabase &database,
        Mat databaseImg,
        string databaseImgKey,
        Matx33f m33,
        DrawingOptions d);

pt::ptree getMatchesForTransformation_hashDistances_json(
        ImageHashDatabase &database,
        Mat databaseImg,
        string databaseImgKey,
        Matx33f m33,
        DrawingOptions d);

double getPerctageOverlap(ring_t s1, ring_t s2, double s1_area);

double getPerctageOverlap(ring_t s1, ring_t s2);

#endif //IMAGECPLUPLUSDEMO2_COMMONTESTFUNCTIONS_H
