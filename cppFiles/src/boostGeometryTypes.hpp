#ifndef BOOSTGEOMETRYTYPES_H
#define BOOSTGEOMETRYTYPES_H

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::ring<point_t> ring_t;
typedef bg::model::linestring<point_t> linestring_t;
typedef bg::model::box<point_t> box_t;

#endif /* BOOSTGEOMETRYTYPES_H */