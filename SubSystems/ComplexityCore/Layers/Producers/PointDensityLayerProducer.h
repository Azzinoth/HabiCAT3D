#pragma once

#include "../LayerManager.h"
using namespace FocalEngine;

class PointDensityLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(PointDensityLayerProducer)

	void CalculateWithJitterAsync(bool bSmootherResult);
private:
	SINGLETON_PRIVATE_PART(PointDensityLayerProducer)

	static void OnJitterCalculationsEnd(DataLayer* NewLayer);
	bool bWaitForJitterResult = false;
};

#define POINT_DENSITY_LAYER_PRODUCER PointDensityLayerProducer::GetInstance()