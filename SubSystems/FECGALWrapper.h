#pragma once

#include "../SubSystems/RugosityManager.h"

#include "CGAL/Simple_cartesian.h"
#include "CGAL/Surface_mesh.h"
#include "CGAL/Surface_mesh_simplification/edge_collapse.h"
#include "CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h"

#include "CGAL/boost/graph/IO/OBJ.h"
//#include "CGAL/Polygon_mesh_processing/orient_polygon_soup.h"
#include "CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h"
#include "CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h"
#include "CGAL/Polygon_mesh_processing/repair_polygon_soup.h"

#include "CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh/Least_squares_plane_fit_region.h"

typedef CGAL::Simple_cartesian<double>  Kernel;
typedef Kernel::Point_3                 Point_3;
typedef CGAL::Surface_mesh<Point_3>		Surface_mesh;

typedef std::vector<std::size_t>		Polygon_3;

namespace SMS = CGAL::Surface_mesh_simplification;
namespace PMP = CGAL::Polygon_mesh_processing;

// Surface_mesh_approximation
#include "CGAL/Surface_mesh_approximation/approximate_triangle_mesh.h"
#include "CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h"
#include "CGAL/Polygon_mesh_processing/orientation.h"

namespace VSA = CGAL::Surface_mesh_approximation;

typedef boost::graph_traits<Surface_mesh>::face_descriptor face_descriptor;
typedef Surface_mesh::Property_map<face_descriptor, std::size_t> Face_proxy_pmap;

// Project triangle on plane
#include <CGAL/Triangle_3.h>
#include <CGAL/Plane_3.h>
#include <CGAL/Direction_3.h>

typedef Kernel::Plane_3 Plane_3;
typedef Kernel::Direction_3 Direction_3;


using namespace FocalEngine;

namespace FocalEngine
{
	class FECGALWrapper
	{

	public:
		SINGLETON_PUBLIC_PART(FECGALWrapper)

		FEMesh* importOBJ(const char* fileName, bool forceOneMesh);

		FEMesh* surfaceMeshToFEMesh(Surface_mesh mesh);
		FEMesh* surfaceMeshToFEMesh(Surface_mesh mesh, float* normals, int normSize);
		Surface_mesh FEMeshToSurfaceMesh(FEMesh* mesh);
		void saveSurfaceMeshToOBJFile(std::string fileName, Surface_mesh mesh);
		FEMesh* SurfaceMeshSimplification(FEMesh* originalMesh, double verticesLeftInPersent);
		FEMesh* SurfaceMeshApproximation(FEMesh* originalMesh, int segmentsMax);

		FEMesh* rawDataToMesh(float* positions, int posSize,
							  float* colors, int colorSize,
							  float* UV, int UVSize,
							  float* normals, int normSize,
							  float* tangents, int tanSize,
							  int* indices, int indexSize,
							  float* matIndexs, int matIndexsSize, int matCount,
							  std::string Name);

		FEMesh* rawDataToMesh(double* positions, int posSize,
							  float* colors, int colorSize,
							  float* UV, int UVSize,
							  float* normals, int normSize,
							  float* tangents, int tanSize,
							  int* indices, int indexSize,
							  float* matIndexs, int matIndexsSize, int matCount,
							  std::string Name);
	private:
		SINGLETON_PRIVATE_PART(FECGALWrapper)

		

		void addRugosityInfo(FEMesh* mesh, std::vector<int> originalTrianglesToSegments, std::vector<glm::vec3> segmentsNormals);
		std::vector<float> calculateNormals(Surface_mesh mesh);
	};

	#define CGALWrapper FECGALWrapper::getInstance()
}