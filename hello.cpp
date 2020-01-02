#include <iostream>
#include <list>
#include <deque>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/foreach.hpp>

#define CHECK_SHAPES_VALID 1


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

//+x, +y
double getTopRightQuadrantArea(ring_t wholeShape, box_t wholeShapeEnvelope) {
	double max_x = wholeShapeEnvelope.max_corner().get<0>();
	double max_y = wholeShapeEnvelope.max_corner().get<1>();
	bg::model::box<point_t> ibox{ {0, 0}, {max_x,max_y} };
	return getQuadrantArea(wholeShape, ibox);
}

//-x, +y
double getTopLeftQuadrantArea(ring_t wholeShape, box_t wholeShapeEnvelope) {
	double min_x = wholeShapeEnvelope.min_corner().get<0>();
	double max_y = wholeShapeEnvelope.max_corner().get<1>();
	bg::model::box<point_t> ibox{ {min_x, 0}, {0, max_y} };
	return getQuadrantArea(wholeShape, ibox);
}

//+x, -y
double getBottomRightQuadrantArea(ring_t wholeShape, box_t wholeShapeEnvelope) {
	double max_x = wholeShapeEnvelope.max_corner().get<0>();
	double min_y = wholeShapeEnvelope.min_corner().get<1>();
	bg::model::box<point_t> ibox{ {0, min_y}, {max_x, 0} };
	return getQuadrantArea(wholeShape, ibox);
}

//-x, -y
double getBottomLeftQuadrantArea(ring_t wholeShape, box_t wholeShapeEnvelope) {
	double min_x = wholeShapeEnvelope.min_corner().get<0>();
	double min_y = wholeShapeEnvelope.min_corner().get<1>();
	bg::model::box<point_t> ibox{ {min_x, min_y}, {0, 0} };
	return getQuadrantArea(wholeShape, ibox);
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
	std::cout << "p1: " << p1.get<0>() << ", " << p1.get<1>() << " p2: " << p2.get<0>() << ", " << p2.get<1>() << std::endl;
	ring_t ring{
		{std::min(p1.get<0>(), p2.get<0>()), std::min(p1.get<1>(), 0.0)}, 
		{std::min(p1.get<0>(), p2.get<0>()), std::max(p1.get<1>(), 0.0)},
		{std::max(p1.get<0>(), p2.get<0>()), std::max(p2.get<1>(), 0.0)}, 
		{std::max(p1.get<0>(), p2.get<0>()), std::min(p2.get<1>(), 0.0)},
		{std::min(p1.get<0>(), p2.get<0>()), std::min(p1.get<1>(), 0.0)}, 
	};

	std::cout << "here: " << boost::geometry::dsv(ring) << std::endl;
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
	if (m == 0)
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
	if (m == 0)
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
			printf("func val %lf\n", func(p, np));
			printf("area %lf\n", area);
			result += func(p, np)*area;
			totalArea += area;
		} else {
			printf("func - %lf\n", func(p, np));
			printf("area %lf\n", area);
			result -= func(p, np)*area;
			totalArea -= area;
		}
	}
	return result/totalArea;
}

