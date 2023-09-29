#pragma once

#include <numeric>
#include "LayerManager.h"
using namespace FocalEngine;

class FractalDimensionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(FractalDimensionLayerProducer)

	void CalculateWithJitterAsync(FEMesh* Mesh, bool bSmootherResult, bool bUseFilter = true);
	void CalculateOnWholeModel(FEMesh* Mesh);
	void RenderDebugInfoForSelectedNode(SDF* Grid);
	void RenderDebugInfoWindow(SDF* Grid);
private:
	SINGLETON_PRIVATE_PART(FractalDimensionLayerProducer)

	static void OnJitterCalculationsEnd(MeshLayer NewLayer);
	bool bWaitForJitterResult = false;
	int DebugBoxSizeIndex = 0;
	int DebugBoxCount = 0;
	double DebugFractalDimension = 0.0;
	std::vector<double> DebugLogInverseSizes;
	std::vector<double> DebugLogCounts;
	std::vector<int> DebugCounts;

	bool bLastUsedUseFilter = true;

	static void WorkOnNode(SDFNode* CurrentNode);
};

#define FRACTAL_DIMENSION_LAYER_PRODUCER FractalDimensionLayerProducer::getInstance()