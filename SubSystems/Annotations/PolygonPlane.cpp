#include "PolygonPlane.h"
using namespace FocalEngine;

void FEPolygon::UpdateTransformedPoints(const PolygonPlane& Plane)
{
	PointsTransformed.clear();
	PointsTransformed.resize(PointsInUV.size());
	for (size_t i = 0; i < PointsInUV.size(); i++)
		PointsTransformed[i] = Plane.UVPositionToWorld(PointsInUV[i]);

	Lines.clear();
	BuildVisualizationLines();
}

void FEPolygon::BuildVisualizationLines()
{
	if (PointsTransformed.size() < 3 || !bFinalized)
		return;

	for (size_t i = 0; i < PointsTransformed.size(); i++)
	{
		if (i == PointsTransformed.size() - 1)
		{
			// Connect last point to first point.
			Lines.push_back(FELine(PointsTransformed[i], PointsTransformed[0], glm::vec3(1.0f), 0.25f));
		}
		else
		{
			Lines.push_back(FELine(PointsTransformed[i], PointsTransformed[i + 1], glm::vec3(1.0f), 0.25f));
		}
	}
}

bool FEPolygon::IsPointInside(const glm::vec3& Point) const
{
	OGRLinearRing Ring;
	for (int i = 0; i < PointsTransformed.size(); i++)
		Ring.addPoint(PointsTransformed[i].x, PointsTransformed[i].y, PointsTransformed[i].z);

	// GDAL rings must be explicitly closed.
	if (PointsTransformed.size() > 0)
		Ring.closeRings();

	// Wrap the ring in an OGRPolygon
	OGRPolygon Polygon;
	Polygon.addRing(&Ring);

	OGRPoint PointToTest(Point.x, Point.y, Point.z);
	return bool(Polygon.Contains(&PointToTest)) == true;
}

PolygonPlane::PolygonPlane() {}
PolygonPlane::~PolygonPlane()
{
	MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(CanvasEntity);
}

