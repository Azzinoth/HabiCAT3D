#pragma once

#include "../LayerManager.h"
using namespace FocalEngine;

class InterpolationLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(InterpolationLayerProducer)

	DataLayer* Calculate(std::vector<DataLayer*> LayersToInterpolate);
private:
	SINGLETON_PRIVATE_PART(InterpolationLayerProducer)
};

#define INTERPOLATION_LAYER_PRODUCER InterpolationLayerProducer::GetInstance()