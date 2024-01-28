#include "TriangleCountLayerProducer.h"
using namespace FocalEngine;

TriangleCountLayerProducer* TriangleCountLayerProducer::Instance = nullptr;

TriangleCountLayerProducer::TriangleCountLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

TriangleCountLayerProducer::~TriangleCountLayerProducer() {}

void TriangleCountLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](SDFNode* CurrentNode) {
		if (CurrentNode->TrianglesInCell.empty())
			return;

		CurrentNode->UserData = CurrentNode->TrianglesInCell.size();
	};

	JITTER_MANAGER.CalculateWithSDFJitterAsync(WorkOnNode, bSmootherResult);
}

void TriangleCountLayerProducer::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	if (!TRIANGLE_COUNT_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	NewLayer.SetType(TRIANGLE_DENSITY);

	TRIANGLE_COUNT_LAYER_PRODUCER.bWaitForJitterResult = false;
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(NewLayer);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetType(LAYER_TYPE::VECTOR_DISPERSION);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle density"));
	LAYER_MANAGER.SetActiveLayerIndex(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 1);
}

void TriangleCountLayerProducer::CalculateOnWholeModel(FEMesh* Mesh)
{
	if (Mesh == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](SDFNode* CurrentNode) {
		if (CurrentNode->TrianglesInCell.empty())
			return;

		CurrentNode->UserData = CurrentNode->TrianglesInCell.size();
	};

	JITTER_MANAGER.CalculateOnWholeModel(WorkOnNode);
}