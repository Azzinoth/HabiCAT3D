#include "CompareLayerProducer.h"

namespace FocalEngine
{
	struct RugosityDataLayerDebugInfo;
}
using namespace FocalEngine;

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
	Result.resize(Original.size());

	float MaxPositive = 0.0f;
	float MaxNegative = 0.0f;

	// Find the maximum absolute values for positive and negative numbers
	for (size_t i = 0; i < Original.size(); i++)
	{
		if (Original[i] > 0)
		{
			MaxPositive = std::max(MaxPositive, Original[i]);
		}
		else if (Original[i] < 0)
		{
			MaxNegative = std::min(MaxNegative, Original[i]);
		}
	}

	// Normalize the vector
	for (size_t i = 0; i < Original.size(); i++)
	{
		if (Original[i] > 0)
		{
			// Normalize positive values to 0 to 1
			Result[i] = Original[i] / MaxPositive;
		}
		else if (Original[i] < 0)
		{
			// Normalize negative values to -1 to 0
			Result[i] = Original[i] / std::abs(MaxNegative);
		}
		else
		{
			// Handle the case where the value is zero
			Result[i] = 0.0f;
		}
	}

	return Result;
}

DataLayer* CompareLayerProducer::Calculate(DataLayer* FirstLayer, DataLayer* SecondLayer)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr || ActiveObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return nullptr;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(ActiveObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return nullptr;

	if (FirstLayer == nullptr || SecondLayer == nullptr)
		return nullptr;

	if (FirstLayer->GetMainParentObject() == nullptr || SecondLayer->GetMainParentObject() == nullptr)
		return nullptr;

	if (FirstLayer->GetMainParentObject() != SecondLayer->GetMainParentObject())
		return nullptr;

	if (FirstLayer->GetType() != SecondLayer->GetType())
		return nullptr;

	DataLayer* NewLayer = new DataLayer({ ActiveObject->GetID() });
	NewLayer->SetType(LAYER_TYPE::COMPARE);

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	std::vector<float> NewData;
	NewData.resize(FirstLayer->ElementsToData.size());

	if (bNormalize)
	{
		std::vector<float> FirstLayerData = Normalize(FirstLayer->ElementsToData);
		std::vector<float> SecondLayerData = Normalize(SecondLayer->ElementsToData);

		for (size_t i = 0; i < FirstLayer->ElementsToData.size(); i++)
		{
			NewData[i] = FirstLayerData[i] - SecondLayerData[i];
		}

		NewData = Normalize(NewData);
	}
	else
	{
		for (size_t i = 0; i < FirstLayer->ElementsToData.size(); i++)
		{
			NewData[i] = FirstLayer->ElementsToData[i] - SecondLayer->ElementsToData[i];
		}

		bool bDeleteOutliers = true;

		if (bDeleteOutliers)
		{
			JITTER_MANAGER.AdjustOutliers(NewData, 0.01f, 0.99f);
		}
	}

	NewLayer->ElementsToData = NewData;
	NewLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Compare"));

	NewLayer->DebugInfo = new DataLayerDebugInfo();
	NewLayer->DebugInfo->Type = "CompareDataLayerDebugInfo";
	NewLayer->DebugInfo->AddEntry("Start time", StartTime);
	NewLayer->DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	std::string TempString = bNormalize ? "Yes" : "No";
	NewLayer->DebugInfo->AddEntry("Normalized", TempString);

	NewLayer->DebugInfo->AddEntry("First layer ID", FirstLayer->GetID());
	NewLayer->DebugInfo->AddEntry("First layer caption", FirstLayer->GetCaption());
	NewLayer->DebugInfo->AddEntry("Second layer ID", SecondLayer->GetID());
	NewLayer->DebugInfo->AddEntry("Second layer caption", SecondLayer->GetCaption());

	return NewLayer;
}