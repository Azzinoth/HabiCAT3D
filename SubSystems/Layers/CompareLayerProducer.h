#pragma once

#include "LayerManager.h"
using namespace FocalEngine;

class CompareLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(CompareLayerProducer)

	MeshLayer Calculate(int FirstLayer, int SecondLayer);

	bool GetShouldNormalize();
	void SetShouldNormalize(bool NewValue);
private:
	SINGLETON_PRIVATE_PART(CompareLayerProducer)

	bool bNormalize;
	std::vector<float> Normalize(std::vector<float> Original);
};

#define COMPARE_LAYER_PRODUCER CompareLayerProducer::getInstance()