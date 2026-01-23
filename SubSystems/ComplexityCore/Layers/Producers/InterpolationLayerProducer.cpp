#include "InterpolationLayerProducer.h"
using namespace FocalEngine;

InterpolationLayerProducer::InterpolationLayerProducer() {}
InterpolationLayerProducer::~InterpolationLayerProducer() {}

DataLayer* InterpolationLayerProducer::Calculate(std::vector<DataLayer*> LayersToInterpolate)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr || ActiveObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return nullptr;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return nullptr;

	if (LayersToInterpolate.empty() || LayersToInterpolate.size() < 2)
		return nullptr;

	DataLayer* NewLayer = new DataLayer({ ActiveObject->GetID() });
	NewLayer->SetType(LAYER_TYPE::INTERPOLATION);
	NewLayer->InterpolationData = new LayerInterpolationData();
	LayerInterpolationData* NewInterpolationData = NewLayer->InterpolationData;

	if (LayersToInterpolate.size() > MAX_INTERPOLATION_LAYERS)
		LayersToInterpolate.erase(LayersToInterpolate.begin() + MAX_INTERPOLATION_LAYERS, LayersToInterpolate.end());
	
	for (size_t i = 0; i < LayersToInterpolate.size(); i++)
		NewInterpolationData->UsedLayerIDs.push_back(LayersToInterpolate[i]->GetID());

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	if (NewInterpolationData != nullptr && LayersToInterpolate.size() > 0)
	{
		for (size_t i = 0; i < LayersToInterpolate.size(); i++)
		{
			if (LayersToInterpolate[i]->RawData.empty())
				LayersToInterpolate[i]->FillRawData();

			NewInterpolationData->RawData.push_back(LayersToInterpolate[i]->RawData);
		}
	}
	
	NewLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Interpolation"));
	NewLayer->DebugInfo = new DataLayerDebugInfo();

	NewLayer->DebugInfo->AddEntry("Start time", StartTime);
	NewLayer->DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return NewLayer;
}