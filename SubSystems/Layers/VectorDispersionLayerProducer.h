#pragma once

#include <numeric>
#include "../JitterManager.h"
#include "LayerManager.h"
using namespace FocalEngine;

class VectorDispersionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(VectorDispersionLayerProducer)

	void CalculateWithJitterAsync(FEMesh* Mesh, bool bSmootherResult);
private:
	SINGLETON_PRIVATE_PART(VectorDispersionLayerProducer)

	static void OnJitterCalculationsEnd(MeshLayer NewLayer);
	bool bWaitForJitterResult = false;
};

#define VECTOR_DISPERSION_LAYER_PRODUCER VectorDispersionLayerProducer::getInstance()