void PolygonPlane::Initialize()
{
	FEScene* MainScene = MAIN_SCENE_MANAGER.GetMainScene();
	if (MainScene == nullptr)
		return;

	APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(std::bind(&PolygonPlane::MouseButtonCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	CanvasMesh = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
	CanvasMaterial = RESOURCE_MANAGER.GetMaterial("6917497A5E0C05454876186F");
	CanvasGameModel = RESOURCE_MANAGER.CreateGameModel(CanvasMesh, CanvasMaterial, "Canvas GameModel");
	CanvasEntity = MainScene->CreateEntity("Canvas Entity");
	CanvasEntity->AddComponent<FEGameModelComponent>(CanvasGameModel);

	OriginalPlaneTrianglePositions = CanvasMesh->GetTrianglePositions();
	TransformedPlaneTrianglePositions = OriginalPlaneTrianglePositions;
	TriangleUVs = CanvasMesh->GetTriangleUVs();

	CanvasPlane = FEPlane(OriginalPlaneTrianglePositions[0][0], OriginalPlaneTrianglePositions[0][1], OriginalPlaneTrianglePositions[0][2]);
}

OGRPolygon PolygonPlane::CreatedPolygonInOGRFormat(FEPolygon* OrdinaryPolygon)
{
	OGRLinearRing Ring;
	for (int i = 0; i < OrdinaryPolygon->PointsTransformed.size(); i++)
		Ring.addPoint(OrdinaryPolygon->PointsTransformed[i].x, OrdinaryPolygon->PointsTransformed[i].y, OrdinaryPolygon->PointsTransformed[i].z);

	// GDAL rings must be explicitly closed.
	if (OrdinaryPolygon->PointsTransformed.size() > 0)
		Ring.closeRings();

	// Wrap the ring in an OGRPolygon
	OGRPolygon Polygon;
	Polygon.addRing(&Ring);

	return Polygon;
}

FEPolygon* PolygonPlane::GetPolygonByIndex(size_t Index)
{
	if (Index >= Polygons.size())
		return nullptr;

	return &Polygons[Index];
}

FEPolygon* PolygonPlane::GetCurrentPolygonInEditingMode()
{
	if (DraftPolygonIndex < 0 || DraftPolygonIndex >= Polygons.size())
		return nullptr;

	return &Polygons[DraftPolygonIndex];
}

std::vector<FEPolygon> PolygonPlane::GetAllPolygons()
{
	return Polygons;
}

bool PolygonPlane::DeletePolygon(size_t Index)
{
	if (Index >= Polygons.size())
		return false;

	OGRPolygons.erase(OGRPolygons.begin() + Index);
	Polygons.erase(Polygons.begin() + Index);
	return true;
}

void PolygonPlane::MouseButtonCallback(int Button, int Action, int Mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
		return;
	}

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
	{
		if (DraftPolygonIndex != -1)
			UpdateRayIntersection();
	}
}

void PolygonPlane::BeginDraftPolygon()
{
	if (DraftPolygonIndex != -1)
		return;

	Polygons.push_back(FEPolygon());
	DraftPolygonIndex = static_cast<int>(Polygons.size()) - 1;
}

void PolygonPlane::FinalizeDraftPolygon()
{
	if (DraftPolygonIndex == -1)
		return;

	if (Polygons[DraftPolygonIndex].Points.size() < 3)
		return;

	OGRPolygons.push_back(CreatedPolygonInOGRFormat(&Polygons[DraftPolygonIndex]));
	Polygons[DraftPolygonIndex].bFinalized = true;
	DraftPolygonIndex = -1;
}

void PolygonPlane::ClearDraftPolygon()
{
	if (DraftPolygonIndex == -1)
		return;

	Polygons.erase(Polygons.begin() + DraftPolygonIndex);
	DraftPolygonIndex = -1;
}

void PolygonPlane::UpdateCanvasTrianglePositions()
{
	if (CanvasEntity == nullptr)
		return;

	for (size_t i = 0; i < OriginalPlaneTrianglePositions.size(); i++)
	{
		TransformedPlaneTrianglePositions[i][0] = glm::vec3(CanvasEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(OriginalPlaneTrianglePositions[i][0], 1.0f));
		TransformedPlaneTrianglePositions[i][1] = glm::vec3(CanvasEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(OriginalPlaneTrianglePositions[i][1], 1.0f));
		TransformedPlaneTrianglePositions[i][2] = glm::vec3(CanvasEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(OriginalPlaneTrianglePositions[i][2], 1.0f));
	}

	for (size_t i = 0; i < Polygons.size(); i++)
	{
		Polygons[i].UpdateTransformedPoints(*this);
		OGRPolygons[i] = CreatedPolygonInOGRFormat(&Polygons[i]);

		if (!Polygons[i].Lines.empty())
		{
			Polygons[i].Lines.clear();
			Polygons[i].BuildVisualizationLines();
		}
	}

	CanvasPlane = FEPlane(TransformedPlaneTrianglePositions[0][0], TransformedPlaneTrianglePositions[0][1], TransformedPlaneTrianglePositions[0][2]);
}

bool PolygonPlane::InteractionRayToCanvasSpace(glm::dvec3 RayOrigin, glm::dvec3 RayDirection, glm::vec2* IntersectionPointInUVCanvasSpace, glm::vec3* IntersectionPointIn3DSpace)
{
	UpdateCanvasTrianglePositions();

	for (size_t i = 0; i < TransformedPlaneTrianglePositions.size(); i++)
	{
		std::vector<glm::dvec3> CurrentTriangle;
		CurrentTriangle.push_back(glm::dvec3(TransformedPlaneTrianglePositions[i][0].x, TransformedPlaneTrianglePositions[i][0].y, TransformedPlaneTrianglePositions[i][0].z));
		CurrentTriangle.push_back(glm::dvec3(TransformedPlaneTrianglePositions[i][1].x, TransformedPlaneTrianglePositions[i][1].y, TransformedPlaneTrianglePositions[i][1].z));
		CurrentTriangle.push_back(glm::dvec3(TransformedPlaneTrianglePositions[i][2].x, TransformedPlaneTrianglePositions[i][2].y, TransformedPlaneTrianglePositions[i][2].z));
		double Distance = 0.0;
		glm::dvec3 HitPoint = glm::dvec3(0.0);
		double U = 0.0;
		double V = 0.0;

		if (GEOMETRY.IsRayIntersectingTriangle(RayOrigin, RayDirection, CurrentTriangle, Distance, &HitPoint, &U, &V))
		{
			if (IntersectionPointIn3DSpace != nullptr)
				*IntersectionPointIn3DSpace = HitPoint;

			// Load texture coordinates of the triangle vertices.
			glm::dvec2 UV0 = TriangleUVs[i][0];
			glm::dvec2 UV1 = TriangleUVs[i][1];
			glm::dvec2 UV2 = TriangleUVs[i][2];

			// Calculate texture coordinates of the hit point using interpolation.
			glm::dvec2 HitUV = (1.0 - U - V) * UV0 + U * UV1 + V * UV2;
			*IntersectionPointInUVCanvasSpace = HitUV;
			return true;
		}
	}

	return false;
}

void PolygonPlane::UpdateRayIntersection()
{
	glm::vec2 HitUV = glm::vec2(0.0f);
	glm::dvec3 MouseRay = MAIN_SCENE_MANAGER.GetMouseRayDirection();
	glm::vec3 IntersectionPoint = glm::vec3(0.0f);
	if (InteractionRayToCanvasSpace(MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MouseRay, &HitUV, &IntersectionPoint))
	{
		Polygons[DraftPolygonIndex].PointsInUV.push_back(HitUV);
		Polygons[DraftPolygonIndex].Points.push_back(IntersectionPoint);
	}
}

glm::vec3 PolygonPlane::UVPositionToWorld(const glm::vec2& UVPosition) const
{
	for (size_t i = 0; i < TriangleUVs.size(); i++)
	{
		glm::dvec2 UV0 = TriangleUVs[i][0];
		glm::dvec2 UV1 = TriangleUVs[i][1];
		glm::dvec2 UV2 = TriangleUVs[i][2];

		// Compute barycentric coordinates of UVPosition in the UV triangle.
		glm::dvec2 V0 = UV1 - UV0;
		glm::dvec2 V1 = UV2 - UV0;
		glm::dvec2 V2 = glm::dvec2(UVPosition) - UV0;

		double Dot00 = glm::dot(V0, V0);
		double Dot01 = glm::dot(V0, V1);
		double Dot02 = glm::dot(V0, V2);
		double Dot11 = glm::dot(V1, V1);
		double Dot12 = glm::dot(V1, V2);

		double InverseDenominator = 1.0 / (Dot00 * Dot11 - Dot01 * Dot01);
		double U = (Dot11 * Dot02 - Dot01 * Dot12) * InverseDenominator;
		double V = (Dot00 * Dot12 - Dot01 * Dot02) * InverseDenominator;

		// Check if point is inside this UV triangle.
		if (U >= 0.0 && V >= 0.0 && (U + V) <= 1.0)
		{
			// Interpolate world position using the same barycentric coordinates.
			glm::vec3 WorldPosition = (float)(1.0 - U - V) * TransformedPlaneTrianglePositions[i][0]
									  + (float)U * TransformedPlaneTrianglePositions[i][1]
									  + (float)V * TransformedPlaneTrianglePositions[i][2];
			return WorldPosition;
		}
	}

	return glm::vec3(0.0f);
}

void PolygonPlane::RenderAdditionalVisualization()
{
	return;
	if (DraftPolygonIndex != -1)
	{
		for (size_t i = 0; i < Polygons[DraftPolygonIndex].Points.size(); i++)
		{
			FELine PointVisualizationLine = FELine(Polygons[DraftPolygonIndex].Points[i], Polygons[DraftPolygonIndex].Points[i] + glm::vec3(0.01f), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f);
			RENDERER.DebugDrawLine(PointVisualizationLine);
		}
	}

	for (auto CurrentPolygon : Polygons)
	{
		for (size_t i = 0; i < CurrentPolygon.Lines.size(); i++)
			RENDERER.DebugDrawLine(CurrentPolygon.Lines[i]);
	}
}

std::vector<int> PolygonPlane::GetTriangleIndicesInPolygon(FEPolygon& CurrentPolygon,
														   OGRPolygon& CurrentOGRPolygon,
														   std::vector<glm::vec3>& ProjectedTriangleCentroids,
														   std::vector<OGRPoint>& OGRProjectedTriangleCentroids)
{
	std::vector<glm::vec3> CurrentPolygonPositions = CurrentPolygon.PointsTransformed;
	std::vector<int> Result = FEPolygon::GetTriangleIndicesInPolygon(CurrentPolygonPositions, CurrentOGRPolygon, ProjectedTriangleCentroids, OGRProjectedTriangleCentroids);

	return Result;
}

std::vector<std::pair<int, std::vector<int>>> PolygonPlane::GetTriangleIndicesInAllPolygons(AnalysisObject* ObjectOfInterest)
{
	std::vector < std::pair<int, std::vector<int>>> Result;
	MeshAnalysisData* CurrentMeshAnalysisData = ObjectOfInterest->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	std::vector<glm::vec3> ProjectedTriangleCentroids;
	std::vector<OGRPoint> OGRProjectedTriangleCentroids;
	ProjectedTriangleCentroids.resize(CurrentMeshAnalysisData->TrianglesCentroids.size());
	OGRProjectedTriangleCentroids.resize(CurrentMeshAnalysisData->TrianglesCentroids.size());
	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		glm::vec3 TriangleCentroid = CurrentMeshAnalysisData->TrianglesCentroids[i];
		glm::vec3 TransformedTriangleCentroid = ObjectOfInterest->GetEntity()->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TriangleCentroid, 1.0f);
		ProjectedTriangleCentroids[i] = CanvasPlane.ProjectPoint(TransformedTriangleCentroid);
		// FE_DEBUG
		//ProjectedTriangleCentroids[i] = glm::vec3(CurrentMeshAnalysisData->TrianglesCentroids[i].x, CurrentMeshAnalysisData->TrianglesCentroids[i].y, 0.0f);
		
		OGRProjectedTriangleCentroids[i] = OGRPoint(ProjectedTriangleCentroids[i].x, ProjectedTriangleCentroids[i].y, ProjectedTriangleCentroids[i].z);
	}
	
	for (size_t i = 0; i < Polygons.size(); i++)
	{
		Polygons[i].UpdateTransformedPoints(*this);
		std::vector<int> CurrentPolygonResult = GetTriangleIndicesInPolygon(Polygons[i], OGRPolygons[i], ProjectedTriangleCentroids, OGRProjectedTriangleCentroids);
		Result.push_back(std::make_pair(static_cast<int>(i), CurrentPolygonResult));
	}

	return Result;
}

