#pragma once

#include "LayerManager.h"
using namespace FocalEngine;

class CompareLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(CompareLayerProducer)

	MeshLayer Calculate(int FirstLayer, int SecondLayer);
private:
	SINGLETON_PRIVATE_PART(CompareLayerProducer)
};

#define COMPARE_LAYER_PRODUCER CompareLayerProducer::getInstance()