#include "VectorDispersionLayerProducer.h"
using namespace FocalEngine;

void(*VectorDispersionLayerProducer::OnCalculationsEndCallbackImpl)(DataLayer) = nullptr;

VectorDispersionLayerProducer::VectorDispersionLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

VectorDispersionLayerProducer::~VectorDispersionLayerProducer() {}

void VectorDispersionLayerProducer::WorkOnNode(GridNode* CurrentNode)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	if (CurrentNode->TrianglesInCell.empty())
		return;

	std::vector<double> NormalX;
	std::vector<double> NormalY;
	std::vector<double> NormalZ;

	for (size_t p = 0; p < CurrentNode->TrianglesInCell.size(); p++)
	{
		std::vector<glm::vec3> CurrentTriangleNormals = CurrentMeshAnalysisData->TrianglesNormals[CurrentNode->TrianglesInCell[p]];

		for (size_t l = 0; l < CurrentTriangleNormals.size(); l++)
		{
			NormalX.push_back(CurrentTriangleNormals[l][0]);
			NormalY.push_back(CurrentTriangleNormals[l][1]);
			NormalZ.push_back(CurrentTriangleNormals[l][2]);
		}
	}

	double MeanX = std::accumulate(NormalX.begin(), NormalX.end(), 0.0) / NormalX.size();
	double MeanY = std::accumulate(NormalY.begin(), NormalY.end(), 0.0) / NormalY.size();
	double MeanZ = std::accumulate(NormalZ.begin(), NormalZ.end(), 0.0) / NormalZ.size();

	double SumX = std::inner_product(NormalX.begin(), NormalX.end(), NormalX.begin(), 0.0);
	double SumY = std::inner_product(NormalY.begin(), NormalY.end(), NormalY.begin(), 0.0);
	double SumZ = std::inner_product(NormalZ.begin(), NormalZ.end(), NormalZ.begin(), 0.0);

	double XPortionOfResult = SumX / NormalX.size() - MeanX * MeanX;
	if (isnan(XPortionOfResult))
		XPortionOfResult = 0.0;
	double YPortionOfResult = SumY / NormalY.size() - MeanY * MeanY;
	if (isnan(YPortionOfResult))
		YPortionOfResult = 0.0;
	double ZPortionOfResult = SumZ / NormalZ.size() - MeanZ * MeanZ;
	if (isnan(ZPortionOfResult))
		ZPortionOfResult = 0.0;

	double DoubleResult = sqrt(XPortionOfResult + YPortionOfResult + ZPortionOfResult);

	if (isnan(DoubleResult))
		DoubleResult = 0.0;

	CurrentNode->UserData = DoubleResult;
}

void VectorDispersionLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr || CurrentObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return;

	bWaitForJitterResult = true;
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	JITTER_MANAGER.CalculateWithGridJitterAsync(WorkOnNode, bSmootherResult);
}

void VectorDispersionLayerProducer::OnJitterCalculationsEnd(DataLayer NewLayer)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	if (!VECTOR_DISPERSION_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	NewLayer.SetType(LAYER_TYPE::VECTOR_DISPERSION);

	VECTOR_DISPERSION_LAYER_PRODUCER.bWaitForJitterResult = false;
	LAYER_MANAGER.AddLayer(NewLayer);
	LAYER_MANAGER.Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Vector dispersion"));
	LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(LAYER_MANAGER.Layers.size() - 1));

	if (VECTOR_DISPERSION_LAYER_PRODUCER.bCalculateStandardDeviation)
	{
		uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
		std::vector<float> TrianglesToStandardDeviation = JITTER_MANAGER.ProduceStandardDeviationData();
		LAYER_MANAGER.AddLayer(DATA_SOURCE_TYPE::MESH, TrianglesToStandardDeviation);
		LAYER_MANAGER.Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));

		LAYER_MANAGER.Layers.back().DebugInfo = new DataLayerDebugInfo();
		DataLayerDebugInfo* DebugInfo = LAYER_MANAGER.Layers.back().DebugInfo;
		DebugInfo->Type = "VectorDispersionDeviationLayerDebugInfo";
		DebugInfo->AddEntry("Start time", StartTime);
		DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
		DebugInfo->AddEntry("Source layer ID", LAYER_MANAGER.Layers[LAYER_MANAGER.Layers.size() - 2].GetID());
		DebugInfo->AddEntry("Source layer caption", LAYER_MANAGER.Layers[LAYER_MANAGER.Layers.size() - 2].GetCaption());
	}

	if (OnCalculationsEndCallbackImpl != nullptr)
		OnCalculationsEndCallbackImpl(NewLayer);
}

void VectorDispersionLayerProducer::RenderDebugInfoForSelectedNode(MeasurementGrid* Grid)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return;

	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	Grid->UpdateRenderedLines();

	GridNode* CurrentlySelectedCell = &Grid->Data[int(Grid->SelectedCell.x)][int(Grid->SelectedCell.y)][int(Grid->SelectedCell.z)];
	for (size_t i = 0; i < CurrentlySelectedCell->TrianglesInCell.size(); i++)
	{
		const auto CurrentTriangle = CurrentMeshAnalysisData->Triangles[CurrentlySelectedCell->TrianglesInCell[i]];

		std::vector<glm::dvec3> TranformedTrianglePoints = CurrentTriangle;
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
	}

	LINE_RENDERER.SyncWithGPU();
}

void VectorDispersionLayerProducer::CalculateOnWholeModel()
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr || CurrentObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return;

	bWaitForJitterResult = true;
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	JITTER_MANAGER.CalculateOnWholeModel(WorkOnNode);
}

void VectorDispersionLayerProducer::SetOnCalculationsEndCallback(void(*Func)(DataLayer))
{
	OnCalculationsEndCallbackImpl = Func;
}

bool VectorDispersionLayerProducer::GetShouldCalculateStandardDeviation()
{
	return bCalculateStandardDeviation;
}
void VectorDispersionLayerProducer::SetShouldCalculateStandardDeviation(bool NewValue)
{
	bCalculateStandardDeviation = NewValue;
}