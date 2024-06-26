#pragma once
#include "../../MeshManager.h"
#include "../MeasurementGrid.h"
#include "../JitterManager.h"

class LayerManager
{
public:
	SINGLETON_PUBLIC_PART(LayerManager)

	MeshLayer* GetActiveLayer();

	std::string SuitableNewLayerCaption(std::string Base);

	void SetActiveLayerIndex(int NewLayerIndex);
	int GetActiveLayerIndex();

	void AddActiveLayerChangedCallback(std::function<void()> Func);
private:
	SINGLETON_PRIVATE_PART(LayerManager)

	int FindHighestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List);

	std::vector<std::function<void()>> ClientAfterActiveLayerChangedCallbacks;
};

#define LAYER_MANAGER LayerManager::getInstance()