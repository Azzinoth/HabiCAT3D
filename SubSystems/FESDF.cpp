#include "FESDF.h"
using namespace FocalEngine;

glm::dvec3 mouseRay(double mouseX, double mouseY, FEBasicCamera* currentCamera)
{
	int W, H;
	APPLICATION.GetWindowSize(&W, &H);

	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / W - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / H;

	const glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(currentCamera->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(currentCamera->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

double SDF::TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC)
{
	const double x1 = PointA.x;
	const double x2 = PointB.x;
	const double x3 = PointC.x;

	const double y1 = PointA.y;
	const double y2 = PointB.y;
	const double y3 = PointC.y;

	const double z1 = PointA.z;
	const double z2 = PointB.z;
	const double z3 = PointC.z;

	return 0.5 * sqrt(pow(x2 * y1 - x3 * y1 - x1 * y2 + x3 * y2 + x1 * y3 - x2 * y3, 2.0) +
					  pow((x2 * z1) - (x3 * z1) - (x1 * z2) + (x3 * z2) + (x1 * z3) - (x2 * z3), 2.0) +
					  pow((y2 * z1) - (y3 * z1) - (y1 * z2) + (y3 * z2) + (y1 * z3) - (y2 * z3), 2.0));
}

bool intersectWithTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& triangleVertices, float& distance)
{
	if (triangleVertices.size() != 3)
		return false;

	const float a = RayDirection[0];
	const float b = triangleVertices[0][0] - triangleVertices[1][0];
	const float c = triangleVertices[0][0] - triangleVertices[2][0];

	const float d = RayDirection[1];
	const float e = triangleVertices[0][1] - triangleVertices[1][1];
	const float f = triangleVertices[0][1] - triangleVertices[2][1];

	const float g = RayDirection[2];
	const float h = triangleVertices[0][2] - triangleVertices[1][2];
	const float j = triangleVertices[0][2] - triangleVertices[2][2];

	const float k = triangleVertices[0][0] - RayOrigin[0];
	const float l = triangleVertices[0][1] - RayOrigin[1];
	const float m = triangleVertices[0][2] - RayOrigin[2];

	const float determinant0 = glm::determinant(glm::mat3(a, b, c,
	                                                      d, e, f,
	                                                      g, h, j));

	const float determinant1 = glm::determinant(glm::mat3(k, b, c,
	                                                      l, e, f,
	                                                      m, h, j));

	const float t = determinant1 / determinant0;

	const float determinant2 = glm::determinant(glm::mat3(a, k, c,
	                                                      d, l, f,
	                                                      g, m, j));
	const float u = determinant2 / determinant0;

	const float determinant3 = glm::determinant(glm::mat3(a, b, k,
	                                                      d, e, l,
	                                                      g, h, m));

	const float v = determinant3 / determinant0;

	if (t >= 0.00001 &&
		u >= 0.00001 && v >= 0.00001 &&
		u <= 1 && v <= 1 &&
		u + v >= 0.00001 &&
		u + v <= 1 && t > 0.00001)
	{
		//Point4 p = v1 + u * (v2 - v1) + v * (v3 - v1);
		//hit = Hit(p, this->N, this, t);

		distance = t;
		return true;
	}

	return false;
}

std::vector<triangleData> SDF::GetTrianglesData()
{
	std::vector<triangleData> Result;

	if (MESH_MANAGER.ActiveMesh == nullptr)
		return Result;

	std::vector<float> FEVertices;
	FEVertices.resize(MESH_MANAGER.ActiveMesh->getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(MESH_MANAGER.ActiveMesh->getPositionsBufferID(), 0, sizeof(float) * FEVertices.size(), FEVertices.data()));

	std::vector<float> FENormals;
	FENormals.resize(MESH_MANAGER.ActiveMesh->getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(MESH_MANAGER.ActiveMesh->getNormalsBufferID(), 0, sizeof(float) * FENormals.size(), FENormals.data()));

	std::vector<int> FEIndices;
	FEIndices.resize(MESH_MANAGER.ActiveMesh->getIndicesCount());
	FE_GL_ERROR(glGetNamedBufferSubData(MESH_MANAGER.ActiveMesh->getIndicesBufferID(), 0, sizeof(int) * FEIndices.size(), FEIndices.data()));

	for (size_t i = 0; i < FEIndices.size(); i += 3)
	{
		int VertexPosition = FEIndices[i] * 3;
		glm::vec3 FirstVertex = glm::vec3(FEVertices[VertexPosition], FEVertices[VertexPosition + 1], FEVertices[VertexPosition + 2]);

		VertexPosition = FEIndices[i + 1] * 3;
		glm::vec3 SecondVertex = glm::vec3(FEVertices[VertexPosition], FEVertices[VertexPosition + 1], FEVertices[VertexPosition + 2]);

		VertexPosition = FEIndices[i + 2] * 3;
		glm::vec3 ThirdVertex = glm::vec3(FEVertices[VertexPosition], FEVertices[VertexPosition + 1], FEVertices[VertexPosition + 2]);

		const glm::vec3 CurrentCentroid = (FirstVertex + SecondVertex + ThirdVertex) / 3.0f;

		// We are taking index of last vertex because all verticies of triangle should have same normal.
		const glm::vec3 TriangleNormal = glm::vec3(FENormals[VertexPosition], FENormals[VertexPosition + 1], FENormals[VertexPosition + 2]);

		// First triangle side lenght
		float FirstSideLenght = glm::length(FirstVertex - SecondVertex);
		float SecondSideLenght = glm::length(FirstVertex - ThirdVertex);
		float ThirdSideLenght = glm::length(SecondVertex - ThirdVertex);

		triangleData data;
		data.centroid = CurrentCentroid;
		data.normal = TriangleNormal;
		data.maxSideLength = std::max(std::max(FirstSideLenght, SecondSideLenght), ThirdSideLenght);
		Result.push_back(data);
	}

	return Result;
}

SDF::SDF()
{

}

SDF::SDF(const int Dimentions, FEAABB AABB, FEBasicCamera* Camera)
{
	if (Dimentions < 1 || Dimentions > 4096)
		return;

	// If dimension is not power of 2, we can't continue.
	if (log2(Dimentions) != static_cast<int>(log2(Dimentions)))
		return;

	TIME.BeginTimeStamp("SDF Generation");

	CurrentCamera = Camera;

	const glm::vec3 center = AABB.getCenter();
	FEAABB SDFAABB = FEAABB(center - glm::vec3(AABB.getSize() / 2.0f), center + glm::vec3(AABB.getSize() / 2.0f));

	Data.resize(Dimentions);
	for (size_t i = 0; i < Dimentions; i++)
	{
		Data[i].resize(Dimentions);
		for (size_t j = 0; j < Dimentions; j++)
		{
			Data[i][j].resize(Dimentions);
		}
	}

	const glm::vec3 start = SDFAABB.getMin();
	const float CellSize = SDFAABB.getSize() / Dimentions;

	/*std::vector<triangleData> centroids = getTrianglesData(mesh);
	averageNormal = glm::vec3(0.0f);
	for (size_t i = 0; i < centroids.size(); i++)
	{
		averageNormal += centroids[i].normal;
	}
	averageNormal /= centroids.size();*/

	for (size_t i = 0; i < Dimentions; i++)
	{
		for (size_t j = 0; j < Dimentions; j++)
		{
			for (size_t k = 0; k < Dimentions; k++)
			{
				glm::vec3 CurrentAABBMin = start + glm::vec3(CellSize * i, CellSize * j, CellSize * k);

				Data[i][j][k].AABB = FEAABB(CurrentAABBMin, CurrentAABBMin + glm::vec3(CellSize));

				//float minDistanceToCentroid = FLT_MAX;
				//int centroidIndex = -1;
				//glm::vec3 cellCenter = data[i][j][k].AABB.getCenter();
				//for (size_t p = 0; p < centroids.size(); p++)
				//{
				//	float currentDistance = glm::distance(centroids[p].centroid, cellCenter);
				//	if (currentDistance < minDistanceToCentroid)
				//	{
				//		minDistanceToCentroid = currentDistance;
				//		centroidIndex = p;
				//	}
				//}

				//if (centroidIndex != -1)
				//{
				//	glm::vec3 vectorToCentroid = centroids[centroidIndex].centroid - cellCenter;

				//	// Test whether cell is inside or outside of mesh
				//	if (glm::dot(vectorToCentroid, centroids[centroidIndex].normal) >= 0)
				//	{
				//		minDistanceToCentroid = -minDistanceToCentroid;
				//	}

				//	// https://mathinsight.org/distance_point_plane
				//	// Normal of plane
				//	glm::vec3 N = centroids[centroidIndex].normal;
				//	// Point on plane
				//	glm::vec3 C = centroids[centroidIndex].centroid;
				//	glm::vec3 P = cellCenter;

				//	float D = -N.x * C.x - N.y * C.y - N.z * C.z;
				//	float distance = abs(N.x * P.x + N.y * P.y + N.z * P.z + D) / glm::length(N);

				//	// Distance to plane could be a lot greater than distance to triangle
				//	data[i][j][k].distanceToTrianglePlane = distance;
				//	if (abs(minDistanceToCentroid) > centroids[centroidIndex].maxSideLength / 2.0f)
				//	{
				//		data[i][j][k].distanceToTrianglePlane = FLT_MAX;
				//		if (minDistanceToCentroid < 0.0f)
				//			minDistanceToCentroid = -minDistanceToCentroid;
				//	}
				//}

				//data[i][j][k].value = minDistanceToCentroid;
			}
		}
	}

	TimeTookToGenerateInMS = static_cast<float>(TIME.EndTimeStamp("SDF Generation"));
}

int DimensionsToPOWDimentions(const int Dimentions)
{
	for (size_t i = 0; i < 12; i++)
	{
		if (Dimentions > pow(2.0, i))
			continue;

		if (Dimentions <= pow(2.0, i))
			return static_cast<int>(pow(2.0, i));
	}

	return 0;
}

void SDF::Init(int Dimensions, FEAABB AABB, FEBasicCamera* Camera, const float ResolutionInM)
{
	TIME.BeginTimeStamp("SDF Generation");

	CurrentCamera = Camera;

	const glm::vec3 center = AABB.getCenter();

	//ResolutionInM = 1.0f;
	const int MinDimensions = AABB.getSize() / ResolutionInM;
	Dimensions = DimensionsToPOWDimentions(MinDimensions);

	if (Dimensions < 1 || Dimensions > 4096)
		return;

	// If dimensions is not power of 2, we can't continue.
	if (log2(Dimensions) != static_cast<int>(log2(Dimensions)))
		return;

	Data.resize(Dimensions);
	for (size_t i = 0; i < Dimensions; i++)
	{
		Data[i].resize(Dimensions);
		for (size_t j = 0; j < Dimensions; j++)
		{
			Data[i][j].resize(Dimensions);
		}
	}

	FEAABB SDFAABB;
	if (ResolutionInM != 0.0f)
	{
		SDFAABB = FEAABB(center - glm::vec3(ResolutionInM * Dimensions / 2.0f), center + glm::vec3(ResolutionInM * Dimensions / 2.0f));
	}
	else
	{
		SDFAABB = FEAABB(center - glm::vec3(AABB.getSize() / 2.0f), center + glm::vec3(AABB.getSize() / 2.0f));
	}

	const glm::vec3 start = SDFAABB.getMin();
	const float CellSize = SDFAABB.getSize() / Dimensions;

	for (size_t i = 0; i < Dimensions; i++)
	{
		for (size_t j = 0; j < Dimensions; j++)
		{
			for (size_t k = 0; k < Dimensions; k++)
			{
				glm::vec3 CurrentAABBMin = start + glm::vec3(CellSize * i, CellSize * j, CellSize * k);

				Data[i][j][k].AABB = FEAABB(CurrentAABBMin, CurrentAABBMin + glm::vec3(CellSize));

				//float minDistanceToCentroid = FLT_MAX;
				//int centroidIndex = -1;
				//glm::vec3 cellCenter = data[i][j][k].AABB.getCenter();
				//for (size_t p = 0; p < centroids.size(); p++)
				//{
				//	float currentDistance = glm::distance(centroids[p].centroid, cellCenter);
				//	if (currentDistance < minDistanceToCentroid)
				//	{
				//		minDistanceToCentroid = currentDistance;
				//		centroidIndex = p;
				//	}
				//}

				//if (centroidIndex != -1)
				//{
				//	glm::vec3 vectorToCentroid = centroids[centroidIndex].centroid - cellCenter;

				//	// Test whether cell is inside or outside of mesh
				//	if (glm::dot(vectorToCentroid, centroids[centroidIndex].normal) >= 0)
				//	{
				//		minDistanceToCentroid = -minDistanceToCentroid;
				//	}

				//	// https://mathinsight.org/distance_point_plane
				//	// Normal of plane
				//	glm::vec3 N = centroids[centroidIndex].normal;
				//	// Point on plane
				//	glm::vec3 C = centroids[centroidIndex].centroid;
				//	glm::vec3 P = cellCenter;

				//	float D = -N.x * C.x - N.y * C.y - N.z * C.z;
				//	float distance = abs(N.x * P.x + N.y * P.y + N.z * P.z + D) / glm::length(N);

				//	// Distance to plane could be a lot greater than distance to triangle
				//	data[i][j][k].distanceToTrianglePlane = distance;
				//	if (abs(minDistanceToCentroid) > centroids[centroidIndex].maxSideLength / 2.0f)
				//	{
				//		data[i][j][k].distanceToTrianglePlane = FLT_MAX;
				//		if (minDistanceToCentroid < 0.0f)
				//			minDistanceToCentroid = -minDistanceToCentroid;
				//	}
				//}

				//data[i][j][k].value = minDistanceToCentroid;
			}
		}
	}

	TimeTookToGenerateInMS = TIME.EndTimeStamp("SDF Generation");
}

void SDF::FillCellsWithTriangleInfo()
{
	TIME.BeginTimeStamp("Fill cells with triangle info");

	const float CellSize = Data[0][0][0].AABB.getSize();
	const glm::vec3 GridMin = Data[0][0][0].AABB.getMin();
	const glm::vec3 GridMax = Data[Data.size() - 1][Data.size() - 1][Data.size() - 1].AABB.getMax();

	DebugTotalTrianglesInCells = 0;

	for (int l = 0; l < MESH_MANAGER.ActiveMesh->Triangles.size(); l++)
	{
		FEAABB TriangleAABB = FEAABB(MESH_MANAGER.ActiveMesh->Triangles[l]);

		int XEnd = Data.size();

		float distance = sqrt(pow(TriangleAABB.getMin().x - GridMin.x, 2.0));
		int XBegin = static_cast<int>(distance / CellSize) - 1;
		if (XBegin < 0)
			XBegin = 0;

		distance = sqrt(pow(TriangleAABB.getMax().x - GridMax.x, 2.0));
		XEnd -= static_cast<int>(distance / CellSize);
		XEnd++;
		if (XEnd > Data.size())
			XEnd = Data.size();

		for (size_t i = XBegin; i < XEnd; i++)
		{
			int YEnd = Data.size();

			distance = sqrt(pow(TriangleAABB.getMin().y - GridMin.y, 2.0));
			int YBegin = static_cast<int>(distance / CellSize) - 1;
			if (YBegin < 0)
				YBegin = 0;

			distance = sqrt(pow(TriangleAABB.getMax().y - GridMax.y, 2.0));
			YEnd -= static_cast<int>(distance / CellSize);
			YEnd++;
			if (YEnd > Data.size())
				YEnd = Data.size();

			for (size_t j = YBegin; j < YEnd; j++)
			{
				int ZEnd = Data.size();

				distance = sqrt(pow(TriangleAABB.getMin().z - GridMin.z, 2.0));
				int ZBegin = static_cast<int>(distance / CellSize) - 1;
				if (ZBegin < 0)
					ZBegin = 0;

				distance = sqrt(pow(TriangleAABB.getMax().z - GridMax.z, 2.0));
				ZEnd -= static_cast<int>(distance / CellSize);
				ZEnd++;
				if (ZEnd > Data.size())
					ZEnd = Data.size();

				for (size_t k = ZBegin; k < ZEnd; k++)
				{
					if (Data[i][j][k].AABB.AABBIntersect(TriangleAABB))
					{
						Data[i][j][k].TrianglesInCell.push_back(l);
						DebugTotalTrianglesInCells++;
					}
				}
			}
		}
	}

	TimeTookFillCellsWithTriangleInfo = TIME.EndTimeStamp("Fill cells with triangle info");
}

void SDF::CalculateRugosity()
{
	TIME.BeginTimeStamp("Calculate rugosity");

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				CalculateCellRugosity(&Data[i][j][k]);
			}
		}
	}

	TimeTookCalculateRugosity = TIME.EndTimeStamp("Calculate rugosity");
}

