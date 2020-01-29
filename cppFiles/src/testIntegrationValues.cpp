//FIXME: STATIC WHY AREN'T LOCAL FUNCTIONS STATIC
#include <numeric>

#include <gtest/gtest.h>

#include "boostGeometryTypes.hpp"
#include "miscUtils.hpp"

#include "shapeNormalise.hpp"

//The brute force approach will only approximate the correct value
//so we just check if it's close
#define ALLOWED_ERROR 0.002
#define BRUTE_FORCE_TESTING 0

//FIXME: explain
#define ALLOWED_ERROR_MORE_ACCURATE 0.0000001

#include "shapeNormalise.cpp"

double bruteForcePointEquationCommon(point_t p1, point_t p2, double (*func)(double x, double y)) {
    double x1 = p1.get<0>();
    double x2 = p2.get<0>();
    double m = getSlopeOfLine(p1, p2);
    double c = getConstantOfLine(p1, p2);
    double result = 0;
    int totalSteps = 2000;
    double step = (x2 - x1)/((double) totalSteps-1);
    double yValsTotal = 0;
    for (int i = 0; i < totalSteps; i++) {
        double line = 0;
        double xVal = x1 + (step*i);
        double yValFinal = xVal*m + c;
        for(int j = 0; j < totalSteps; j++) {
            double yVal = yValFinal * ((double)j/(double)totalSteps);

            line += func(xVal, yVal);
        }
        result += (line/totalSteps)*yValFinal;
        yValsTotal += yValFinal;
    }
    return result/yValsTotal;
}

double ySquared(double x, double y) {
    return pow(y, 2);
}

double ySquaredFromX1ToX2Wrapper_bruteForce(point_t p1, point_t p2) {
    return bruteForcePointEquationCommon(p1, p2, ySquared);
}

double xMulty(double x, double y) {
    return x*y;
}

double xyFromX1ToX2Wrapper_bruteForce(point_t p1, point_t p2) {
    return bruteForcePointEquationCommon(p1, p2, xMulty);
}

static double customGetAverageVal_insideCheck(std::vector<ring_t> polyList, double (*func)(double x, double y)) {
    std::vector<double> avgvec;
    double totalAreaFix = 0;
    for (auto &poly : polyList) {
        double result = 0;
        long pointCount = 0;
        box_t boundingBox = bg::return_envelope<box_t>(poly);
        double min_x = boundingBox.min_corner().get<0>();
        double min_y = boundingBox.min_corner().get<1>();
        double max_x = boundingBox.max_corner().get<0>();
        double max_y = boundingBox.max_corner().get<1>();

        int totalSteps = 2000;
        double stepx = (max_x - min_x)/((double) totalSteps-1);
        double stepy = (max_y - min_y)/((double) totalSteps-1);

        for (double i = min_x; i <= max_x; i += stepx) {
            for(double j = min_y; j <= max_y; j += stepy) {
                point_t p(i, j);
                if (bg::within(p, poly)) {
                    result += func(i, j);
                    pointCount++;
                }
            }
        }
        double area = bg::area(poly);
        totalAreaFix += area;
        double avg = (result / pointCount);
        avgvec.push_back(avg*area);
    }
    double avgres = std::accumulate( avgvec.begin(), avgvec.end(), 0.0);
    return avgres/totalAreaFix;
}

double getXYAvgVal(std::vector<ring_t> polylist) {
    return customGetAverageVal(polylist, xyFromX1ToX2Wrapper);
}

double getXYAvgVal_bruteForce(std::vector<ring_t> polylist) {
    return customGetAverageVal(polylist, xyFromX1ToX2Wrapper_bruteForce);
}

double getXYAvgVal_bruteForce_insideCheck(std::vector<ring_t> polylist) {
    return customGetAverageVal_insideCheck(polylist, xMulty);
}

double getXYAvgVal_bruteForce_insideCheck_twoPoints(point_t p1, point_t p2) {
    ring_t ring{
            {std::min(p1.get<0>(), p2.get<0>()), std::min(p1.get<1>(), 0.0)},
            {std::min(p1.get<0>(), p2.get<0>()), std::max(p1.get<1>(), 0.0)},
            {std::max(p1.get<0>(), p2.get<0>()), std::max(p2.get<1>(), 0.0)},
            {std::max(p1.get<0>(), p2.get<0>()), std::min(p2.get<1>(), 0.0)},
            {std::min(p1.get<0>(), p2.get<0>()), std::min(p1.get<1>(), 0.0)},
    };

    std::vector<ring_t> v = {ring};
    return customGetAverageVal_insideCheck(v, xMulty);
}

