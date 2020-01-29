#include "shapeNormalise.hpp"
#include "boostGeometryTypes.hpp"
#include "miscUtils.hpp"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

//used to get the average x/y/x^2/y^2/x*y value of every point in a polygon segment
static double customGetAverageVal(std::vector<ring_t> polyList, double (*func)(point_t p1, point_t p2)) {
    double result = 0;
    double totalArea = 0;

    for (auto &poly : polyList) {
        //the first point is always repeated at the end so poly.size() - 1 is enough loops
        for (int i = 0; i < poly.size() - 1; i++) {
            point_t p = poly[i];
            point_t np = poly[i + 1];
            double x1 = p.get<0>();
            double x2 = np.get<0>();
            double area = getAreaUnderTwoPoints(p, np);

            if (abs(area) < 0.000001)
                continue;

            //check direction
            if (x1 < x2) {
                double funcVal = func(p, np);
                result += funcVal * area;
                totalArea += area;
            } else {
                double funcVal = func(p, np);
                result -= funcVal * area;
                totalArea -= area;
            }
        }
    }

    return result / totalArea;
}

//[int_from p = 0 to (mx+c), x=k1 ... (p)*k1]/(mx+c)
// = (p)^2*k1/2(mx+c) = (mx+c)*x/2.0
//
// [int_from x1 to x2 (mx+c)*x/2.0 dx ] / (x2-x1)
//
//
//
static double xyFromZeroToX(double m, double c, double x, double signFix) {
    return 0.5 * ((0.5 * pow(c,2) * pow(x,2)) + (2.0/3.0 * c * m * pow(x,3)) + (1.0/4.0 * pow(m,2) * pow(x,4)));
}

static double xyFromX1ToX2(double m, double c, double x1, double x2, double signFix) {
    double first = xyFromZeroToX(m, c, x2, signFix);
    double second = xyFromZeroToX(m, c, x1, signFix);
    return first - second;
}

static double iszero(double in){
    return abs(in) < 0.000000001;
}

static double xyFromX1ToX2Wrapper(point_t p1, point_t p2) {
    double x1 = p1.get<0>();
    double y1 = p1.get<1>();
    double x2 = p2.get<0>();
    double y2 = p2.get<1>();
    double signFix = -1;
    double m = getSlopeOfLine(p1, p2);
    double c = getConstantOfLine(p1, p2);
    double top = xyFromX1ToX2(m, c, x1, x2, signFix);
    double bottom = getAreaUnderTwoPoints(p1, p2);
    double result = top/bottom;
    if (((iszero(x1) || x1 <= 0) && (iszero(x2) || x2 <= 0) && ((iszero(y1) || y1 <= 0) && (iszero(y2) || y2 <= 0)))
                || ((iszero(x1) || x1 >= 0) && (iszero(x2) || x2 >= 0) && ((iszero(y1) || y1 >= 0) && (iszero(y2) || y2 >= 0))))
        return (result < 0)? result * -1 : result;
    else
        return (result >= 0)? result * -1 : result;
}

//[int_(0 to mx+c)=p (p)^2 d(mx+c)]/(mx+c)
// = ((mx+c)^3/3.0 - zero)/(mx+c)
//
// now int_(0 to x) (mx+c)^2/3.0
//
static double ySquaredFromZeroToX(double m, double c, double x) {
    //We don't want to divide by a value VERY close to 0 or we get too much error
    if (abs(m) < MIN_SLOPE_VAL)
        return pow(c, 3)*x/3.0;

    return pow(c + m*x, 4)/(12.0*m);
}

//f(x) = y = mx + c
static double ySquaredFromX1ToX2(double m, double c, double x1, double x2) {
    double first    = ySquaredFromZeroToX(m, c, x2);
    double second   = ySquaredFromZeroToX(m, c, x1);
    return (first - second);
}

static double ySquaredFromX1ToX2Wrapper(point_t p1, point_t p2) {
    double x1 = p1.get<0>();
    double x2 = p2.get<0>();
    double m = getSlopeOfLine(p1, p2);
    double c = getConstantOfLine(p1, p2);
    double result = abs(ySquaredFromX1ToX2(m, c, x1, x2));
    double area = getAreaUnderTwoPoints(p1, p2);
    return result/area;
}

static double getArea(std::vector<ring_t> shape) {
    double result = 0;
    for (auto& v : shape)
        result += bg::area(v);
    return result;
}

static double getAverageValForEveryQuadrant(ring_t inPoly, double (*func)(point_t p1, point_t p2)) {

#if ASSERT_SHAPES_VALID
    assert(bg::is_valid(inPoly));
#endif

//    std::cout << "----start " << std::endl;
    //FIXME: rename these below to the actual tr...

    double result = 0;
    double totalArea = 0;
    box_t boundingBox = bg::return_envelope<box_t>(inPoly);

    std::vector<ring_t> tr;
    if (getTopRightQuadrant(inPoly, boundingBox, tr)) {
        double areaTr   = getArea(tr);
        double valTr    = customGetAverageVal(tr, func);
        result += valTr*areaTr;
        totalArea += areaTr;
    }

    std::vector<ring_t> tl;
    if (getTopLeftQuadrant(inPoly, boundingBox, tl)) {
        double areaTl   = getArea(tl);
        double valTl    = customGetAverageVal(tl, func);
        result += valTl*areaTl;
        totalArea += areaTl;
    }

    std::vector<ring_t> br;
    if (getBottomRightQuadrant(inPoly, boundingBox, br)) {
        double areaBr   = getArea(br);
        double valBr    = customGetAverageVal(br, func);
        result += valBr * areaBr;
        totalArea += areaBr;
    }

    std::vector<ring_t> bl;
    if (getBottomLeftQuadrant(inPoly, boundingBox, bl)) {
        double areaBl   = getArea(bl);
        double valBl    = customGetAverageVal(bl, func);
        result += valBl*areaBl;
        totalArea += areaBl;
    }

    double fin = result/totalArea;
    return fin;
}

static double getXYAverage(ring_t inPoly) {
     return getAverageValForEveryQuadrant(inPoly, xyFromX1ToX2Wrapper);
}

static double getYSquaredAverage(ring_t inPoly) {
    return getAverageValForEveryQuadrant(inPoly, ySquaredFromX1ToX2Wrapper);
}

static double getXSquaredAverage(ring_t inPoly) {
    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(90);
    bg::transform(inPoly, transformedPoly, rotate);
    return getAverageValForEveryQuadrant(transformedPoly, ySquaredFromX1ToX2Wrapper);
}

static double getA(ring_t inPoly) {
    double ys = getYSquaredAverage(inPoly);
    double xs = getXSquaredAverage(inPoly);
    double xy = getXYAverage(inPoly);
    double val = pow(ys, 2) / (xs*ys - pow(xy, 2));
    return sqrt(sqrt( val ));
}

static double getB(ring_t inPoly) {
    double ys = getYSquaredAverage(inPoly);
    double xy = getXYAverage(inPoly);
    return getA(inPoly)*(-xy/ys);
}

std::tuple<double, double> getAandB(ring_t inPoly) {
    double a = getA(inPoly);
    double b = getB(inPoly);
    return  {a, b};
}

bool convert_to_boost(std::vector<cv::Point> inPoly, ring_t &result) {
    std::vector<point_t> points;
    for (int i = 0; i < inPoly.size(); i++) {
        points.push_back(point_t(inPoly[i].x, inPoly[i].y));
    }
    bg::assign_points(result, points);

    //fix it up
    bg::remove_spikes(result);
    bg::correct(result);

    return bg::is_valid(result);
}

