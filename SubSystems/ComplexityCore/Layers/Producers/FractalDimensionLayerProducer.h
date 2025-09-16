#pragma once

#include <numeric>
#include "../LayerManager.h"
using namespace FocalEngine;

class FractalDimensionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(FractalDimensionLayerProducer)

	void CalculateWithJitterAsync(bool bSmootherResult);
	void CalculateOnWholeModel();
	void RenderDebugInfoForSelectedNode(MeasurementGrid* Grid);
	void RenderDebugInfoWindow(MeasurementGrid* Grid);

	void SetOnCalculationsEndCallback(void(*Func)(DataLayer*));

	bool GetShouldCalculateStandardDeviation();
	void SetShouldCalculateStandardDeviation(bool NewValue);

	bool GetShouldFilterFractalDimensionValues();
	void SetShouldFilterFractalDimensionValues(bool NewValue);
private:
	SINGLETON_PRIVATE_PART(FractalDimensionLayerProducer)

	static void OnJitterCalculationsEnd(DataLayer* NewLayer);
	bool bWaitForJitterResult = false;
	bool bFilterFractalDimensionValues = true;
	bool bCalculateStandardDeviation = false;

	int DebugBoxSizeIndex = 0;
	int DebugBoxCount = 0;
	double DebugFractalDimension = 0.0;
	std::vector<double> DebugLogInverseSizes;
	std::vector<double> DebugLogCounts;
	std::vector<int> DebugCounts;

	static void WorkOnNode(GridNode* CurrentNode);

	static void(*OnCalculationsEndCallbackImpl)(DataLayer*);

	// Used to not repeat same code for main calculations and for debug info rendering
	double RunOnAllInternalNodesWithTriangles(GridNode* OuterNode, std::function<void(int BoxSizeIndex, FEAABB BoxAABB)> FunctionWithAdditionalCode = nullptr);
};

#define FRACTAL_DIMENSION_LAYER_PRODUCER FractalDimensionLayerProducer::GetInstance()