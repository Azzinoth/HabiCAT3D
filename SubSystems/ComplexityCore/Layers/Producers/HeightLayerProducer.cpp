#include "HeightLayerProducer.h"
using namespace FocalEngine;

HeightLayerProducer::HeightLayerProducer() {}
HeightLayerProducer::~HeightLayerProducer() {}

DataLayer* HeightLayerProducer::Calculate()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr || ActiveObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return nullptr;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return nullptr;

	DataLayer* NewLayer = new DataLayer({ ActiveObject->GetID() });
	NewLayer->SetType(LAYER_TYPE::HEIGHT);

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	glm::mat4 WorldMatrix = CurrentMeshAnalysisData->Position->GetWorldMatrix();
	double Min = DBL_MAX;
	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		double AverageTriangleHeight = 0.0;
		for (size_t j = 0; j < 3; j++)
		{
			double CurrentHeight = glm::dot(glm::vec3(WorldMatrix * glm::vec4(CurrentMeshAnalysisData->Triangles[i][j], 1.0)), CurrentMeshAnalysisData->GetAverageNormal());
			AverageTriangleHeight += CurrentHeight;
		}

		NewLayer->ElementsToData.push_back(static_cast<float>(AverageTriangleHeight / 3.0));
		Min = std::min(float(Min), NewLayer->ElementsToData.back());
	}

	// Smallest value should be 0.0f.
	for (size_t i = 0; i < NewLayer->ElementsToData.size(); i++)
	{
		NewLayer->ElementsToData[i] += static_cast<float>(abs(Min));
	}
	
	NewLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Height"));
	NewLayer->DebugInfo = new DataLayerDebugInfo();

	NewLayer->DebugInfo->AddEntry("Start time", StartTime);
	NewLayer->DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return NewLayer;
}