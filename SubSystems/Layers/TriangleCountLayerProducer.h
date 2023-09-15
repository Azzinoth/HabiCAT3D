#pragma once

#include "LayerManager.h"
using namespace FocalEngine;

class TriangleCountLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(TriangleCountLayerProducer)

	void CalculateWithJitterAsync(FEMesh* Mesh, bool bSmootherResult);
private:
	SINGLETON_PRIVATE_PART(TriangleCountLayerProducer)

	static void OnJitterCalculationsEnd(MeshLayer NewLayer);
	bool bWaitForJitterResult = false;
};

#define TRIANGLE_COUNT_LAYER_PRODUCER TriangleCountLayerProducer::getInstance()