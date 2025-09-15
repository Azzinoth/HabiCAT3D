#include "AreaLayerProducer.h"
using namespace FocalEngine;

AreaLayerProducer::AreaLayerProducer() {}
AreaLayerProducer::~AreaLayerProducer() {}

DataLayer AreaLayerProducer::Calculate()
{
	DataLayer Result(DATA_SOURCE_TYPE::MESH);
	Result.SetType(LAYER_TYPE::TRIANGLE_AREA);

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		Result.ElementsToData.push_back(static_cast<float>(CurrentMeshAnalysisData->TrianglesArea[i]));
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle area"));
	Result.DebugInfo = new DataLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StartTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return Result;
}