//FIXME: explain
double getXYAvgVal_insideCheck_withSamePath(std::vector<ring_t> polylist) {
    return customGetAverageVal(polylist, getXYAvgVal_bruteForce_insideCheck_twoPoints);
}

void splitIntoQuadsAndTest(ring_t inPoly, std::vector<double> quadrantCorrectVals, double (*func)(point_t p1, point_t p2), double rotation=0) {

    assert(quadrantCorrectVals.size() == 4);

    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(rotation);
    bg::transform(inPoly, transformedPoly, rotate);

#if ASSERT_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(inPoly));
    ASSERT_TRUE(bg::is_valid(transformedPoly));
#endif

    box_t boundingBox = bg::return_envelope<box_t>(transformedPoly);
    std::vector<ring_t> tr;
    getTopRightQuadrant(transformedPoly, boundingBox, tr);
    std::vector<ring_t> tl;
    getTopLeftQuadrant(transformedPoly, boundingBox, tl);
    std::vector<ring_t> br;
    getBottomRightQuadrant(transformedPoly, boundingBox, br);
    std::vector<ring_t> bl;
    getBottomLeftQuadrant(transformedPoly, boundingBox, bl);

    double result;
    ring_t something;

    result = customGetAverageVal(tr, func);
    EXPECT_NEAR(result, quadrantCorrectVals[0], ALLOWED_ERROR);

    result = customGetAverageVal(tl, func);
    EXPECT_NEAR(result, quadrantCorrectVals[1], ALLOWED_ERROR);

    result = customGetAverageVal(br, func);
    EXPECT_NEAR(result, quadrantCorrectVals[2], ALLOWED_ERROR);

    result = customGetAverageVal(bl, func);
    EXPECT_NEAR(result, quadrantCorrectVals[3], ALLOWED_ERROR);
}

void splitIntoQuadsAndTest_bruteForce_insideCheck(ring_t inPoly, std::vector<double> quadrantCorrectVals, double (*func)(double x, double y), double rotation=0) {

    assert(quadrantCorrectVals.size() == 4);

    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(rotation);
    bg::transform(inPoly, transformedPoly, rotate);

#if ASSERT_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(inPoly));
    ASSERT_TRUE(bg::is_valid(transformedPoly));
#endif

    box_t boundingBox = bg::return_envelope<box_t>(transformedPoly);
    std::vector<ring_t> tr;
    getTopRightQuadrant(transformedPoly, boundingBox, tr);
    std::vector<ring_t> tl;
    getTopLeftQuadrant(transformedPoly, boundingBox, tl);
    std::vector<ring_t> br;
    getBottomRightQuadrant(transformedPoly, boundingBox, br);
    std::vector<ring_t> bl;
    getBottomLeftQuadrant(transformedPoly, boundingBox, bl);

    double result;

    result = customGetAverageVal_insideCheck(bl, func);
    EXPECT_NEAR(result, quadrantCorrectVals[0], ALLOWED_ERROR);

    result = customGetAverageVal_insideCheck(br, func);
    EXPECT_NEAR(result, quadrantCorrectVals[1], ALLOWED_ERROR);

    result = customGetAverageVal_insideCheck(tl, func);
    EXPECT_NEAR(result, quadrantCorrectVals[2], ALLOWED_ERROR);

    result = customGetAverageVal_insideCheck(tr, func);
    EXPECT_NEAR(result, quadrantCorrectVals[3], ALLOWED_ERROR);
}

TEST(EquationTest, testYSquared_1x1square) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };

    double rotation = 0;
    std::vector<double> correctVals(4, 0.3333333333333333);
    splitIntoQuadsAndTest(red, correctVals, ySquaredFromX1ToX2Wrapper, rotation);
}

