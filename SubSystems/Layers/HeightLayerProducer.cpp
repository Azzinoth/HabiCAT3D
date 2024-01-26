#include "HeightLayerProducer.h"
using namespace FocalEngine;

HeightLayerProducer* HeightLayerProducer::Instance = nullptr;

HeightLayerProducer::HeightLayerProducer() {}
HeightLayerProducer::~HeightLayerProducer() {}

MeshLayer HeightLayerProducer::Calculate()
{
	MeshLayer Result;
	Result.SetType(HEIGHT);

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return Result;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto& ComplexityMetric = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo;

	double Min = DBL_MAX;
	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		float AverageTriangleHeight = 0.0f;
		for (size_t j = 0; j < 3; j++)
		{
			double CurrentHeight = glm::dot(glm::vec3(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->getTransformMatrix() * glm::vec4(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[i][j], 1.0)), COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AverageNormal);
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