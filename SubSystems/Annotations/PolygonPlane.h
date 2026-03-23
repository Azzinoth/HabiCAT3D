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
};

class PolygonPlane
{
	friend class AnnotationManager;
private:
	std::vector<FEPolygon> Polygons;
	int DraftPolygonIndex = -1;
	FEPlane<float> CanvasPlane;

	FEGameModel* CanvasGameModel = nullptr;
	FEMaterial* CanvasMaterial = nullptr;
	FEMesh* CanvasMesh = nullptr;

	std::vector<std::vector<glm::vec3>> OriginalPlaneTrianglePositions;
	std::vector<std::vector<glm::vec3>> TransformedPlaneTrianglePositions;
	std::vector<std::vector<glm::vec2>> TriangleUVs;

	void MouseButtonCallback(int Button, int Action, int Mods);
public:
	PolygonPlane();
	~PolygonPlane();

	void Initialize();

	FEPolygon* GetPolygonByIndex(size_t Index);
	FEPolygon* GetCurrentPolygonInEditingMode();
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

	std::vector<int> GetTriangleIndicesInPolygon(AnalysisObject* ObjectOfInterest, int PolygonIndex);
	std::vector<std::pair<int, std::vector<int>>> GetTriangleIndicesInAllPolygons(AnalysisObject* ObjectOfInterest);

	glm::vec3 UVPositionToWorld(const glm::vec2& UVPosition) const;
};