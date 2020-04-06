#ifndef SHAPENORMALISE_H
#define SHAPENORMALISE_H

#include "boostGeometryTypes.hpp"

#include "opencv2/imgproc/imgproc.hpp"

std::tuple<double, double> getAandB(ring_t inPoly);

bool convert_to_boost(std::vector<cv::Point> inPoly, ring_t &result);

bool convert_to_boost_linestring(std::vector<cv::Point> inPoly, linestring_t &result);

#endif /* SHAPENORMALISE_H */