void SDF::MouseClick(const double MouseX, const double MouseY, const glm::mat4 TransformMat)
{
	SelectedCell = glm::vec3(0.0);

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				Data[i][j][k].bSelected = false;
			}
		}
	}

	float DistanceToCell = 999999.0f;
	float LastDistanceToCell = 999999.0f;
	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				Data[i][j][k].bSelected = false;
				if (!Data[i][j][k].bWasRenderedLastFrame)
					continue;

				FEAABB FinalAABB = Data[i][j][k].AABB.transform(TransformMat).transform(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix());
				if (FinalAABB.rayIntersect(CurrentCamera->getPosition(), mouseRay(MouseX, MouseY, CurrentCamera), DistanceToCell))
				{
					if (LastDistanceToCell > DistanceToCell)
					{
						LastDistanceToCell = DistanceToCell;
						SelectedCell = glm::vec3(i, j, k);
					}
				}
			}
		}
	}

	if (DistanceToCell != 999999.0f)
		Data[static_cast<int>(SelectedCell.x)][static_cast<int>(SelectedCell.y)][static_cast<int>(SelectedCell.z)].bSelected = true;
}

void SDF::FillMeshWithRugosityData()
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	TIME.BeginTimeStamp("FillMeshWithRugosityData");

	const int PosSize = MESH_MANAGER.ActiveMesh->getPositionsCount();
	float* positions = new float[PosSize];
	FE_GL_ERROR(glGetNamedBufferSubData(MESH_MANAGER.ActiveMesh->getPositionsBufferID(), 0, sizeof(float) * PosSize, positions));

	const int IndexSize = MESH_MANAGER.ActiveMesh->getIndicesCount();
	int* indices = new int[IndexSize];
	FE_GL_ERROR(glGetNamedBufferSubData(MESH_MANAGER.ActiveMesh->getIndicesBufferID(), 0, sizeof(int) * IndexSize, indices));

	std::vector<float> PositionsVector;
	for (size_t i = 0; i < PosSize; i++)
	{
		PositionsVector.push_back(positions[i]);
	}

	std::vector<int> IndexVector;
	for (size_t i = 0; i < IndexSize; i++)
	{
		IndexVector.push_back(indices[i]);
	}

	delete positions;
	delete indices;

	float* colors = new float[PosSize];

	auto SetColorOfVertex = [&](const int Index, const glm::vec3 Color) {
		colors[Index * 3] = Color.x;
		colors[Index * 3 + 1] = Color.y;
		colors[Index * 3 + 2] = Color.z;
	};

	auto GetVertexOfFace = [&](const int FaceIndex) {
		std::vector<int> result;
		result.push_back(IndexVector[FaceIndex * 3]);
		result.push_back(IndexVector[FaceIndex * 3 + 1]);
		result.push_back(IndexVector[FaceIndex * 3 + 2]);

		return result;
	};

	auto setColorOfFace = [&](int faceIndex, glm::vec3 color) {
		const std::vector<int> faceVertex = GetVertexOfFace(faceIndex);

		for (size_t i = 0; i < faceVertex.size(); i++)
		{
			SetColorOfVertex(faceVertex[i], color);
		}
	};

	std::vector<int> TrianglesRugosityCount;
	TrianglesRugosity.resize(MESH_MANAGER.ActiveMesh->Triangles.size());
	TrianglesRugosityCount.resize(MESH_MANAGER.ActiveMesh->Triangles.size());

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				for (size_t l = 0; l < Data[i][j][k].TrianglesInCell.size(); l++)
				{
					const int TriangleIndex = Data[i][j][k].TrianglesInCell[l];
					TrianglesRugosityCount[TriangleIndex]++;
					TrianglesRugosity[TriangleIndex] += static_cast<float>(Data[i][j][k].Rugosity);
				}
			}
		}
	}

	for (size_t i = 0; i < TrianglesRugosity.size(); i++)
	{
		TrianglesRugosity[i] /= TrianglesRugosityCount[i];
	}

	TimeTookFillMeshWithRugosityData = TIME.EndTimeStamp("FillMeshWithRugosityData");
}

