#include "FractalDimensionLayerProducer.h"
using namespace FocalEngine;

FractalDimensionLayerProducer* FractalDimensionLayerProducer::Instance = nullptr;

FractalDimensionLayerProducer::FractalDimensionLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

FractalDimensionLayerProducer::~FractalDimensionLayerProducer() {}

// Custom linear regression function
std::pair<double, double> linearRegression(const std::vector<double>& x, const std::vector<double>& y)
{
	double n = x.size();
	double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;

	for (size_t i = 0; i < n; ++i)
	{
		sumX += x[i];
		sumY += y[i];
		sumXY += x[i] * y[i];
		sumX2 += x[i] * x[i];
	}

	double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
	double intercept = (sumY - slope * sumX) / n;

	return std::make_pair(slope, intercept);
}

std::vector<double> generateBoxSizes(double minSize, double maxSize, double factor)
{
	std::vector<double> sizes;
	for (double size = maxSize; size >= minSize; size /= factor)
		sizes.push_back(size);
	
	return sizes;
}

double calculateFractalDimension(const FEAABB& BoundingBox, const std::vector<Triangle>& triangles)
{
	// Generate a sequence of box sizes
	std::vector<double> boxSizes = generateBoxSizes(1.0 / 128, 1.0, 2);

	std::vector<double> logInverseSizes;
	std::vector<double> logCounts;

	for (size_t i = 0; i < boxSizes.size(); ++i) {
		double boxSize = boxSizes[i];

		// Create a 3D grid that covers the entire bounding box
		int gridX = static_cast<int>(glm::ceil((Boundingbox.Get Max.X - bbox.minX) / boxSize));
		int gridY = static_cast<int>(glm::ceil((bbox.maxY - bbox.minY) / boxSize));
		int gridZ = static_cast<int>(glm::ceil((bbox.maxZ - bbox.minZ) / boxSize));

		int count = 0;
		std::vector<std::vector<std::vector<bool>>> grid(gridX, std::vector<std::vector<bool>>(gridY, std::vector<bool>(gridZ, false)));

		// Iterate through all the triangles
		for (const Triangle& triangle : triangles) {
			// Calculate the grid cells that the triangle intersects or is contained in
			BoundingBox triangleBBox = triangle.boundingBox();
			int minGridX = static_cast<int>((triangleBBox.minX - bbox.minX) / boxSize);
			int minGridY = static_cast<int>((triangleBBox.minY - bbox.minY) / boxSize);
			int minGridZ = static_cast<int>((triangleBBox.minZ - bbox.minZ) / boxSize);
			int maxGridX = static_cast<int>((triangleBBox.maxX - bbox.minX) / boxSize);
			int maxGridY = static_cast<int>((triangleBBox.maxY - bbox.minY) / boxSize);
			int maxGridZ = static_cast<int>((triangleBBox.maxZ - bbox.minZ) / boxSize);

			for (int x = minGridX; x <= maxGridX; ++x) {
				for (int y = minGridY; y <= maxGridY; ++y) {
					for (int z = minGridZ; z <= maxGridZ; ++z) {
						if (!grid[x][y][z]) {
							glm::vec3 boxMin(x * boxSize + bbox.minX, y * boxSize + bbox.minY, z * boxSize + bbox.minZ);
							glm::vec3 boxMax((x + 1) * boxSize + bbox.minX, (y + 1) * boxSize + bbox.minY, (z + 1) * boxSize + bbox.minZ);
							BoundingBox box(boxMin, boxMax);
							if (intersect(triangle, box)) {
								grid[x][y][z] = true;
								++count;
							}
						}
					}
				}
			}
		}

		// Store the logarithm values for linear regression
		logInverseSizes.push_back(std::log(1.0 / boxSize));
		logCounts.push_back(std::log(static_cast<double>(count)));
	}

	// Perform linear regression to estimate the fractal dimension
	std::pair<double, double> coefficients = linearRegression(logInverseSizes, logCounts);
	double fractalDimension = coefficients.first;

	return fractalDimension;
}

