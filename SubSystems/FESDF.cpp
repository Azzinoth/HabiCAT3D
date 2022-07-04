#include "FESDF.h"
using namespace FocalEngine;

glm::dvec3 mouseRay(double mouseX, double mouseY, FEFreeCamera* currentCamera)
{
	int W, H;
	APPLICATION.getWindowSize(&W, &H);

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

double SDF::TriangleArea(glm::vec3 PointA, glm::vec3 PointB, glm::vec3 PointC)
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

SDF::SDF(FEMesh* mesh, int dimentions, FEAABB AABB, FEFreeCamera* camera)
{
	if (dimentions < 1 || dimentions > 4096)
		return;

	// If dimentions is not power of 2, we can't continue.
	if (log2(dimentions) != int(log2(dimentions)))
		return;

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

	std::vector<triangleData> centroids = getTrianglesData(mesh);
	averageNormal = glm::vec3(0.0f);
	for (size_t i = 0; i < centroids.size(); i++)
	{
		averageNormal += centroids[i].normal;
	}
	averageNormal /= centroids.size();

	for (size_t i = 0; i < dimentions; i++)
	{
		for (size_t j = 0; j < dimentions; j++)
		{
			for (size_t k = 0; k < dimentions; k++)
			{
				currentAABBMin = start + glm::vec3(cellSize * i, cellSize * j, cellSize * k);

				data[i][j][k].AABB = FEAABB(currentAABBMin, currentAABBMin + glm::vec3(cellSize));

				float minDistanceToCentroid = FLT_MAX;
				int centroidIndex = -1;
				glm::vec3 cellCenter = data[i][j][k].AABB.getCenter();
				for (size_t p = 0; p < centroids.size(); p++)
				{
					float currentDistance = glm::distance(centroids[p].centroid, cellCenter);
					if (currentDistance < minDistanceToCentroid)
					{
						minDistanceToCentroid = currentDistance;
						centroidIndex = p;
					}
				}

				if (centroidIndex != -1)
				{
					glm::vec3 vectorToCentroid = centroids[centroidIndex].centroid - cellCenter;

					// Test whether cell is inside or outside of mesh
					if (glm::dot(vectorToCentroid, centroids[centroidIndex].normal) >= 0)
					{
						minDistanceToCentroid = -minDistanceToCentroid;
					}

					// https://mathinsight.org/distance_point_plane
					// Normal of plane
					glm::vec3 N = centroids[centroidIndex].normal;
					// Point on plane
					glm::vec3 C = centroids[centroidIndex].centroid;
					glm::vec3 P = cellCenter;

					float D = -N.x * C.x - N.y * C.y - N.z * C.z;
					float distance = abs(N.x * P.x + N.y * P.y + N.z * P.z + D) / glm::length(N);

					//if (i == 30 && j == 2 && k == 28)
					//{
					//	int y = 0;
					//	y++;
					//}

					// Distance to plane could be a lot greater than distance to triangle
					data[i][j][k].distanceToTrianglePlane = distance;
					if (abs(minDistanceToCentroid) > centroids[centroidIndex].maxSideLength / 2.0f)
					{
						data[i][j][k].distanceToTrianglePlane = FLT_MAX;
						if (minDistanceToCentroid < 0.0f)
							minDistanceToCentroid = -minDistanceToCentroid;
					}
				}

				data[i][j][k].value = minDistanceToCentroid;
			}
		}
	}
}

void SDF::fillCellsWithTriangleInfo()
{
	if (mesh->Triangles.empty())
		mesh->fillTrianglesData();

	for (size_t l = 0; l < mesh->Triangles.size(); l++)
	{
		FEAABB triangleAABB = FEAABB(mesh->Triangles[l]);

		for (size_t i = 0; i < data.size(); i++)
		{
			for (size_t j = 0; j < data[i].size(); j++)
			{
				for (size_t k = 0; k < data[i][j].size(); k++)
				{
					if (data[i][j][k].AABB.AABBIntersect(triangleAABB))
					{
						data[i][j][k].trianglesInCell.push_back(l);
					}
				}
			}
		}
	}
}

void SDF::calculateRugosity()
{
	if (mesh->Triangles.empty())
		mesh->fillTrianglesData();

	for (size_t i = 0; i < data.size(); i++)
	{
		for (size_t j = 0; j < data[i].size(); j++)
		{
			for (size_t k = 0; k < data[i][j].size(); k++)
			{
				if (data[i][j][k].trianglesInCell.size() != 0)
				{
					for (size_t l = 0; l < data[i][j][k].trianglesInCell.size(); l++)
					{
						std::vector<glm::vec3> currentTriangle = mesh->Triangles[data[i][j][k].trianglesInCell[l]];
						std::vector<glm::vec3> currentTriangleNormals = mesh->TrianglesNormals[data[i][j][k].trianglesInCell[l]];

						data[i][j][k].averageCellNormal += currentTriangleNormals[0];
						data[i][j][k].averageCellNormal += currentTriangleNormals[1];
						data[i][j][k].averageCellNormal += currentTriangleNormals[2];

						data[i][j][k].CellTrianglesCentroid += currentTriangle[0];
						data[i][j][k].CellTrianglesCentroid += currentTriangle[1];
						data[i][j][k].CellTrianglesCentroid += currentTriangle[2];
					}

					data[i][j][k].averageCellNormal /= data[i][j][k].trianglesInCell.size() * 3;
					data[i][j][k].CellTrianglesCentroid /= data[i][j][k].trianglesInCell.size() * 3;

					if (data[i][j][k].approximateProjectionPlane != nullptr)
						delete data[i][j][k].approximateProjectionPlane;

					data[i][j][k].approximateProjectionPlane = new FEPlane(data[i][j][k].CellTrianglesCentroid, data[i][j][k].averageCellNormal);

					for (size_t l = 0; l < data[i][j][k].trianglesInCell.size(); l++)
					{
						std::vector<glm::vec3> currentTriangle = mesh->Triangles[data[i][j][k].trianglesInCell[l]];

						double originalArea = TriangleArea(currentTriangle[0], currentTriangle[1], currentTriangle[2]);

						glm::vec3 aProjection = data[i][j][k].approximateProjectionPlane->ProjectPoint(currentTriangle[0]);
						glm::vec3 bProjection = data[i][j][k].approximateProjectionPlane->ProjectPoint(currentTriangle[1]);
						glm::vec3 cProjection = data[i][j][k].approximateProjectionPlane->ProjectPoint(currentTriangle[2]);

						double projectionArea = TriangleArea(aProjection, bProjection, cProjection);

						data[i][j][k].rugosity += originalArea / projectionArea;
					}

					data[i][j][k].rugosity /= data[i][j][k].trianglesInCell.size();
				}
			}
		}
	}
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

				FEAABB FinalAABB = data[i][j][k].AABB.transform(transformMat);
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

	int posSize = mesh->getPositionsCount();
	float* positions = new float[posSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getPositionsBufferID(), 0, sizeof(float) * posSize, positions));

	/*int UVSize = mesh->getUVCount();
	float* UV = new float[UVSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getUVBufferID(), 0, sizeof(float) * UVSize, UV));

	int normSize = mesh->getNormalsCount();
	float* normalsFloat = new float[normSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getNormalsBufferID(), 0, sizeof(float) * normSize, normalsFloat));

	int tanSize = mesh->getTangentsCount();
	float* tangents = new float[tanSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getTangentsBufferID(), 0, sizeof(float) * tanSize, tangents));*/

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














	std::vector<float> TrianglesRugosity;
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

						/*std::vector<glm::vec3> currentTriangle = mesh->Triangles[data[i][j][k].trianglesInCell[l]];
						std::vector<glm::vec3> currentTriangleNormals = mesh->TrianglesNormals[data[i][j][k].trianglesInCell[l]];

						data[i][j][k].averageCellNormal += currentTriangleNormals[0];
						data[i][j][k].averageCellNormal += currentTriangleNormals[1];
						data[i][j][k].averageCellNormal += currentTriangleNormals[2];

						data[i][j][k].CellTrianglesCentroid += currentTriangle[0];
						data[i][j][k].CellTrianglesCentroid += currentTriangle[1];
						data[i][j][k].CellTrianglesCentroid += currentTriangle[2];*/
					}
			}
		}
	}


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









	glm::vec3 darkBlue = glm::vec3(0.0f, 0.0f, 0.4f);
	glm::vec3 lightCyan = glm::vec3(27.0f / 255.0f, 213.0f / 255.0f, 200.0f / 255.0f);
	glm::vec3 green = glm::vec3(0.0f / 255.0f, 255.0f / 255.0f, 64.0f / 255.0f);
	glm::vec3 yellow = glm::vec3(225.0f / 255.0f, 225.0f / 255.0f, 0.0f / 255.0f);
	glm::vec3 red = glm::vec3(225.0f / 255.0f, 0 / 255.0f, 0.0f / 255.0f);

	for (size_t i = 0; i < mesh->Triangles.size(); i++)
	{
		/*double normalizedRugorsity = TrianglesRugosity[i];

		if (normalizedRugorsity <= 1.1 && normalizedRugorsity > 1.0)
		{
			setColorOfFace(i, darkBlue);
		}
		else if (normalizedRugorsity <= 1.2 && normalizedRugorsity > 1.1)
		{
			setColorOfFace(i, lightCyan);
		}
		else if (normalizedRugorsity <= 1.3 && normalizedRugorsity > 1.2)
		{
			setColorOfFace(i, green);
		}
		else if (normalizedRugorsity <= 1.4 && normalizedRugorsity > 1.3)
		{
			setColorOfFace(i, yellow);
		}
		else
		{
			setColorOfFace(i, red);
		}*/



		double normalizedRugorsity = (TrianglesRugosity[i] - minRugorsity) / (maxRugorsity - minRugorsity);

		if (normalizedRugorsity <= 0.125 && normalizedRugorsity > 0.0)
		{
			setColorOfFace(i, darkBlue);
		}
		else if (normalizedRugorsity <= 0.25 && normalizedRugorsity > 0.125)
		{
			setColorOfFace(i, lightCyan);
		}
		else if (normalizedRugorsity <= 0.5 && normalizedRugorsity > 0.25)
		{
			setColorOfFace(i, green);
		}
		else if (normalizedRugorsity <= 0.75 && normalizedRugorsity > 0.5)
		{
			setColorOfFace(i, yellow);
		}
		else
		{
			setColorOfFace(i, red);
		}
	}

	mesh->addColorToVertices(colors, colorSize);
	//mesh->addSegmentsColorToVertices(segmentsColors, segmentsColorsSize);

	mesh->minRugorsity = minRugorsity;
	mesh->maxRugorsity = maxRugorsity;
}