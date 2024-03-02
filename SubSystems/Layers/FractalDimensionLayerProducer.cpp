#include "FractalDimensionLayerProducer.h"
using namespace FocalEngine;

FractalDimensionLayerProducer* FractalDimensionLayerProducer::Instance = nullptr;
void(*FractalDimensionLayerProducer::OnCalculationsEndCallbackImpl)(MeshLayer) = nullptr;

FractalDimensionLayerProducer::FractalDimensionLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

FractalDimensionLayerProducer::~FractalDimensionLayerProducer() {}

// Custom linear regression function
std::pair<double, double> linearRegression(const std::vector<double>& x, const std::vector<double>& y)
{
	double n = static_cast<double>(x.size());
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

void FractalDimensionLayerProducer::WorkOnNode(GridNode* CurrentNode)
{
	if (CurrentNode->TrianglesInCell.empty())
		return;

	// Generate a sequence of box sizes
	double VozelSize = CurrentNode->AABB.GetMax()[0] - CurrentNode->AABB.GetMin()[0];

	std::vector<double> DivisionFactors = { 32.0, 16.0, 8.0, 4.0 };
	std::vector<double> BoxSizes;
	for (size_t i = 0; i < DivisionFactors.size(); i++)
		BoxSizes.push_back(VozelSize / DivisionFactors[i]);

	std::vector<double> logInverseSizes;
	std::vector<double> logCounts;
	std::vector<int> Counts;

	for (size_t i = 0; i < BoxSizes.size(); i++)
	{
		double boxSize = BoxSizes[i];

		// Create a 3D grid that covers the entire bounding box
		int gridX = static_cast<int>(DivisionFactors[i]);
		int gridY = static_cast<int>(DivisionFactors[i]);
		int gridZ = static_cast<int>(DivisionFactors[i]);

		int count = 0;
		std::vector<std::vector<std::vector<bool>>> grid(gridX, std::vector<std::vector<bool>>(gridY, std::vector<bool>(gridZ, false)));

		// Iterate through all the triangles
		for (size_t j = 0; j < CurrentNode->TrianglesInCell.size(); j++)
		{
			std::vector<glm::vec3> CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[CurrentNode->TrianglesInCell[j]];

			// Calculate the grid cells that the triangle intersects or is contained in
			FEAABB TriangleBBox = FEAABB(CurrentTriangle);
			int minGridX = static_cast<int>((TriangleBBox.GetMin()[0] - CurrentNode->AABB.GetMin()[0]) / boxSize);
			int minGridY = static_cast<int>((TriangleBBox.GetMin()[1] - CurrentNode->AABB.GetMin()[1]) / boxSize);
			int minGridZ = static_cast<int>((TriangleBBox.GetMin()[2] - CurrentNode->AABB.GetMin()[2]) / boxSize);
			int maxGridX = static_cast<int>((TriangleBBox.GetMax()[0] - CurrentNode->AABB.GetMin()[0]) / boxSize);
			int maxGridY = static_cast<int>((TriangleBBox.GetMax()[1] - CurrentNode->AABB.GetMin()[1]) / boxSize);
			int maxGridZ = static_cast<int>((TriangleBBox.GetMax()[2] - CurrentNode->AABB.GetMin()[2]) / boxSize);

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
								glm::vec3 boxMin(x * boxSize + CurrentNode->AABB.GetMin()[0], y * boxSize + CurrentNode->AABB.GetMin()[1], z * boxSize + CurrentNode->AABB.GetMin()[2]);
								glm::vec3 boxMax((x + 1) * boxSize + CurrentNode->AABB.GetMin()[0], (y + 1) * boxSize + CurrentNode->AABB.GetMin()[1], (z + 1) * boxSize + CurrentNode->AABB.GetMin()[2]);
								FEAABB box(boxMin, boxMax);

								if (GEOMETRY.IsAABBIntersectTriangle(box, CurrentTriangle))
								{
									grid[x][y][z] = true;
									count++;
								}
							}
						}
					}
				}
			}
		}

		// Store the logarithm values for linear regression
		logInverseSizes.push_back(std::log10(1.0 / boxSize));
		Counts.push_back(count);
		logCounts.push_back(std::log10(static_cast<double>(count)));
	}

	// Perform linear regression to estimate the fractal dimension
	std::pair<double, double> coefficients = linearRegression(logInverseSizes, logCounts);
	double FractalDimension = coefficients.first;

	if (isnan(FractalDimension))
	{
		FractalDimension = 0;
	}
	
	// I did not encounter any fractal dimension values greater than 3.0, but I am limiting it to 3.0 just in case.
	if (FractalDimension > 3.0)
		FractalDimension = 3.0;

	CurrentNode->UserData = FractalDimension;
}

void FractalDimensionLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	// Before each run, we set the IgnoreValueFunction relevant to the fractal dimension calculation.
	if (bFilterFractalDimensionValues)
	{
		JITTER_MANAGER.SetIgnoreValueFunction([](float Value) -> bool {
			return Value < 2.0f;
		});
	}
	else
	{
		JITTER_MANAGER.SetIgnoreValueFunction([](float Value) -> bool {
			return false;
		});
	}
		
	JITTER_MANAGER.SetFallbackValue(2.0f);
	JITTER_MANAGER.CalculateWithGridJitterAsync(WorkOnNode, bSmootherResult);
}

void FractalDimensionLayerProducer::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	if (!FRACTAL_DIMENSION_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	NewLayer.SetType(FRACTAL_DIMENSION);

	FRACTAL_DIMENSION_LAYER_PRODUCER.bWaitForJitterResult = false;

	NewLayer.DebugInfo->AddEntry("FD outliers: ", std::string(FRACTAL_DIMENSION_LAYER_PRODUCER.bFilterFractalDimensionValues ? "Yes" : "No"));
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(NewLayer);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetType(LAYER_TYPE::FRACTAL_DIMENSION);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Fractal dimension"));
	LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 1));

	if (FRACTAL_DIMENSION_LAYER_PRODUCER.bCalculateStandardDeviation)
	{
		uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
		std::vector<float> TrianglesToStandardDeviation = JITTER_MANAGER.ProduceStandardDeviationData();
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(TrianglesToStandardDeviation);
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));

		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo = new MeshLayerDebugInfo();
		MeshLayerDebugInfo* DebugInfo = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo;
		DebugInfo->Type = "FractalDimensionDeviationLayerDebugInfo";
		DebugInfo->AddEntry("Start time", StarTime);
		DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
		DebugInfo->AddEntry("Source layer ID", COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 2].GetID());
		DebugInfo->AddEntry("Source layer caption", COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 2].GetCaption());
	}

	if (OnCalculationsEndCallbackImpl != nullptr)
		OnCalculationsEndCallbackImpl(NewLayer);
}

void FractalDimensionLayerProducer::RenderDebugInfoForSelectedNode(MeasurementGrid* Grid)
{
	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	Grid->UpdateRenderedLines();

	GridNode* CurrentNode = &Grid->Data[int(Grid->SelectedCell.x)][int(Grid->SelectedCell.y)][int(Grid->SelectedCell.z)];
	if (CurrentNode->TrianglesInCell.empty())
		return;

	// Generate a sequence of box sizes
	double VozelSize = CurrentNode->AABB.GetMax()[0] - CurrentNode->AABB.GetMin()[0];

	std::vector<double> DivisionFactors = { 32.0, 16.0, 8.0, 4.0 };
	std::vector<double> BoxSizes;
	for (size_t i = 0; i < DivisionFactors.size(); i++)
		BoxSizes.push_back(VozelSize / DivisionFactors[i]);

	std::vector<double> logInverseSizes;
	std::vector<double> logCounts;
	std::vector<int> Counts;

	DebugBoxCount = 0;
	for (size_t i = 0; i < BoxSizes.size(); i++)
	{
		double boxSize = BoxSizes[i];

		// Create a 3D grid that covers the entire bounding box
		int gridX = static_cast<int>(DivisionFactors[i]);
		int gridY = static_cast<int>(DivisionFactors[i]);
		int gridZ = static_cast<int>(DivisionFactors[i]);

		int count = 0;
		std::vector<std::vector<std::vector<bool>>> grid(gridX, std::vector<std::vector<bool>>(gridY, std::vector<bool>(gridZ, false)));

		// Iterate through all the triangles
		for (size_t j = 0; j < CurrentNode->TrianglesInCell.size(); j++)
		{
			std::vector<glm::vec3> CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[CurrentNode->TrianglesInCell[j]];

			// Calculate the grid cells that the triangle intersects or is contained in
			FEAABB TriangleBBox = FEAABB(CurrentTriangle);
			int minGridX = static_cast<int>((TriangleBBox.GetMin()[0] - CurrentNode->AABB.GetMin()[0]) / boxSize);
			int minGridY = static_cast<int>((TriangleBBox.GetMin()[1] - CurrentNode->AABB.GetMin()[1]) / boxSize);
			int minGridZ = static_cast<int>((TriangleBBox.GetMin()[2] - CurrentNode->AABB.GetMin()[2]) / boxSize);
			int maxGridX = static_cast<int>((TriangleBBox.GetMax()[0] - CurrentNode->AABB.GetMin()[0]) / boxSize);
			int maxGridY = static_cast<int>((TriangleBBox.GetMax()[1] - CurrentNode->AABB.GetMin()[1]) / boxSize);
			int maxGridZ = static_cast<int>((TriangleBBox.GetMax()[2] - CurrentNode->AABB.GetMin()[2]) / boxSize);

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
								glm::vec3 boxMin(x * boxSize + CurrentNode->AABB.GetMin()[0], y * boxSize + CurrentNode->AABB.GetMin()[1], z * boxSize + CurrentNode->AABB.GetMin()[2]);
								glm::vec3 boxMax((x + 1) * boxSize + CurrentNode->AABB.GetMin()[0], (y + 1) * boxSize + CurrentNode->AABB.GetMin()[1], (z + 1) * boxSize + CurrentNode->AABB.GetMin()[2]);
								FEAABB box(boxMin, boxMax);

								if (GEOMETRY.IsAABBIntersectTriangle(box, CurrentTriangle))
								{
									grid[x][y][z] = true;
									count++;

									if (i == DebugBoxSizeIndex)
									{
										box = box.Transform(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->GetTransformMatrix());
										LINE_RENDERER.RenderAABB(box, glm::vec3(1.0, 0.0, 0.0));
										DebugBoxCount++;
									}
								}
							}
						}
					}
				}
			}
		}

		// Store the logarithm values for linear regression
		logInverseSizes.push_back(std::log10(1.0 / boxSize));
		Counts.push_back(count);
		logCounts.push_back(std::log10(static_cast<double>(count)));
	}

	// Perform linear regression to estimate the fractal dimension
	std::pair<double, double> coefficients = linearRegression(logInverseSizes, logCounts);
	double FractalDimension = coefficients.first;

	if (isnan(FractalDimension))
	{
		FractalDimension = 0;
	}

	DebugFractalDimension = FractalDimension;
	DebugLogInverseSizes = logInverseSizes;
	DebugLogCounts = logCounts;
	DebugCounts = Counts;

	LINE_RENDERER.SyncWithGPU();
}

