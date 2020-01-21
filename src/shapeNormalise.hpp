#ifndef SHAPENORMALISE_H
#define SHAPENORMALISE_H

#include "boostGeometryTypes.hpp"

#include "opencv2/imgproc/imgproc.hpp"

double xyFromX1ToX2Wrapper(point_t p1, point_t p2);

double ySquaredFromX1ToX2Wrapper(point_t p1, point_t p2);

double getXYAverage(ring_t inPoly);

//TODO: remove
bool is_valid_getAandBCV(std::vector<cv::Point> inPoly);

double getYSquaredAverage(ring_t inPoly);

double getXSquaredAverage(ring_t inPoly);

std::tuple<double, double> getAandB(ring_t inPoly);

std::tuple<double, double> getAandBCV(std::vector<cv::Point> inPoly);

bool convert_to_boost(std::vector<cv::Point> inPoly, ring_t &result, std::string &reason);

//FIMXE: remove these two below, only here for testing
double getA(ring_t inPoly);

double getB(ring_t inPoly);

#endif /* SHAPENORMALISE_H */