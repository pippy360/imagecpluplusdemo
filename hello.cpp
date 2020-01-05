#include <iostream>
#include <fstream>
#include <list>
#include <deque>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/io/svg/write_svg.hpp>

#include <boost/foreach.hpp>

#include <gtest/gtest.h>

#define CHECK_SHAPES_VALID 1
#define ALLOWED_ERROR 0.001
#define MIN_SLOPE_VAL 0.0000001
#define SKIP_BRUTE_FORCE_TESTING 0


namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::ring<point_t> ring_t;
typedef bg::model::box<point_t> box_t;


template <typename Geometry1>
void create_svg(std::string const& filename, Geometry1 const& a)
{

    ring_t b;
    bg::read_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", b);
     box_t box;
     bg::envelope(a, box);
    typedef typename boost::geometry::point_type<Geometry1>::type point_type;
    std::ofstream svg(filename.c_str());

    boost::geometry::svg_mapper<point_type> mapper(svg, 400, 400);
    mapper.add(a);
    mapper.add(box);
    mapper.add(b);

    mapper.map(a, "fill-opacity:0.5;fill:rgb(153,204,0);stroke:rgb(153,204,0);stroke-width:2");
    mapper.map(b, "fill-opacity:0.5;fill:rgb(153,204,0);stroke:rgb(153,204,0);stroke-width:2");
        mapper.map(box, "opacity:0.8;fill:none;stroke:rgb(255,128,0);stroke-width:4;stroke-dasharray:1,7;stroke-linecap:round");

}

std::vector<ring_t> getQuadrant(ring_t wholeShape, box_t intersectBox) {
	std::vector<ring_t> output;
	bg::intersection(intersectBox, wholeShape, output);

#ifdef CHECK_SHAPES_VALID
	assert(bg::is_valid(wholeShape));
	assert(bg::is_valid(intersectBox));
	for (auto& v : output)
		assert(bg::is_valid(v));

	assert(output.size() > 0);
#endif
	return output;
}

std::vector<ring_t> getTopRightQuadrant(ring_t wholeShape, box_t shapeBoundingBox) {
	double max_x = shapeBoundingBox.max_corner().get<0>();
	double max_y = shapeBoundingBox.max_corner().get<1>();

	bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };
	return getQuadrant(wholeShape, tr_box);
}

std::vector<ring_t> getTopLeftQuadrant(ring_t wholeShape, box_t shapeBoundingBox) {
	double min_x = shapeBoundingBox.min_corner().get<0>();
	double max_y = shapeBoundingBox.max_corner().get<1>();

	bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
	return getQuadrant(wholeShape, tl_box);
}

std::vector<ring_t> getBottomRightQuadrant(ring_t wholeShape, box_t shapeBoundingBox) {
	double min_y = shapeBoundingBox.min_corner().get<1>();
	double max_x = shapeBoundingBox.max_corner().get<0>();

    bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
	return getQuadrant(wholeShape, br_box);
}

std::vector<ring_t> getBottomLeftQuadrant(ring_t wholeShape, box_t shapeBoundingBox) {
	double min_y = shapeBoundingBox.min_corner().get<1>();
	double min_x = shapeBoundingBox.min_corner().get<0>();

    bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
	return getQuadrant(wholeShape, bl_box);
}


double getAreaUnderTwoPoints(point_t p1, point_t p2) {

	//catch some zero area polys
	if (abs(p1.get<0>() - p2.get<0>()) < MIN_SLOPE_VAL
			|| (abs(p1.get<1>() - 0) < MIN_SLOPE_VAL
				&& abs(p2.get<1>() - 0) < MIN_SLOPE_VAL))
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

//[int_from p = 0 to (mx+c), x=k1 ... (p)*k1]/(mx+c)
// = (p)^2*k1/2(mx+c) = (mx+c)*x/2.0
//
// [int_from x1 to x2 (mx+c)*x/2.0 dx ] / (x2-x1)
//
// 
//
double xyFromZeroToX(double m, double c, double x) {
	if (abs(m) < MIN_SLOPE_VAL)
		return c*pow(x, 2)/4.0;

	return ((c*pow(x, 2)/4.0) + 1.0/6.0*(m*pow(x,3)));
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

//[int_(0 to mx+c)=p (p)^2 d(mx+c)]/(mx+c)
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

//used to get the average x/y/x^2/y^2/x*y value of every point in a polygon segment
double customGetAverageVal(std::vector<ring_t> polyList, double (*func)(point_t p1, point_t p2)) {

	double result = 0;
	double totalArea = 0;

	for (auto& poly : polyList) {
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
	}

	return result/totalArea;
}

double calcVectorPolyArea(std::vector<ring_t> vec) {
	double result = 0;
	for (auto& v : vec)
		result += bg::area(v);
	return result;
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

#if SKIP_BRUTE_FORCE_TESTING
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

#if SKIP_BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_1x2square_rotated90_bruteForce) {
    ring_t red{
            {-1.0, -2.0}, {-1.0, 2.0}, {1.0, 2.0}, {1.0, -2.0}, {-1.0, -2.0}
    };

	double rotation = 90;
	std::vector<double> correctVals{0.5, -0.5, -0.5, 0.5};
	splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, rotation);
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

#if SKIP_BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_diamond_rotated90_bruteForce) {
    ring_t red{
            {-1.0, 0.0}, {0.0, 2.0}, {1.0, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

	double rotation = 90;
	std::vector<double> correctVals{0.166667, -0.166667, -0.166667, 0.166667};
	splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, rotation);
}
#endif

TEST(EquationTest, testXY_complexShape) {
    ring_t red{
            {-1.0, 0.0}, {-1.0, 4}, {-0.5, 0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 1.0}, {0.5, 1.0}, {0.5, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

	double rotation = 0;
	std::vector<double> correctVals{0.54166666666666663, -0.58333333333333326, -0.083333333333333343, 0.16666666666666669};
	splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper, rotation);
}

#if SKIP_BRUTE_FORCE_TESTING
TEST(EquationTest, testXY_complexShape_bruteForce) {
    ring_t red{
            {-1.0, 0.0}, {-1.0, 4}, {-0.5, 0}, {0.0, 2.0}, {1.0, 2.0}, {1.0, 1.0}, {0.5, 1.0}, {0.5, 0.0}, {0.0, -2.0}, {-1.0, 0.0}
    };

	double rotation = 0;
	std::vector<double> correctVals{0.54166666666666663, -0.58333333333333326, -0.083333333333333343, 0.16666666666666669};
	splitIntoQuadsAndTest(red, correctVals, xyFromX1ToX2Wrapper_bruteForce, rotation);
}
#endif