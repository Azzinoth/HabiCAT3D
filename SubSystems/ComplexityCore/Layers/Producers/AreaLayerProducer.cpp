#include "AreaLayerProducer.h"
using namespace FocalEngine;

AreaLayerProducer::AreaLayerProducer() {}
AreaLayerProducer::~AreaLayerProducer() {}

DataLayer* AreaLayerProducer::Calculate()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr || ActiveObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return nullptr;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	DataLayer* NewLayer = new DataLayer({ ActiveObject->GetID() });
	NewLayer->SetType(LAYER_TYPE::TRIANGLE_AREA);

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		NewLayer->ElementsToData.push_back(static_cast<float>(CurrentMeshAnalysisData->TrianglesArea[i]));
	}
	
	NewLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle area"));
	NewLayer->DebugInfo = new DataLayerDebugInfo();

	NewLayer->DebugInfo->AddEntry("Start time", StartTime);
	NewLayer->DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return NewLayer;
}