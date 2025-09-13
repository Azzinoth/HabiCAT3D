#include "AreaLayerProducer.h"
using namespace FocalEngine;

AreaLayerProducer::AreaLayerProducer() {}
AreaLayerProducer::~AreaLayerProducer() {}

DataLayer AreaLayerProducer::Calculate()
{
	DataLayer Result(DATA_SOURCE_TYPE::MESH);
	Result.SetType(LAYER_TYPE::TRIANGLE_AREA);

	if (!ANALYSIS_OBJECT_MANAGER.HaveMeshData())
		return Result;

	if (ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData == nullptr)
		return Result;

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size(); i++)
	{
		Result.ElementsToData.push_back(static_cast<float>(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->TrianglesArea[i]));
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle area"));
	Result.DebugInfo = new DataLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StartTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return Result;
}