TEST(EquationTest, testYSquared_1x2square_rotated90) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };

    double rotation = 90;
    std::vector<double> correctVals(4, 0.3333333333333333);
    splitIntoQuadsAndTest(red, correctVals, ySquaredFromX1ToX2Wrapper, rotation);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testYSquared_2x1square_bruteForce) {
    ring_t red{
            {-2.0, -1.0}, {-2.0, 1.0}, {2.0, 1.0}, {2.0, -1.0}, {-2.0, -1.0}
    };

    double rotation = 0;
    std::vector<double> correctVals(4, 0.3333333333333333);
    splitIntoQuadsAndTest(red, correctVals, ySquaredFromX1ToX2Wrapper_bruteForce, rotation);
}

TEST(EquationTest, testYSquared_1x2square_bruteForce) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };

    double rotation = 0;
    std::vector<double> correctVals(4, 1.3333333333333333);
    splitIntoQuadsAndTest(red, correctVals, ySquaredFromX1ToX2Wrapper_bruteForce, rotation);
}

TEST(EquationTest, testYSquared_2x1square_rotated90_bruteForce) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };

    double rotation = 90;
    std::vector<double> correctVals(4, 0.3333333333333333);
    splitIntoQuadsAndTest(red, correctVals, ySquaredFromX1ToX2Wrapper_bruteForce, rotation);
}
#endif

TEST(EquationTest, testXY_1x2square_rotated90) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };

    double rotation = 90;
    std::vector<double> correctVals{0.5, -0.5, -0.5, 0.5};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper, rotation);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_1x2square_rotated90_bruteForce) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };

    double rotation = 90;
    std::vector<double> correctVals{0.5, -0.5, -0.5, 0.5};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, rotation);
}

TEST(EquationTest, testXY_1x2square_rotated90_bruteForce_insideCheck) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };

    double rotation = 90;
    std::vector<double> correctVals{0.5, -0.5, -0.5, 0.5};
    splitIntoQuadsAndTest_bruteForce_insideCheck(red, correctVals, xMulty, rotation);
}
#endif

