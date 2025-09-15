#include "LayerManager.h"
using namespace FocalEngine;

LayerManager::LayerManager() {}
LayerManager::~LayerManager() {}

void LayerManager::AddLayer(DATA_SOURCE_TYPE LayerDataSource, std::vector<float> ElementsToData)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr || CurrentObject->GetType() != LayerDataSource)
		return;

	switch (LayerDataSource)
	{
		case DATA_SOURCE_TYPE::MESH:
		{
			MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
			if (CurrentMeshAnalysisData == nullptr)
				return;

			if (ElementsToData.size() == CurrentMeshAnalysisData->Triangles.size())
				Layers.push_back(DataLayer(DATA_SOURCE_TYPE::MESH, ElementsToData));

			break;
		}

		case DATA_SOURCE_TYPE::POINT_CLOUD:
		{
			break;
		}
	}
}

void LayerManager::AddLayer(DataLayer NewLayer)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr || CurrentObject->GetType() != NewLayer.GetDataSourceType())
		return;

	switch (NewLayer.GetDataSourceType())
	{
		case DATA_SOURCE_TYPE::MESH:
		{
			MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
			if (CurrentMeshAnalysisData == nullptr)
				return;

			if (NewLayer.ElementsToData.size() == CurrentMeshAnalysisData->Triangles.size())
			{
				Layers.push_back(NewLayer);
				Layers.back().ComputeStatistics();
			}
			
			break;
		}

		case DATA_SOURCE_TYPE::POINT_CLOUD:
		{
			PointCloudAnalysisData* CurrentPointCloudAnalysisData = static_cast<PointCloudAnalysisData*>(CurrentObject->GetGeometryData());
			if (CurrentPointCloudAnalysisData == nullptr)
				return;

			if (NewLayer.ElementsToData.size() == CurrentPointCloudAnalysisData->RawPointCloudData.size())
			{
				Layers.push_back(NewLayer);
				Layers.back().ComputeStatistics();
			}

			break;
		}
	}
}

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

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	std::vector<std::string> CaptionList;
	for (size_t i = 0; i < Layers.size(); i++)
	{
		CaptionList.push_back(Layers[i].GetCaption());
	}

	int IndexToAdd = FindHighestIntPostfix(Base, "_", CaptionList);
	IndexToAdd++;
	if (IndexToAdd < 2)
	{
		std::transform(Base.begin(), Base.end(), Base.begin(), [](const unsigned char Character) {
			return std::tolower(Character);
		});

		for (size_t i = 0; i < Layers.size(); i++)
		{
			std::string CurrentCaption = Layers[i].GetCaption();
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

#include "../../AnalysisObjectManager.h"
void LayerManager::SetActiveLayerIndex(const int NewLayerIndex)
{
	if (NewLayerIndex < -1 || NewLayerIndex >= int(Layers.size()))
		return;

	if (NewLayerIndex == -1)
	{
		if (CurrentLayerIndex != -1)
		{
			// FIX ME: Not sure if that is a good solution.
			DataLayer& CurrentLayer = Layers[CurrentLayerIndex];
			if (CurrentLayer.GetDataSourceType() == DATA_SOURCE_TYPE::POINT_CLOUD)
			{
				AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
				if (CurrentObject == nullptr)
					return;

				PointCloudAnalysisData* CurrentPointCloudAnalysisData = static_cast<PointCloudAnalysisData*>(CurrentObject->GetGeometryData());
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
				FEEntity* PointCloudEntity = CurrentObject->GetEntity();
				FEPointCloud* OldPointCloud = static_cast<FEPointCloud*>(CurrentObject->GetEngineResource());
				if (PointCloudEntity != nullptr)
				{
					PointCloudEntity->RemoveComponent<FEPointCloudComponent>();
					RESOURCE_MANAGER.DeleteFEPointCloud(OldPointCloud);
					CurrentObject->EngineResource = PointCloud;
					PointCloudEntity->AddComponent<FEPointCloudComponent>(PointCloud);
				}
				/*ANALYSIS_OBJECT_MANAGER.CurrentPointCloudEntity->RemoveComponent<FEPointCloudComponent>();
				RESOURCE_MANAGER.DeleteFEPointCloud(ANALYSIS_OBJECT_MANAGER.CurrentPointCloud);
				ANALYSIS_OBJECT_MANAGER.CurrentPointCloud = PointCloud;
				ANALYSIS_OBJECT_MANAGER.CurrentPointCloudEntity->AddComponent<FEPointCloudComponent>(ANALYSIS_OBJECT_MANAGER.CurrentPointCloud);*/
			}
		}

		CurrentLayerIndex = -1;

		for (size_t i = 0; i < ClientAfterActiveLayerChangedCallbacks.size(); i++)
		{
			if (ClientAfterActiveLayerChangedCallbacks[i] == nullptr)
				continue;

			ClientAfterActiveLayerChangedCallbacks[i]();
		}

		return;
	}

	DataLayer& NewLayer = Layers[NewLayerIndex];
	if (NewLayer.GetDataSourceType() == DATA_SOURCE_TYPE::MESH)
	{
		AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (CurrentObject == nullptr || CurrentObject->GetType() != DATA_SOURCE_TYPE::MESH)
			return;

		MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
		if (CurrentMeshAnalysisData == nullptr || CurrentObject->GetEngineResource() == nullptr)
			return;

		CurrentLayerIndex = NewLayerIndex;

		if (NewLayerIndex != -1)
			ANALYSIS_OBJECT_MANAGER.ComplexityMetricDataToGPU(NewLayerIndex);
	}
	else if (NewLayer.GetDataSourceType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (CurrentObject == nullptr || CurrentObject->GetType() != DATA_SOURCE_TYPE::POINT_CLOUD)
			return;

		PointCloudAnalysisData* CurrentPointCloudAnalysisData = static_cast<PointCloudAnalysisData*>(CurrentObject->GetGeometryData());
		if (CurrentPointCloudAnalysisData == nullptr || CurrentObject->GetEngineResource() == nullptr)
			return;

		CurrentLayerIndex = NewLayerIndex;

		if (NewLayerIndex != -1)
			ANALYSIS_OBJECT_MANAGER.ComplexityMetricDataToGPU(NewLayerIndex);
	}

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
	if (ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject() == nullptr)
		return -1;

	return CurrentLayerIndex;
}

DataLayer* LayerManager::GetActiveLayer()
{
	if (ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject() == nullptr)
		return nullptr;

	if (CurrentLayerIndex == -1)
		return nullptr;

	return &Layers[CurrentLayerIndex];
}

size_t LayerManager::GetLayerCount()
{
	return Layers.size();
}