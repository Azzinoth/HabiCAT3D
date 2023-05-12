#pragma once

#include <numeric>
#include "LayerManager.h"
using namespace FocalEngine;

class FractalDimensionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(FractalDimensionLayerProducer)

	void CalculateWithJitterAsync(FEMesh* Mesh, bool bSmootherResult);
	void RenderDebugInfoForSelectedNode(SDF* Grid);
private:
	SINGLETON_PRIVATE_PART(FractalDimensionLayerProducer)

	static void OnJitterCalculationsEnd(MeshLayer NewLayer);
	bool bWaitForJitterResult = false;

	static void WorkOnNode(SDFNode* CurrentNode);
};

#define FRACTAL_DIMENSION_LAYER_PRODUCER FractalDimensionLayerProducer::getInstance()