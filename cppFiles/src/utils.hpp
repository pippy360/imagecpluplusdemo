#ifndef utils_utils_hpp
#define utils_utils_hpp

#include <vector>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <string>
#include <regex>

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <math.h>
#include <iostream>
#include <tuple>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "PerceptualHash.hpp"
#include "mainImageProcessingFunctions.hpp"


namespace pt = boost::property_tree;
using namespace std;

static unsigned long x=123456789, y=362436069, z=521288629;

inline unsigned long xorshf96(void) {          //period 2^96-1
    unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
}

static void drawSingleShapeOntoImage(ring_t shape, cv::Mat inputImage, bool setColour = false,
        cv::Scalar colourInput = cv::Scalar(0,0,0)) {
    auto prevPoint = shape;

    int r = (int) xorshf96();
    int g = (int) xorshf96();
    int b = (int) xorshf96();
//    for (int i = 0; i < 3; i++){
//        auto currentPoint = keypoints[i];
//        auto colour = (setColour)? colourInput: cv::Scalar(b,g,r);
//
//        cv::line(inputImage, cv::Point(prevPoint.x, prevPoint.y), cv::Point(currentPoint.x, currentPoint.y),
//                 colour);
//        //cv::imshow("something", inputImage);
//        //cv::waitKey(10);
//        prevPoint = currentPoint;
//    }
}

static void drawShapeOntoImage(const vector<ring_t> shapes, const cv::Mat inputImage, bool randomColours = true)
{
    for (auto shape: shapes){
        drawSingleShapeOntoImage(shape, inputImage, !randomColours);
    }
}

static ring_t getShapeFromRedisEntry(const string redisEntry)
{
    pt::ptree root;
    std::stringstream ss;
    ss << redisEntry;
    pt::read_json(ss, root);

    ring_t outPoly;
    bg::read_wkt(root.get<string>("shape"), outPoly);
    return outPoly;
}

static string getImageNameFromRedisEntry(const string redisEntry)
{
    pt::ptree root;
    std::stringstream ss;
    ss << redisEntry;
    pt::read_json(ss, root);
    return root.get<string>("imageName");
}

static string convertToRedisEntryJson(const string imageName, const ring_t shape)
{
    pt::ptree root;
    root.put("imageName", imageName);

    std::ostringstream stream;
    stream << bg::wkt(shape);
    root.put("shape", stream.str());

    std::ostringstream buf;
    pt::write_json(buf, root, false);
    return buf.str();
}

#endif//utils_utils_hpp