double getAandB(ring_t poly) {

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

	double bl_area = bg::area(bl); 
	double br_area = bg::area(br); 
	double tl_area = bg::area(tl); 
	double tr_area = bg::area(tr); 

	double poly_ys = (
			(customGetAverageVal(bl, ySquaredFromX1ToX2Wrapper)*bl_area)
			*(customGetAverageVal(br, ySquaredFromX1ToX2Wrapper)*br_area)
			*(customGetAverageVal(tl, ySquaredFromX1ToX2Wrapper)*tl_area)
			*(customGetAverageVal(tr, ySquaredFromX1ToX2Wrapper)*tr_area)
		)/bg::area(poly);

	ring_t rotatedPoly;
	boost::geometry::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(90.0);
	bg::transform(rotatedPoly, poly, rotate);
	//TODO: ....actually use the rotated value
	double poly_xs = (
			(customGetAverageVal(bl, ySquaredFromX1ToX2Wrapper)*bl_area)
			*(customGetAverageVal(br, ySquaredFromX1ToX2Wrapper)*br_area)
			*(customGetAverageVal(tl, ySquaredFromX1ToX2Wrapper)*tl_area)
			*(customGetAverageVal(tr, ySquaredFromX1ToX2Wrapper)*tr_area)
		)/bg::area(poly);

	double poly_xy = (
			(customGetAverageVal(bl, xyFromX1ToX2Wrapper)*bl_area)
			*(customGetAverageVal(br, xyFromX1ToX2Wrapper)*br_area)
			*(customGetAverageVal(tl, xyFromX1ToX2Wrapper)*tl_area)
			*(customGetAverageVal(tr, xyFromX1ToX2Wrapper)*tr_area)
		)/bg::area(poly);

	return poly_xy + poly_xs + poly_ys;
}

int main()
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
	
	int i = 0;
	//std::cout << loopPolyAndApplyFunc(green, calcYAverage)/bg::area(green) << std::endl;

	std::list<ring_t> output;
	bg::model::box<point_t> box1;
	bg::intersection(blue, green, output);

	bg::envelope(green, box1);
	double max_x = box1.max_corner().get<0>();
	double max_y = box1.max_corner().get<1>();
	double min_x = box1.min_corner().get<0>();
	double min_y = box1.min_corner().get<1>();

	bg::model::box<point_t> tr{ {0, 0}, {max_x,max_y} };
	bg::model::box<point_t> tl{ {min_x, 0}, {0,max_y} };
	bg::model::box<point_t> br{ {0, min_y}, {max_x,0} };
	bg::model::box<point_t> bl{ {0, 0}, {min_x,min_y} };
	
	output.clear();
	if(!bg::is_valid(green))
		std::cout << "invalid";

	if(!bg::is_valid(br))
		std::cout << "invalid";

	bg::intersection(tr, green, output);

	std::cout << "green && blue:" << std::endl;
	BOOST_FOREACH(auto& p, output)
	{
		std::cout << i++ << ": " << boost::geometry::dsv(p) << std::endl;
		std::cout << i++ << ": " << boost::geometry::area(p) << std::endl;
	}
	output.clear();
	bg::intersection(tl, green, output);

	std::cout << "green && blue:" << std::endl;
	BOOST_FOREACH(auto& p, output)
	{
		std::cout << i++ << ": " << boost::geometry::dsv(tl) << std::endl;
		std::cout << i++ << ": " << boost::geometry::dsv(p) << std::endl;
		std::cout << i++ << ": " << boost::geometry::area(p) << std::endl;
	}
	output.clear();
	bg::intersection(br, green, output);

	std::cout << "green && blue:" << std::endl;
	BOOST_FOREACH(auto& p, output)
	{
		std::cout << i++ << ": " << boost::geometry::dsv(br) << std::endl;
		std::cout << i++ << ": " << boost::geometry::dsv(p) << std::endl;
		std::cout << i++ << ": " << boost::geometry::area(p) << std::endl;
	}
	output.clear();
	bg::intersection(bl, green, output);

	std::cout << "green && blue:" << std::endl;
	BOOST_FOREACH(auto& p, output)
	{
		std::cout << i++ << ": " << boost::geometry::dsv(p) << std::endl;
		std::cout << i++ << ": " << boost::geometry::area(p) << std::endl;
	}

	std::cout << customGetAverageVal(red, ySquaredFromX1ToX2Wrapper) << std::endl;
	std::cout << customGetAverageVal(red2, ySquaredFromX1ToX2Wrapper) << std::endl;
	std::cout << customGetAverageVal(red, xyFromX1ToX2Wrapper) << std::endl;
	std::cout << customGetAverageVal(red2, xyFromX1ToX2Wrapper) << std::endl;
	return 0;
}
