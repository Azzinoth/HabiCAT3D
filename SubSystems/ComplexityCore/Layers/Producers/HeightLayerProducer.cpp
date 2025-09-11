#include "HeightLayerProducer.h"
using namespace FocalEngine;

HeightLayerProducer::HeightLayerProducer() {}
HeightLayerProducer::~HeightLayerProducer() {}

DataLayer HeightLayerProducer::Calculate()
{
	DataLayer Result(DATA_SOURCE_TYPE::MESH);
	Result.SetType(HEIGHT);

	if (!ANALYSIS_OBJECT_MANAGER.HaveMeshData())
		return Result;

	MeshGeometryData* CurrentMesh = ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData;
	if (CurrentMesh == nullptr)
		return Result;

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	double Min = DBL_MAX;
	for (size_t i = 0; i < CurrentMesh->Triangles.size(); i++)
	{
		double AverageTriangleHeight = 0.0;
		for (size_t j = 0; j < 3; j++)
		{
			double CurrentHeight = glm::dot(glm::vec3(CurrentMesh->Position->GetWorldMatrix() * glm::vec4(CurrentMesh->Triangles[i][j], 1.0)), CurrentMesh->GetAverageNormal());
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