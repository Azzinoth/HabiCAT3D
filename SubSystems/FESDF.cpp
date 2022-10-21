#include "FESDF.h"
using namespace FocalEngine;

glm::dvec3 mouseRay(double mouseX, double mouseY, FEFreeCamera* currentCamera)
{
	int W, H;
	APPLICATION.GetWindowSize(&W, &H);

	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / W - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / H;

	glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(currentCamera->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(currentCamera->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

double SDF::TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC)
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

bool intersectWithTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& triangleVertices, float& distance)
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

	float determinant0 = glm::determinant(glm::mat3(a, b, c,
		d, e, f,
		g, h, j));

	float determinant1 = glm::determinant(glm::mat3(k, b, c,
		l, e, f,
		m, h, j));

	float t = determinant1 / determinant0;

	float determinant2 = glm::determinant(glm::mat3(a, k, c,
		d, l, f,
		g, m, j));
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
		//Point4 p = v1 + u * (v2 - v1) + v * (v3 - v1);
		//hit = Hit(p, this->N, this, t);

		distance = t;
		return true;
	}

	return false;
}

std::vector<triangleData> SDF::getTrianglesData(FEMesh* mesh)
{
	std::vector<triangleData> result;

	if (mesh == nullptr)
		return result;

	std::vector<float> FEVertices;
	FEVertices.resize(mesh->getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getPositionsBufferID(), 0, sizeof(float) * FEVertices.size(), FEVertices.data()));

	std::vector<float> FENormals;
	FENormals.resize(mesh->getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getNormalsBufferID(), 0, sizeof(float) * FENormals.size(), FENormals.data()));

	std::vector<int> FEIndices;
	FEIndices.resize(mesh->getIndicesCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getIndicesBufferID(), 0, sizeof(int) * FEIndices.size(), FEIndices.data()));

	for (size_t i = 0; i < FEIndices.size(); i += 3)
	{
		int vertexPosition = FEIndices[i] * 3;
		glm::vec3 firstVertex = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		vertexPosition = FEIndices[i + 1] * 3;
		glm::vec3 secondVertex = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		vertexPosition = FEIndices[i + 2] * 3;
		glm::vec3 thirdVertex = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		glm::vec3 currentCentroid = (firstVertex + secondVertex + thirdVertex) / 3.0f;

		// We are taking index of last vertex because all verticies of triangle should have same normal.
		glm::vec3 triangleNormal = glm::vec3(FENormals[vertexPosition], FENormals[vertexPosition + 1], FENormals[vertexPosition + 2]);

		// First triangle side lenght
		float firstSideLenght = glm::length(firstVertex - secondVertex);
		float secondSideLenght = glm::length(firstVertex - thirdVertex);
		float thirdSideLenght = glm::length(secondVertex - thirdVertex);

		triangleData data;
		data.centroid = currentCentroid;
		data.normal = triangleNormal;
		data.maxSideLength = std::max(std::max(firstSideLenght, secondSideLenght), thirdSideLenght);
		result.push_back(data);
	}

	return result;
}

SDF::SDF()
{

}