TEST(EquationTest, testXY_diamond_rotated90) {
    ring_t red{
            {-1.0, 0.0}, {0.0, 2.0}, {1.0, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 90;
    std::vector<double> correctVals{0.166667, -0.166667, -0.166667, 0.166667};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper, rotation);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_diamond_rotated90_bruteForce) {
    ring_t red{
            {-1.0, 0.0}, {0.0, 2.0}, {1.0, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 90;
    std::vector<double> correctVals{0.166667, -0.166667, -0.166667, 0.166667};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, rotation);
}

TEST(EquationTest, testXY_diamond_rotated90_bruteForce_insideCheck) {
    ring_t red{
            {-1.0, 0.0}, {0.0, 2.0}, {1.0, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 90;
    std::vector<double> correctVals{0.166667, -0.166667, -0.166667, 0.166667};
    splitIntoQuadsAndTest_bruteForce_insideCheck(red, correctVals, xMulty, rotation);
}
#endif

void combiningTestCommon(double (*func)(std::vector<ring_t>)) {
    ring_t top{
            {0.0, 0.5}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.5}, {0.0, 0.5},
    };
    ring_t bottom{
            {0.0, 0.0}, {0.0, 0.5}, {1.0, 0.5}, {1.0, 0.0}, {0.0, 0.0},
    };
    ring_t full{
            {0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}, {0.0, 0.0},
    };

    ASSERT_TRUE(bg::is_valid(top));
    ASSERT_TRUE(bg::is_valid(bottom));

    box_t boundingBoxTop = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trTop;
    getTopRightQuadrant(top, boundingBoxTop, trTop);

    box_t boundingBoxBottom = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trBottom;
    getTopRightQuadrant(bottom, boundingBoxBottom, trBottom);

    double result;
    result = func(trTop);
    EXPECT_NEAR(result, 0.375, ALLOWED_ERROR);

    result = func(trBottom);
    EXPECT_NEAR(result, 0.125, ALLOWED_ERROR);

    box_t boundingBoxFull = bg::return_envelope<box_t>(full);
    std::vector<ring_t> trFull;
    getTopRightQuadrant(full, boundingBoxFull, trFull);

    result = func(trFull);
    EXPECT_NEAR(result, 0.25, ALLOWED_ERROR);
}

void combiningTestCommonSquare(double (*func)(std::vector<ring_t>)) {
    ring_t top{
            {0.0, 0.5}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.5}, {0.0, 0.5},
    };
    ring_t bottom{
            {0.0, 0.0}, {0.0, 0.5}, {1.0, 0.5}, {1.0, 0.0}, {0.0, 0.0},
    };
    ring_t full{
            {0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}, {0.0, 0.0},
    };

    ASSERT_TRUE(bg::is_valid(top));
    ASSERT_TRUE(bg::is_valid(bottom));

    box_t boundingBoxTop = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trTop;
    getTopRightQuadrant(top, boundingBoxTop, trTop);

    box_t boundingBoxBottom = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trBottom;
    getTopRightQuadrant(bottom, boundingBoxBottom, trBottom);

    double result;
    result = func(trTop);
    EXPECT_NEAR(result, 0.375, ALLOWED_ERROR);

    result = func(trBottom);
    EXPECT_NEAR(result, 0.125, ALLOWED_ERROR);

    box_t boundingBoxFull = bg::return_envelope<box_t>(full);
    std::vector<ring_t> trFull;
    getTopRightQuadrant(full, boundingBoxFull, trFull);

    result = func(trFull);
    EXPECT_NEAR(result, 0.25, ALLOWED_ERROR);
}

TEST(EquationTest, testXY_combiningAreas) {
    combiningTestCommon(getXYAvgVal);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_combiningAreas_bruteForce) {
    combiningTestCommon(getXYAvgVal_bruteForce);
}

TEST(EquationTest, testXY_combiningAreas_bruteForce_insideCheck) {
    combiningTestCommon(getXYAvgVal_bruteForce_insideCheck);
}
#endif

void combiningTestCommonTriangle1x1(double (*func)(std::vector<ring_t>)) {
    ring_t top{
            {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}, {0.0, 1.0},
    };
    ring_t bottom{
            {0.0, 1.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 1.0},
    };
    ring_t full{
            {0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}, {0.0, 0.0},
    };

    ASSERT_TRUE(bg::is_valid(top));
    ASSERT_TRUE(bg::is_valid(bottom));

    box_t boundingBoxTop = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trTop;
    getTopRightQuadrant(top, boundingBoxTop, trTop);

    box_t boundingBoxBottom = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trBottom;
    getTopRightQuadrant(bottom, boundingBoxBottom, trBottom);

    double result;
    result = func(trTop);
    EXPECT_NEAR(result, 0.41666666666666663, ALLOWED_ERROR);

    result = func(trBottom);
    EXPECT_NEAR(result, 0.083333333333333343, ALLOWED_ERROR);

    box_t boundingBoxFull = bg::return_envelope<box_t>(full);
    std::vector<ring_t> trFull;
    getTopRightQuadrant(full, boundingBoxFull, trFull);

    result = func(trFull);
    EXPECT_NEAR(result, 0.25, ALLOWED_ERROR);
}

TEST(EquationTest, testXY_combiningAreasTriangle) {
    combiningTestCommonTriangle1x1(getXYAvgVal);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_combiningAreasTriangle_bruteForce) {
    combiningTestCommonTriangle1x1(getXYAvgVal_bruteForce);
}

TEST(EquationTest, testXY_combiningAreasTriangle_bruteForce_insideCheck) {
    combiningTestCommonTriangle1x1(getXYAvgVal_bruteForce_insideCheck);
}
#endif

void combiningTestCommonTriangle2x1(double (*func)(std::vector<ring_t>)) {
    ring_t top{
            {0.0, 1.0}, {2.0, 1.0}, {2.0, 0.0}, {0.0, 1.0},
    };
    ring_t bottom{
            {0.0, 1.0}, {2.0, 0.0}, {0.0, 0.0}, {0.0, 1.0},
    };
    ring_t full{
            {0.0, 0.0}, {0.0, 1.0}, {2.0, 1.0}, {2.0, 0.0}, {0.0, 0.0},
    };

    ASSERT_TRUE(bg::is_valid(top));
    ASSERT_TRUE(bg::is_valid(bottom));

    box_t boundingBoxTop = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trTop;
    getTopRightQuadrant(top, boundingBoxTop, trTop);

    box_t boundingBoxBottom = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trBottom;
    getTopRightQuadrant(bottom, boundingBoxBottom, trBottom);

    double result;
    result = func(trTop);
    EXPECT_NEAR(result, 0.83333333333333326, ALLOWED_ERROR);

    result = func(trBottom);
    EXPECT_NEAR(result, 0.16666666666666669, ALLOWED_ERROR);

    box_t boundingBoxFull = bg::return_envelope<box_t>(full);
    std::vector<ring_t> trFull;
    getTopRightQuadrant(full, boundingBoxFull, trFull);

    result = func(trFull);
    EXPECT_NEAR(result, 0.5, ALLOWED_ERROR);
}

TEST(EquationTest, testXY_combiningAreasTriangle2x1) {
    combiningTestCommonTriangle2x1(getXYAvgVal);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_combiningAreasTriangle2x1_bruteForce) {
    combiningTestCommonTriangle2x1(getXYAvgVal_bruteForce);
}

TEST(EquationTest, testXY_combiningAreasTriangle2x1_bruteForce_insideCheck) {
    combiningTestCommonTriangle2x1(getXYAvgVal_bruteForce_insideCheck);
}
#endif
void combiningTestCommonTriangle1x1Rotated10(double (*func)(std::vector<ring_t>)) {
    ring_t full{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };

    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(0);
    bg::transform(full, transformedPoly, rotate);

    box_t boundingBoxFull = bg::return_envelope<box_t>(transformedPoly);
    std::vector<ring_t> trFull;
    getTopRightQuadrant(transformedPoly, boundingBoxFull, trFull);
    std::vector<ring_t> tlFull;
    getTopLeftQuadrant(transformedPoly, boundingBoxFull, tlFull);
    std::vector<ring_t> brFull;
    getBottomRightQuadrant(transformedPoly, boundingBoxFull, brFull);
    std::vector<ring_t> blFull;
    getBottomLeftQuadrant(transformedPoly, boundingBoxFull, blFull);

    double result;
    result = func(trFull);
    EXPECT_NEAR(result, 0.25, ALLOWED_ERROR);
    result = func(tlFull);
    EXPECT_NEAR(result, -0.25, ALLOWED_ERROR);
    result = func(brFull);
    EXPECT_NEAR(result, -0.25, ALLOWED_ERROR);
    result = func(blFull);
    EXPECT_NEAR(result, 0.25, ALLOWED_ERROR);
}

TEST(EquationTest, testXY_combiningAreasTriangle1x1Rotated10) {
    combiningTestCommonTriangle1x1Rotated10(getXYAvgVal);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_combiningAreasTriangle1x1Rotated10_bruteForce) {
    combiningTestCommonTriangle1x1Rotated10(getXYAvgVal_bruteForce);
}

TEST(EquationTest, testXY_combiningAreasTriangle1x1Rotated10_bruteForce_insideCheck) {
    combiningTestCommonTriangle1x1Rotated10(getXYAvgVal_bruteForce_insideCheck);
}
#endif

TEST(EquationTest, testXY_combiningAreasTriangle1x1Rotated10_segment) {
    ring_t full{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t tr{
            {0,1.0154266118857449}, {1.1584559306791384,0.81115957534527772}, {1.0154266118857449,0}, {0,0}, {0,1.0154266118857449}
    };
    ring_t tl{
            {-0.81115957534527772, 1.1584559306791387}, {0, 1.0154266118857449}, {0, 0}, {-1.0154266118857449, 0}, {-0.81115957534527772, 1.1584559306791387}
    };

    ring_t tr_seg1{
            {0,1.0154266118857449}, {1.1584559306791384,0.81115957534527772}, {1.1584559306791384,0}, {0,0}, {0,1.0154266118857449}
    };
    ring_t tr_seg2{
            {1.0154266118857449,0}, {1.1584559306791384,0.81115957534527772}, {1.1584559306791384,0}, {1.0154266118857449,0}
    };


    ASSERT_TRUE(bg::is_valid(tr));
    ASSERT_TRUE(bg::is_valid(tl));
    ASSERT_TRUE(bg::is_valid(tr_seg1));
    ASSERT_TRUE(bg::is_valid(tr_seg2));

    std::vector<ring_t> v1 = {tr};
    std::vector<ring_t> v2 = {tr_seg1};
    std::vector<ring_t> v3 = {tr_seg2};

    double result;

    // 0.25634746735956537
    result = getXYAvgVal(v1);
    EXPECT_NEAR(result, 0.24267351385912578, ALLOWED_ERROR);

#if BRUTE_FORCE_TESTING
    // 0.25620858146980419
    result = getXYAvgVal_bruteForce(v1);
    EXPECT_NEAR(result, 0.24267351385912578, ALLOWED_ERROR);

    //
    result = getXYAvgVal_bruteForce_insideCheck(v1);
    EXPECT_NEAR(result, 0.24267351385912578, ALLOWED_ERROR);
#endif

    // 0.25450550408243183
    result = getXYAvgVal(v2);
    EXPECT_NEAR(result, 0.24607205238233901, ALLOWED_ERROR);

#if BRUTE_FORCE_TESTING
    // 0.25450550408243183
    result = getXYAvgVal_bruteForce(v2);
    EXPECT_NEAR(result, 0.24607205238233901, ALLOWED_ERROR);

    //
    result = getXYAvgVal_bruteForce_insideCheck(v2);
    EXPECT_NEAR(result, 0.24607205238233901, ALLOWED_ERROR);
#endif

    // 0.22525485507194642
    result = getXYAvgVal(v3);
    EXPECT_NEAR(result, 0.30370581149783565, ALLOWED_ERROR);

#if BRUTE_FORCE_TESTING
    // ~0.30370581149783565
    result = getXYAvgVal_bruteForce(v3);
    EXPECT_NEAR(result, 0.30370581149783565, ALLOWED_ERROR);

    //
    result = getXYAvgVal_bruteForce_insideCheck(v3);
    EXPECT_NEAR(result, 0.30370581149783565, ALLOWED_ERROR);

    //
    result = getXYAvgVal_insideCheck_withSamePath(v3);
    EXPECT_NEAR(result, 0.30370581149783565, ALLOWED_ERROR);
#endif
}

//FIXME: remote this, it's copy pasted below
TEST(EquationTest, testab4) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(45);
    bg::transform(red, transformedPoly, rotate);

    double a = getA(transformedPoly);
    double b = getB(transformedPoly);
    EXPECT_NEAR(1, a, ALLOWED_ERROR);
    EXPECT_NEAR(0, b, ALLOWED_ERROR);
}


TEST(EquationTest, testXY_complexShape) {
    ring_t red{
            {-1.0, 0.0}, {-1.0, 4}, {-0.5, 0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 1.0}, {0.5, 1.0}, {0.5, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 0;
    std::vector<double> correctVals{0.54166666666666663, -0.80555555555555591, -0.083333333333333343, 0.16666666666666669};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper, rotation);
}


#if BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_complexShape_bruteForce) {
    ring_t red{
            {-1.0, 0.0}, {-1.0, 4}, {-0.5, 0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 1.0}, {0.5, 1.0}, {0.5, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 0;
    std::vector<double> correctVals{0.54166666666666663, -0.80555555555555591, -0.083333333333333343, 0.16666666666666669};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, rotation);
}

TEST(EquationTest, testXY_complexShape_bruteForce_insideCheck) {
    ring_t red{
            {-1.0, 0.0}, {-1.0, 4}, {-0.5, 0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 1.0}, {0.5, 1.0}, {0.5, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 0;
    std::vector<double> correctVals{0.54166666666666663, -0.80555555555555591, -0.083333333333333343, 0.16666666666666669};
    splitIntoQuadsAndTest_bruteForce_insideCheck(red, correctVals, xMulty, rotation);
}
#endif

TEST(EquationTest, testYSquared_1x1square_rotated10) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };

    double rotation = 90;
    std::vector<double> correctVals{0.24239391403659419, -0.24239391403659419, -0.24239391403659419, 0.24239391403659419};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper, 10);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testYSquared_1x1square_rotated10_bruteForce) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };

    double rotation = 90;
    std::vector<double> correctVals{0.24239391403659419, -0.24239391403659419, -0.24239391403659419, 0.24239391403659419};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, 10);
}
#endif