void SDF::CalculateCellRugosity(SDFNode* Node, std::string* DebugInfo)
{
	if (Node->TrianglesInCell.empty())
		return;

	float TotalArea = 0.0f;
	for (size_t l = 0; l < Node->TrianglesInCell.size(); l++)
	{
		TotalArea += static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[Node->TrianglesInCell[l]]);
	}

	auto CalculateCellRugosity = [&](const glm::vec3 PointOnPlane, const glm::vec3 PlaneNormal) {
		double Result = 0.0;
		const FEPlane* ProjectionPlane = new FEPlane(PointOnPlane, PlaneNormal);

		std::vector<float> Rugosities;
		for (int l = 0; l < Node->TrianglesInCell.size(); l++)
		{
			std::vector<glm::vec3> CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[Node->TrianglesInCell[l]];

			glm::vec3 AProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[0]);
			glm::vec3 BProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[1]);
			glm::vec3 CProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[2]);

			const double ProjectionArea = TriangleArea(AProjection, BProjection, CProjection);
			const double OriginalArea = MESH_MANAGER.ActiveMesh->TrianglesArea[Node->TrianglesInCell[l]];
			Rugosities.push_back(static_cast<float>(OriginalArea / ProjectionArea));

			if (OriginalArea == 0.0 || ProjectionArea == 0.0)
				Rugosities.back() = 1.0f;

			if (Rugosities.back() > 100.0f)
				Rugosities.back() = 100.0f;
		}

		// Weighted by triangle area rugosity.
		for (int l = 0; l < Node->TrianglesInCell.size(); l++)
		{
			const float CurrentTriangleCoef = static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[Node->TrianglesInCell[l]] / TotalArea);

			Result += Rugosities[l] * CurrentTriangleCoef;

			if (isnan(Result))
				Result = 1.0f;
		}

		delete ProjectionPlane;
		return Result;
	};

	if (bCGALVariant)
	{
		std::vector<float> FEVerticesFinal;
		std::vector<int> FEIndicesFinal;

		for (int l = 0; l < Node->TrianglesInCell.size(); l++)
		{
			const int TriangleIndex = Node->TrianglesInCell[l];

			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][0][0]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][0][1]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][0][2]);
			FEIndicesFinal.push_back(l * 3);

			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][1][0]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][1][1]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][1][2]);
			FEIndicesFinal.push_back(l * 3 + 1);

			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][2][0]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][2][1]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][2][2]);
			FEIndicesFinal.push_back(l * 3 + 2);
		}

		// Formating data to CGAL format.
		std::vector<Polygon_3> CGALFaces;
		CGALFaces.resize(FEIndicesFinal.size() / 3);
		int count = 0;
		for (size_t i = 0; i < FEIndicesFinal.size(); i += 3)
		{
			CGALFaces[count].push_back(FEIndicesFinal[i]);
			CGALFaces[count].push_back(FEIndicesFinal[i + 1]);
			CGALFaces[count].push_back(FEIndicesFinal[i + 2]);
			count++;
		}

		std::vector<Point_3> CGALPoints;
		for (size_t i = 0; i < FEVerticesFinal.size(); i += 3)
		{
			CGALPoints.push_back(Point_3(FEVerticesFinal[i], FEVerticesFinal[i + 1], FEVerticesFinal[i + 2]));
		}

		Surface_mesh result;

		if (!PMP::is_polygon_soup_a_polygon_mesh(CGALFaces))
		{
			PMP::repair_polygon_soup(CGALPoints, CGALFaces);
			PMP::orient_polygon_soup(CGALPoints, CGALFaces);
		}

		PMP::polygon_soup_to_polygon_mesh(CGALPoints, CGALFaces, result);

		Kernel::Plane_3 plane;
		Kernel::Point_3 centroid;

		Kernel::FT quality = linear_least_squares_fitting_3(result.points().begin(), result.points().end(), plane, centroid,
		                                                    CGAL::Dimension_tag<0>());


		const auto CGALNormal = plane.perpendicular_line(centroid);

		glm::vec3 Normal = glm::vec3(CGALNormal.direction().vector().x(),
									 CGALNormal.direction().vector().y(),
									 CGALNormal.direction().vector().z());

		Normal = glm::normalize(Normal);

		Node->Rugosity = CalculateCellRugosity(Node->CellTrianglesCentroid, Normal);

		return;
	}

	// ******* Getting average normal *******
	for (size_t l = 0; l < Node->TrianglesInCell.size(); l++)
	{
		std::vector<glm::vec3> CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[Node->TrianglesInCell[l]];
		std::vector<glm::vec3> CurrentTriangleNormals = MESH_MANAGER.ActiveMesh->TrianglesNormals[Node->TrianglesInCell[l]];

		if (bWeightedNormals)
		{
			const float CurrentTriangleCoef = static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[Node->TrianglesInCell[l]] / TotalArea);

			Node->AverageCellNormal += CurrentTriangleNormals[0] * CurrentTriangleCoef;
			Node->AverageCellNormal += CurrentTriangleNormals[1] * CurrentTriangleCoef;
			Node->AverageCellNormal += CurrentTriangleNormals[2] * CurrentTriangleCoef;
		}
		else
		{
			Node->AverageCellNormal += CurrentTriangleNormals[0];
			Node->AverageCellNormal += CurrentTriangleNormals[1];
			Node->AverageCellNormal += CurrentTriangleNormals[2];
		}

		Node->CellTrianglesCentroid += CurrentTriangle[0];
		Node->CellTrianglesCentroid += CurrentTriangle[1];
		Node->CellTrianglesCentroid += CurrentTriangle[2];
	}

	if (!bWeightedNormals)
		Node->AverageCellNormal /= Node->TrianglesInCell.size() * 3;

	if (bNormalizedNormals)
		Node->AverageCellNormal = glm::normalize(Node->AverageCellNormal);
	Node->CellTrianglesCentroid /= Node->TrianglesInCell.size() * 3;
	// ******* Getting average normal END *******

	if (bFindSmallestRugosity)
	{
		std::unordered_map<int, float> TriangleNormalsToRugosity;
		TriangleNormalsToRugosity[-1] = static_cast<float>(CalculateCellRugosity(Node->CellTrianglesCentroid, Node->AverageCellNormal));

		/*for (int i = 0; i < node->trianglesInCell.size(); i++)
		{
			TriangleNormalsToRugosity[i] = CalculateCellRugosity(mesh->Triangles[node->trianglesInCell[i]][2], mesh->TrianglesNormals[node->trianglesInCell[i]][2]);
		}*/

		for (int i = 0; i < SphereVectors.size(); i++)
		{
			TriangleNormalsToRugosity[i] = static_cast<float>(CalculateCellRugosity(glm::vec3(0.0f), SphereVectors[i]));
		}

		double Min = FLT_MAX;
		double Max = -FLT_MAX;
		auto MapIt = TriangleNormalsToRugosity.begin();
		while (MapIt != TriangleNormalsToRugosity.end())
		{
			if (MapIt->second < Min)
			{
				Min = MapIt->second;
				Node->Rugosity = Min;
			}

			MapIt++;
		}
	}
	else
	{
		Node->Rugosity = CalculateCellRugosity(Node->CellTrianglesCentroid, Node->AverageCellNormal);
		if (isnan(Node->Rugosity))
			Node->Rugosity = 1.0f;
	}
}

