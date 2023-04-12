#include "LayerManager.h"
using namespace FocalEngine;

LayerManager* LayerManager::Instance = nullptr;

LayerManager::LayerManager()
{
	
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

	if (MESH_MANAGER.ActiveMesh == nullptr)
		return Result;

	FEMesh* Mesh = MESH_MANAGER.ActiveMesh;

	std::vector<std::string> CaptionList;
	for (size_t i = 0; i < Mesh->Layers.size(); i++)
	{
		CaptionList.push_back(Mesh->Layers[i].GetCaption());
	}

	int IndexToAdd = FindHigestIntPostfix(Base, "_", CaptionList);
	IndexToAdd++;
	if (IndexToAdd < 2)
	{
		std::transform(Base.begin(), Base.end(), Base.begin(), [](const unsigned char C) { return std::tolower(C); });
		for (size_t i = 0; i < Mesh->Layers.size(); i++)
		{
			std::string CurrentCaption = Mesh->Layers[i].GetCaption();
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
	if (MESH_MANAGER.ActiveMesh == nullptr || NewLayerIndex < -1 || NewLayerIndex >= int(MESH_MANAGER.ActiveMesh->Layers.size()))
		return;

	MESH_MANAGER.ActiveMesh->CurrentLayerIndex = NewLayerIndex;

	if (NewLayerIndex != -1)
		MESH_MANAGER.ActiveMesh->Layers[MESH_MANAGER.ActiveMesh->CurrentLayerIndex].FillDataToGPU();

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
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return -1;

	return MESH_MANAGER.ActiveMesh->CurrentLayerIndex;
}