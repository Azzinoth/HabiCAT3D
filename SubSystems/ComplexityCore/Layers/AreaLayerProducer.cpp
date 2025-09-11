#include "AreaLayerProducer.h"
using namespace FocalEngine;

AreaLayerProducer::AreaLayerProducer() {}
AreaLayerProducer::~AreaLayerProducer() {}

DataLayer AreaLayerProducer::Calculate()
{
	DataLayer Result;
	Result.SetType(TRIANGLE_AREA);

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return Result;

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->CurrentMeshGeometryData == nullptr)
		return Result;

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->CurrentMeshGeometryData->Triangles.size(); i++)
	{
		Result.ElementsToData.push_back(static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->CurrentMeshGeometryData->TrianglesArea[i]));
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle area"));
	Result.DebugInfo = new DataLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StartTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return Result;
}