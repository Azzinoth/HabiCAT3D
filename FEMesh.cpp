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

GLuint FEMesh::getVaoID() const
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

	float a = RayDirection[0];
	float b = triangleVertices[0][0] - triangleVertices[1][0];
	float c = triangleVertices[0][0] - triangleVertices[2][0];

	float d = RayDirection[1];
	float e = triangleVertices[0][1] - triangleVertices[1][1];
	float f = triangleVertices[0][1] - triangleVertices[2][1];

	float g = RayDirection[2];
	float h = triangleVertices[0][2] - triangleVertices[1][2];
	float j = triangleVertices[0][2] - triangleVertices[2][2];

	float k = triangleVertices[0][0] - RayOrigin[0];
	float l = triangleVertices[0][1] - RayOrigin[1];
	float m = triangleVertices[0][2] - RayOrigin[2];

	glm::mat3 temp0 = glm::mat3(a, b, c,
		d, e, f,
		g, h, j);

	float determinant0 = glm::determinant(temp0);

	glm::mat3 temp1 = glm::mat3(k, b, c,
		l, e, f,
		m, h, j);

	float determinant1 = glm::determinant(temp1);

	float t = determinant1 / determinant0;


	glm::mat3 temp2 = glm::mat3(a, k, c,
		d, l, f,
		g, m, j);

	float determinant2 = glm::determinant(temp2);
	float u = determinant2 / determinant0;

	float determinant3 = glm::determinant(glm::mat3(a, b, k,
		d, e, l,
		g, h, m));

	float v = determinant3 / determinant0;

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

void FEMesh::fillTrianglesData()
{
	Triangles.clear();
	TrianglesNormals.clear();
	TrianglesArea.clear();
	TrianglesCentroids.clear();
	TotalArea = 0;

	std::vector<float> FEVertices;
	FEVertices.resize(getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(getPositionsBufferID(), 0, sizeof(float) * FEVertices.size(), FEVertices.data()));

	std::vector<int> FEIndices;
	FEIndices.resize(getIndicesCount());
	FE_GL_ERROR(glGetNamedBufferSubData(getIndicesBufferID(), 0, sizeof(int) * FEIndices.size(), FEIndices.data()));

	std::vector<float> FENormals;
	if (getNormalsCount() > 0)
	{
		FENormals.resize(getNormalsCount());
		FE_GL_ERROR(glGetNamedBufferSubData(getNormalsBufferID(), 0, sizeof(int) * FENormals.size(), FENormals.data()));
	}

	std::vector<glm::vec3> triangle;
	triangle.resize(3);

	for (size_t i = 0; i < FEIndices.size(); i += 3)
	{
		int vertexPosition = FEIndices[i] * 3;
		triangle[0] = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		vertexPosition = FEIndices[i + 1] * 3;
		triangle[1] = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		vertexPosition = FEIndices[i + 2] * 3;
		triangle[2] = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		Triangles.push_back(triangle);
		TrianglesArea.push_back(TriangleArea(triangle[0], triangle[1], triangle[2]));
		TotalArea += float(TrianglesArea.back());

		TrianglesCentroids.push_back((triangle[0] + triangle[1] + triangle[2]) / 3.0f);

		if (!FENormals.empty())
		{
			vertexPosition = FEIndices[i] * 3;
			triangle[0] = glm::vec3(FENormals[vertexPosition], FENormals[vertexPosition + 1], FENormals[vertexPosition + 2]);

			vertexPosition = FEIndices[i + 1] * 3;
			triangle[1] = glm::vec3(FENormals[vertexPosition], FENormals[vertexPosition + 1], FENormals[vertexPosition + 2]);

			vertexPosition = FEIndices[i + 2] * 3;
			triangle[2] = glm::vec3(FENormals[vertexPosition], FENormals[vertexPosition + 1], FENormals[vertexPosition + 2]);

			TrianglesNormals.push_back(triangle);
		}
	}
}

