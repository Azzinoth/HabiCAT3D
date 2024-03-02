#pragma once

#include <numeric>
#include "LayerManager.h"
using namespace FocalEngine;

class VectorDispersionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(VectorDispersionLayerProducer)

	void CalculateWithJitterAsync(bool bSmootherResult);
	void CalculateOnWholeModel();
	void RenderDebugInfoForSelectedNode(MeasurementGrid* Grid);

	void SetOnCalculationsEndCallback(void(*Func)(MeshLayer));
	bool bCalculateStandardDeviation = false;
private:
	SINGLETON_PRIVATE_PART(VectorDispersionLayerProducer)

	static void OnJitterCalculationsEnd(MeshLayer NewLayer);
	bool bWaitForJitterResult = false;
	static void WorkOnNode(GridNode* CurrentNode);

	static void(*OnCalculationsEndCallbackImpl)(MeshLayer);
};

#define VECTOR_DISPERSION_LAYER_PRODUCER VectorDispersionLayerProducer::getInstance()