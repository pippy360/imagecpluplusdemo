#include <iostream>
#include <list>
#include <deque>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/foreach.hpp>

#include <gtest/gtest.h>

#define CHECK_SHAPES_VALID 1
#define ALLOWED_ERROR 0.001
#define MIN_SLOPE_VAL 0.0000001


namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::ring<point_t> ring_t;
typedef bg::model::box<point_t> box_t;


double getQuadrantArea(ring_t wholeShape, box_t intersectBox) {
	std::vector<ring_t> output;
	bg::intersection(intersectBox, wholeShape, output);

#ifdef CHECK_SHAPES_VALID
	assert(bg::is_valid(wholeShape));
	assert(bg::is_valid(intersectBox));
	assert(bg::is_valid(output[0]));
	assert(output.size() == 1);
#endif
	return bg::area(output[0]);
}

ring_t getQuadrant(ring_t wholeShape, box_t intersectBox) {
	std::vector<ring_t> output;
	bg::intersection(intersectBox, wholeShape, output);

#ifdef CHECK_SHAPES_VALID
	assert(bg::is_valid(wholeShape));
	assert(bg::is_valid(intersectBox));
	assert(bg::is_valid(output[0]));
	assert(output.size() == 1);
#endif
	return output[0];
}

double getAreaUnderTwoPoints(point_t p1, point_t p2) {

	//catch some zero area polys
	if (p1.get<0>() == p2.get<0>() || (p1.get<1>() == 0 && p2.get<1>() == 0))
		return 0;

	//make sure the polygon we are about to create is going to be clockwise
	//otherwise boost will fail
	ring_t ring{
		{std::min(p1.get<0>(), p2.get<0>()), std::min(p1.get<1>(), 0.0)}, 
		{std::min(p1.get<0>(), p2.get<0>()), std::max(p1.get<1>(), 0.0)},
		{std::max(p1.get<0>(), p2.get<0>()), std::max(p2.get<1>(), 0.0)}, 
		{std::max(p1.get<0>(), p2.get<0>()), std::min(p2.get<1>(), 0.0)},
		{std::min(p1.get<0>(), p2.get<0>()), std::min(p1.get<1>(), 0.0)}, 
	};

#ifdef CHECK_SHAPES_VALID
	assert(bg::is_valid(ring));
#endif
	return bg::area(ring);
}

double calcYSquaredAverage(point_t p1, point_t p2) {
	return 0;
}

double calcYAverage(point_t p1, point_t p2) {
	double result = (p1.get<1>() + p2.get<1>())/2.0;
	signed int sign = (p1.get<0>() < p2.get<0>())? 1 : -1;
	return (result * sign)/getAreaUnderTwoPoints(p1, p2);
}

double calcXAverage(point_t p1, point_t p2) {
	double result = (p1.get<1>() + p2.get<1>())/2.0;
	signed int sign = (p1.get<0>() < p2.get<0>())? 1 : -1;
	return result * sign;
}

double getSlopeOfLine(point_t p1, point_t p2) {
	double x1 = p1.get<0>();
	double x2 = p2.get<0>();
	double y1 = p1.get<1>();
	double y2 = p2.get<1>();
	return (y2-y1)/(x2-x1);
}

double getConstantOfLine(point_t p1, point_t p2) {
	double x1 = p1.get<0>();
	double x2 = p2.get<0>();
	double y1 = p1.get<1>();
	double y2 = p2.get<1>();
	return y2 - (getSlopeOfLine(p1, p2) * x2);
}

//[int_from 0 to (mx+c) (mx+c)*x]/(mx+c)
// = (mx^3/3.0 + cx^2/2.0)/(mx+c)
//
// [int_from x1 to x2 ((c x^2)/2 + (m x^3)/3)/(mx+c) dx ] / (x2-x1)
//
// (m x (-6 c^2 + 3 c m x + 4 m^2 x^2) + 6 c^3 log(c + m x))/(36 m^3)
//
double xyFromZeroToX(double m, double c, double x) {
	if (abs(m) < MIN_SLOPE_VAL)
		return pow(x, 3)/6.0;

	return (
		(m*x*(-6.0*pow(c, 2) + 3.0*c*m*x + 4.0*pow(m, 2)*pow(x, 2)) + 6.0*pow(c, 3)*log(c+m*x))/(36.0*pow(m, 3))
		);
}