void FractalDimensionLayerProducer::WorkOnNode(SDFNode* CurrentNode)
{
	if (CurrentNode->TrianglesInCell.empty())
		return;

	std::vector<double> NormalX;
	std::vector<double> NormalY;
	std::vector<double> NormalZ;

	for (size_t p = 0; p < CurrentNode->TrianglesInCell.size(); p++)
	{
		std::vector<glm::vec3> CurrentTriangleNormals = MESH_MANAGER.ActiveMesh->TrianglesNormals[CurrentNode->TrianglesInCell[p]];

		for (size_t l = 0; l < CurrentTriangleNormals.size(); l++)
		{
			NormalX.push_back(CurrentTriangleNormals[l][0]);
			NormalY.push_back(CurrentTriangleNormals[l][1]);
			NormalZ.push_back(CurrentTriangleNormals[l][2]);
		}
	}

	double meanX = std::accumulate(NormalX.begin(), NormalX.end(), 0.0) / NormalX.size();
	double meanY = std::accumulate(NormalY.begin(), NormalY.end(), 0.0) / NormalY.size();
	double meanZ = std::accumulate(NormalZ.begin(), NormalZ.end(), 0.0) / NormalZ.size();

	double sumX = std::inner_product(NormalX.begin(), NormalX.end(), NormalX.begin(), 0.0);
	double sumY = std::inner_product(NormalY.begin(), NormalY.end(), NormalY.begin(), 0.0);
	double sumZ = std::inner_product(NormalZ.begin(), NormalZ.end(), NormalZ.begin(), 0.0);

	double DoubleResult = sqrt(sumX / NormalX.size() - meanX * meanX + sumY / NormalY.size() - meanY * meanY + sumZ / NormalZ.size() - meanZ * meanZ);

	CurrentNode->UserData = DoubleResult;
}

void FractalDimensionLayerProducer::CalculateWithJitterAsync(FEMesh* Mesh, bool bSmootherResult)
{
	if (Mesh == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	/*auto WorkOnNode = [&](SDFNode* CurrentNode) {
		if (CurrentNode->TrianglesInCell.empty())
			return;

		std::vector<double> NormalX;
		std::vector<double> NormalY;
		std::vector<double> NormalZ;

		for (size_t p = 0; p < CurrentNode->TrianglesInCell.size(); p++)
		{
			std::vector<glm::vec3> CurrentTriangleNormals = MESH_MANAGER.ActiveMesh->TrianglesNormals[CurrentNode->TrianglesInCell[p]];

			for (size_t l = 0; l < CurrentTriangleNormals.size(); l++)
			{
				NormalX.push_back(CurrentTriangleNormals[l][0]);
				NormalY.push_back(CurrentTriangleNormals[l][1]);
				NormalZ.push_back(CurrentTriangleNormals[l][2]);
			}
		}

		double meanX = std::accumulate(NormalX.begin(), NormalX.end(), 0.0) / NormalX.size();
		double meanY = std::accumulate(NormalY.begin(), NormalY.end(), 0.0) / NormalY.size();
		double meanZ = std::accumulate(NormalZ.begin(), NormalZ.end(), 0.0) / NormalZ.size();

		double sumX = std::inner_product(NormalX.begin(), NormalX.end(), NormalX.begin(), 0.0);
		double sumY = std::inner_product(NormalY.begin(), NormalY.end(), NormalY.begin(), 0.0);
		double sumZ = std::inner_product(NormalZ.begin(), NormalZ.end(), NormalZ.begin(), 0.0);

		double DoubleResult = sqrt(sumX / NormalX.size() - meanX * meanX + sumY / NormalY.size() - meanY * meanY + sumZ / NormalZ.size() - meanZ * meanZ);

		CurrentNode->UserData = DoubleResult;
	};*/

	JITTER_MANAGER.CalculateWithSDFJitterAsync(WorkOnNode, bSmootherResult);
}

void FractalDimensionLayerProducer::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	if (!FRACTAL_DIMENSION_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	FRACTAL_DIMENSION_LAYER_PRODUCER.bWaitForJitterResult = false;
	MESH_MANAGER.ActiveMesh->AddLayer(NewLayer);
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Fractal dimension"));
	LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);
}