void FEMesh::SelectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera)
{
	float currentDistance = 0.0f;
	float lastDistance = 9999.0f;

	int TriangeIndex = -1;
	TriangleSelected.clear();

	if (Triangles.empty())
		fillTrianglesData();

	for (int i = 0; i < Triangles.size(); i++)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		bool hit = intersectWithTriangle(currentCamera->getPosition(), MouseRay, TranformedTrianglePoints, currentDistance);

		if (hit && currentDistance < lastDistance)
		{
			lastDistance = currentDistance;
			TriangeIndex = i;
		}
	}

	if (TriangeIndex != -1)
		TriangleSelected.push_back(TriangeIndex);
}

glm::vec3 FEMesh::IntersectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera)
{
	float currentDistance = 0.0f;
	float lastDistance = 9999.0f;
	glm::vec3 HitPosition;

	if (Triangles.empty())
		fillTrianglesData();

	for (size_t i = 0; i < Triangles.size(); i++)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		glm::vec3 HitPosition;
		//bool hit = intersectWithTriangle(currentCamera->getPosition(), MouseRay, Triangles[i], currentDistance, &HitPosition);
		bool hit = intersectWithTriangle(currentCamera->getPosition(), MouseRay, TranformedTrianglePoints, currentDistance, &HitPosition);

		if (hit && currentDistance < lastDistance)
		{
			lastDistance = currentDistance;

			glm::mat4 Inverse = glm::inverse(Position->getTransformMatrix());
			return Inverse * glm::vec4(HitPosition, 1.0f);
		}
	}
}

void FEMesh::SelectTrianglesInRadius(glm::dvec3 MouseRay, FEBasicCamera* currentCamera, float Radius)
{
	SelectTriangle(MouseRay, currentCamera);

	if (TriangleSelected.size() == 0)
		return;

	LastMeasuredRugosityAreaRadius = Radius;
	LastMeasuredRugosityAreaCenter = Position->getTransformMatrix() * glm::vec4(TrianglesCentroids[TriangleSelected[0]], 1.0f);

	glm::vec3 FirstSelectedTriangleCentroid = TrianglesCentroids[TriangleSelected[0]];

	for (size_t i = 0; i < Triangles.size(); i++)
	{
		if (i == TriangleSelected[0])
			continue;

		if (glm::distance(FirstSelectedTriangleCentroid, TrianglesCentroids[i]) <= Radius)
		{
			TriangleSelected.push_back(int(i));
		}
	}
}

void FEMesh::SelectTrianglesInRadius(glm::vec3 CenterPoint, float Radius)
{
	TriangleSelected.clear();

	LastMeasuredRugosityAreaRadius = Radius;
	LastMeasuredRugosityAreaCenter = Position->getTransformMatrix() * glm::vec4(CenterPoint, 1.0f);

	glm::vec3 FirstSelectedTriangleCentroid = CenterPoint;

	for (size_t i = 0; i < Triangles.size(); i++)
	{
		if (glm::distance(FirstSelectedTriangleCentroid, TrianglesCentroids[i]) <= Radius)
		{
			TriangleSelected.push_back(int(i));
		}
	}
}

