#include "FractalDimensionLayerProducer.h"
using namespace FocalEngine;
#include "../../../UI/UIManager.h"

void(*FractalDimensionLayerProducer::OnCalculationsEndCallbackImpl)(DataLayer*) = nullptr;

FractalDimensionLayerProducer::FractalDimensionLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
	if (!APPLICATION.HasConsoleWindow())
		DEVELOPER_MODE.AddOnDebugGridSelectedCellChangedCallback(OnDebugGridSelectedCellChanged);
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
	double FractalDimension = FRACTAL_DIMENSION_LAYER_PRODUCER.RunOnAllInternalNodesWithData(CurrentNode);

	if (isnan(FractalDimension))
		FractalDimension = 0;
	
	// I did not encounter any fractal dimension values greater than 3.0, but I am limiting it to 3.0 just in case.
	if (FractalDimension > 3.0)
		FractalDimension = 3.0;

	CurrentNode->UserData = FractalDimension;
}

void FractalDimensionLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
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
		
	JITTER_MANAGER.SetFallbackValue(ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH ? 2.0f : 0.0f);
	JITTER_MANAGER.CalculateWithGridJitterAsync(WorkOnNode, bSmootherResult);
}

void FractalDimensionLayerProducer::OnJitterCalculationsEnd(DataLayer* NewLayer)
{
	AnalysisObject* CurrentObject = NewLayer->GetMainParentObject();
	if (CurrentObject == nullptr)
		return;

	if (!FRACTAL_DIMENSION_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	NewLayer->SetType(LAYER_TYPE::FRACTAL_DIMENSION);
	NewLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Fractal Dimension"));

	FRACTAL_DIMENSION_LAYER_PRODUCER.bWaitForJitterResult = false;

	NewLayer->DebugInfo->AddEntry("FD outliers: ", std::string(FRACTAL_DIMENSION_LAYER_PRODUCER.bFilterFractalDimensionValues ? "Yes" : "No"));

	CurrentObject->AddLayer(NewLayer);
	CurrentObject->SetActiveLayer(NewLayer->GetID());

	if (FRACTAL_DIMENSION_LAYER_PRODUCER.bCalculateStandardDeviation)
	{
		uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
		std::vector<float> TrianglesToStandardDeviation = JITTER_MANAGER.ProduceStandardDeviationData();
		DataLayer* StandardDeviationLayer = new DataLayer({ CurrentObject->GetID() }, TrianglesToStandardDeviation);
		StandardDeviationLayer->SetType(LAYER_TYPE::STANDARD_DEVIATION);
		StandardDeviationLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));
		
		StandardDeviationLayer->DebugInfo = new DataLayerDebugInfo();
		DataLayerDebugInfo* DebugInfo = StandardDeviationLayer->DebugInfo;
		DebugInfo->Type = "FractalDimensionDeviationLayerDebugInfo";
		DebugInfo->AddEntry("Start time", StartTime);
		DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
		DebugInfo->AddEntry("Source layer ID", CurrentObject->Layers.back()->GetID());
		DebugInfo->AddEntry("Source layer caption", CurrentObject->Layers.back()->GetCaption());

		CurrentObject->AddLayer(StandardDeviationLayer);
	}

	if (OnCalculationsEndCallbackImpl != nullptr)
		OnCalculationsEndCallbackImpl(NewLayer);
}

