#include "FESDF.h"
using namespace FocalEngine;

glm::dvec3 mouseRay(double mouseX, double mouseY, FEBasicCamera* currentCamera)
{
	int W, H;
	APPLICATION.GetMainWindow()->GetSize(&W, &H);

	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / W - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / H;

	const glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(currentCamera->GetProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(currentCamera->GetViewMatrix()) * eyeCoords;
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

SDF::SDF() {}

SDF::~SDF()
{
	Data.clear();
}

int DimensionsToPOWDimensions(const int Dimensions)
{
	for (size_t i = 0; i < 12; i++)
	{
		if (Dimensions > pow(2.0, i))
			continue;

		if (Dimensions <= pow(2.0, i))
			return static_cast<int>(pow(2.0, i));
	}

	return 0;
}

void SDF::Init(int Dimensions, FEAABB AABB, const float ResolutionInM)
{
	TIME.BeginTimeStamp("SDF Generation");

	const glm::vec3 center = AABB.GetCenter();

	int AdditionalDimensions = 0;
	Dimensions = 1;
	if (ResolutionInM > 0)
	{
		AdditionalDimensions = 2;
		const int MinDimensions = static_cast<int>(AABB.GetSize() / ResolutionInM);
		Dimensions = DimensionsToPOWDimensions(MinDimensions) + AdditionalDimensions;

		if (Dimensions < 1 || Dimensions > 4096)
			return;
	}

	// If dimensions is not power of 2, we can't continue.
	/*if (log2(Dimensions) != static_cast<int>(log2(Dimensions)))
		return;*/

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
	if (ResolutionInM > 0.0f)
	{
		SDFAABB = FEAABB(center - glm::vec3(ResolutionInM * Dimensions / 2.0f), center + glm::vec3(ResolutionInM * Dimensions / 2.0f));
	}
	else
	{
		SDFAABB = FEAABB(center - glm::vec3(AABB.GetSize() / 2.0f), center + glm::vec3(AABB.GetSize() / 2.0f));
	}

	float CellSize;
	if (ResolutionInM > 0)
	{
		CellSize = ResolutionInM;
	}
	else
	{
		CellSize = SDFAABB.GetSize();
	}

	const glm::vec3 Start = SDFAABB.GetMin();
	for (size_t i = 0; i < Dimensions; i++)
	{
		for (size_t j = 0; j < Dimensions; j++)
		{
			for (size_t k = 0; k < Dimensions; k++)
			{
				glm::vec3 CurrentAABBMin = Start + glm::vec3(CellSize * i, CellSize * j, CellSize * k);
				Data[i][j][k].AABB = FEAABB(CurrentAABBMin, CurrentAABBMin + glm::vec3(CellSize));
				//float SignedDistance = GetSignedDistanceForNode(&Data[i][j][k]);
			}
		}
	}

	TimeTakenToGenerateInMS = static_cast<float>(TIME.EndTimeStamp("SDF Generation"));
}

void SDF::FillCellsWithTriangleInfo()
{
	TIME.BeginTimeStamp("Fill cells with triangle info");

	const float CellSize = Data[0][0][0].AABB.GetSize();
	const glm::vec3 GridMin = Data[0][0][0].AABB.GetMin();
	const glm::vec3 GridMax = Data[Data.size() - 1][Data.size() - 1][Data.size() - 1].AABB.GetMax();

	DebugTotalTrianglesInCells = 0;

	for (int l = 0; l < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); l++)
	{
		FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

		int XEnd = static_cast<int>(Data.size());

		float distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
		int XBegin = static_cast<int>(distance / CellSize) - 1;
		if (XBegin < 0)
			XBegin = 0;

		distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
		XEnd -= static_cast<int>(distance / CellSize);
		XEnd++;
		if (XEnd > Data.size())
			XEnd = static_cast<int>(Data.size());

		for (size_t i = XBegin; i < XEnd; i++)
		{
			int YEnd = static_cast<int>(Data.size());

			distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
			int YBegin = static_cast<int>(distance / CellSize) - 1;
			if (YBegin < 0)
				YBegin = 0;

			distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
			YEnd -= static_cast<int>(distance / CellSize);
			YEnd++;
			if (YEnd > Data.size())
				YEnd = static_cast<int>(Data.size());

			for (size_t j = YBegin; j < YEnd; j++)
			{
				int ZEnd = static_cast<int>(Data.size());

				distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
				int ZBegin = static_cast<int>(distance / CellSize) - 1;
				if (ZBegin < 0)
					ZBegin = 0;

				distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
				ZEnd -= static_cast<int>(distance / CellSize);
				ZEnd++;
				if (ZEnd > Data.size())
					ZEnd = static_cast<int>(Data.size());

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

	TimeTakenFillCellsWithTriangleInfo = static_cast<float>(TIME.EndTimeStamp("Fill cells with triangle info"));
}

void SDF::MouseClick(const double MouseX, const double MouseY, const glm::mat4 TransformMat)
{
	SelectedCell = glm::vec3(-1.0);

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

				// FIX ME
				FEAABB FinalAABB = Data[i][j][k].AABB/*.Transform(TransformMat).Transform(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix())*/;
				if (FinalAABB.RayIntersect(ENGINE.GetCamera()->GetPosition(), mouseRay(MouseX, MouseY, ENGINE.GetCamera()), DistanceToCell))
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

void SDF::FillMeshWithUserData()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	TIME.BeginTimeStamp("FillMeshWithUserData");

	std::vector<int> TrianglesRugosityCount;
	TrianglesUserData.resize(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size());
	TrianglesRugosityCount.resize(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size());

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
					TrianglesUserData[TriangleIndex] += static_cast<float>(Data[i][j][k].UserData);
				}
			}
		}
	}

	for (size_t i = 0; i < TrianglesUserData.size(); i++)
	{
		TrianglesUserData[i] /= TrianglesRugosityCount[i];
	}

	TimeTakenToFillMeshWithUserData = static_cast<float>(TIME.EndTimeStamp("FillMeshWithUserData"));
}

void SDF::AddLinesOfSDF()
{
	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				bool bNeedToRender = false;
				Data[i][j][k].bWasRenderedLastFrame = false;

				if (!Data[i][j][k].TrianglesInCell.empty() || RenderingMode == 2)
					bNeedToRender = true;

				if (bNeedToRender)
				{
					glm::vec3 color = glm::vec3(0.1f, 0.6f, 0.1f);
					if (Data[i][j][k].bSelected)
						color = glm::vec3(0.9f, 0.1f, 0.1f);

					// FIX ME
					//LINE_RENDERER.RenderAABB(Data[i][j][k].AABB.Transform(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix()), color);

					Data[i][j][k].bWasRenderedLastFrame = true;

					/*if (bShowTrianglesInCells && Data[i][j][k].bSelected)
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
					}*/
				}
			}
		}
	}
}

void SDF::UpdateRenderedLines()
{
#ifndef NEW_LINES
	// FIX ME
	//LINE_RENDERER.clearAll();
	if (RenderingMode != 0)
		AddLinesOfSDF();
	// FIX ME
	//LINE_RENDERER.SyncWithGPU();
#endif
}

void SDF::RunOnAllNodes(std::function<void(SDFNode* currentNode)> Func)
{
	if (Func == nullptr)
		return;

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				Func(&Data[i][j][k]);
			}
		}
	}
}