SDF::SDF(FEMesh* mesh, int dimentions, FEAABB AABB, FEFreeCamera* camera)
{
	if (dimentions < 1 || dimentions > 4096)
		return;

	// If dimentions is not power of 2, we can't continue.
	if (log2(dimentions) != int(log2(dimentions)))
		return;

	TIME.BeginTimeStamp("SDF Generation");

	currentCamera = camera;
	this->mesh = mesh;

	glm::vec3 center = AABB.getCenter();
	FEAABB SDFAABB = FEAABB(center - glm::vec3(AABB.getSize() / 2.0f), center + glm::vec3(AABB.getSize() / 2.0f));

	data.resize(dimentions);
	for (size_t i = 0; i < dimentions; i++)
	{
		data[i].resize(dimentions);
		for (size_t j = 0; j < dimentions; j++)
		{
			data[i][j].resize(dimentions);
		}
	}

	glm::vec3 start = SDFAABB.getMin();
	glm::vec3 currentAABBMin;
	float cellSize = SDFAABB.getSize() / dimentions;

	/*std::vector<triangleData> centroids = getTrianglesData(mesh);
	averageNormal = glm::vec3(0.0f);
	for (size_t i = 0; i < centroids.size(); i++)
	{
		averageNormal += centroids[i].normal;
	}
	averageNormal /= centroids.size();*/

	for (size_t i = 0; i < dimentions; i++)
	{
		for (size_t j = 0; j < dimentions; j++)
		{
			for (size_t k = 0; k < dimentions; k++)
			{
				currentAABBMin = start + glm::vec3(cellSize * i, cellSize * j, cellSize * k);

				data[i][j][k].AABB = FEAABB(currentAABBMin, currentAABBMin + glm::vec3(cellSize));

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

int DimensionsToPOWDimentions(int Dimentions)
{
	for (size_t i = 0; i < 12; i++)
	{
		if (Dimentions > pow(2.0, i))
			continue;

		if (Dimentions <= pow(2.0, i))
			return pow(2.0, i);
	}

	return 0;
}

void SDF::Init(FEMesh* mesh, int dimensions, FEAABB AABB, FEFreeCamera* camera, float ResolutionInM)
{
	TIME.BeginTimeStamp("SDF Generation");

	currentCamera = camera;
	this->mesh = mesh;

	glm::vec3 center = AABB.getCenter();

	//ResolutionInM = 1.0f;
	int MinDimensions = AABB.getSize() / ResolutionInM;
	dimensions = DimensionsToPOWDimentions(MinDimensions);

	if (dimensions < 1 || dimensions > 4096)
		return;

	// If dimensions is not power of 2, we can't continue.
	if (log2(dimensions) != int(log2(dimensions)))
		return;

	data.resize(dimensions);
	for (size_t i = 0; i < dimensions; i++)
	{
		data[i].resize(dimensions);
		for (size_t j = 0; j < dimensions; j++)
		{
			data[i][j].resize(dimensions);
		}
	}

	FEAABB SDFAABB;
	if (ResolutionInM != 0.0f)
	{
		SDFAABB = FEAABB(center - glm::vec3(ResolutionInM * dimensions / 2.0f), center + glm::vec3(ResolutionInM * dimensions / 2.0f));
	}
	else
	{
		SDFAABB = FEAABB(center - glm::vec3(AABB.getSize() / 2.0f), center + glm::vec3(AABB.getSize() / 2.0f));
	}

	glm::vec3 start = SDFAABB.getMin();
	glm::vec3 currentAABBMin;
	float cellSize = SDFAABB.getSize() / dimensions;

	for (size_t i = 0; i < dimensions; i++)
	{
		for (size_t j = 0; j < dimensions; j++)
		{
			for (size_t k = 0; k < dimensions; k++)
			{
				currentAABBMin = start + glm::vec3(cellSize * i, cellSize * j, cellSize * k);

				data[i][j][k].AABB = FEAABB(currentAABBMin, currentAABBMin + glm::vec3(cellSize));

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

void SDF::fillCellsWithTriangleInfo()
{
	TIME.BeginTimeStamp("Fill cells with triangle info");

	float cellSize = data[0][0][0].AABB.getSize();
	glm::vec3 gridMin = data[0][0][0].AABB.getMin();
	glm::vec3 gridMax = data[data.size() - 1][data.size() - 1][data.size() - 1].AABB.getMax();

	float distance = 0.0f;
	debugTotalTrianglesInCells = 0;

	for (size_t l = 0; l < mesh->Triangles.size(); l++)
	{
		FEAABB triangleAABB = FEAABB(mesh->Triangles[l]);

		int XBegin = 0;
		int XEnd = data.size();

		distance = sqrt(pow(triangleAABB.getMin().x - gridMin.x, 2.0));
		XBegin = int(distance / cellSize) - 1;
		if (XBegin < 0)
			XBegin = 0;

		distance = sqrt(pow(triangleAABB.getMax().x - gridMax.x, 2.0));
		XEnd -= int(distance / cellSize);
		XEnd++;
		if (XEnd > data.size())
			XEnd = data.size();

		for (size_t i = XBegin; i < XEnd; i++)
		{
			int YBegin = 0;
			int YEnd = data.size();

			distance = sqrt(pow(triangleAABB.getMin().y - gridMin.y, 2.0));
			YBegin = int(distance / cellSize) - 1;
			if (YBegin < 0)
				YBegin = 0;

			distance = sqrt(pow(triangleAABB.getMax().y - gridMax.y, 2.0));
			YEnd -= int(distance / cellSize);
			YEnd++;
			if (YEnd > data.size())
				YEnd = data.size();

			for (size_t j = YBegin; j < YEnd; j++)
			{
				int ZBegin = 0;
				int ZEnd = data.size();

				distance = sqrt(pow(triangleAABB.getMin().z - gridMin.z, 2.0));
				ZBegin = int(distance / cellSize) - 1;
				if (ZBegin < 0)
					ZBegin = 0;

				distance = sqrt(pow(triangleAABB.getMax().z - gridMax.z, 2.0));
				ZEnd -= int(distance / cellSize);
				ZEnd++;
				if (ZEnd > data.size())
					ZEnd = data.size();

				for (size_t k = ZBegin; k < ZEnd; k++)
				{
					if (data[i][j][k].AABB.AABBIntersect(triangleAABB))
					{
						data[i][j][k].trianglesInCell.push_back(l);
						debugTotalTrianglesInCells++;
					}
				}
			}
		}
	}

	TimeTookFillCellsWithTriangleInfo = TIME.EndTimeStamp("Fill cells with triangle info");
}

void SDF::calculateRugosity()
{
	if (mesh->Triangles.empty())
		mesh->fillTrianglesData();

	TIME.BeginTimeStamp("Calculate rugosity");

	for (size_t i = 0; i < data.size(); i++)
	{
		for (size_t j = 0; j < data[i].size(); j++)
		{
			for (size_t k = 0; k < data[i][j].size(); k++)
			{
				calculateCellRugosity(&data[i][j][k]);
			}
		}
	}

	TimeTookCalculateRugosity = TIME.EndTimeStamp("Calculate rugosity");
}

void SDF::mouseClick(double mouseX, double mouseY, glm::mat4 transformMat)
{
	selectedCell = glm::vec3(0.0);

	for (size_t i = 0; i < data.size(); i++)
	{
		for (size_t j = 0; j < data[i].size(); j++)
		{
			for (size_t k = 0; k < data[i][j].size(); k++)
			{
				data[i][j][k].selected = false;
			}
		}
	}

	float distanceToCell = 999999.0f;
	float lastDistanceToCell = 999999.0f;
	for (size_t i = 0; i < data.size(); i++)
	{
		for (size_t j = 0; j < data[i].size(); j++)
		{
			for (size_t k = 0; k < data[i][j].size(); k++)
			{
				data[i][j][k].selected = false;
				if (!data[i][j][k].wasRenderedLastFrame)
					continue;

				FEAABB FinalAABB = data[i][j][k].AABB.transform(transformMat).transform(mesh->Position->getTransformMatrix());
				if (FinalAABB.rayIntersect(currentCamera->getPosition(), mouseRay(mouseX, mouseY, currentCamera), distanceToCell))
				{
					if (lastDistanceToCell > distanceToCell)
					{
						lastDistanceToCell = distanceToCell;
						selectedCell = glm::vec3(i, j, k);
					}
				}
			}
		}
	}

	if (distanceToCell != 999999.0f)
		data[selectedCell.x][selectedCell.y][selectedCell.z].selected = true;
}

void SDF::fillMeshWithRugosityData()
{
	if (mesh == nullptr)
		return;

	TIME.BeginTimeStamp("FillMeshWithRugosityData");

	int posSize = mesh->getPositionsCount();
	float* positions = new float[posSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getPositionsBufferID(), 0, sizeof(float) * posSize, positions));

	int indexSize = mesh->getIndicesCount();
	int* indices = new int[indexSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getIndicesBufferID(), 0, sizeof(int) * indexSize, indices));

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

	int colorSize = posSize;
	float* colors = new float[posSize];
	int vertexIndex = 0;

	

	auto setColorOfVertex = [&](int index, glm::vec3 color) {
		colors[index * 3] = color.x;
		colors[index * 3 + 1] = color.y;
		colors[index * 3 + 2] = color.z;
	};

	auto getVertexOfFace = [&](int faceIndex) {
		std::vector<int> result;
		result.push_back(indexVector[faceIndex * 3]);
		result.push_back(indexVector[faceIndex * 3 + 1]);
		result.push_back(indexVector[faceIndex * 3 + 2]);

		return result;
	};

	auto setColorOfFace = [&](int faceIndex, glm::vec3 color) {
		std::vector<int> faceVertex = getVertexOfFace(faceIndex);

		for (size_t i = 0; i < faceVertex.size(); i++)
		{
			setColorOfVertex(faceVertex[i], color);
		}
	};














	std::vector<int> TrianglesRugosityCount;
	TrianglesRugosity.resize(mesh->Triangles.size());
	TrianglesRugosityCount.resize(mesh->Triangles.size());


	for (size_t i = 0; i < data.size(); i++)
	{
		for (size_t j = 0; j < data[i].size(); j++)
		{
			for (size_t k = 0; k < data[i][j].size(); k++)
			{
				for (size_t l = 0; l < data[i][j][k].trianglesInCell.size(); l++)
				{
					int TriangleIndex = data[i][j][k].trianglesInCell[l];
					TrianglesRugosityCount[TriangleIndex]++;
					TrianglesRugosity[TriangleIndex] += data[i][j][k].rugosity;
				}
			}
		}
	}


	
	/*std::vector<float> TrianglesRugositySorted = TrianglesRugosity;
	std::sort(TrianglesRugositySorted.begin(), TrianglesRugositySorted.end(), [](float lhs,
		float rhs) { return rhs < lhs; });

	int numbOfPoints = int(TrianglesRugositySorted.size() * 0.05f);
	float mean = 0.0f;
	for (size_t i = 0; i < numbOfPoints; i++)
	{
		mean += TrianglesRugositySorted[i];
	}

	if (numbOfPoints != 0)
		mean /= numbOfPoints;*/

	double minRugorsity = DBL_MAX;
	double maxRugorsity = -DBL_MAX;
	for (size_t i = 0; i < TrianglesRugosity.size(); i++)
	{
		TrianglesRugosity[i] /= TrianglesRugosityCount[i];

		if (TrianglesRugosity[i] > maxRugorsity)
			maxRugorsity = TrianglesRugosity[i];

		if (TrianglesRugosity[i] < minRugorsity)
			minRugorsity = TrianglesRugosity[i];
	}

	//maxRugorsity = 3.0f;
	//maxRugorsity = mean;

	mesh->minRugorsity = minRugorsity;
	mesh->maxRugorsity = maxRugorsity;
	mesh->minVisibleRugorsity = 1.0;
	mesh->maxVisibleRugorsity = maxRugorsity;

	


	//bool bFirstTime = true;

	// addRugosityToVertices
	//std::vector<float> oldData;
	//if (!mesh->rugosityData.empty())
	//{
		//bFirstTime = false;
		//mesh->jitteredData.push_back(mesh->rugosityData);

		//oldData = mesh->rugosityData;
	//}

	/*mesh->rugosityData.resize(posSize);
	auto setRugosityOfVertex = [&](int index, float value) {
		mesh->rugosityData[index * 3] = value;
		mesh->rugosityData[index * 3 + 1] = value;
		mesh->rugosityData[index * 3 + 2] = value;
	};


	auto setRugosityOfFace = [&](int faceIndex, float value) {
		std::vector<int> faceVertex = getVertexOfFace(faceIndex);

		for (size_t i = 0; i < faceVertex.size(); i++)
		{
			setRugosityOfVertex(faceVertex[i], value);
		}
	};

	for (size_t i = 0; i < mesh->Triangles.size(); i++)
	{
		setRugosityOfFace(i, TrianglesRugosity[i]);
	}*/








	/*std::vector<float> tempRugosity = mesh->rugosityData;
	if (!mesh->jitteredData.empty())
	{
		for (size_t i = 0; i < mesh->rugosityData.size(); i++)
		{
			for (size_t j = 0; j < mesh->jitteredData.size(); j++)
			{
				mesh->rugosityData[i] += mesh->jitteredData[j][i];
			}

			mesh->rugosityData[i] /= mesh->jitteredData.size() + 1;
		}
	}
	mesh->jitteredData.push_back(tempRugosity);*/

	/*if (bFinalJitter)
	{
		FE_GL_ERROR(glBindVertexArray(mesh->vaoID));

		mesh->rugosityBufferID = 0;
		FE_GL_ERROR(glGenBuffers(1, &mesh->rugosityBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, mesh->colorBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->rugosityData.size(), mesh->rugosityData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(8, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}*/

	TimeTookFillMeshWithRugosityData = TIME.EndTimeStamp("FillMeshWithRugosityData");
}

void SDF::calculateCellRugosity(SDFNode* node, std::string* debugInfo)
{
	if (node->trianglesInCell.size() == 0)
		return;

	std::vector<float> originalAreas;
	float totalArea = 0.0f;
	for (size_t l = 0; l < node->trianglesInCell.size(); l++)
	{
		std::vector<glm::vec3> currentTriangle = mesh->Triangles[node->trianglesInCell[l]];
		double originalArea = TriangleArea(currentTriangle[0], currentTriangle[1], currentTriangle[2]);
		originalAreas.push_back(originalArea);
		totalArea += originalArea;
	}

	auto CalculateCellRugosity = [&](glm::vec3 PointOnPlane, glm::vec3 PlaneNormal) {
		double Result = 0.0;
		FEPlane* ProjectionPlane = new FEPlane(PointOnPlane, PlaneNormal);

		std::vector<float> rugosities;
		for (size_t l = 0; l < node->trianglesInCell.size(); l++)
		{
			std::vector<glm::vec3> currentTriangle = mesh->Triangles[node->trianglesInCell[l]];

			glm::vec3 aProjection = ProjectionPlane->ProjectPoint(currentTriangle[0]);
			glm::vec3 bProjection = ProjectionPlane->ProjectPoint(currentTriangle[1]);
			glm::vec3 cProjection = ProjectionPlane->ProjectPoint(currentTriangle[2]);

			double projectionArea = TriangleArea(aProjection, bProjection, cProjection);

			rugosities.push_back(originalAreas[l] / projectionArea);

			if (originalAreas[l] == 0.0 || projectionArea == 0.0)
				rugosities.back() = 1.0f;

			if (rugosities.back() > 100.0f)
				rugosities.back() = 100.0f;
		}

		// Weighted by triangle area rugosity.
		for (size_t l = 0; l < node->trianglesInCell.size(); l++)
		{
			float currentTriangleCoef = originalAreas[l] / totalArea;
			Result += rugosities[l] * currentTriangleCoef;

			if (isnan(Result))
				Result = 1.0f;
		}

		delete ProjectionPlane;
		return Result;
	};

	// ******* Getting average normal *******
	for (size_t l = 0; l < node->trianglesInCell.size(); l++)
	{
		std::vector<glm::vec3> currentTriangle = mesh->Triangles[node->trianglesInCell[l]];
		std::vector<glm::vec3> currentTriangleNormals = mesh->TrianglesNormals[node->trianglesInCell[l]];

		if (bWeightedNormals)
		{
			float currentTriangleCoef = originalAreas[l] / totalArea;

			node->averageCellNormal += currentTriangleNormals[0] * currentTriangleCoef;
			node->averageCellNormal += currentTriangleNormals[1] * currentTriangleCoef;
			node->averageCellNormal += currentTriangleNormals[2] * currentTriangleCoef;
		}
		else
		{
			node->averageCellNormal += currentTriangleNormals[0];
			node->averageCellNormal += currentTriangleNormals[1];
			node->averageCellNormal += currentTriangleNormals[2];
		}

		node->CellTrianglesCentroid += currentTriangle[0];
		node->CellTrianglesCentroid += currentTriangle[1];
		node->CellTrianglesCentroid += currentTriangle[2];
	}

	if (!bWeightedNormals)
		node->averageCellNormal /= node->trianglesInCell.size() * 3;

	if (bNormalizedNormals)
		node->averageCellNormal = glm::normalize(node->averageCellNormal);
	node->CellTrianglesCentroid /= node->trianglesInCell.size() * 3;
	// ******* Getting average normal END *******

	if (bFindSmallestRugosity)
	{
		std::unordered_map<int, float> TriangleNormalsToRugosity;
		TriangleNormalsToRugosity[-1] = CalculateCellRugosity(node->CellTrianglesCentroid, node->averageCellNormal);

		for (int i = 0; i < node->trianglesInCell.size(); i++)
		{
			TriangleNormalsToRugosity[i] = CalculateCellRugosity(mesh->Triangles[node->trianglesInCell[i]][2], mesh->TrianglesNormals[node->trianglesInCell[i]][2]);
		}

		double Min = FLT_MAX;
		double Max = -FLT_MAX;
		auto MapIt = TriangleNormalsToRugosity.begin();
		while (MapIt != TriangleNormalsToRugosity.end())
		{
			if (MapIt->second < Min)
			{
				Min = MapIt->second;
				node->rugosity = Min;
			}

			MapIt++;
		}
	}
	else
	{
		node->rugosity = CalculateCellRugosity(node->CellTrianglesCentroid, node->averageCellNormal);
		if (isnan(node->rugosity))
			node->rugosity = 1.0f;
	}
}

void SDF::addLinesOFSDF()
{
	for (size_t i = 0; i < data.size(); i++)
	{
		for (size_t j = 0; j < data[i].size(); j++)
		{
			for (size_t k = 0; k < data[i][j].size(); k++)
			{
				bool render = false;
				data[i][j][k].wasRenderedLastFrame = false;

				if (!data[i][j][k].trianglesInCell.empty() || RenderingMode == 2)
					render = true;

				if (render)
				{
					glm::vec3 color = glm::vec3(0.1f, 0.6f, 0.1f);
					if (data[i][j][k].selected)
						color = glm::vec3(0.9f, 0.1f, 0.1f);

					LINE_RENDERER.RenderAABB(data[i][j][k].AABB.transform(mesh->Position->getTransformMatrix()), color);

					data[i][j][k].wasRenderedLastFrame = true;

					if (showTrianglesInCells && data[i][j][k].selected)
					{
						for (size_t l = 0; l < data[i][j][k].trianglesInCell.size(); l++)
						{
							auto currentTriangle = mesh->Triangles[data[i][j][k].trianglesInCell[l]];

							std::vector<glm::vec3> TranformedTrianglePoints = currentTriangle;
							for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
							{
								TranformedTrianglePoints[j] = mesh->Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
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
		addLinesOFSDF();
	LINE_RENDERER.SyncWithGPU();
}