#include "PointDensityLayerProducer.h"
using namespace FocalEngine;

PointDensityLayerProducer::PointDensityLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

PointDensityLayerProducer::~PointDensityLayerProducer() {}

void PointDensityLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
	if (CurrentPointCloudAnalysisData == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](GridNode* CurrentNode) {
		if (CurrentNode->PointsInCell.empty())
			return;

		CurrentNode->UserData = static_cast<double>(CurrentNode->PointsInCell.size());
	};

	JITTER_MANAGER.CalculateWithGridJitterAsync(WorkOnNode, bSmootherResult);
}

void PointDensityLayerProducer::OnJitterCalculationsEnd(DataLayer* NewLayer)
{
	if (!POINT_DENSITY_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	POINT_DENSITY_LAYER_PRODUCER.bWaitForJitterResult = false;

	AnalysisObject* CurrentObject = NewLayer->GetMainParentObject();
	if (CurrentObject == nullptr)
		return;

	NewLayer->DebugInfo->Type = "PointDensityDataLayerDebugInfo";
	NewLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Point Density"));
	CurrentObject->AddLayer(NewLayer);
	CurrentObject->SetActiveLayer(NewLayer->GetID());
}

void PointDensityLayerProducer::RenderDebugInfoForSelectedNode(MeasurementGrid* Grid)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
	if (CurrentPointCloudAnalysisData == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return;

	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	Grid->UpdateLineRepresentation();
}