void FEMesh::fillRugosityDataToGPU(int RugosityLayerIndex)
{
	int posSize = getPositionsCount();
	float* positions = new float[posSize];
	FE_GL_ERROR(glGetNamedBufferSubData(getPositionsBufferID(), 0, sizeof(float) * posSize, positions));

	int indexSize = getIndicesCount();
	int* indices = new int[indexSize];
	FE_GL_ERROR(glGetNamedBufferSubData(getIndicesBufferID(), 0, sizeof(int) * indexSize, indices));

	std::vector<float> positionsVector;
	for (size_t i = 0; i < posSize; i++)
	{
		positionsVector.push_back(positions[i]);
	}

	std::vector<int> indexVector;
	for (size_t i = 0; i < indexSize; i++)
	{
		indexVector.push_back(indices[i]);
	}

	delete positions;
	delete indices;

	rugosityData.resize(posSize);
	auto getVertexOfFace = [&](int faceIndex) {
		std::vector<int> result;
		result.push_back(indexVector[faceIndex * 3]);
		result.push_back(indexVector[faceIndex * 3 + 1]);
		result.push_back(indexVector[faceIndex * 3 + 2]);

		return result;
	};

	auto setRugosityOfVertex = [&](int index, float value) {
		rugosityData[index * 3] = value;
		rugosityData[index * 3 + 1] = value;
		rugosityData[index * 3 + 2] = value;
	};

	auto setRugosityOfFace = [&](int faceIndex, float value) {
		std::vector<int> faceVertex = getVertexOfFace(faceIndex);

		for (size_t i = 0; i < faceVertex.size(); i++)
		{
			setRugosityOfVertex(faceVertex[i], value);
		}
	};

	for (size_t i = 0; i < Triangles.size(); i++)
	{
		setRugosityOfFace(int(i), TrianglesRugosity[i]);
	}

	FE_GL_ERROR(glBindVertexArray(vaoID));

	if (RugosityLayerIndex == 0)
	{
		rugosityBufferID = 0;
		vertexAttributes |= FE_RUGOSITY_FIRST;
		FE_GL_ERROR(glGenBuffers(1, &rugosityBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, rugosityBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * rugosityData.size(), rugosityData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(8, 3, GL_FLOAT, false, 0, 0));
	}
	else
	{
		rugositySecondBufferID = 0;
		vertexAttributes |= FE_RUGOSITY_SECOND;
		FE_GL_ERROR(glGenBuffers(1, &rugositySecondBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, rugositySecondBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * rugosityData.size(), rugosityData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(9, 3, GL_FLOAT, false, 0, 0));
	}

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void FEMesh::UpdateAverageNormal()
{
	if (Triangles.empty())
		fillTrianglesData();

	AverageNormal = glm::vec3();

	std::vector<float> originalAreas;
	float totalArea = 0.0f;
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		double originalArea = TriangleArea(Triangles[i][0], Triangles[i][1], Triangles[i][2]);
		originalAreas.push_back(float(originalArea));
		totalArea += originalArea;
	}

	// ******* Geting average normal *******
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		float currentTriangleCoef = originalAreas[i] / totalArea;

		AverageNormal += TrianglesNormals[i][0] * currentTriangleCoef;
		AverageNormal += TrianglesNormals[i][1] * currentTriangleCoef;
		AverageNormal += TrianglesNormals[i][2] * currentTriangleCoef;
	}

	AverageNormal = glm::normalize(AverageNormal);
	
	MinHeight = DBL_MAX;
	MaxHeight = -DBL_MAX;
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			double CurrentHeight = glm::dot(glm::vec3(Position->getTransformMatrix() * glm::vec4(Triangles[i][j], 1.0)), AverageNormal);

			MinHeight = std::min(CurrentHeight, MinHeight);
			MaxHeight = std::max(CurrentHeight, MaxHeight);
		}
	}
}

glm::vec3 FEMesh::GetAverageNormal()
{
	return AverageNormal;
}

double FEMesh::TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC)
{
	double x1 = PointA.x;
	double x2 = PointB.x;
	double x3 = PointC.x;

	double y1 = PointA.y;
	double y2 = PointB.y;
	double y3 = PointC.y;

	double z1 = PointA.z;
	double z2 = PointB.z;
	double z3 = PointC.z;

	return 0.5 * sqrt(pow(x2 * y1 - x3 * y1 - x1 * y2 + x3 * y2 + x1 * y3 - x2 * y3, 2.0) +
		pow((x2 * z1) - (x3 * z1) - (x1 * z2) + (x3 * z2) + (x1 * z3) - (x2 * z3), 2.0) +
		pow((y2 * z1) - (y3 * z1) - (y1 * z2) + (y3 * z2) + (y1 * z3) - (y2 * z3), 2.0));
}