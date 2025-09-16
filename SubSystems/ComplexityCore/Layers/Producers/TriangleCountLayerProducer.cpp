#include "TriangleCountLayerProducer.h"
using namespace FocalEngine;

TriangleCountLayerProducer::TriangleCountLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

TriangleCountLayerProducer::~TriangleCountLayerProducer() {}

void TriangleCountLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(ActiveObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](GridNode* CurrentNode) {
		if (CurrentNode->TrianglesInCell.empty())
			return;

		CurrentNode->UserData = static_cast<double>(CurrentNode->TrianglesInCell.size());
	};

	JITTER_MANAGER.CalculateWithGridJitterAsync(WorkOnNode, bSmootherResult);
}

void TriangleCountLayerProducer::OnJitterCalculationsEnd(DataLayer* NewLayer)
{
	if (!TRIANGLE_COUNT_LAYER_PRODUCER.bWaitForJitterResult)
		return;
	TRIANGLE_COUNT_LAYER_PRODUCER.bWaitForJitterResult = false;

	AnalysisObject* CurrentObject = NewLayer->GetMainParentObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	NewLayer->SetType(LAYER_TYPE::TRIANGLE_DENSITY);

	CurrentObject->AddLayer(NewLayer);
	CurrentObject->SetActiveLayer(NewLayer->GetID());
}

void TriangleCountLayerProducer::CalculateOnWholeModel()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(ActiveObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](GridNode* CurrentNode) {
		if (CurrentNode->TrianglesInCell.empty())
			return;

		CurrentNode->UserData = static_cast<double>(CurrentNode->TrianglesInCell.size());
	};

	JITTER_MANAGER.CalculateOnWholeModel(WorkOnNode);
}