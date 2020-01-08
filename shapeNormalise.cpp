#include "shapeNormalise.hpp"
#include "boostGeometryTypes.hpp"
#include "miscUtils.hpp"

#include <iostream>

//[int_from p = 0 to (mx+c), x=k1 ... (p)*k1]/(mx+c)
// = (p)^2*k1/2(mx+c) = (mx+c)*x/2.0
//
// [int_from x1 to x2 (mx+c)*x/2.0 dx ] / (x2-x1)
//
//
//
static double xyFromZeroToX(double m, double c, double x) {
    return ((c*pow(x, 2)/4.0) + (1.0/6.0)*(m*pow(x,3)));
}

static double xyFromX1ToX2(double m, double c, double x1, double x2) {
    return (xyFromZeroToX(m, c, x2) - xyFromZeroToX(m, c, x1))
           /
           (x2-x1);
}

double xyFromX1ToX2Wrapper(point_t p1, point_t p2) {
    double x1 = p1.get<0>();
    double x2 = p2.get<0>();
    double m = getSlopeOfLine(p1, p2);
    double c = getConstantOfLine(p1, p2);
    return xyFromX1ToX2(m, c, x1, x2);
}

//[int_(0 to mx+c)=p (p)^2 d(mx+c)]/(mx+c)
// = ((mx+c)^3/3.0 - zero)/(mx+c)
//
// now int_(0 to x) (mx+c)^2/3.0
//
static double ySquaredFromZeroToX(double m, double c, double x) {
    //We don't want to divide by a value VERY close to 0 or we get too much error
    if (abs(m) < MIN_SLOPE_VAL)
        return pow(c, 2)*x/3.0;

    return pow(c + m*x, 3)/(9.0*m);
}

//f(x) = y = mx + c
static double ySquaredFromX1ToX2(double m, double c, double x1, double x2) {
    return (ySquaredFromZeroToX(m, c, x2) - ySquaredFromZeroToX(m, c, x1))
           /
           (x2-x1);
}

double ySquaredFromX1ToX2Wrapper(point_t p1, point_t p2) {
    double x1 = p1.get<0>();
    double x2 = p2.get<0>();
    double m = getSlopeOfLine(p1, p2);
    double c = getConstantOfLine(p1, p2);
    return ySquaredFromX1ToX2(m, c, x1, x2);
}

static double getArea(std::vector<ring_t> shape) {
    double result = 0;
    for (auto& v : shape)
        result += bg::area(v);
    return result;
}

double getAverageValForEveryQuadrant(ring_t inPoly, double (*func)(point_t p1, point_t p2)) {

#ifdef CHECK_SHAPES_VALID
    assert(bg::is_valid(inPoly));
#endif

    box_t boundingBox = bg::return_envelope<box_t>(inPoly);
    std::vector<ring_t> bl = getTopRightQuadrant(inPoly, boundingBox);
    std::vector<ring_t> br = getTopLeftQuadrant(inPoly, boundingBox);
    std::vector<ring_t> tl = getBottomRightQuadrant(inPoly, boundingBox);
    std::vector<ring_t> tr = getBottomLeftQuadrant(inPoly, boundingBox);

    double result = 0;
    double areaBl = getArea(bl);
    result += customGetAverageVal(bl, func)*areaBl;

    double areaBr = getArea(br);
    result += customGetAverageVal(br, func)*areaBr;

    double areaTl = getArea(tl);
    result += customGetAverageVal(tl, func)*areaTl;

    double areaTr = getArea(tr);
    result += customGetAverageVal(tr, func)*areaTr;

    return result/(areaBl + areaBr + areaTl + areaTr);
}

double getXYAverage(ring_t inPoly) {
    return getAverageValForEveryQuadrant(inPoly, xyFromX1ToX2Wrapper);
}

double getYSquaredAverage(ring_t inPoly) {
    return getAverageValForEveryQuadrant(inPoly, ySquaredFromX1ToX2Wrapper);
}

double getXSquaredAverage(ring_t inPoly) {
    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(90);
    bg::transform(inPoly, transformedPoly, rotate);
    return getAverageValForEveryQuadrant(transformedPoly, ySquaredFromX1ToX2Wrapper);
}

static double getA(ring_t inPoly) {
    double ys = getYSquaredAverage(inPoly);
    double xs = getXSquaredAverage(inPoly);
    double xy = getXYAverage(inPoly);
    return sqrt(sqrt( pow(ys, 2) / (xs*ys - pow(xy, 2)) ));
}

static double getB(ring_t inPoly) {
    double ys = getYSquaredAverage(inPoly);
    double xy = getXYAverage(inPoly);
/*
    std::cout << "her2:" << std::endl;
    std::cout << ys << std::endl;
    std::cout << xy << std::endl;
    std::cout << getXSquaredAverage(inPoly) << std::endl;
*/
    return getA(inPoly)*(-xy/ys);
}

std::tuple<double, double> getAandB(ring_t inPoly) {
/*

    std::cout << "here:" << std::endl;
    std::cout << getA(inPoly) << std::endl;
    std::cout << getB(inPoly) << std::endl;
    */
    return  {getA(inPoly), getB(inPoly)};
}