TEST(EquationTest, testab) {
    ring_t red{
        {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    auto[a, b] = getAandB(red);
    EXPECT_NEAR(1.0, a, ALLOWED_ERROR_MORE_ACCURATE);
    EXPECT_NEAR(0, b, ALLOWED_ERROR_MORE_ACCURATE);
}

TEST(EquationTest, testab2) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    std::vector<std::vector<double> > mat = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    bg::strategy::transform::matrix_transformer<double, 2, 2> rotation(mat[0][0], mat[0][1],mat[0][2],
            mat[1][0], mat[1][1], mat[1][2],
            mat[2][0], mat[2][1], mat[2][2]);
    bg::transform(red, transformedPoly, rotation);

    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1, a, ALLOWED_ERROR_MORE_ACCURATE);
    EXPECT_NEAR(0, b, ALLOWED_ERROR_MORE_ACCURATE);
}

TEST(EquationTest, testab3) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(10);
    bg::transform(red, transformedPoly, rotate);

    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1, a, ALLOWED_ERROR_MORE_ACCURATE);
    EXPECT_NEAR(0, b, ALLOWED_ERROR_MORE_ACCURATE);
}

TEST(EquationTest, testab4_) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(45);
    bg::transform(red, transformedPoly, rotate);

    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1, a, ALLOWED_ERROR_MORE_ACCURATE);
    EXPECT_NEAR(0, b, ALLOWED_ERROR_MORE_ACCURATE);
}

