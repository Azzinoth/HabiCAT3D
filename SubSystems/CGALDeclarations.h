#pragma once

#include <CGAL/Polygon_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/convexity_check_2.h>

#include "CGAL/Surface_mesh.h"
#include "CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h"
#include "CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h"
#include "CGAL/Polygon_mesh_processing/repair_polygon_soup.h"

#include "CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh/Least_squares_plane_fit_region.h"

typedef CGAL::Simple_cartesian<double>					   Kernel;
typedef Kernel::Point_3									   Point_3;
typedef CGAL::Surface_mesh<Point_3>						   Surface_mesh;

typedef std::vector<std::size_t>						   Polygon_3;
namespace PMP = CGAL::Polygon_mesh_processing;

typedef CGAL::Exact_predicates_exact_constructions_kernel  Kernel2;
typedef Kernel2::Point_2                                   Point_2;
typedef CGAL::Polygon_2<Kernel2>                           Polygon_2;
typedef std::vector<Polygon_2>                             Polygon_vector;
typedef CGAL::Polygon_set_2<Kernel2>                       Polygon_set_2;

#define CGAL_FOR_PROJECTION

#ifdef CGAL_FOR_PROJECTION
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Plane_3.h>

typedef Kernel::Point_3 Point_3;
typedef Kernel::Vector_3 Vector_3;
typedef Kernel::Plane_3 Plane_3;
#endif

#include <CGAL/Bbox_3.h>
#include <CGAL/Triangle_3.h>
#include <CGAL/intersections.h>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Triangle_3 Triangle_3;
typedef CGAL::Bbox_3 Bbox_3;

#include <CGAL/convex_hull_2.h>