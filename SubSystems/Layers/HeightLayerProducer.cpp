#include "HeightLayerProducer.h"
using namespace FocalEngine;

HeightLayerProducer* HeightLayerProducer::Instance = nullptr;

HeightLayerProducer::HeightLayerProducer() {}
HeightLayerProducer::~HeightLayerProducer() {}

MeshLayer HeightLayerProducer::Calculate(FEMesh* Mesh)
{
	MeshLayer Result;

	if (Mesh == nullptr)
		return Result;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	double Min = DBL_MAX;
	for (size_t i = 0; i < Mesh->Triangles.size(); i++)
	{
		float AverageTriangleHeight = 0.0f;
		for (size_t j = 0; j < 3; j++)
		{
			double CurrentHeight = glm::dot(glm::vec3(Mesh->Position->getTransformMatrix() * glm::vec4(Mesh->Triangles[i][j], 1.0)), Mesh->AverageNormal);
			AverageTriangleHeight += CurrentHeight;
		}

		Result.TrianglesToData.push_back(AverageTriangleHeight / 3.0f);
		Min = std::min(float(Min), Result.TrianglesToData.back());
	}

	// Smallest value should be 0.0f.
	for (size_t i = 0; i < Result.TrianglesToData.size(); i++)
	{
		Result.TrianglesToData[i] += abs(Min);
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Height"));
	Result.DebugInfo = new MeshLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StarTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return Result;
}