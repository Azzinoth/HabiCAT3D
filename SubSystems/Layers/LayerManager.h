#pragma once
#include "../MeshManager.h"
using namespace FocalEngine;

namespace FocalEngine
{
	class LayerManager
	{
	public:
		SINGLETON_PUBLIC_PART(LayerManager)

		std::string SuitableNewLayerCaption(std::string Base);
		//bool AddLayer(std::vector<float> TrianglesToData);
		//bool AddLayer(MeshLayer NewLayer);

		void SetActiveLayerIndex(int NewLayerIndex);
		int GetActiveLayerIndex();

		void AddActiveLayerChangedCallback(std::function<void()> Func);
	private:
		SINGLETON_PRIVATE_PART(LayerManager)

		int FindHigestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List);

		//std::vector<std::function<void()>> ClientAfterNewLayerAddedCallbacks;
		std::vector<std::function<void()>> ClientAfterActiveLayerChangedCallbacks;
	};

	#define LAYER_MANAGER LayerManager::getInstance()
}