void FractalDimensionLayerProducer::RenderDebugInfoForSelectedNode(MeasurementGrid* Grid)
{
	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	RenderDebugInfoWindow(Grid);
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
				for (int BoxSizeIndex = 0; BoxSizeIndex < 4; BoxSizeIndex++)
				{
					bool bIsSelected = (DebugBoxSizeIndex == BoxSizeIndex);
					if (ImGui::Selectable(BoxSizeStrings[BoxSizeIndex].c_str(), bIsSelected))
					{
						DebugBoxSizeIndex = BoxSizeIndex;
						FRACTAL_DIMENSION_LAYER_PRODUCER.UpdateDebugBoxes(Grid->SelectedCell);
					}

					if (bIsSelected)
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

void FractalDimensionLayerProducer::CalculateOnEntireObject()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
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

	JITTER_MANAGER.SetFallbackValue(ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH ? 2.0f : 0.0f);
	JITTER_MANAGER.CalculateOnEntireObject(WorkOnNode);
}

void FractalDimensionLayerProducer::SetOnCalculationsEndCallback(void(*Func)(DataLayer*))
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

double FractalDimensionLayerProducer::RunOnAllInternalNodesWithData(GridNode* OuterNode, std::function<void(int BoxSizeIndex, FEAABB BoxAABB)> FunctionWithAdditionalCode)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return 0.0;

	DATA_SOURCE_TYPE CurrentType = ActiveObject->GetType();

	ResourceAnalysisData* CurrentAnalysisData = ActiveObject->GetAnalysisData();
	if (CurrentAnalysisData == nullptr)
		return 0.0;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentType == DATA_SOURCE_TYPE::MESH && CurrentMeshAnalysisData == nullptr)
		return 0.0;
	PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
	if (CurrentType == DATA_SOURCE_TYPE::POINT_CLOUD && CurrentPointCloudAnalysisData == nullptr)
		return 0.0;

	if (CurrentType == DATA_SOURCE_TYPE::MESH && OuterNode->TrianglesInCell.empty() ||
		CurrentType == DATA_SOURCE_TYPE::POINT_CLOUD && OuterNode->PointsInCell.empty())
		return 0.0;

	// Generate a sequence of box sizes
	double VoxelSize = OuterNode->AABB.GetMax()[0] - OuterNode->AABB.GetMin()[0];

	std::vector<double> DivisionFactors = { 32.0, 16.0, 8.0, 4.0 };
	std::vector<double> BoxSizes;
	for (size_t i = 0; i < DivisionFactors.size(); i++)
		BoxSizes.push_back(VoxelSize / DivisionFactors[i]);

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

		size_t ElementCount = CurrentType == DATA_SOURCE_TYPE::MESH ? OuterNode->TrianglesInCell.size() : OuterNode->PointsInCell.size();
		// Iterate through all geometry elements (triangles or points).
		for (size_t j = 0; j < ElementCount; j++)
		{
			if (CurrentType == DATA_SOURCE_TYPE::POINT_CLOUD)
			{
				glm::dvec3 CurrentPoint = glm::dvec3(CurrentPointCloudAnalysisData->RawPointCloudData[OuterNode->PointsInCell[j]].X,
													 CurrentPointCloudAnalysisData->RawPointCloudData[OuterNode->PointsInCell[j]].Y,
													 CurrentPointCloudAnalysisData->RawPointCloudData[OuterNode->PointsInCell[j]].Z);

				// A point can occupy only one box; points exactly on the max face of the node AABB belong to the last box.
				int X = static_cast<int>((CurrentPoint[0] - OuterNode->AABB.GetMin()[0]) / BoxSize);
				int Y = static_cast<int>((CurrentPoint[1] - OuterNode->AABB.GetMin()[1]) / BoxSize);
				int Z = static_cast<int>((CurrentPoint[2] - OuterNode->AABB.GetMin()[2]) / BoxSize);

				if (X < 0)
					X = 0;

				if (X >= GridX)
					X = GridX - 1;

				if (Y < 0)
					Y = 0;

				if (Y >= GridY)
					Y = GridY - 1;

				if (Z < 0)
					Z = 0;

				if (Z >= GridZ)
					Z = GridZ - 1;

				if (!Grid[X][Y][Z])
				{
					Grid[X][Y][Z] = true;
					Count++;

					if (FunctionWithAdditionalCode != nullptr)
					{
						glm::vec3 BoxMin(X * BoxSize + OuterNode->AABB.GetMin()[0], Y * BoxSize + OuterNode->AABB.GetMin()[1], Z * BoxSize + OuterNode->AABB.GetMin()[2]);
						glm::vec3 BoxMax((X + 1) * BoxSize + OuterNode->AABB.GetMin()[0], (Y + 1) * BoxSize + OuterNode->AABB.GetMin()[1], (Z + 1) * BoxSize + OuterNode->AABB.GetMin()[2]);
						FunctionWithAdditionalCode(static_cast<int>(i), FEAABB(BoxMin, BoxMax));
					}
				}

				continue;
			}

			std::vector<glm::dvec3> CurrentTriangle = CurrentMeshAnalysisData->Triangles[OuterNode->TrianglesInCell[j]];

			FEAABB TriangleBBox = FEAABB(CurrentTriangle);
			int MinGridX = static_cast<int>((TriangleBBox.GetMin()[0] - OuterNode->AABB.GetMin()[0]) / BoxSize);
			int MinGridY = static_cast<int>((TriangleBBox.GetMin()[1] - OuterNode->AABB.GetMin()[1]) / BoxSize);
			int MinGridZ = static_cast<int>((TriangleBBox.GetMin()[2] - OuterNode->AABB.GetMin()[2]) / BoxSize);
			int MaxGridX = static_cast<int>((TriangleBBox.GetMax()[0] - OuterNode->AABB.GetMin()[0]) / BoxSize);
			int MaxGridY = static_cast<int>((TriangleBBox.GetMax()[1] - OuterNode->AABB.GetMin()[1]) / BoxSize);
			int MaxGridZ = static_cast<int>((TriangleBBox.GetMax()[2] - OuterNode->AABB.GetMin()[2]) / BoxSize);

			for (int X = MinGridX; X <= MaxGridX; ++X)
			{
				for (int Y = MinGridY; Y <= MaxGridY; ++Y)
				{
					for (int Z = MinGridZ; Z <= MaxGridZ; ++Z)
					{
						if (X >= 0 && X < GridX && Y >= 0 && Y < GridY && Z >= 0 && Z < GridZ)
						{
							if (!Grid[X][Y][Z])
							{
								glm::vec3 BoxMin(X * BoxSize + OuterNode->AABB.GetMin()[0], Y * BoxSize + OuterNode->AABB.GetMin()[1], Z * BoxSize + OuterNode->AABB.GetMin()[2]);
								glm::vec3 BoxMax((X + 1) * BoxSize + OuterNode->AABB.GetMin()[0], (Y + 1) * BoxSize + OuterNode->AABB.GetMin()[1], (Z + 1) * BoxSize + OuterNode->AABB.GetMin()[2]);
								FEAABB Box(BoxMin, BoxMax);

								if (GEOMETRY.IsAABBIntersectTriangle(Box, CurrentTriangle))
								{
									Grid[X][Y][Z] = true;
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

		// Store the logarithm values for linear regression.
		LogInverseSizes.push_back(std::log10(1.0 / BoxSize));
		Counts.push_back(Count);
		LogCounts.push_back(std::log10(static_cast<double>(Count)));
	}

	// Perform linear regression to estimate the fractal dimension.
	std::pair<double, double> Coefficients = LinearRegression(LogInverseSizes, LogCounts);
	double FractalDimension = Coefficients.first;

	return FractalDimension;
}

void FractalDimensionLayerProducer::UpdateDebugBoxes(glm::vec3 SelectedCell)
{
	MeasurementGrid* DebugGrid = DEVELOPER_MODE.GetDebugGrid();
	if (DebugGrid == nullptr)
	{
		MAIN_SCENE_MANAGER.ClearLinesFromEntity(FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity);
		return;
	}

	if (SelectedCell == glm::vec3(-1.0))
	{
		MAIN_SCENE_MANAGER.ClearLinesFromEntity(FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity);
		return;
	}

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
	{
		MAIN_SCENE_MANAGER.ClearLinesFromEntity(FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity);
		return;
	}

	FEEntity* ActiveEntity = ActiveObject->GetEntity();
	if (ActiveEntity == nullptr)
	{
		MAIN_SCENE_MANAGER.ClearLinesFromEntity(FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity);
		return;
	}

	DataLayer* ActiveLayer = ActiveObject->GetActiveLayer();
	if (ActiveLayer == nullptr || ActiveLayer->GetType() != LAYER_TYPE::FRACTAL_DIMENSION)
	{
		MAIN_SCENE_MANAGER.ClearLinesFromEntity(FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity);
		return;
	}

	GridNode* CurrentNode = &DebugGrid->Data[int(DebugGrid->SelectedCell.x)][int(DebugGrid->SelectedCell.y)][int(DebugGrid->SelectedCell.z)];

	std::vector<FELine> LinesToRender;
	double FractalDimension = FRACTAL_DIMENSION_LAYER_PRODUCER.RunOnAllInternalNodesWithData(CurrentNode, [&](int BoxSizeIndex, FEAABB BoxAABB) {
		if (BoxSizeIndex == FRACTAL_DIMENSION_LAYER_PRODUCER.DebugBoxSizeIndex)
		{
			std::vector<FELine> AABBLines = GEOMETRY.GetAABBEdges(BoxAABB);
			for (size_t i = 0; i < AABBLines.size(); i++)
			{
				AABBLines[i].Color = glm::vec3(1.0, 0.0, 0.0);
				AABBLines[i].Width = 0.2f;
				LinesToRender.push_back(AABBLines[i]);
			}
			FRACTAL_DIMENSION_LAYER_PRODUCER.DebugBoxCount++;
		}
	});

	MAIN_SCENE_MANAGER.AddLinesToEntity(&FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity, LinesToRender);
	if (FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity != nullptr && FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity->GetParentEntity() != ActiveEntity)
		ActiveEntity->AttachChild(FRACTAL_DIMENSION_LAYER_PRODUCER.DebugLinesEntity, false);

	if (isnan(FractalDimension))
		FractalDimension = 0;

	FRACTAL_DIMENSION_LAYER_PRODUCER.DebugFractalDimension = FractalDimension;
}

void FractalDimensionLayerProducer::OnDebugGridSelectedCellChanged(glm::vec3 NewSelectedCell)
{
	FRACTAL_DIMENSION_LAYER_PRODUCER.UpdateDebugBoxes(NewSelectedCell);
}