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
		double UserData = 0.0;
	};

	struct SDF
	{
		static FEBasicCamera* CurrentCamera;

		std::vector<std::vector<std::vector<SDFNode>>> Data;
		glm::vec3 AverageNormal;
		glm::vec3 SelectedCell = glm::vec3(-1.0);

		std::vector<triangleData> GetTrianglesData();
		SDF();
		SDF(int Dimentions, FEAABB AABB);

		void Init(int Dimensions, FEAABB AABB, float ResolutionInM = 0.0f);
		void FillCellsWithTriangleInfo();

		void MouseClick(double MouseX, double MouseY, glm::mat4 TransformMat = glm::identity<glm::mat4>());

		static double TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC);

		void FillMeshWithRugosityData();

		float TimeTookToGenerateInMS = 0.0f;
		float TimeTookFillCellsWithTriangleInfo = 0.0f;
		float TimeTookCalculateRugosity = 0.0f;
		float TimeTookFillMeshWithRugosityData = 0.0f;

		int DebugTotalTrianglesInCells = 0;
		bool bWeightedNormals = false;
		bool bNormalizedNormals = false;
		bool bFindSmallestRugosity = false;
		bool bCGALVariant = false;

		std::vector<float> TrianglesUserData;
		
		bool bFullyLoaded = false;

		int RenderingMode = 0;
		bool bShowTrianglesInCells = true;
		void UpdateRenderedLines();

		void RunOnAllNodes(std::function<void(SDFNode* currentNode)> Func);
		//float GetSignedDistanceForNode(SDFNode* Node);

		~SDF()
		{
			Data.clear();
		}

		void AddLinesOfSDF();
	};
}