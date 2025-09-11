#pragma once
#include "../JitterManager.h"

class LayerManager
{
	friend class MeshManager;
public:
	SINGLETON_PUBLIC_PART(LayerManager)

	std::string SuitableNewLayerCaption(std::string Base);

	std::vector<DataLayer> Layers;

	void AddLayer(DATA_SOURCE_TYPE LayerDataSource, std::vector<float> ElementsToData);
	void AddLayer(DataLayer NewLayer);

	void SetActiveLayerIndex(int NewLayerIndex);
	int GetActiveLayerIndex();
	DataLayer* GetActiveLayer();
	size_t GetLayerCount();

	void AddActiveLayerChangedCallback(std::function<void()> Func);
private:
	SINGLETON_PRIVATE_PART(LayerManager)

	int CurrentLayerIndex = -1;

	int FindHighestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List);

	std::vector<std::function<void()>> ClientAfterActiveLayerChangedCallbacks;
};

#define LAYER_MANAGER LayerManager::GetInstance()