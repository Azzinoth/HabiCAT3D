#pragma once

#include "LayerManager.h"
using namespace FocalEngine;

class AreaLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(AreaLayerProducer)

	MeshLayer Calculate();
private:
	SINGLETON_PRIVATE_PART(AreaLayerProducer)
};

#define AREA_LAYER_PRODUCER AreaLayerProducer::GetInstance()