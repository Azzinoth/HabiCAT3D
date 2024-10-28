#pragma once

#include <numeric>
#include "LayerManager.h"
using namespace FocalEngine;

class VectorDispersionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(VectorDispersionLayerProducer)

	bool GetShouldCalculateStandardDeviation();
	void SetShouldCalculateStandardDeviation(bool NewValue);

	void CalculateWithJitterAsync(bool bSmootherResult);
	void CalculateOnWholeModel();
	void RenderDebugInfoForSelectedNode(MeasurementGrid* Grid);

	void SetOnCalculationsEndCallback(void(*Func)(MeshLayer));
private:
	SINGLETON_PRIVATE_PART(VectorDispersionLayerProducer)

	static void OnJitterCalculationsEnd(MeshLayer NewLayer);
	bool bWaitForJitterResult = false;
	bool bCalculateStandardDeviation = false;
	static void WorkOnNode(GridNode* CurrentNode);

	static void(*OnCalculationsEndCallbackImpl)(MeshLayer);
};

#define VECTOR_DISPERSION_LAYER_PRODUCER VectorDispersionLayerProducer::GetInstance()