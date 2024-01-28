#include "AreaLayerProducer.h"
using namespace FocalEngine;

AreaLayerProducer* AreaLayerProducer::Instance = nullptr;

AreaLayerProducer::AreaLayerProducer() {}
AreaLayerProducer::~AreaLayerProducer() {}

MeshLayer AreaLayerProducer::Calculate()
{
	MeshLayer Result;
	Result.SetType(TRIANGLE_AREA);

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return Result;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		Result.TrianglesToData.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[i]);
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle area"));
	Result.DebugInfo = new MeshLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StarTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return Result;
}