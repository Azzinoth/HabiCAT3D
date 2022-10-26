#pragma once

#include "../FEMesh.h"
#include "FEFreeCamera.h"
#include "../SubSystems/FELinesRenderer.h"



#include "CGAL/Surface_mesh.h"
#include "CGAL/Surface_mesh_simplification/edge_collapse.h"
#include "CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h"

#include "CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h"
#include "CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h"
#include "CGAL/Polygon_mesh_processing/repair_polygon_soup.h"

#include "CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh/Least_squares_plane_fit_region.h"

typedef CGAL::Simple_cartesian<double>  Kernel;
typedef Kernel::Point_3                 Point_3;
typedef CGAL::Surface_mesh<Point_3>		Surface_mesh;

typedef std::vector<std::size_t>		Polygon_3;
namespace PMP = CGAL::Polygon_mesh_processing;

namespace FocalEngine
{
	struct triangleData
	{
		glm::vec3 centroid;
		glm::vec3 normal;
		float maxSideLength = 0.0f;
	};

	struct FEPlane
	{
		glm::vec3 PointOnPlane;
		glm::vec3 Normal;
		float Distance = 0.0f;

		FEPlane(const glm::vec3 PointOnPlane, const glm::vec3 Normal)
		{
			this->PointOnPlane = PointOnPlane;
			this->Normal = Normal;

			// Distance is length of perpendicular from origin to plane.
			float PlaneD = glm::length(glm::dot(PointOnPlane, Normal));
		}

		glm::vec3 ProjectPoint(const glm::vec3& Point) const
		{
			// 3D Math Primer, page 719
			// Plane equation :
			// dot(p, n) = d
			// Formula to project point q on plane:
			// q' = q + (d - dot(q, n)) * n

			return Point + (Distance - glm::dot(Point, Normal)) * Normal;
		}
	};

	struct SDFNode
	{
		float value = 0.0f;
		float distanceToTrianglePlane = 0.0f;
		FEAABB AABB;
		bool wasRenderedLastFrame = false;
		bool selected = false;
		std::vector<int> trianglesInCell;

		// Rugosity info.
		glm::vec3 averageCellNormal = glm::vec3(0.0f);
		glm::vec3 CellTrianglesCentroid = glm::vec3(0.0f);
		FEPlane* approximateProjectionPlane = nullptr;
		double rugosity = 0.0;
	};

	struct SDF
	{
		FEFreeCamera* currentCamera = nullptr;

		std::vector<std::vector<std::vector<SDFNode>>> data;
		glm::vec3 averageNormal;
		FEMesh* mesh;

		std::vector<triangleData> getTrianglesData(FEMesh* mesh);
		SDF();
		SDF(FEMesh* mesh, int dimentions, FEAABB AABB, FEFreeCamera* camera);

		void Init(FEMesh* mesh, int dimensions, FEAABB AABB, FEFreeCamera* camera, float ResolutionInM = 0.0f);

		glm::vec3 selectedCell = glm::vec3(0.0);

		void fillCellsWithTriangleInfo();
		void calculateRugosity();

		//std::vector<glm::vec3> highlightedCells;

		void mouseClick(double mouseX, double mouseY, glm::mat4 transformMat = glm::identity<glm::mat4>());

		static double TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC);

		void fillMeshWithRugosityData();

		void calculateCellRugosity(SDFNode* node, std::string* debugInfo = nullptr);

		float TimeTookToGenerateInMS = 0.0f;
		float TimeTookFillCellsWithTriangleInfo = 0.0f;
		float TimeTookCalculateRugosity = 0.0f;
		float TimeTookFillMeshWithRugosityData = 0.0f;

		int debugTotalTrianglesInCells = 0;
		//bool bFinalJitter = false;
		bool bWeightedNormals = false;
		bool bNormalizedNormals = false;
		bool bFindSmallestRugosity = false;
		bool bCGALVariant = false;

		std::vector<float> TrianglesRugosity;
		
		bool bFullyLoaded = false;

		int RenderingMode = 0;
		bool showTrianglesInCells = true;
		void UpdateRenderLines();

		~SDF()
		{
			data.clear();
		}

	private:
		void addLinesOFSDF();
	};
}