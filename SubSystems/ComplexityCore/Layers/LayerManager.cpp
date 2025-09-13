#include "LayerManager.h"
using namespace FocalEngine;

LayerManager::LayerManager() {}
LayerManager::~LayerManager() {}

void LayerManager::AddLayer(DATA_SOURCE_TYPE LayerDataSource, std::vector<float> ElementsToData)
{
	switch (LayerDataSource)
	{
		case DATA_SOURCE_TYPE::MESH:
		{
			if (ANALYSIS_OBJECT_MANAGER.HaveMeshData())
			{
				if (ElementsToData.size() == ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size())
					Layers.push_back(DataLayer(DATA_SOURCE_TYPE::MESH, ElementsToData));
			}

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
	switch (NewLayer.GetDataSourceType())
	{
		case DATA_SOURCE_TYPE::MESH:
		{
			if (ANALYSIS_OBJECT_MANAGER.HaveMeshData())
			{
				if (NewLayer.ElementsToData.size() == ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size())
				{
					Layers.push_back(NewLayer);
					Layers.back().ComputeStatistics();
				}	
			}
			
			break;
		}

		case DATA_SOURCE_TYPE::POINT_CLOUD:
		{
			if (ANALYSIS_OBJECT_MANAGER.HavePointCloudData())
			{
				if (NewLayer.ElementsToData.size() == ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData->RawPointCloudData.size())
				{
					Layers.push_back(NewLayer);
					Layers.back().ComputeStatistics();
				}
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

	if (!ANALYSIS_OBJECT_MANAGER.HaveAnyData())
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

#include "../../SceneResources.h"
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
				for (size_t i = 0; i < ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData->RawPointCloudData.size(); i++)
				{
					std::vector<unsigned char> OriginalColor = ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData->OriginalColors[i];

					ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData->RawPointCloudData[i].R = OriginalColor[0];
					ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData->RawPointCloudData[i].G = OriginalColor[1];
					ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData->RawPointCloudData[i].B = OriginalColor[2];
					ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData->RawPointCloudData[i].A = OriginalColor[3];
				}

				FEPointCloud* PointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData->RawPointCloudData);

				SCENE_RESOURCES.CurrentPointCloudEntity->RemoveComponent<FEPointCloudComponent>();
				RESOURCE_MANAGER.DeleteFEPointCloud(SCENE_RESOURCES.CurrentPointCloud);
				SCENE_RESOURCES.CurrentPointCloud = PointCloud;
				SCENE_RESOURCES.CurrentPointCloudEntity->AddComponent<FEPointCloudComponent>(SCENE_RESOURCES.CurrentPointCloud);
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
		if (!ANALYSIS_OBJECT_MANAGER.HaveMeshData() || SCENE_RESOURCES.ActiveMesh == nullptr)
			return;

		CurrentLayerIndex = NewLayerIndex;

		if (NewLayerIndex != -1)
			SCENE_RESOURCES.ComplexityMetricDataToGPU(NewLayerIndex);
	}
	else if (NewLayer.GetDataSourceType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		if (!ANALYSIS_OBJECT_MANAGER.HavePointCloudData() || SCENE_RESOURCES.CurrentPointCloud == nullptr)
			return;

		CurrentLayerIndex = NewLayerIndex;

		if (NewLayerIndex != -1)
			SCENE_RESOURCES.ComplexityMetricDataToGPU(NewLayerIndex);
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
	if (!ANALYSIS_OBJECT_MANAGER.HaveAnyData())
		return -1;

	return CurrentLayerIndex;
}

DataLayer* LayerManager::GetActiveLayer()
{
	if (!ANALYSIS_OBJECT_MANAGER.HaveAnyData())
		return nullptr;

	if (CurrentLayerIndex == -1)
		return nullptr;

	return &Layers[CurrentLayerIndex];
}

size_t LayerManager::GetLayerCount()
{
	return Layers.size();
}