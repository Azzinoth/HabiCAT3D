#include "TriangleCountLayerProducer.h"
using namespace FocalEngine;

TriangleCountLayerProducer::TriangleCountLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

TriangleCountLayerProducer::~TriangleCountLayerProducer() {}

void TriangleCountLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	if (!ANALYSIS_OBJECT_MANAGER.HaveMeshData())
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

void TriangleCountLayerProducer::OnJitterCalculationsEnd(DataLayer NewLayer)
{
	if (!ANALYSIS_OBJECT_MANAGER.HaveMeshData())
		return;

	if (!TRIANGLE_COUNT_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	NewLayer.SetType(TRIANGLE_DENSITY);

	TRIANGLE_COUNT_LAYER_PRODUCER.bWaitForJitterResult = false;
	LAYER_MANAGER.AddLayer(NewLayer);
	LAYER_MANAGER.Layers.back().SetType(LAYER_TYPE::VECTOR_DISPERSION);
	LAYER_MANAGER.Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle density"));
	LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(LAYER_MANAGER.Layers.size() - 1));
}

void TriangleCountLayerProducer::CalculateOnWholeModel()
{
	if (!ANALYSIS_OBJECT_MANAGER.HaveMeshData())
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