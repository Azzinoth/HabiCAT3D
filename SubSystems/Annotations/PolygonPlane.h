#pragma once

#include "../AnalysisObjectManager.h"
using namespace FocalEngine;

#include "ogrsf_frmts.h"

class PolygonPlane;
struct FEPolygon
{
	std::vector<glm::vec2> PointsInUV;
	std::vector<glm::vec3> Points;
	std::vector<glm::vec3> PointsTransformed;
	std::vector<FELine> Lines;
	bool bFinalized = false;

	void UpdateTransformedPoints(const PolygonPlane& Plane);
	void BuildVisualizationLines();
	bool IsPointInside(const glm::vec3& Point) const;

	static OGRPolygon CreatedOGRPolygonFromPoints(std::vector<glm::vec3> Points)
	{
		OGRLinearRing Ring;
		for (int i = 0; i < Points.size(); i++)
			Ring.addPoint(Points[i].x, Points[i].y, Points[i].z);

		// GDAL rings must be explicitly closed.
		if (Points.size() > 0)
			Ring.closeRings();

		// Wrap the ring in an OGRPolygon
		OGRPolygon Polygon;
		Polygon.addRing(&Ring);

		return Polygon;
	}

	static std::vector<int> GetTriangleIndicesInPolygon(std::vector<glm::vec3>& PolygonPoints,
														OGRPolygon& OGRPolygon,
														std::vector<glm::vec3>& ProjectedTriangleCentroids,
														std::vector<OGRPoint>& OGRProjectedTriangleCentroids)
	{
		std::vector<int> Result;
		FEAABB CurrentPolygonAABB = FEAABB(PolygonPoints);
		for (size_t i = 0; i < ProjectedTriangleCentroids.size(); i++)
		{
			glm::vec3 ProjectedTriangleCentroid = ProjectedTriangleCentroids[i];

			bool bInAABB = CurrentPolygonAABB.ContainsPoint(ProjectedTriangleCentroid);
			if (bInAABB && OGRPolygon.Contains(&OGRProjectedTriangleCentroids[i]))
				Result.push_back(static_cast<int>(i));
		}

		return Result;
	}
};

class PolygonPlane
{
	friend class AnnotationManager;
private:
	std::vector<FEPolygon> Polygons;
	OGRPolygon CreatedPolygonInOGRFormat(FEPolygon* OrdinaryPolygon);
	std::vector<OGRPolygon> OGRPolygons;
	int DraftPolygonIndex = -1;
	FEPlane<float> CanvasPlane;

	FEGameModel* CanvasGameModel = nullptr;
	FEMaterial* CanvasMaterial = nullptr;
	FEMesh* CanvasMesh = nullptr;
	FELineCollection* CanvasLineCollection = nullptr;

	std::vector<std::vector<glm::vec3>> OriginalPlaneTrianglePositions;
	std::vector<std::vector<glm::vec3>> TransformedPlaneTrianglePositions;
	std::vector<std::vector<glm::vec2>> TriangleUVs;

	void MouseButtonCallback(int Button, int Action, int Mods);
public:
	PolygonPlane();
	~PolygonPlane();

	void Initialize();

	FEPolygon* AddPolygon(std::vector<glm::vec2> PointsInUV);
	FEPolygon* GetPolygonByIndex(size_t Index);
	FEPolygon* GetCurrentPolygonInEditingMode();
	size_t GetPolygonIndex(FEPolygon* Polygon);
	std::vector<FEPolygon> GetAllPolygons();
	bool DeletePolygon(size_t Index);

	void BeginDraftPolygon();
	void FinalizeDraftPolygon();
	void ClearDraftPolygon();

	void RenderAdditionalVisualization();

	FEEntity* CanvasEntity = nullptr;

	void UpdateCanvasTrianglePositions();
	void UpdateRayIntersection();
	bool InteractionRayToCanvasSpace(glm::dvec3 RayOrigin, glm::dvec3 RayDirection, glm::vec2* IntersectionPointInUVCanvasSpace, glm::vec3* IntersectionPointIn3DSpace = nullptr);

	std::vector<int> GetTriangleIndicesInPolygon(FEPolygon& CurrentPolygon, OGRPolygon& CurrentOGRPolygon, std::vector<glm::vec3>& ProjectedTriangleCentroids, std::vector<OGRPoint>& OGRProjectedTriangleCentroids);
	std::vector<std::pair<int, std::vector<int>>> GetTriangleIndicesInAllPolygons(AnalysisObject* ObjectOfInterest);

	glm::vec3 UVPositionToWorld(const glm::vec2& UVPosition) const;
};