FEPolygon* PolygonPlane::AddPolygon(std::vector<glm::vec2> PointsInUV)
{
	FEPolygon NewPolygon;
	NewPolygon.PointsInUV = PointsInUV;
	NewPolygon.UpdateTransformedPoints(*this);

	NewPolygon.bFinalized = true;
	OGRPolygons.push_back(CreatedPolygonInOGRFormat(&NewPolygon));
	Polygons.push_back(NewPolygon);

	UpdateCanvasTrianglePositions();

	std::vector<FELine> LinesToLoad;
	for (auto CurrentPolygon : Polygons)
	{
		for (size_t i = 0; i < CurrentPolygon.Lines.size(); i++)
			LinesToLoad.push_back(CurrentPolygon.Lines[i]);
	}

	if (CanvasLineCollection != nullptr)
	{
		RESOURCE_MANAGER.DeleteFELineCollection(CanvasLineCollection);
		CanvasLineCollection = nullptr;
	}
		
	CanvasLineCollection = RESOURCE_MANAGER.RawDataToFELineCollection(LinesToLoad);
	if (CanvasEntity != nullptr)
	{
		if (CanvasEntity->HasComponent<FELineComponent>())
		{
			CanvasEntity->GetComponent<FELineComponent>().SetLineCollection(CanvasLineCollection);
		}
		else
		{
			CanvasEntity->AddComponent<FELineComponent>(CanvasLineCollection);
		}
	}

	return &Polygons.back();
}

size_t PolygonPlane::GetPolygonIndex(FEPolygon* Polygon)
{
	for (size_t i = 0; i < Polygons.size(); i++)
	{
		if (&Polygons[i] == Polygon)
			return i;
	}

	return -1;
}