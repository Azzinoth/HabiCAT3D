#pragma once

#include <numeric>
#include "LayerManager.h"
using namespace FocalEngine;

class VectorDispersionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(VectorDispersionLayerProducer)

	void CalculateWithJitterAsync(FEMesh* Mesh, bool bSmootherResult);
	void CalculateOnWholeModel(FEMesh* Mesh);
	void RenderDebugInfoForSelectedNode(SDF* Grid);
private:
	SINGLETON_PRIVATE_PART(VectorDispersionLayerProducer)

	static void OnJitterCalculationsEnd(MeshLayer NewLayer);
	bool bWaitForJitterResult = false;
	static void WorkOnNode(SDFNode* CurrentNode);
};

#define VECTOR_DISPERSION_LAYER_PRODUCER VectorDispersionLayerProducer::getInstance()