void FractalDimensionLayerProducer::RenderDebugInfoWindow(MeasurementGrid* Grid)
{
	if (ImGui::GetCurrentContext()->WithinFrameScope)
	{
		if (ImGui::Begin("Fractal dimension debug settings"))
		{
			std::vector<std::string> boxSizeStrs = { "0", "1", "2", "3" };

			if (ImGui::BeginCombo("Box sizes depth", boxSizeStrs[DebugBoxSizeIndex].c_str()))
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
			ImGui::Text("Fractal value: %.2f", DebugFractalDimension);

			for (size_t i = 0; i < DebugLogInverseSizes.size(); i++)
			{
				ImGui::Text(("Log inverse size " + std::to_string(i) + ": %.2f").c_str(), DebugLogInverseSizes[i]);
			}

			for (size_t i = 0; i < DebugLogInverseSizes.size(); i++)
			{
				ImGui::Text(("Log count " + std::to_string(i) + ": %.2f").c_str(), DebugLogCounts[i]);
			}

			for (size_t i = 0; i < DebugLogInverseSizes.size(); i++)
			{
				ImGui::Text(("Count " + std::to_string(i) + ": %d").c_str(), DebugCounts[i]);
			}

			ImGui::Text("That is how fractal value will be calculated: \n");
			ImGui::Text("std::pair<double, double> coefficients = linearRegression(logInverseSizes, logCounts);\n");
			ImGui::Text("double FractalDimension = coefficients.first;");

			ImGui::End();
		}
	}
}

void FractalDimensionLayerProducer::CalculateOnWholeModel()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	// Before each run, we set the IgnoreValueFunction relevant to the fractal dimension calculation.
	JITTER_MANAGER.SetIgnoreValueFunction([](float Value) -> bool {
		return Value < 2.0f;
	});

	JITTER_MANAGER.SetFallbackValue(2.0f);
	JITTER_MANAGER.CalculateOnWholeModel(WorkOnNode);
}

void FractalDimensionLayerProducer::SetOnCalculationsEndCallback(void(*Func)(MeshLayer))
{
	OnCalculationsEndCallbackImpl = Func;
}

bool FractalDimensionLayerProducer::GetShouldFilterFractalDimensionValues()
{
	return bFilterFractalDimensionValues;
}

void FractalDimensionLayerProducer::SetShouldFilterFractalDimensionValues(bool NewValue)
{
	bFilterFractalDimensionValues = NewValue;
}