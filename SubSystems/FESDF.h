#pragma once

#include "../FEMesh.h"
#include "FEFreeCamera.h"
#include "../SubSystems/FELinesRenderer.h"

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
		SDF(FEMesh* mesh, int dimentions, FEAABB AABB, FEFreeCamera* camera);
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

		~SDF()
		{
			data.clear();
		}
	};
}