#ifndef MISCUTILS_H
#define MISCUTILS_H

#include "boostGeometryTypes.hpp"

#define ASSERT_SHAPES_VALID 0
#define MIN_SLOPE_VAL 0.0000001


static double getSlopeOfLine(point_t p1, point_t p2) {
    double x1 = p1.get<0>();
    double x2 = p2.get<0>();
    double y1 = p1.get<1>();
    double y2 = p2.get<1>();

    /* Avoid using tiny slopes because it leads to huge errors when we divide */
    if (abs((y2-y1)/(x2-x1)) < MIN_SLOPE_VAL)
        return 0;

    return (y2-y1)/(x2-x1);
}

static double getConstantOfLine(point_t p1, point_t p2) {
    double x2 = p2.get<0>();
    double y2 = p2.get<1>();
    return y2 - (getSlopeOfLine(p1, p2) * x2);
}


static double getAreaUnderTwoPoints(point_t p1, point_t p2) {

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

#if ASSERT_SHAPES_VALID
    assert(bg::is_valid(ring));
#endif
    return bg::area(ring);
}

static bool getQuadrant(ring_t wholeShape, box_t intersectBox, std::vector<ring_t> &output) {
    bg::intersection(intersectBox, wholeShape, output);

#if ASSERT_SHAPES_VALID
    assert(bg::is_valid(wholeShape));
    assert(bg::is_valid(intersectBox));
    for (auto& v : output)
        assert(bg::is_valid(v));

    assert(output.size() > 0);
#endif
    bool valid = true;
    for (auto& v : output) {
        valid = valid && bg::is_valid(v);
        double segArea = bg::area(v);
        valid = valid && (segArea > 0);
    }

    return  valid && bg::is_valid(wholeShape) && bg::is_valid(intersectBox);
}

static bool getTopRightQuadrant(ring_t wholeShape, box_t shapeBoundingBox, std::vector<ring_t> &output) {
    double max_x = shapeBoundingBox.max_corner().get<0>();
    double max_y = shapeBoundingBox.max_corner().get<1>();

    bg::model::box<point_t> tr_box{ {0, 0}, {max_x,max_y} };
    return getQuadrant(wholeShape, tr_box, output);
}

static bool getTopLeftQuadrant(ring_t wholeShape, box_t shapeBoundingBox, std::vector<ring_t> &output) {
    double min_x = shapeBoundingBox.min_corner().get<0>();
    double max_y = shapeBoundingBox.max_corner().get<1>();

    bg::model::box<point_t> tl_box{ {min_x, 0}, {0, max_y} };
    return getQuadrant(wholeShape, tl_box, output);
}

static bool getBottomRightQuadrant(ring_t wholeShape, box_t shapeBoundingBox, std::vector<ring_t> &output) {
    double min_y = shapeBoundingBox.min_corner().get<1>();
    double max_x = shapeBoundingBox.max_corner().get<0>();

    bg::model::box<point_t> br_box{ {0, min_y}, {max_x, 0} };
    return getQuadrant(wholeShape, br_box, output);
}

static bool getBottomLeftQuadrant(ring_t wholeShape, box_t shapeBoundingBox, std::vector<ring_t> &output) {
    double min_y = shapeBoundingBox.min_corner().get<1>();
    double min_x = shapeBoundingBox.min_corner().get<0>();

    bg::model::box<point_t> bl_box{ {min_x, min_y}, {0, 0} };
    return getQuadrant(wholeShape, bl_box, output);
}

#endif /* MISCUTILS_H */