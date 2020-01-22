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

unsigned long xorshf96(void) {          //period 2^96-1
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

void drawSingleShapeOntoImage(ring_t shape, cv::Mat inputImage, bool setColour = false, cv::Scalar colourInput = cv::Scalar(0,0,0)){
    auto prevPoint = shape;

    int r = (int) xorshf96();
    int g = (int) xorshf96();
    int b = (int) xorshf96();
    for (int i = 0; i < 3; i++){
        /*
        auto currentPoint = keypoints[i];
        auto colour = (setColour)? colourInput: cv::Scalar(b,g,r);

        cv::line(inputImage, cv::Point(prevPoint.x, prevPoint.y), cv::Point(currentPoint.x, currentPoint.y),
                 colour);
        //cv::imshow("something", inputImage);
        //cv::waitKey(10);
        prevPoint = currentPoint;
         */
    }
}

void drawShapeOntoImage(vector<ring_t> shapes, cv::Mat inputImage, bool randomColours = true)
{
    for (auto shape: shapes){
        drawSingleShapeOntoImage(shape, inputImage, !randomColours);
    }
}

ring_t getShapeFromRedisEntry(string redisEntry)
{
    pt::ptree root;
    std::stringstream ss;
    ss << redisEntry;
    pt::read_json(ss, root);

    ring_t keypoints;
    for (auto pt_j : root.get_child("triangle"))
    {
        double x = pt_j.second.get<double>("x");
        double y = pt_j.second.get<double>("y");
        //keypoints.push_back((x, y));
    }
    return ring_t(keypoints);
}

string getImageNameFromRedisEntry(string redisEntry)
{
    pt::ptree root;
    std::stringstream ss;
    ss << redisEntry;
    pt::read_json(ss, root);
    return root.get<string>("imageName");
}

string convertToRedisEntryJson(string imageName, ring_t tri)
{
    /*
    pt::ptree root;
    root.put("imageName", imageName);

    pt::ptree points;
    for (auto pt : tri.toKeypoints())
    {
        pt::ptree point;
        point.put("x", pt.x);
        point.put("y", pt.y);
        points.push_back(std::make_pair("", point));
    }
    root.add_child("triangle", points);

    std::ostringstream buf;
    pt::write_json(buf, root, false);
    return buf.str();
     */
    return "";
}

vector<ring_t> getTriangles(string filename)
{
    return {};//readShapesFromKeypointJsonFile(filename);
}

template<typename T>
const vector<T> readJsonHashesFile(std::ifstream *file)
{
    /*
    vector<T> ret;
    vector<ring_t> image1OutputTriangles;
    vector<ring_t> image2OutputTriangles;
    try {
        pt::ptree pt;
        pt::read_json(*file, pt);

        for (auto label0 : pt) {
            if (label0.first == "output") {
                for (auto label1 : label0.second) {
                    if (label1.first == "imageName") {
                        //TODO: process the imageName
                    }
                    else if (label1.first == "hashes") {
                        for (auto hash_item : label1.second) {
                            ret.push_back(T(hash_item.second.get_value<std::string>()));
                        }
                    }
                }
            }
        }

    }
    catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }

    return ret;
     */
    return {};
}

template<typename T>
const vector<T> readJsonHashesFile(const string filename)
{
    std::ifstream file(filename);
    return readJsonHashesFile<T>(&file);
}

ring_t readShapesFromJsonFile(std::ifstream *file)
{
    /*
    ring_t result;
    try {
        pt::ptree pt;
        pt::read_json(*file, pt);

        for (auto label0 : pt) {
            if (label0.first == "output") {
                for (auto label1 : label0.second) {
                    if (label1.first == "keypoints") {
                        for (auto kp : label1.second){
                            double x, y;
                            for (auto pt : kp.second){
                                if (pt.first == "x"){
                                    x = pt.second.get_value<double>();
                                }
                                else{
                                    y = pt.second.get_value<double>();
                                }
                            }
                            result.push_back(Keypoint(x, y));
                        }
                    }
                }
            }
        }
    }
    catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
    return result;
     */
    return {};
}

ring_t readShapesFromJsonFile(string filename)
{
    std::ifstream file(filename);
    return readShapesFromJsonFile(&file);
}


#endif//utils_utils_hpp
