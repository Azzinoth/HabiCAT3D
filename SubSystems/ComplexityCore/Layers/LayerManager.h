#pragma once
#include "../JitterManager.h"

enum class LAYER_EVENT_TYPE
{
	UNKNOWN = 0,
	LAYER_ADDED = 1,
	LAYER_REMOVED = 2,
	LAYER_MODIFIED = 3,
	LAYER_ACTIVE_ID_CHANGED = 4
};

struct LayerEvent
{
	LAYER_EVENT_TYPE Type = LAYER_EVENT_TYPE::UNKNOWN;
	FEUUID ParentObjectID;
	FEUUID PrimaryLayerID;
	std::vector<FEUUID> OtherLayerIDs = std::vector<FEUUID>();

	LayerEvent();
	LayerEvent(LAYER_EVENT_TYPE Type, FEUUID ParentObjectID, FEUUID PrimaryLayerID, std::vector<FEUUID> OtherLayerIDs = std::vector<FEUUID>());
};

class LayerManager
{
	friend class AnalysisObject;
	friend class AnalysisObjectManager;
public:
	SINGLETON_PUBLIC_PART(LayerManager)

	std::string SuitableNewLayerCaption(std::string Base);

	std::vector<DataLayer*> GetAllLayersOfActiveObject();

	int LayerManager::GetActiveLayerIndex();
	DataLayer* GetActiveLayer();
	size_t GetLayerCount();

	void AddActiveLayerChangedCallback(std::function<void()> Func);
private:
	SINGLETON_PRIVATE_PART(LayerManager)

	int FindHighestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List);

	std::vector<std::function<void()>> ClientAfterActiveLayerChangedCallbacks;

	void PropagateLayerEvent(LayerEvent Event);
};

#define LAYER_MANAGER LayerManager::GetInstance()