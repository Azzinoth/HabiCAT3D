#pragma once

#include "MeshManager.h"
#include "FEFreeCamera.h"
#include "FEModelViewCamera.h"
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
	static std::vector<glm::vec3> SphereVectors = {
		glm::vec3(0.0000, 1.0000, -0.0000),
		glm::vec3(0.5257, 0.8507, -0.0000),
		glm::vec3(0.1625, 0.8507, -0.5000),
		glm::vec3(-0.4253, 0.8507, -0.3090),
		glm::vec3(-0.4253, 0.8507, 0.3090),
		glm::vec3(0.1625, 0.8507, 0.5000),
		glm::vec3(0.8944, 0.4472, -0.0000),
		glm::vec3(0.2764, 0.4472, -0.8507),
		glm::vec3(-0.7236, 0.4472, -0.5257),
		glm::vec3(-0.7236, 0.4472, 0.5257),
		glm::vec3(0.2764, 0.4472, 0.8507),
		glm::vec3(0.6882, 0.5257, -0.5000),
		glm::vec3(-0.2629, 0.5257, -0.8090),
		glm::vec3(-0.8507, 0.5257, 0.0000),
		glm::vec3(-0.2629, 0.5257, 0.8090),
		glm::vec3(0.6882, 0.5257, 0.5000),
		glm::vec3(0.9511, 0.0000, -0.3090),
		glm::vec3(0.5878, 0.0000, -0.8090),
		glm::vec3(-0.0000, 0.0000, -1.0000),
		glm::vec3(-0.5878, 0.0000, -0.8090),
		glm::vec3(-0.9511, 0.0000, -0.3090),
		glm::vec3(-0.9511, 0.0000, 0.3090),
		glm::vec3(-0.5878, 0.0000, 0.8090),
		glm::vec3(0.0000, 0.0000, 1.0000),
		glm::vec3(0.5878, 0.0000, 0.8090),
		glm::vec3(0.9511, 0.0000, 0.3090),
		glm::vec3(0.2733, 0.9619, -0.0000),
		glm::vec3(0.0844, 0.9619, -0.2599),
		glm::vec3(-0.2211, 0.9619, -0.1606),
		glm::vec3(-0.2211, 0.9619, 0.1606),
		glm::vec3(0.0844, 0.9619, 0.2599),
		glm::vec3(0.3618, 0.8944, -0.2629),
		glm::vec3(-0.1382, 0.8944, -0.4253),
		glm::vec3(-0.4472, 0.8944, 0.0000),
		glm::vec3(-0.1382, 0.8944, 0.4253),
		glm::vec3(0.3618, 0.8944, 0.2629),
		glm::vec3(0.7382, 0.6746, -0.0000),
		glm::vec3(0.2281, 0.6746, -0.7020),
		glm::vec3(-0.5972, 0.6746, -0.4339),
		glm::vec3(-0.5972, 0.6746, 0.4339),
		glm::vec3(0.2281, 0.6746, 0.7020),
		glm::vec3(0.6382, 0.7236, -0.2629),
		glm::vec3(-0.0528, 0.7236, -0.6882),
		glm::vec3(-0.6708, 0.7236, -0.1625),
		glm::vec3(-0.3618, 0.7236, 0.5878),
		glm::vec3(0.4472, 0.7236, 0.5257),
		glm::vec3(0.6382, 0.7236, 0.2629),
		glm::vec3(0.4472, 0.7236, -0.5257),
		glm::vec3(-0.3618, 0.7236, -0.5878),
		glm::vec3(-0.6708, 0.7236, 0.1625),
		glm::vec3(-0.0528, 0.7236, 0.6882),
		glm::vec3(0.8226, 0.5057, -0.2599),
		glm::vec3(0.0070, 0.5057, -0.8627),
		glm::vec3(-0.8183, 0.5057, -0.2733),
		glm::vec3(-0.5128, 0.5057, 0.6938),
		glm::vec3(0.5014, 0.5057, 0.7020),
		glm::vec3(0.8226, 0.5057, 0.2599),
		glm::vec3(0.5014, 0.5057, -0.7020),
		glm::vec3(-0.5128, 0.5057, -0.6938),
		glm::vec3(-0.8183, 0.5057, 0.2733),
		glm::vec3(0.0070, 0.5057, 0.8627),
		glm::vec3(0.9593, 0.2325, -0.1606),
		glm::vec3(0.8618, 0.2764, -0.4253),
		glm::vec3(0.6708, 0.2764, -0.6882),
		glm::vec3(0.4492, 0.2325, -0.8627),
		glm::vec3(0.1437, 0.2325, -0.9619),
		glm::vec3(-0.1382, 0.2764, -0.9511),
		glm::vec3(-0.4472, 0.2764, -0.8507),
		glm::vec3(-0.6816, 0.2325, -0.6938),
		glm::vec3(-0.8705, 0.2325, -0.4339),
		glm::vec3(-0.9472, 0.2764, -0.1625),
		glm::vec3(-0.9472, 0.2764, 0.1625),
		glm::vec3(-0.8705, 0.2325, 0.4339),
		glm::vec3(-0.6816, 0.2325, 0.6938),
		glm::vec3(-0.4472, 0.2764, 0.8507),
		glm::vec3(-0.1382, 0.2764, 0.9511),
		glm::vec3(0.1437, 0.2325, 0.9619),
		glm::vec3(0.4492, 0.2325, 0.8627),
		glm::vec3(0.6708, 0.2764, 0.6882),
		glm::vec3(0.8618, 0.2764, 0.4253),
		glm::vec3(0.9593, 0.2325, 0.1606),
		glm::vec3(0.8090, 0.0000, -0.5878),
		glm::vec3(0.3090, 0.0000, -0.9511),
		glm::vec3(-0.3090, 0.0000, -0.9511),
		glm::vec3(-0.8090, 0.0000, -0.5878),
		glm::vec3(-1.0000, 0.0000, 0.0000),
		glm::vec3(-0.8090, 0.0000, 0.5878),
		glm::vec3(-0.3090, 0.0000, 0.9511),
		glm::vec3(0.3090, 0.0000, 0.9511),
		glm::vec3(0.8090, 0.0000, 0.5878),
		glm::vec3(1.0000, 0.0000, -0.0000)
	};

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
		float Value = 0.0f;
		float DistanceToTrianglePlane = 0.0f;
		FEAABB AABB;
		bool bWasRenderedLastFrame = false;
		bool bSelected = false;
		std::vector<int> TrianglesInCell;

		// Rugosity info.
		glm::vec3 AverageCellNormal = glm::vec3(0.0f);
		glm::vec3 CellTrianglesCentroid = glm::vec3(0.0f);
		FEPlane* ApproximateProjectionPlane = nullptr;
		double Rugosity = 0.0;
	};

	struct SDF
	{
		FEBasicCamera* CurrentCamera = nullptr;

		std::vector<std::vector<std::vector<SDFNode>>> Data;
		glm::vec3 AverageNormal;

		std::vector<triangleData> GetTrianglesData();
		SDF();
		SDF(int Dimentions, FEAABB AABB, FEBasicCamera* Camera);

		void Init(int Dimensions, FEAABB AABB, FEBasicCamera* Camera, float ResolutionInM = 0.0f);

		glm::vec3 SelectedCell = glm::vec3(0.0);

		void FillCellsWithTriangleInfo();
		void CalculateRugosity();

		//std::vector<glm::vec3> highlightedCells;

		void MouseClick(double MouseX, double MouseY, glm::mat4 TransformMat = glm::identity<glm::mat4>());

		static double TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC);

		void FillMeshWithRugosityData();

		void CalculateCellRugosity(SDFNode* Node, std::string* DebugInfo = nullptr);

		float TimeTookToGenerateInMS = 0.0f;
		float TimeTookFillCellsWithTriangleInfo = 0.0f;
		float TimeTookCalculateRugosity = 0.0f;
		float TimeTookFillMeshWithRugosityData = 0.0f;

		int DebugTotalTrianglesInCells = 0;
		bool bWeightedNormals = false;
		bool bNormalizedNormals = false;
		bool bFindSmallestRugosity = false;
		bool bCGALVariant = false;

		std::vector<float> TrianglesRugosity;
		
		bool bFullyLoaded = false;

		int RenderingMode = 0;
		bool bShowTrianglesInCells = true;
		void UpdateRenderLines();

		~SDF()
		{
			Data.clear();
		}

	private:
		void AddLinesOfsdf();
	};
}