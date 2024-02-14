#include "FEMesh.h"
using namespace FocalEngine;

FEMesh::FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, std::string Name)
{
	vaoID = VaoID;
	vertexCount = VertexCount;
	vertexAttributes = VertexBuffersTypes;
}

FEMesh::~FEMesh()
{
	FE_GL_ERROR(glDeleteVertexArrays(1, &vaoID));
}

GLuint FEMesh::GetVaoID() const
{
	return vaoID;
}

GLuint FEMesh::getVertexCount() const
{
	return vertexCount;
}

GLuint FEMesh::getIndicesBufferID() const
{
	return indicesBufferID;
}

GLuint FEMesh::getIndicesCount() const
{
	return indicesCount;
}

GLuint FEMesh::getTriangleCount() const
{
	return vertexCount / 3;
}

GLuint FEMesh::getPositionsBufferID() const
{
	return positionsBufferID;
}

GLuint FEMesh::getPositionsCount() const
{
	return positionsCount;
}

GLuint FEMesh::getNormalsBufferID() const
{
	return normalsBufferID;
}

GLuint FEMesh::getNormalsCount() const
{
	return normalsCount;
}

GLuint FEMesh::getTangentsBufferID() const
{
	return tangentsBufferID;
}

GLuint FEMesh::getTangentsCount() const
{
	return tangentsCount;
}

GLuint FEMesh::getUVBufferID() const
{
	return UVBufferID;
}

GLuint FEMesh::getUVCount() const
{
	return UVCount;
}

GLuint FEMesh::getColorBufferID() const
{
	return colorBufferID;
}

GLuint FEMesh::getColorCount() const
{
	return colorCount;
}

void FEMesh::addColorToVertices(float* colors, int colorSize)
{
	if (colors == nullptr || colorSize <= 0)
		return;

	FE_GL_ERROR(glBindVertexArray(vaoID));

	colorCount = colorSize;
	colorBufferID = 0;
	vertexAttributes |= FE_COLOR;
	FE_GL_ERROR(glGenBuffers(1, &colorBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, colorBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colorCount, colors, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(1/*FE_COLOR*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void FEMesh::addSegmentsColorToVertices(float* colors, int colorSize)
{
	if (colors == nullptr || colorSize <= 0)
		return;

	FE_GL_ERROR(glBindVertexArray(vaoID));

	segmentsColorsCount = colorSize;
	segmentsColorsBufferID = 0;
	vertexAttributes |= FE_SEGMENTS_COLORS;
	FE_GL_ERROR(glGenBuffers(1, &segmentsColorsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, segmentsColorsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * segmentsColorsCount, colors, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(7/*FE_SEGMENTS_COLORS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

bool FEMesh::intersectWithTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& triangleVertices, float& distance, glm::vec3* HitPoint)
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

	const glm::mat3 temp0 = glm::mat3(a, b, c,
	                                  d, e, f,
	                                  g, h, j);

	const float determinant0 = glm::determinant(temp0);

	const glm::mat3 temp1 = glm::mat3(k, b, c,
	                                  l, e, f,
	                                  m, h, j);

	const float determinant1 = glm::determinant(temp1);

	const float t = determinant1 / determinant0;


	const glm::mat3 temp2 = glm::mat3(a, k, c,
	                                  d, l, f,
	                                  g, m, j);

	const float determinant2 = glm::determinant(temp2);
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
		if (HitPoint != nullptr)
			*HitPoint = triangleVertices[0] + u * (triangleVertices[1] - triangleVertices[0]) + v * (triangleVertices[2] - triangleVertices[0]);

		distance = t;
		return true;
	}

	return false;
}

bool FEMesh::SelectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera)
{
	float currentDistance = 0.0f;
	float lastDistance = 9999.0f;

	int TriangeIndex = -1;
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.clear();

	for (int i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		const bool hit = intersectWithTriangle(currentCamera->GetPosition(), MouseRay, TranformedTrianglePoints, currentDistance);

		if (hit && currentDistance < lastDistance)
		{
			lastDistance = currentDistance;
			TriangeIndex = i;
		}
	}

	if (TriangeIndex != -1)
	{
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.push_back(TriangeIndex);
		return true;
	}

	return false;
}

glm::vec3 FEMesh::IntersectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera)
{
	float currentDistance = 0.0f;
	float lastDistance = 9999.0f;

	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		glm::vec3 HitPosition;
		//bool hit = intersectWithTriangle(currentCamera->getPosition(), MouseRay, Triangles[i], currentDistance, &HitPosition);
		const bool hit = intersectWithTriangle(currentCamera->GetPosition(), MouseRay, TranformedTrianglePoints, currentDistance, &HitPosition);

		if (hit && currentDistance < lastDistance)
		{
			lastDistance = currentDistance;

			const glm::mat4 Inverse = glm::inverse(Position->getTransformMatrix());
			return Inverse * glm::vec4(HitPosition, 1.0f);
		}
	}

	return glm::vec3(0.0f);
}

bool FEMesh::SelectTrianglesInRadius(glm::dvec3 MouseRay, FEBasicCamera* currentCamera, float Radius)
{
	bool Result = false;
	SelectTriangle(MouseRay, currentCamera);

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() == 0)
		return Result;

	LastMeasuredRugosityAreaRadius = Radius;
	LastMeasuredRugosityAreaCenter = Position->getTransformMatrix() * glm::vec4(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesCentroids[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]], 1.0f);

	const glm::vec3 FirstSelectedTriangleCentroid = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesCentroids[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]];

	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		if (i == COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0])
			continue;

		if (glm::distance(FirstSelectedTriangleCentroid, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesCentroids[i]) <= Radius)
		{
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.push_back(static_cast<int>(i));
			Result = true;
		}
	}

	return Result;
}

bool FEMesh::SelectTrianglesInRadius(glm::vec3 CenterPoint, float Radius)
{
	bool Result = false;

	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.clear();

	LastMeasuredRugosityAreaRadius = Radius;
	LastMeasuredRugosityAreaCenter = Position->getTransformMatrix() * glm::vec4(CenterPoint, 1.0f);

	const glm::vec3 FirstSelectedTriangleCentroid = CenterPoint;

	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		if (glm::distance(FirstSelectedTriangleCentroid, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesCentroids[i]) <= Radius)
		{
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.push_back(static_cast<int>(i));
			Result = true;
		}
	}

	return Result;
}

void FEMesh::ComplexityMetricDataToGPU(int LayerIndex, int GPULayerIndex)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	if (LayerIndex < 0 || LayerIndex >= COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
		return;

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.empty())
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].FillRawData();

	FE_GL_ERROR(glBindVertexArray(vaoID));

	if (GPULayerIndex == 0)
	{
		FirstLayerBufferID = 0;
		vertexAttributes |= FE_RUGOSITY_FIRST;
		FE_GL_ERROR(glGenBuffers(1, &FirstLayerBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, FirstLayerBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.size(), COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(7, 3, GL_FLOAT, false, 0, nullptr));
	}
	else
	{
		SecondLayerBufferID = 0;
		vertexAttributes |= FE_RUGOSITY_SECOND;
		FE_GL_ERROR(glGenBuffers(1, &SecondLayerBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, SecondLayerBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.size(), COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(8, 3, GL_FLOAT, false, 0, nullptr));
	}

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}