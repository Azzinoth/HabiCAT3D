#include "HeightLayerProducer.h"
using namespace FocalEngine;

HeightLayerProducer::HeightLayerProducer() {}
HeightLayerProducer::~HeightLayerProducer() {}

DataLayer HeightLayerProducer::Calculate()
{
	DataLayer Result(DATA_SOURCE_TYPE::MESH);
	Result.SetType(LAYER_TYPE::HEIGHT);

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr || CurrentObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

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

		Result.ElementsToData.push_back(static_cast<float>(AverageTriangleHeight / 3.0));
		Min = std::min(float(Min), Result.ElementsToData.back());
	}

	// Smallest value should be 0.0f.
	for (size_t i = 0; i < Result.ElementsToData.size(); i++)
	{
		Result.ElementsToData[i] += static_cast<float>(abs(Min));
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Height"));
	Result.DebugInfo = new DataLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StartTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return Result;
}