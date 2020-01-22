#ifndef SHAPENORMALISE_H
#define SHAPENORMALISE_H

#include "boostGeometryTypes.hpp"

#include "opencv2/imgproc/imgproc.hpp"

std::tuple<double, double> getAandB(ring_t inPoly);

#endif /* SHAPENORMALISE_H */