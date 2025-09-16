#include "LayerManager.h"
#include "../../AnalysisObjectManager.h"
using namespace FocalEngine;

LayerEvent::LayerEvent() {}

LayerEvent::LayerEvent(LAYER_EVENT_TYPE Type, std::string ParentObjectID, std::string PrimaryLayerID, std::vector<std::string> OtherLayerIDs)
{
	this->Type = Type;
	this->ParentObjectID = ParentObjectID;
	this->PrimaryLayerID = PrimaryLayerID;
	this->OtherLayerIDs = OtherLayerIDs;
}

LayerManager::LayerManager() {}
LayerManager::~LayerManager() {}

int LayerManager::FindHighestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List)
{
	int Result = 0;
	std::transform(Prefix.begin(), Prefix.end(), Prefix.begin(), [](const unsigned char C) { return std::tolower(C); });
	std::transform(Delimiter.begin(), Delimiter.end(), Delimiter.begin(), [](const unsigned char C) { return std::tolower(C); });

	for (size_t i = 0; i < List.size(); i++)
	{
		std::transform(List[i].begin(), List[i].end(), List[i].begin(), [](const unsigned char C) { return std::tolower(C); });

		size_t PrefixPos = List[i].find(Prefix);
		if (PrefixPos != std::string::npos)
		{
			size_t DelimiterPos = List[i].find(Delimiter);
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

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return Result;

	std::vector<std::string> CaptionList;
	for (size_t i = 0; i < ActiveObject->Layers.size(); i++)
	{
		CaptionList.push_back(ActiveObject->Layers[i]->GetCaption());
	}

	int IndexToAdd = FindHighestIntPostfix(Base, "_", CaptionList);
	IndexToAdd++;
	if (IndexToAdd < 2)
	{
		std::transform(Base.begin(), Base.end(), Base.begin(), [](const unsigned char Character) {
			return std::tolower(Character);
		});

		for (size_t i = 0; i < ActiveObject->Layers.size(); i++)
		{
			std::string CurrentCaption = ActiveObject->Layers[i]->GetCaption();
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
}

void LayerManager::AddActiveLayerChangedCallback(std::function<void()> Func)
{
	ClientAfterActiveLayerChangedCallbacks.push_back(Func);
}

int LayerManager::GetActiveLayerIndex()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return -1;

	return ActiveObject->GetActiveLayerIndex();
}

DataLayer* LayerManager::GetActiveLayer()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return nullptr;

	return ActiveObject->GetActiveLayer();
}

size_t LayerManager::GetLayerCount()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return 0;

	return ActiveObject->GetLayerCount();
}

void LayerManager::PropagateLayerEvent(LayerEvent Event)
{
	if (Event.Type == LAYER_EVENT_TYPE::UNKNOWN)
		return;

	if (Event.Type == LAYER_EVENT_TYPE::LAYER_ACTIVE_ID_CHANGED)
	{
		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (ActiveObject == nullptr)
			return;

		// FIX ME: Is it good idea to reset the colors here?
		if (ActiveObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
		{
			PointCloudAnalysisData* CurrentPointCloudAnalysisData = static_cast<PointCloudAnalysisData*>(ActiveObject->GetAnalysisData());
			if (CurrentPointCloudAnalysisData == nullptr)
				return;

			for (size_t i = 0; i < CurrentPointCloudAnalysisData->RawPointCloudData.size(); i++)
			{
				std::vector<unsigned char> OriginalColor = CurrentPointCloudAnalysisData->OriginalColors[i];

				CurrentPointCloudAnalysisData->RawPointCloudData[i].R = OriginalColor[0];
				CurrentPointCloudAnalysisData->RawPointCloudData[i].G = OriginalColor[1];
				CurrentPointCloudAnalysisData->RawPointCloudData[i].B = OriginalColor[2];
				CurrentPointCloudAnalysisData->RawPointCloudData[i].A = OriginalColor[3];
			}

			FEPointCloud* PointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(CurrentPointCloudAnalysisData->RawPointCloudData);

			// FIX ME: Should it be here?
			FEEntity* PointCloudEntity = ActiveObject->GetEntity();
			FEPointCloud* OldPointCloud = static_cast<FEPointCloud*>(ActiveObject->GetEngineResource());
			if (PointCloudEntity != nullptr)
			{
				PointCloudEntity->RemoveComponent<FEPointCloudComponent>();
				RESOURCE_MANAGER.DeleteFEPointCloud(OldPointCloud);
				ActiveObject->EngineResource = PointCloud;
				PointCloudEntity->AddComponent<FEPointCloudComponent>(PointCloud);
			}
		}

		ANALYSIS_OBJECT_MANAGER.ComplexityMetricDataToGPU(Event.PrimaryLayerID);
	}

	// FIX ME: Right now ClientAfterActiveLayerChangedCallbacks was not created to be called on every event.
	if (Event.Type == LAYER_EVENT_TYPE::LAYER_REMOVED)
		return;

	for (size_t i = 0; i < ClientAfterActiveLayerChangedCallbacks.size(); i++)
	{
		if (ClientAfterActiveLayerChangedCallbacks[i] == nullptr)
			continue;
		ClientAfterActiveLayerChangedCallbacks[i]();
	}
}