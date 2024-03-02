#pragma once

#include <numeric>
#include "LayerManager.h"
using namespace FocalEngine;

class FractalDimensionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(FractalDimensionLayerProducer)

	void CalculateWithJitterAsync(bool bSmootherResult);
	void CalculateOnWholeModel();
	void RenderDebugInfoForSelectedNode(MeasurementGrid* Grid);
	void RenderDebugInfoWindow(MeasurementGrid* Grid);

	void SetOnCalculationsEndCallback(void(*Func)(MeshLayer));
	bool bCalculateStandardDeviation = false;

	bool GetShouldFilterFractalDimensionValues();
	void SetShouldFilterFractalDimensionValues(bool NewValue);
private:
	SINGLETON_PRIVATE_PART(FractalDimensionLayerProducer)

	static void OnJitterCalculationsEnd(MeshLayer NewLayer);
	bool bWaitForJitterResult = false;
	bool bFilterFractalDimensionValues = true;

	int DebugBoxSizeIndex = 0;
	int DebugBoxCount = 0;
	double DebugFractalDimension = 0.0;
	std::vector<double> DebugLogInverseSizes;
	std::vector<double> DebugLogCounts;
	std::vector<int> DebugCounts;

	static void WorkOnNode(GridNode* CurrentNode);

	static void(*OnCalculationsEndCallbackImpl)(MeshLayer);
};

#define FRACTAL_DIMENSION_LAYER_PRODUCER FractalDimensionLayerProducer::getInstance()