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
	double VozelSize = CurrentNode->AABB.getMax()[0] - CurrentNode->AABB.getMin()[0];
	std::vector<double> boxSizes = { /*VozelSize / 64.0,*/ VozelSize / 32.0, VozelSize / 16.0, VozelSize / 8.0, VozelSize / 4.0 };//generateBoxSizes(1.0 / 64.0, 0.5, 2.0);

	std::vector<double> logInverseSizes;
	std::vector<double> logCounts;
	std::vector<int> Counts;

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

								/*if (!box.AABBIntersect(TriangleBBox) && box.IntersectsTriangle(CurrentTriangle[0], CurrentTriangle[1], CurrentTriangle[2]))
								{
									bool Result = box.IntersectsTriangle(CurrentTriangle[0], CurrentTriangle[1], CurrentTriangle[2]);
									int y = 0;
									y++;

								}*/

								//if (box.IntersectsTriangle(CurrentTriangle[0], CurrentTriangle[1], CurrentTriangle[2]))
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
		Counts.push_back(count);
		logCounts.push_back(std::log(static_cast<double>(count)));
	}

	// Perform linear regression to estimate the fractal dimension
	std::pair<double, double> coefficients = linearRegression(logInverseSizes, logCounts);
	double FractalDimension = coefficients.first;

	if (isnan(FractalDimension))
	{
		FractalDimension = 0;
	}

	/*if (FractalDimension < 2.0)
		FractalDimension = 2.0;
	else if (FractalDimension > 3.0)
		FractalDimension = 3.0;*/

	CurrentNode->UserData = FractalDimension;
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
	MESH_MANAGER.ActiveMesh->Layers.back().SetType(LAYER_TYPE::FRACTAL_DIMENSION);
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Fractal dimension"));
	LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);
}

void FractalDimensionLayerProducer::RenderDebugInfoForSelectedNode(SDF* Grid)
{
	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	Grid->UpdateRenderedLines();

	SDFNode* CurrentNode = &Grid->Data[int(Grid->SelectedCell.x)][int(Grid->SelectedCell.y)][int(Grid->SelectedCell.z)];

	// Generate a sequence of box sizes
	double VozelSize = CurrentNode->AABB.getMax()[0] - CurrentNode->AABB.getMin()[0];
	std::vector<double> boxSizes = { /*VozelSize / 64.0,*/ VozelSize / 32.0, VozelSize / 16.0, VozelSize / 8.0, VozelSize / 4.0 };
	double boxSize = boxSizes[DebugBoxSizeIndex];

	std::vector<double> logInverseSizes;
	std::vector<double> logCounts;

	// Create a 3D grid that covers the entire bounding box
	int gridX = static_cast<int>(glm::ceil((CurrentNode->AABB.getMax()[0] - CurrentNode->AABB.getMin()[0]) / boxSize));
	int gridY = static_cast<int>(glm::ceil((CurrentNode->AABB.getMax()[1] - CurrentNode->AABB.getMin()[1]) / boxSize));
	int gridZ = static_cast<int>(glm::ceil((CurrentNode->AABB.getMax()[2] - CurrentNode->AABB.getMin()[2]) / boxSize));

	int count = 0;
	std::vector<std::vector<std::vector<bool>>> grid(gridX, std::vector<std::vector<bool>>(gridY, std::vector<bool>(gridZ, false)));

	int minGridX = static_cast<int>(CurrentNode->AABB.getMin()[0] / boxSize);
	int minGridY = static_cast<int>(CurrentNode->AABB.getMin()[1] / boxSize);
	int minGridZ = static_cast<int>(CurrentNode->AABB.getMin()[2] / boxSize);
	int maxGridX = static_cast<int>(CurrentNode->AABB.getMax()[0] / boxSize);
	int maxGridY = static_cast<int>(CurrentNode->AABB.getMax()[1] / boxSize);
	int maxGridZ = static_cast<int>(CurrentNode->AABB.getMax()[2] / boxSize);


	for (size_t i = 0; i < CurrentNode->TrianglesInCell.size(); i++)
	{
		std::vector<glm::vec3> CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[CurrentNode->TrianglesInCell[i]];
		FEAABB TriangleBBox = FEAABB(CurrentTriangle);
		TriangleBBox = TriangleBBox.transform(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix());
		LINE_RENDERER.RenderAABB(TriangleBBox, glm::vec3(0.0, 0.0, 1.0));
	}

	/*for (int x = minGridX; x <= maxGridX; ++x)
	{
		for (int y = minGridY; y <= maxGridY; ++y)
		{
			for (int z = minGridZ; z <= maxGridZ; ++z)
			{*/

	DebugBoxCount = 0;
	for (int x = 0; x < maxGridX - minGridX; ++x)
	{
		for (int y = 0; y < maxGridY - minGridY; ++y)
		{
			for (int z = 0; z < maxGridZ - minGridZ; ++z)
			{
				glm::vec3 boxMin(x * boxSize /*+ CurrentNode->AABB.getMin()[0]*/, y * boxSize /*+ CurrentNode->AABB.getMin()[1]*/, z * boxSize /*+ CurrentNode->AABB.getMin()[2]*/);
				boxMin += CurrentNode->AABB.getMin();

				glm::vec3 boxMax((x + 1) * boxSize /*+ CurrentNode->AABB.getMin()[0]*/, (y + 1) * boxSize /*+ CurrentNode->AABB.getMin()[1]*/, (z + 1) * boxSize /*+ CurrentNode->AABB.getMin()[2]*/);
				boxMax += CurrentNode->AABB.getMin();

				FEAABB box(boxMin, boxMax);
				//box = box.transform(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix());

				// Iterate through all the triangles
				for (size_t i = 0; i < CurrentNode->TrianglesInCell.size(); i++)
				{
					std::vector<glm::vec3> CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[CurrentNode->TrianglesInCell[i]];

					// Calculate the grid cells that the triangle intersects or is contained in
					FEAABB TriangleBBox = FEAABB(CurrentTriangle);

					/*if (!box.AABBIntersect(TriangleBBox) && box.IntersectsTriangle(CurrentTriangle[0], CurrentTriangle[1], CurrentTriangle[2]))
					{
						bool Result = box.IntersectsTriangle(CurrentTriangle[0], CurrentTriangle[1], CurrentTriangle[2]);
						int y = 0;
						y++;
					}*/

					//if (box.IntersectsTriangle(CurrentTriangle[0], CurrentTriangle[1], CurrentTriangle[2]))
					if (box.AABBIntersect(TriangleBBox))
					{
						box = box.transform(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix());
						LINE_RENDERER.RenderAABB(box, glm::vec3(1.0, 0.0, 0.0));
						DebugBoxCount++;
						break;
					}
				}
			}
		}
	}

	LINE_RENDERER.SyncWithGPU();
}

void FractalDimensionLayerProducer::RenderDebugInfoWindow(SDF* Grid)
{
	if (ImGui::GetCurrentContext()->WithinFrameScope)
	{
		if (ImGui::Begin("Fractal dimension debug settings"))
		{
			std::vector<std::string> boxSizeStrs = { "0", "1", "2", "3" };

			if (ImGui::BeginCombo("Combo box", boxSizeStrs[DebugBoxSizeIndex].c_str()))
			{
				for (int n = 0; n < 4; n++)
				{
					bool isSelected = (DebugBoxSizeIndex == n);

					if (ImGui::Selectable(boxSizeStrs[n].c_str(), isSelected))
					{
						DebugBoxSizeIndex = n;
						FRACTAL_DIMENSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(Grid);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Text("Number of boxes: %d", DebugBoxCount);

			ImGui::End();
		}
	}
}