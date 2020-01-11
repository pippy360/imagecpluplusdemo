
#include <gtest/gtest.h>

#include "boostGeometryTypes.hpp"
#include "miscUtils.hpp"

#include "shapeNormalise.hpp"

//The brute force approach will only approximate the correct value
//so we just check if it's close
#define ALLOWED_ERROR 0.002
#define BRUTE_FORCE_TESTING 1


double bruteForcePointEquationCommon(point_t p1, point_t p2, double (*func)(double x, double y)) {
    double x1 = p1.get<0>();
    double x2 = p2.get<0>();
    double m = getSlopeOfLine(p1, p2);
    double c = getConstantOfLine(p1, p2);
    double result = 0;
    int totalSteps = 2000;
    double step = (x2 - x1)/((double) totalSteps-1);
    for (int i = 0; i < totalSteps; i++) {
        double line = 0;
        for(int j = 0; j < totalSteps; j++) {
            double xVal = x1 + (step*i);
            double yVal = (xVal*m + c) * ((double)j/(double)totalSteps);

            line += func(xVal, yVal);
        }
        result += line/totalSteps;
    }
    return result/totalSteps;
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

void splitIntoQuadsAndTest(ring_t inPoly, std::vector<double> quadrantCorrectVals, double (*func)(point_t p1, point_t p2), double rotation=0) {

    assert(quadrantCorrectVals.size() == 4);

    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(rotation);
    bg::transform(inPoly, transformedPoly, rotate);

#ifdef CHECK_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(inPoly));
    ASSERT_TRUE(bg::is_valid(transformedPoly));
#endif

    box_t boundingBox = bg::return_envelope<box_t>(transformedPoly);
    std::vector<ring_t> bl = getTopRightQuadrant(transformedPoly, boundingBox);
    std::vector<ring_t> br = getTopLeftQuadrant(transformedPoly, boundingBox);
    std::vector<ring_t> tl = getBottomRightQuadrant(transformedPoly, boundingBox);
    std::vector<ring_t> tr = getBottomLeftQuadrant(transformedPoly, boundingBox);

    double result;

    result = customGetAverageVal(bl, func);
    EXPECT_NEAR(result, quadrantCorrectVals[0], ALLOWED_ERROR);

    result = customGetAverageVal(br, func);
    EXPECT_NEAR(result, quadrantCorrectVals[1], ALLOWED_ERROR);

    result = customGetAverageVal(tl, func);
    EXPECT_NEAR(result, quadrantCorrectVals[2], ALLOWED_ERROR);

    result = customGetAverageVal(tr, func);
    EXPECT_NEAR(result, quadrantCorrectVals[3], ALLOWED_ERROR);
}

static double customGetAverageVal_insideCheck(std::vector<ring_t> polyList, double (*func)(double x, double y)) {

    double result = 0;
    long pointCount = 0;
    for (auto &poly : polyList) {
        box_t boundingBox = bg::return_envelope<box_t>(poly);
        double min_x = boundingBox.min_corner().get<0>();
        double min_y = boundingBox.min_corner().get<1>();
        double max_x = boundingBox.max_corner().get<0>();
        double max_y = boundingBox.max_corner().get<1>();

        int totalSteps = 1000;
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
    }
    return result / pointCount;
}

void splitIntoQuadsAndTest_bruteForce_insideCheck(ring_t inPoly, std::vector<double> quadrantCorrectVals, double (*func)(double x, double y), double rotation=0) {

    assert(quadrantCorrectVals.size() == 4);

    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(rotation);
    bg::transform(inPoly, transformedPoly, rotate);

#ifdef CHECK_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(inPoly));
    ASSERT_TRUE(bg::is_valid(transformedPoly));
#endif

    box_t boundingBox = bg::return_envelope<box_t>(transformedPoly);
    std::vector<ring_t> bl = getTopRightQuadrant(transformedPoly, boundingBox);
    std::vector<ring_t> br = getTopLeftQuadrant(transformedPoly, boundingBox);
    std::vector<ring_t> tl = getBottomRightQuadrant(transformedPoly, boundingBox);
    std::vector<ring_t> tr = getBottomLeftQuadrant(transformedPoly, boundingBox);

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

double getXYAvgVal(std::vector<ring_t> polylist) {
    return customGetAverageVal(polylist, xyFromX1ToX2Wrapper);
}

double getXYAvgVal_bruteForce(std::vector<ring_t> polylist) {
    return customGetAverageVal(polylist, xyFromX1ToX2Wrapper_bruteForce);
}

double getXYAvgVal_bruteForce_insideCheck(std::vector<ring_t> polylist) {
    return customGetAverageVal_insideCheck(polylist, xMulty);
}

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
    std::vector<ring_t> trTop = getTopRightQuadrant(top, boundingBoxTop);

    box_t boundingBoxBottom = bg::return_envelope<box_t>(top);
    std::vector<ring_t> trBottom = getTopRightQuadrant(bottom, boundingBoxBottom);

    double result;
    result = func(trTop);
    EXPECT_NEAR(result, 0.375, ALLOWED_ERROR);

    result = func(trBottom);
    EXPECT_NEAR(result, 0.125, ALLOWED_ERROR);

    box_t boundingBoxFull = bg::return_envelope<box_t>(full);
    std::vector<ring_t> trFull = getTopRightQuadrant(full, boundingBoxBottom);

    result = func(trFull);
    EXPECT_NEAR(result, 0.25, ALLOWED_ERROR);
}

TEST(EquationTest, testXY_combiningAreas) {
    combiningTestCommon(getXYAvgVal);
}

TEST(EquationTest, testXY_combiningAreas_bruteForce) {
    combiningTestCommon(getXYAvgVal_bruteForce);
}

TEST(EquationTest, testXY_combiningAreas_bruteForce_insideCheck) {
    combiningTestCommon(getXYAvgVal_bruteForce_insideCheck);
}

TEST(EquationTest, testXY_complexShape) {
    ring_t red{
            {-1.0, 0.0}, {-1.0, 4}, {-0.5, 0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 1.0}, {0.5, 1.0}, {0.5, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 0;
    std::vector<double> correctVals{0.54166666666666663, -0.58333333333333326, -0.083333333333333343, 0.16666666666666669};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper, rotation);
}

#if BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_complexShape_bruteForce) {
    ring_t red{
            {-1.0, 0.0}, {-1.0, 4}, {-0.5, 0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 1.0}, {0.5, 1.0}, {0.5, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 0;
    std::vector<double> correctVals{0.54166666666666663, -0.58333333333333326, -0.083333333333333343, 0.16666666666666669};
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, rotation);
}

TEST(EquationTest, testXY_complexShape_bruteForce_insideCheck) {
    ring_t red{
            {-1.0, 0.0}, {-1.0, 4}, {-0.5, 0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 1.0}, {0.5, 1.0}, {0.5, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

    double rotation = 0;
    std::vector<double> correctVals{0.54166666666666663, -0.58333333333333326, -0.083333333333333343, 0.16666666666666669};
    splitIntoQuadsAndTest_bruteForce_insideCheck(red, correctVals, xMulty, rotation);
}

#endif
/*

TEST(EquationTest, testYSquared_1x1square_rotated10_bruteForce) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };

    double rotation = 90;
    std::vector<double> correctVals(4, 0);
    splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, 10);
}

TEST(EquationTest, testab) {
    ring_t red{
        {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    auto[a, b] = getAandB(red);
    EXPECT_NEAR(1.0, a, ALLOWED_ERROR);
    EXPECT_NEAR(0, b, ALLOWED_ERROR);
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
    EXPECT_NEAR(1, a, ALLOWED_ERROR);
    EXPECT_NEAR(0, b, ALLOWED_ERROR);
}

TEST(EquationTest, testab3) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(10);
    bg::transform(red, transformedPoly, rotate);

    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1, a, ALLOWED_ERROR);
    EXPECT_NEAR(0, b, ALLOWED_ERROR);
}

TEST(EquationTest, testab4) {
    ring_t red{
            {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
    };
    ring_t transformedPoly;
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(45);
    bg::transform(red, transformedPoly, rotate);

    auto[a, b] = getAandB(transformedPoly);
    EXPECT_NEAR(1, a, ALLOWED_ERROR);
    EXPECT_NEAR(0, b, ALLOWED_ERROR);
}
*/
