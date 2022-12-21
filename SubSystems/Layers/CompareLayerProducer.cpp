#include "CompareLayerProducer.h"

namespace FocalEngine
{
	struct RugosityMeshLayerDebugInfo;
}

using namespace FocalEngine;

CompareLayerProducer* CompareLayerProducer::Instance = nullptr;

CompareLayerProducer::CompareLayerProducer() {}
CompareLayerProducer::~CompareLayerProducer() {}

bool CompareLayerProducer::GetShouldNormalize()
{
	return bNormalize;
}

void CompareLayerProducer::SetShouldNormalize(bool NewValue)
{
	bNormalize = NewValue;
}

std::vector<float> CompareLayerProducer::Normalize(std::vector<float> Original)
{
	std::vector<float> Result;

	float Min = FLT_MAX;
	float Max = -FLT_MAX;
	for (size_t i = 0; i < Original.size(); i++)
	{
		Min = std::min(Min, Original[i]);
		Max = std::max(Max, Original[i]);
	}

	Result.resize(Original.size());
	for (size_t i = 0; i < Original.size(); i++)
	{
		Result[i] = (Original[i] - Min) / (Max - Min);
	}

	return Result;
}

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

	if (bNormalize)
	{
		std::vector<float> FirstLayerData = Normalize(First->TrianglesToData);
		std::vector<float> SecondLayerData = Normalize(Second->TrianglesToData);

		for (size_t i = 0; i < First->TrianglesToData.size(); i++)
		{
			NewData[i] = abs(FirstLayerData[i] - SecondLayerData[i]);
		}

		NewData = Normalize(NewData);
	}
	else
	{
		for (size_t i = 0; i < First->TrianglesToData.size(); i++)
		{
			NewData[i] = abs(First->TrianglesToData[i] - Second->TrianglesToData[i]);
		}
	}

	Result.TrianglesToData = NewData;
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Compare"));

	Result.DebugInfo = new MeshLayerDebugInfo();
	Result.DebugInfo->Type = "CompareMeshLayerDebugInfo";
	Result.DebugInfo->AddEntry("Start time", StarTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	std::string TempString = bNormalize ? "Yes" : "No";
	Result.DebugInfo->AddEntry("Normalized", TempString);
	Result.DebugInfo->AddEntry("First layer caption", MESH_MANAGER.ActiveMesh->Layers[FirstLayer].GetCaption());
	Result.DebugInfo->AddEntry("Second layer caption", MESH_MANAGER.ActiveMesh->Layers[SecondLayer].GetCaption());

	return Result;
}