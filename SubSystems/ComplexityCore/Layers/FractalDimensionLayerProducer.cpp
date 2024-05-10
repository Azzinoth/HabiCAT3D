#include "FractalDimensionLayerProducer.h"
using namespace FocalEngine;

FractalDimensionLayerProducer* FractalDimensionLayerProducer::Instance = nullptr;
void(*FractalDimensionLayerProducer::OnCalculationsEndCallbackImpl)(MeshLayer) = nullptr;

FractalDimensionLayerProducer::FractalDimensionLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

FractalDimensionLayerProducer::~FractalDimensionLayerProducer() {}

std::pair<double, double> LinearRegression(const std::vector<double>& XValues, const std::vector<double>& YValues)
{
	double NumPoints = static_cast<double>(XValues.size());
	double SumX = 0.0, SumY = 0.0, sumXY = 0.0, SumXSquared = 0.0;

	for (size_t i = 0; i < NumPoints; i++)
	{
		SumX += XValues[i];
		SumY += YValues[i];
		sumXY += XValues[i] * YValues[i];
		SumXSquared += XValues[i] * XValues[i];
	}

	double Slope = (NumPoints * sumXY - SumX * SumY) / (NumPoints * SumXSquared - SumX * SumX);
	double Intercept = (SumY - Slope * SumX) / NumPoints;

	return std::make_pair(Slope, Intercept);
}

std::vector<double> GenerateBoxSizes(double MinSize, double MaxSize, double Factor)
{
	std::vector<double> Sizes;
	for (double Size = MaxSize; Size >= MinSize; Size /= Factor)
		Sizes.push_back(Size);
	
	return Sizes;
}

void FractalDimensionLayerProducer::WorkOnNode(GridNode* CurrentNode)
{
	double FractalDimension = FRACTAL_DIMENSION_LAYER_PRODUCER.RunOnAllInternalNodesWithTriangles(CurrentNode);

	if (isnan(FractalDimension))
		FractalDimension = 0;
	
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
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

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
		uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
		std::vector<float> TrianglesToStandardDeviation = JITTER_MANAGER.ProduceStandardDeviationData();
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(TrianglesToStandardDeviation);
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));

		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo = new MeshLayerDebugInfo();
		MeshLayerDebugInfo* DebugInfo = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo;
		DebugInfo->Type = "FractalDimensionDeviationLayerDebugInfo";
		DebugInfo->AddEntry("Start time", StartTime);
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

	double FractalDimension = RunOnAllInternalNodesWithTriangles(CurrentNode, [&](int BoxSizeIndex, FEAABB BoxAABB) {
		if (BoxSizeIndex == DebugBoxSizeIndex)
		{
			FEAABB TransformedBox = BoxAABB.Transform(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->GetTransformMatrix());
			LINE_RENDERER.RenderAABB(TransformedBox, glm::vec3(1.0, 0.0, 0.0));
			DebugBoxCount++;
		}
	});

	if (isnan(FractalDimension))
		FractalDimension = 0;

	DebugFractalDimension = FractalDimension;
	/*DebugLogInverseSizes = LogInverseSizes;
	DebugLogCounts = LogCounts;
	DebugCounts = Counts;*/

	LINE_RENDERER.SyncWithGPU();
}

