#include "CompareLayerProducer.h"

namespace FocalEngine
{
	struct RugosityMeshLayerDebugInfo;
}

using namespace FocalEngine;

CompareLayerProducer* CompareLayerProducer::Instance = nullptr;

CompareLayerProducer::CompareLayerProducer() {}
CompareLayerProducer::~CompareLayerProducer() {}

MeshLayer CompareLayerProducer::Calculate(const int FirstLayer, const int SecondLayer)
{
	MeshLayer Result;

	if (MESH_MANAGER.ActiveMesh == nullptr || FirstLayer == -1 || SecondLayer == -1)
		return Result;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	MeshLayer* First = &MESH_MANAGER.ActiveMesh->Layers[FirstLayer];
	MeshLayer* Second = &MESH_MANAGER.ActiveMesh->Layers[SecondLayer];

	std::vector<float> NewData;
	NewData.resize(First->TrianglesToData.size());

	//float Min = FLT_MAX;
	//float Max = -FLT_MAX;
	for (size_t i = 0; i < First->TrianglesToData.size(); i++)
	{
		NewData[i] = First->TrianglesToData[i] - Second->TrianglesToData[i];
		//Min = std::min(Min, NewData[i]);
		//Max = std::max(Max, NewData[i]);
	}

	/*for (size_t i = 0; i < First->TrianglesToData.size(); i++)
	{
		float NormalizedValue = (NewData[i] - Min) / (Max - Min);
		if (NormalizedValue > 1.0f || NormalizedValue < 0.0f)
		{
			int y = 0;
			y++;
		}

		NewData[i] = NormalizedValue;
	}*/

	Result.TrianglesToData = NewData;
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Compare"));

	Result.DebugInfo = new MeshLayerDebugInfo();
	Result.DebugInfo->Type = "CompareMeshLayerDebugInfo";
	Result.DebugInfo->AddEntry("Start time", StarTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	Result.DebugInfo->AddEntry("First layer caption", MESH_MANAGER.ActiveMesh->Layers[FirstLayer].GetCaption());
	Result.DebugInfo->AddEntry("Second layer caption", MESH_MANAGER.ActiveMesh->Layers[SecondLayer].GetCaption());

	return Result;
}