double xyFromX1ToX2(double m, double c, double x1, double x2) {
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

//[int_(0 to mx+c) (mx+c)^2 d(mx+c)]/(mx+c)
// = ((mx+c)^3/3.0 - zero)/(mx+c)
// 
// now int_(0 to x) (mx+c)^2/3.0
//
double ySquaredFromZeroToX(double m, double c, double x) {
    if (abs(m) < MIN_SLOPE_VAL)
            return pow(c, 2)*x/3.0;

	return pow(c + m*x, 3)/(9.0*m);
}

//f(x) = y = mx + c
double ySquaredFromX1ToX2(double m, double c, double x1, double x2) {
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

double ySquaredFromX1ToX2Wrapper_bruteForce(point_t p1, point_t p2) {
    double x1 = p1.get<0>();
    double x2 = p2.get<0>();
    double m = getSlopeOfLine(p1, p2);
    double c = getConstantOfLine(p1, p2);
    int i = 0;
    double result = 0;
    int totalSteps = 1000;
    double step = (x2 - x1)/((double) totalSteps-1);
    for (; i < totalSteps; i++) {
        for(int j = 0; j < totalSteps; j++) {
            double xVal = x1 + (step*i);
            double yVal;
            if (abs(m) < MIN_SLOPE_VAL)
                yVal = (c) * ((double)j/(double)totalSteps);
            else
                yVal = (xVal*m + c) * ((double)j/(double)totalSteps);

            result += pow(yVal, 2);
        }
    }
    return result/pow(totalSteps, 2);
}

double xyFromX1ToX2Wrapper_bruteForce(point_t p1, point_t p2) {
    double x1 = p1.get<0>();
    double x2 = p2.get<0>();
    double m = getSlopeOfLine(p1, p2);
    double c = getConstantOfLine(p1, p2);
    int i = 0;
    double result = 0;
    int totalSteps = 1000;
    double step = (x2 - x1)/((double) totalSteps-1);
    for (; i < totalSteps; i++) {
        double line = 0;
        for(int j = 0; j < totalSteps; j++) {
            double xVal = x1 + (step*i);
            double yVal;
            if (abs(m) < MIN_SLOPE_VAL)
                yVal = (c) * ((double)j/((double)totalSteps - 1));
            else
                yVal = (xVal*m + c) * ((double)j/((double)totalSteps - 1));

            line += xVal*yVal;
        }
        double lineAvg = line/totalSteps;
        result += lineAvg;
    }
    return result/totalSteps;
}

//used to get the average x/y/x^2/y^2/x*y value of every point in a polygon segment
double customGetAverageVal(ring_t poly, double (*func)(point_t p1, point_t p2)) {

	double result = 0;
	double totalArea = 0;
	for (int i = 0; i < poly.size(); i++) {
		point_t p = poly[i];
		point_t np = (i == poly.size()-1)? poly[0] : poly[i+1];
		double x1 = p.get<0>();
		double x2 = np.get<0>();
		double area = getAreaUnderTwoPoints(p, np);

		if (abs(area) < 0.000001)
			continue;

		//check direction
		if ( x1 < x2 ) {
			result += func(p, np)*area;
			totalArea += area;
		} else {
			result -= func(p, np)*area;
			totalArea -= area;
		}
	}
	return result/totalArea;
}

double applyFuncForEachQuadrant(ring_t poly, double rotation, double (*func)(point_t p1, point_t p2)) {
	bg::model::box<point_t> boundingBox;
	bg::envelope(poly, boundingBox);

	double min_x = boundingBox.min_corner().get<0>();
	double min_y = boundingBox.min_corner().get<1>();
	double max_x = boundingBox.max_corner().get<0>();
	double max_y = boundingBox.max_corner().get<1>();

	bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
	bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
	bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
	bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };
	ring_t rotatedPoly;

	if (rotation != 0) {
		bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(rotation);
		bg::transform(rotatedPoly, poly, rotate);
	} else {
		rotatedPoly = poly;
	}

	ring_t bl = getQuadrant(poly, bl_box);
	ring_t br = getQuadrant(poly, br_box);
	ring_t tl = getQuadrant(poly, tl_box);
	ring_t tr = getQuadrant(poly, tr_box);

	double bl_area = bg::area(bl); 
	double br_area = bg::area(br); 
	double tl_area = bg::area(tl); 
	double tr_area = bg::area(tr); 

	return (
			 (customGetAverageVal(bl, func)*bl_area)
			*(customGetAverageVal(br, func)*br_area)
			*(customGetAverageVal(tl, func)*tl_area)
			*(customGetAverageVal(tr, func)*tr_area)
		)/bg::area(poly);
}

double getAandB(ring_t poly) {

	double poly_ys = applyFuncForEachQuadrant(poly, 0, ySquaredFromX1ToX2Wrapper);
	double poly_xs = applyFuncForEachQuadrant(poly, 90, ySquaredFromX1ToX2Wrapper);
	double poly_xy = applyFuncForEachQuadrant(poly, 0, xyFromX1ToX2Wrapper);

	//std::cout << poly_ys << " : " << poly_xs << " : " << poly_xy << " : " << std::endl;
	return poly_xy + poly_xs + poly_ys;
}



TEST(testbasic, testbasic) {
	ring_t red{
		{-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}
	};
	ring_t poly = red;
	bg::model::box<point_t> boundingBox;
	bg::envelope(poly, boundingBox);

	double min_x = boundingBox.min_corner().get<0>();
	double min_y = boundingBox.min_corner().get<1>();
	double max_x = boundingBox.max_corner().get<0>();
	double max_y = boundingBox.max_corner().get<1>();
	
	bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
	bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
	bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
	bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };

	ring_t bl = getQuadrant(poly, bl_box);
	ring_t br = getQuadrant(poly, br_box);
	ring_t tl = getQuadrant(poly, tl_box);
	ring_t tr = getQuadrant(poly, tr_box);

	double blVal = customGetAverageVal(bl, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
}

TEST(testbasic, testbasic2) {
	ring_t red{
		{-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
	};
	ring_t poly = red;
	bg::model::box<point_t> boundingBox;
	bg::envelope(poly, boundingBox);

	double min_x = boundingBox.min_corner().get<0>();
	double min_y = boundingBox.min_corner().get<1>();
	double max_x = boundingBox.max_corner().get<0>();
	double max_y = boundingBox.max_corner().get<1>();
	
	bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
	bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
	bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
	bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };

	ring_t bl = getQuadrant(poly, bl_box);
	ring_t br = getQuadrant(poly, br_box);
	ring_t tl = getQuadrant(poly, tl_box);
	ring_t tr = getQuadrant(poly, tr_box);

	double blVal;
	blVal = customGetAverageVal(bl, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 1.33333333333333331);
	blVal = customGetAverageVal(br, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 1.33333333333333331);
	blVal = customGetAverageVal(tl, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 1.33333333333333331);
	blVal = customGetAverageVal(tr, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 1.33333333333333331);
}

TEST(testbasic, testbasicZeroRotation) {
	ring_t red{
		{-2.0, -1.0}, {-2.0, 1.0}, {2.0, 1.0}, {2.0, -1.0}, {-2.0, -1.0}
	};
	ring_t poly = red;

#ifdef CHECK_SHAPES_VALID
	ASSERT_TRUE(bg::is_valid(red));
#endif
	bg::model::box<point_t> boundingBox;
	bg::envelope(poly, boundingBox);

	double min_x = boundingBox.min_corner().get<0>();
	double min_y = boundingBox.min_corner().get<1>();
	double max_x = boundingBox.max_corner().get<0>();
	double max_y = boundingBox.max_corner().get<1>();

	bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
	bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
	bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
	bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };

	ring_t bl = getQuadrant(poly, bl_box);
	ring_t br = getQuadrant(poly, br_box);
	ring_t tl = getQuadrant(poly, tl_box);
	ring_t tr = getQuadrant(poly, tr_box);

	double blVal;
	blVal = customGetAverageVal(bl, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
	blVal = customGetAverageVal(br, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
	blVal = customGetAverageVal(tl, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
	blVal = customGetAverageVal(tr, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
}


TEST(testbasic, testbasicRotation) {
	ring_t red{
		{-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
	};
	ring_t poly;

#ifdef CHECK_SHAPES_VALID
	ASSERT_TRUE(bg::is_valid(red));
#endif

	bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(90);
	bg::transform(red, poly, rotate);

#ifdef CHECK_SHAPES_VALID
	ASSERT_TRUE(bg::is_valid(poly));
	ASSERT_TRUE(bg::is_valid(red));
#endif

	bg::model::box<point_t> boundingBox;
	bg::envelope(poly, boundingBox);

	double min_x = boundingBox.min_corner().get<0>();
	double min_y = boundingBox.min_corner().get<1>();
	double max_x = boundingBox.max_corner().get<0>();
	double max_y = boundingBox.max_corner().get<1>();
	
	bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
	bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
	bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
	bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };

	ring_t bl = getQuadrant(poly, bl_box);
	ring_t br = getQuadrant(poly, br_box);
	ring_t tl = getQuadrant(poly, tl_box);
	ring_t tr = getQuadrant(poly, tr_box);

	double blVal;
	blVal = customGetAverageVal(bl, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
	blVal = customGetAverageVal(br, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
	blVal = customGetAverageVal(tl, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
	blVal = customGetAverageVal(tr, ySquaredFromX1ToX2Wrapper);
	EXPECT_DOUBLE_EQ(blVal, 0.33333333333333331);
}

TEST(testbasic, testbasicXY) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };
    ring_t poly;

#ifdef CHECK_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(red));
#endif

    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(90);
    bg::transform(red, poly, rotate);

#ifdef CHECK_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(poly));
    ASSERT_TRUE(bg::is_valid(red));
#endif

    bg::model::box<point_t> boundingBox;
    bg::envelope(poly, boundingBox);

    double min_x = boundingBox.min_corner().get<0>();
    double min_y = boundingBox.min_corner().get<1>();
    double max_x = boundingBox.max_corner().get<0>();
    double max_y = boundingBox.max_corner().get<1>();

    bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
    bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
    bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
    bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };

    ring_t bl = getQuadrant(poly, bl_box);
    ring_t br = getQuadrant(poly, br_box);
    ring_t tl = getQuadrant(poly, tl_box);
    ring_t tr = getQuadrant(poly, tr_box);

    double blVal;
    blVal = customGetAverageVal(bl, xyFromX1ToX2Wrapper);
    EXPECT_LT(abs(blVal - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(br, xyFromX1ToX2Wrapper);
    EXPECT_LT(abs(blVal - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(tl, xyFromX1ToX2Wrapper);
    EXPECT_LT(abs(blVal - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(tr, xyFromX1ToX2Wrapper);
    EXPECT_LT(abs(blVal - 0.33333333333333331), ALLOWED_ERROR);
}

TEST(testbasic, testbasicBruteForceYSquared) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };
    ring_t poly;

#ifdef CHECK_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(red));
#endif

    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(90);
    bg::transform(red, poly, rotate);

#ifdef CHECK_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(poly));
    ASSERT_TRUE(bg::is_valid(red));
#endif

    bg::model::box<point_t> boundingBox;
    bg::envelope(poly, boundingBox);

    double min_x = boundingBox.min_corner().get<0>();
    double min_y = boundingBox.min_corner().get<1>();
    double max_x = boundingBox.max_corner().get<0>();
    double max_y = boundingBox.max_corner().get<1>();

    bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
    bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
    bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
    bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };

    ring_t bl = getQuadrant(poly, bl_box);
    ring_t br = getQuadrant(poly, br_box);
    ring_t tl = getQuadrant(poly, tl_box);
    ring_t tr = getQuadrant(poly, tr_box);

    double blVal;
    blVal = customGetAverageVal(bl, ySquaredFromX1ToX2Wrapper_bruteForce);
    EXPECT_LT(abs(blVal - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(br, ySquaredFromX1ToX2Wrapper_bruteForce);
    EXPECT_LT(abs(blVal - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(tl, ySquaredFromX1ToX2Wrapper_bruteForce);
    EXPECT_LT(abs(blVal - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(tr, ySquaredFromX1ToX2Wrapper_bruteForce);
    EXPECT_LT(abs(blVal - 0.33333333333333331), ALLOWED_ERROR);
}

TEST(testbasic, testbasicBruteForceXY) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };
    ring_t poly;

#ifdef CHECK_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(red));
#endif

    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(90);
    bg::transform(red, poly, rotate);

#ifdef CHECK_SHAPES_VALID
    ASSERT_TRUE(bg::is_valid(poly));
    ASSERT_TRUE(bg::is_valid(red));
#endif

    bg::model::box<point_t> boundingBox;
    bg::envelope(poly, boundingBox);

    double min_x = boundingBox.min_corner().get<0>();
    double min_y = boundingBox.min_corner().get<1>();
    double max_x = boundingBox.max_corner().get<0>();
    double max_y = boundingBox.max_corner().get<1>();

    bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
    bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
    bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
    bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };

    ring_t bl = getQuadrant(poly, bl_box);
    ring_t br = getQuadrant(poly, br_box);
    ring_t tl = getQuadrant(poly, tl_box);
    ring_t tr = getQuadrant(poly, tr_box);

    double blVal;
    //FIXME: why do we only need abs here? shouldn't the sign of the other one also be negative?
    blVal = customGetAverageVal(bl, xyFromX1ToX2Wrapper_bruteForce);
    EXPECT_LT(abs(abs(blVal) - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(br, xyFromX1ToX2Wrapper_bruteForce);
    EXPECT_LT(abs(abs(blVal) - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(tl, xyFromX1ToX2Wrapper_bruteForce);
    EXPECT_LT(abs(abs(blVal) - 0.33333333333333331), ALLOWED_ERROR);
    blVal = customGetAverageVal(tr, xyFromX1ToX2Wrapper_bruteForce);
    EXPECT_LT(abs(abs(blVal) - 0.33333333333333331), ALLOWED_ERROR);
}

int main2()
{
	ring_t green{
		{-2.0, -2.0}, {-2.0, 5.0}, {5.0, 5.0}, {5.0, -2.0}, {-2.0, -2.0}
	};

	ring_t blue{
		{1.0, 1.0}, {1.0, 5.0}, {5.0, 5.0}, {5.0, 1.0}, {1.0, 1.0}
	};
	
	ring_t red{
		{0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}, {0.0, 0.0}
	};
	
	ring_t red2{
		{0.0, 0.0}, {0.0, 1.0}, {2.0, 1.0}, {2.0, 0.0}, {0.0, 0.0}
	};
	

	std::cout << customGetAverageVal(red, ySquaredFromX1ToX2Wrapper) << std::endl;
	std::cout << customGetAverageVal(red2, ySquaredFromX1ToX2Wrapper) << std::endl;
	std::cout << customGetAverageVal(red, xyFromX1ToX2Wrapper) << std::endl;
	std::cout << customGetAverageVal(red2, xyFromX1ToX2Wrapper) << std::endl;
	getAandB(green);

	return 0;
}
