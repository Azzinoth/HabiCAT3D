#include "PointDensityLayerProducer.h"
using namespace FocalEngine;

PointDensityLayerProducer::PointDensityLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

PointDensityLayerProducer::~PointDensityLayerProducer() {}

void PointDensityLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	if (!ANALYSIS_OBJECT_MANAGER.HavePointCloudData())
		return;

	bWaitForJitterResult = true;
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](GridNode* CurrentNode) {
		if (CurrentNode->PointsInCell.empty())
			return;

		CurrentNode->UserData = static_cast<double>(CurrentNode->PointsInCell.size());
	};

	JITTER_MANAGER.CalculateWithGridJitterAsync(WorkOnNode, bSmootherResult);
}

void PointDensityLayerProducer::OnJitterCalculationsEnd(DataLayer NewLayer)
{
	if (!POINT_DENSITY_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	NewLayer.SetDataSourceType(DATA_SOURCE_TYPE::POINT_CLOUD);
	NewLayer.SetType(LAYER_TYPE::POINT_DENSITY);

	POINT_DENSITY_LAYER_PRODUCER.bWaitForJitterResult = false;
	NewLayer.DebugInfo->Type = "PointDensityDataLayerDebugInfo";
	LAYER_MANAGER.AddLayer(NewLayer);
	LAYER_MANAGER.Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Point density"));
	LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(LAYER_MANAGER.Layers.size() - 1));

	//float Min = FLT_MAX;
	//float Max = -FLT_MAX;
	//for (size_t i = 0; i < NewLayer.ElementsToData.size(); i++)
	//{
	//	if (NewLayer.ElementsToData[i] < Min)
	//		Min = NewLayer.ElementsToData[i];
	//	if (NewLayer.ElementsToData[i] > Max)
	//		Max = NewLayer.ElementsToData[i];
	//}

	//PointCloudGeometryData* CurrentPointCloudData = ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData;

	//// Update color based on min/max
	//for (size_t i = 0; i < CurrentPointCloudData->RawPointCloudData.size(); i++)
	//{
	//	float NormalizedValue = (NewLayer.ElementsToData[i] - Min) / (Max - Min);
	//	glm::vec3 NewColor = GetTurboColorMap(NormalizedValue);
	//	CurrentPointCloudData->RawPointCloudData[i].R = static_cast<unsigned char>(NewColor.x * 255.0f);
	//	CurrentPointCloudData->RawPointCloudData[i].G = static_cast<unsigned char>(NewColor.y * 255.0f);
	//	CurrentPointCloudData->RawPointCloudData[i].B = static_cast<unsigned char>(NewColor.z * 255.0f);
	//}

	//FEPointCloud* PointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(CurrentPointCloudData->RawPointCloudData);

	//SCENE_RESOURCES.CurrentPointCloudEntity->RemoveComponent<FEPointCloudComponent>();
	//RESOURCE_MANAGER.DeleteFEPointCloud(SCENE_RESOURCES.CurrentPointCloud);
	//SCENE_RESOURCES.CurrentPointCloud = PointCloud;
	//SCENE_RESOURCES.CurrentPointCloudEntity->AddComponent<FEPointCloudComponent>(SCENE_RESOURCES.CurrentPointCloud);
}