#pragma once

#include "../FEMesh.h"

#include "CGAL/Simple_cartesian.h"
#include "CGAL/Surface_mesh.h"
#include "CGAL/Surface_mesh_simplification/edge_collapse.h"
#include "CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h"

#include "CGAL/boost/graph/IO/OBJ.h"
//#include "CGAL/Polygon_mesh_processing/orient_polygon_soup.h"
#include "CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h"
#include "CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h"
#include "CGAL/Polygon_mesh_processing/repair_polygon_soup.h"

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

using namespace FocalEngine;

namespace FocalEngine
{
	class FECGALWrapper
	{

	public:
		SINGLETON_PUBLIC_PART(FECGALWrapper)

		FEMesh* importOBJ(const char* fileName, bool forceOneMesh);

		FEMesh* surfaceMeshToFEMesh(Surface_mesh mesh);
		Surface_mesh FEMeshToSurfaceMesh(FEMesh* mesh);
		void saveSurfaceMeshToOBJFile(std::string fileName, Surface_mesh mesh);
		FEMesh* SurfaceMeshSimplification(FEMesh* originalMesh, double verticesLeftInPersent);
		FEMesh* SurfaceMeshApproximation(FEMesh* originalMesh, double verticesLeftInPersent);
	private:
		SINGLETON_PRIVATE_PART(FECGALWrapper)

		FEMesh* rawDataToMesh(float* positions, int posSize,
							  float* UV, int UVSize,
							  float* normals, int normSize,
							  float* tangents, int tanSize,
							  int* indices, int indexSize,
							  float* matIndexs, int matIndexsSize, int matCount,
							  std::string Name);
	};

	#define CGALWrapper FECGALWrapper::getInstance()
}