void SDF::AddLinesOfsdf()
{
	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				bool render = false;
				Data[i][j][k].bWasRenderedLastFrame = false;

				if (!Data[i][j][k].TrianglesInCell.empty() || RenderingMode == 2)
					render = true;

				if (render)
				{
					glm::vec3 color = glm::vec3(0.1f, 0.6f, 0.1f);
					if (Data[i][j][k].bSelected)
						color = glm::vec3(0.9f, 0.1f, 0.1f);

					LINE_RENDERER.RenderAABB(Data[i][j][k].AABB.transform(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix()), color);

					Data[i][j][k].bWasRenderedLastFrame = true;

					if (bShowTrianglesInCells && Data[i][j][k].bSelected)
					{
						for (size_t l = 0; l < Data[i][j][k].TrianglesInCell.size(); l++)
						{
							const auto CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[Data[i][j][k].TrianglesInCell[l]];

							std::vector<glm::vec3> TranformedTrianglePoints = CurrentTriangle;
							for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
							{
								TranformedTrianglePoints[j] = MESH_MANAGER.ActiveMesh->Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
							}

							LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
							LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
							LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
						}
					}
				}
			}
		}
	}
}

void SDF::UpdateRenderLines()
{
	LINE_RENDERER.clearAll();
	if (RenderingMode != 0)
		AddLinesOfsdf();
	LINE_RENDERER.SyncWithGPU();
}