TEST(EquationTest, testab7) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    std::vector<std::vector<double> > mat = {{2.0,  0.0,        0},
                                             {0,    1.0/2.0,        0},
                                             {0,    0,          1.0}};
    bg::strategy::transform::matrix_transformer<double, 2, 2> rotation(mat[0][0], mat[0][1],mat[0][2],
                                                                       mat[1][0], mat[1][1], mat[1][2],
                                                                       mat[2][0], mat[2][1], mat[2][2]);
    bg::transform(red, transformedPoly, rotation);
    ASSERT_TRUE(bg::is_valid(transformedPoly));
    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1.0/2.0, a, ALLOWED_ERROR_MORE_ACCURATE);
    EXPECT_NEAR(0.0, b, ALLOWED_ERROR_MORE_ACCURATE);
}

TEST(EquationTest, testab6) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    std::vector<std::vector<double> > mat = {{1.0,  1.0,        0},
                                             {0,    1.0,        0},
                                             {0,    0,          1.0}};
    bg::strategy::transform::matrix_transformer<double, 2, 2> rotation(mat[0][0], mat[0][1],mat[0][2],
                                                                       mat[1][0], mat[1][1], mat[1][2],
                                                                       mat[2][0], mat[2][1], mat[2][2]);
    bg::transform(red, transformedPoly, rotation);
    ASSERT_TRUE(bg::is_valid(transformedPoly));
    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1.0, a, ALLOWED_ERROR_MORE_ACCURATE);
    EXPECT_NEAR(-1.0, b, ALLOWED_ERROR_MORE_ACCURATE);
}

