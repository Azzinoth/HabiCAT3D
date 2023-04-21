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

void FractalDimensionLayerProducer::WorkOnNode(SDFNode* CurrentNode)
{
	if (CurrentNode->TrianglesInCell.empty())
		return;

	// Generate a sequence of box sizes
	std::vector<double> boxSizes = generateBoxSizes(1.0 / 256, 0.5, 2);

	std::vector<double> logInverseSizes;
	std::vector<double> logCounts;

	for (size_t i = 0; i < boxSizes.size(); i++)
	{
		double boxSize = boxSizes[i];

		// Create a 3D grid that covers the entire bounding box
		int gridX = static_cast<int>(glm::ceil((CurrentNode->AABB.getMax()[0] - CurrentNode->AABB.getMin()[0]) / boxSize));
		int gridY = static_cast<int>(glm::ceil((CurrentNode->AABB.getMax()[1] - CurrentNode->AABB.getMin()[1]) / boxSize));
		int gridZ = static_cast<int>(glm::ceil((CurrentNode->AABB.getMax()[2] - CurrentNode->AABB.getMin()[2]) / boxSize));

		int count = 0;
		std::vector<std::vector<std::vector<bool>>> grid(gridX, std::vector<std::vector<bool>>(gridY, std::vector<bool>(gridZ, false)));

		// Iterate through all the triangles
		for (size_t j = 0; j < CurrentNode->TrianglesInCell.size(); j++)
		{
			std::vector<glm::vec3> CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[CurrentNode->TrianglesInCell[j]];

			// Calculate the grid cells that the triangle intersects or is contained in
			FEAABB TriangleBBox = FEAABB(CurrentTriangle);
			int minGridX = static_cast<int>((TriangleBBox.getMin()[0] - CurrentNode->AABB.getMin()[0]) / boxSize);
			int minGridY = static_cast<int>((TriangleBBox.getMin()[1] - CurrentNode->AABB.getMin()[1]) / boxSize);
			int minGridZ = static_cast<int>((TriangleBBox.getMin()[2] - CurrentNode->AABB.getMin()[2]) / boxSize);
			int maxGridX = static_cast<int>((TriangleBBox.getMax()[0] - CurrentNode->AABB.getMin()[0]) / boxSize);
			int maxGridY = static_cast<int>((TriangleBBox.getMax()[1] - CurrentNode->AABB.getMin()[1]) / boxSize);
			int maxGridZ = static_cast<int>((TriangleBBox.getMax()[2] - CurrentNode->AABB.getMin()[2]) / boxSize);

			for (int x = minGridX; x <= maxGridX; ++x)
			{
				for (int y = minGridY; y <= maxGridY; ++y)
				{
					for (int z = minGridZ; z <= maxGridZ; ++z)
					{
						if (x >= 0 && x < gridX && y >= 0 && y < gridY && z >= 0 && z < gridZ)
						{
							if (!grid[x][y][z])
							{
								glm::vec3 boxMin(x * boxSize + CurrentNode->AABB.getMin()[0], y * boxSize + CurrentNode->AABB.getMin()[1], z * boxSize + CurrentNode->AABB.getMin()[2]);
								glm::vec3 boxMax((x + 1) * boxSize + CurrentNode->AABB.getMin()[0], (y + 1) * boxSize + CurrentNode->AABB.getMin()[1], (z + 1) * boxSize + CurrentNode->AABB.getMin()[2]);
								FEAABB box(boxMin, boxMax);

								//if (intersect(triangle, box))
								if (box.AABBIntersect(TriangleBBox))
								{
									grid[x][y][z] = true;
									++count;
								}
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
	double FractalDimension = coefficients.first;

	CurrentNode->UserData = FractalDimension;

	//std::vector<double> NormalX;
	//std::vector<double> NormalY;
	//std::vector<double> NormalZ;

	//for (size_t p = 0; p < CurrentNode->TrianglesInCell.size(); p++)
	//{
	//	std::vector<glm::vec3> CurrentTriangleNormals = MESH_MANAGER.ActiveMesh->TrianglesNormals[CurrentNode->TrianglesInCell[p]];

	//	for (size_t l = 0; l < CurrentTriangleNormals.size(); l++)
	//	{
	//		NormalX.push_back(CurrentTriangleNormals[l][0]);
	//		NormalY.push_back(CurrentTriangleNormals[l][1]);
	//		NormalZ.push_back(CurrentTriangleNormals[l][2]);
	//	}
	//}

	//double meanX = std::accumulate(NormalX.begin(), NormalX.end(), 0.0) / NormalX.size();
	//double meanY = std::accumulate(NormalY.begin(), NormalY.end(), 0.0) / NormalY.size();
	//double meanZ = std::accumulate(NormalZ.begin(), NormalZ.end(), 0.0) / NormalZ.size();

	//double sumX = std::inner_product(NormalX.begin(), NormalX.end(), NormalX.begin(), 0.0);
	//double sumY = std::inner_product(NormalY.begin(), NormalY.end(), NormalY.begin(), 0.0);
	//double sumZ = std::inner_product(NormalZ.begin(), NormalZ.end(), NormalZ.begin(), 0.0);

	//double DoubleResult = sqrt(sumX / NormalX.size() - meanX * meanX + sumY / NormalY.size() - meanY * meanY + sumZ / NormalZ.size() - meanZ * meanZ);

	//CurrentNode->UserData = DoubleResult;
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