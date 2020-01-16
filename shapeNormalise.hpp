#ifndef SHAPENORMALISE_H
#define SHAPENORMALISE_H

#include "boostGeometryTypes.hpp"

double xyFromX1ToX2Wrapper(point_t p1, point_t p2);

double ySquaredFromX1ToX2Wrapper(point_t p1, point_t p2);

double getXYAverage(ring_t inPoly);

double getYSquaredAverage(ring_t inPoly);

double getXSquaredAverage(ring_t inPoly);

std::tuple<double, double> getAandB(ring_t inPoly);

//FIMXE: remove these two below, only here for testing
double getA(ring_t inPoly);

double getB(ring_t inPoly);

#endif /* SHAPENORMALISE_H */