TEST(EquationTest, testab5) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    std::vector<std::vector<double> > mat = {{2.0,  1.0,        0},
                                             {0,    1.0/2.0,    0},
                                             {0,    0,          1.0}};
    bg::strategy::transform::matrix_transformer<double, 2, 2> rotation(mat[0][0], mat[0][1],mat[0][2],
                                                                       mat[1][0], mat[1][1], mat[1][2],
                                                                       mat[2][0], mat[2][1], mat[2][2]);
    bg::transform(red, transformedPoly, rotation);
    ASSERT_TRUE(bg::is_valid(transformedPoly));
    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1.0/2.0, a, ALLOWED_ERROR_MORE_ACCURATE);
    EXPECT_NEAR(-1, b, ALLOWED_ERROR_MORE_ACCURATE);
}


TEST(EquationTest, testab77) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    std::vector<std::vector<double> > mat = {{200.0,  400.0,        0},
                                             {0,    1.0/200.0,    0},
                                             {0,    0,          1.0}};
    bg::strategy::transform::matrix_transformer<double, 2, 2> rotation(mat[0][0], mat[0][1],mat[0][2],
                                                                       mat[1][0], mat[1][1], mat[1][2],
                                                                       mat[2][0], mat[2][1], mat[2][2]);
    bg::transform(red, transformedPoly, rotation);
    ASSERT_TRUE(bg::is_valid(transformedPoly));
    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1.0/200.0, a, ALLOWED_ERROR_MORE_ACCURATE);
    EXPECT_NEAR(-400, b, ALLOWED_ERROR_MORE_ACCURATE);
}