void FractalDimensionLayerProducer::RenderDebugInfoWindow(MeasurementGrid* Grid)
{
	if (ImGui::GetCurrentContext()->WithinFrameScope)
	{
		if (ImGui::Begin("Fractal dimension debug settings"))
		{
			std::vector<std::string> BoxSizeStrings = { "0", "1", "2", "3" };

			if (ImGui::BeginCombo("Box sizes depth", BoxSizeStrings[DebugBoxSizeIndex].c_str()))
			{
				for (int n = 0; n < 4; n++)
				{
					bool isSelected = (DebugBoxSizeIndex == n);

					if (ImGui::Selectable(BoxSizeStrings[n].c_str(), isSelected))
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
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

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

bool FractalDimensionLayerProducer::GetShouldCalculateStandardDeviation()
{
	return bCalculateStandardDeviation;
}

void FractalDimensionLayerProducer::SetShouldCalculateStandardDeviation(bool NewValue)
{
	bCalculateStandardDeviation = NewValue;
}

double FractalDimensionLayerProducer::RunOnAllInternalNodesWithTriangles(GridNode* OuterNode, std::function<void(int BoxSizeIndex, FEAABB BoxAABB)> FunctionWithAdditionalCode)
{
	if (OuterNode->TrianglesInCell.empty())
		return 0.0;

	// Generate a sequence of box sizes
	double VozelSize = OuterNode->AABB.GetMax()[0] - OuterNode->AABB.GetMin()[0];

	std::vector<double> DivisionFactors = { 32.0, 16.0, 8.0, 4.0 };
	std::vector<double> BoxSizes;
	for (size_t i = 0; i < DivisionFactors.size(); i++)
		BoxSizes.push_back(VozelSize / DivisionFactors[i]);

	std::vector<double> LogInverseSizes;
	std::vector<double> LogCounts;
	std::vector<int> Counts;

	for (size_t i = 0; i < BoxSizes.size(); i++)
	{
		double BoxSize = BoxSizes[i];

		// Create a 3D grid that covers the entire bounding box
		int GridX = static_cast<int>(DivisionFactors[i]);
		int GridY = static_cast<int>(DivisionFactors[i]);
		int GridZ = static_cast<int>(DivisionFactors[i]);

		int Count = 0;
		std::vector<std::vector<std::vector<bool>>> Grid(GridX, std::vector<std::vector<bool>>(GridY, std::vector<bool>(GridZ, false)));

		// Iterate through all the triangles
		for (size_t j = 0; j < OuterNode->TrianglesInCell.size(); j++)
		{
			std::vector<glm::dvec3> CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[OuterNode->TrianglesInCell[j]];

			// Calculate the grid cells that the triangle intersects or is contained in
			FEAABB TriangleBBox = FEAABB(CurrentTriangle);
			int MinGridX = static_cast<int>((TriangleBBox.GetMin()[0] - OuterNode->AABB.GetMin()[0]) / BoxSize);
			int MinGridY = static_cast<int>((TriangleBBox.GetMin()[1] - OuterNode->AABB.GetMin()[1]) / BoxSize);
			int MinGridZ = static_cast<int>((TriangleBBox.GetMin()[2] - OuterNode->AABB.GetMin()[2]) / BoxSize);
			int MaxGridX = static_cast<int>((TriangleBBox.GetMax()[0] - OuterNode->AABB.GetMin()[0]) / BoxSize);
			int MaxGridY = static_cast<int>((TriangleBBox.GetMax()[1] - OuterNode->AABB.GetMin()[1]) / BoxSize);
			int MaxGridZ = static_cast<int>((TriangleBBox.GetMax()[2] - OuterNode->AABB.GetMin()[2]) / BoxSize);

			for (int x = MinGridX; x <= MaxGridX; ++x)
			{
				for (int y = MinGridY; y <= MaxGridY; ++y)
				{
					for (int z = MinGridZ; z <= MaxGridZ; ++z)
					{
						if (x >= 0 && x < GridX && y >= 0 && y < GridY && z >= 0 && z < GridZ)
						{
							if (!Grid[x][y][z])
							{
								glm::vec3 BoxMin(x * BoxSize + OuterNode->AABB.GetMin()[0], y * BoxSize + OuterNode->AABB.GetMin()[1], z * BoxSize + OuterNode->AABB.GetMin()[2]);
								glm::vec3 BoxMax((x + 1) * BoxSize + OuterNode->AABB.GetMin()[0], (y + 1) * BoxSize + OuterNode->AABB.GetMin()[1], (z + 1) * BoxSize + OuterNode->AABB.GetMin()[2]);
								FEAABB Box(BoxMin, BoxMax);

								if (GEOMETRY.IsAABBIntersectTriangle(Box, CurrentTriangle))
								{
									Grid[x][y][z] = true;
									Count++;

									if (FunctionWithAdditionalCode != nullptr)
									{
										// If the user has provided a function to run on each box, we run it here.
										// This is useful for debugging purposes.
										FunctionWithAdditionalCode(static_cast<int>(i), Box);
									}
								}
							}
						}
					}
				}
			}
		}

		// Store the logarithm values for linear regression
		LogInverseSizes.push_back(std::log10(1.0 / BoxSize));
		Counts.push_back(Count);
		LogCounts.push_back(std::log10(static_cast<double>(Count)));
	}

	// Perform linear regression to estimate the fractal dimension
	std::pair<double, double> Coefficients = LinearRegression(LogInverseSizes, LogCounts);
	double FractalDimension = Coefficients.first;

	return FractalDimension;
}