#include "LayerManager.h"
using namespace FocalEngine;

LayerManager* LayerManager::Instance = nullptr;

LayerManager::LayerManager()
{
	RUGOSITY_MANAGER.SetOnRugosityCalculationsEndCallback(OnRugosityCalculationsEnd);
}

LayerManager::~LayerManager() {}

int LayerManager::FindHigestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List)
{
	int Result = 0;
	std::transform(Prefix.begin(), Prefix.end(), Prefix.begin(), [](const unsigned char C) { return std::tolower(C); });
	std::transform(Delimiter.begin(), Delimiter.end(), Delimiter.begin(), [](const unsigned char C) { return std::tolower(C); });

	for (size_t i = 0; i < List.size(); i++)
	{
		std::transform(List[i].begin(), List[i].end(), List[i].begin(), [](const unsigned char C) { return std::tolower(C); });

		int PrefixPos = List[i].find(Prefix);
		if (PrefixPos != std::string::npos)
		{
			int DelimiterPos = List[i].find(Delimiter);
			if (DelimiterPos != std::string::npos && List[i].size() > Prefix.size() + Delimiter.size())
			{
				std::string PostfixPart = List[i].substr(DelimiterPos + 1, List[i].size() - (DelimiterPos + 1));
				Result = std::max(Result, atoi(PostfixPart.c_str()));
			}
		}
	}

	return Result;
}

std::string LayerManager::SuitableNewLayerCaption(std::string Base)
{
	std::string Result = Base;

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return Result;

	std::vector<std::string> CaptionList;
	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size(); i++)
	{
		CaptionList.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].GetCaption());
	}

	int IndexToAdd = FindHigestIntPostfix(Base, "_", CaptionList);
	IndexToAdd++;
	if (IndexToAdd < 2)
	{
		std::transform(Base.begin(), Base.end(), Base.begin(), [](const unsigned char C) { return std::tolower(C); });
		for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size(); i++)
		{
			std::string CurrentCaption = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].GetCaption();
			std::transform(CurrentCaption.begin(), CurrentCaption.end(), CurrentCaption.begin(), [](const unsigned char C) { return std::tolower(C); });

			if (CurrentCaption.find(Base) != std::string::npos)
			{
				IndexToAdd = 2;
				break;
			}
		}
	}

	if (IndexToAdd > 1)
		Result += "_" + std::to_string(IndexToAdd);

	return Result;
	return Result;
}

//bool LayerManager::AddLayer(std::vector<float> TrianglesToData)
//{
//	
//}
//
//bool LayerManager::AddLayer(MeshLayer NewLayer)
//{
//	
//}

void LayerManager::SetActiveLayerIndex(const int NewLayerIndex)
{
	if (MESH_MANAGER.ActiveMesh == nullptr || NewLayerIndex < -1 || NewLayerIndex >= int(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size()))
		return;

	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->CurrentLayerIndex = NewLayerIndex;

	if (NewLayerIndex != -1)
		MESH_MANAGER.ActiveMesh->ComplexityMetricDataToGPU(NewLayerIndex);

	for (size_t i = 0; i < ClientAfterActiveLayerChangedCallbacks.size(); i++)
	{
		if (ClientAfterActiveLayerChangedCallbacks[i] == nullptr)
			continue;

		ClientAfterActiveLayerChangedCallbacks[i]();
	}
}

void LayerManager::AddActiveLayerChangedCallback(std::function<void()> Func)
{
	ClientAfterActiveLayerChangedCallbacks.push_back(Func);
}

int LayerManager::GetActiveLayerIndex()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return -1;

	return COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->CurrentLayerIndex;
}

MeshLayer* LayerManager::GetActiveLayer()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return nullptr;

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->CurrentLayerIndex == -1)
		return nullptr;

	return &COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->CurrentLayerIndex];
}

float LayerManager::FindStandardDeviation(std::vector<float> DataPoints)
{
	float Mean = 0.0f;
	for (int i = 0; i < DataPoints.size(); i++)
	{
		Mean += DataPoints[i];
	}
	Mean /= DataPoints.size();

	float Variance = 0.0f;
	for (int i = 0; i < DataPoints.size(); i++)
	{
		DataPoints[i] -= Mean;
		DataPoints[i] = std::pow(DataPoints[i], 2.0);
		Variance += DataPoints[i];
	}
	Variance /= DataPoints.size();

	return std::sqrt(Variance);
}

void LayerManager::OnRugosityCalculationsEnd(MeshLayer NewLayer)
{
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(NewLayer);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetType(LAYER_TYPE::RUGOSITY);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Rugosity"));

	LAYER_MANAGER.SetActiveLayerIndex(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 1);

	if (RUGOSITY_MANAGER.bCalculateStandardDeviation)
	{
		uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
		std::vector<float> TrianglesToStandardDeviation;
		for (int i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
		{
			std::vector<float> CurrentTriangleResults;
			for (int j = 0; j < JITTER_MANAGER.JitterToDoCount; j++)
			{
				CurrentTriangleResults.push_back(JITTER_MANAGER.PerJitterResult[j][i]);
			}

			TrianglesToStandardDeviation.push_back(LAYER_MANAGER.FindStandardDeviation(CurrentTriangleResults));
		}
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(TrianglesToStandardDeviation);
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));

		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo = new MeshLayerDebugInfo();
		MeshLayerDebugInfo* DebugInfo = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo;
		DebugInfo->Type = "RugosityStandardDeviationLayerDebugInfo";
		DebugInfo->AddEntry("Start time", StarTime);
		DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
		DebugInfo->AddEntry("Source layer ID", COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 2].GetID());
		DebugInfo->AddEntry("Source layer Caption", COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 2].GetCaption());
	}
}