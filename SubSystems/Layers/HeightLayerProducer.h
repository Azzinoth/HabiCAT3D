#pragma once

#include "LayerManager.h"
using namespace FocalEngine;

class HeightLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(HeightLayerProducer)

	MeshLayer Calculate(FEMesh* Mesh);
private:
	SINGLETON_PRIVATE_PART(HeightLayerProducer)
};

#define HEIGHT_LAYER_PRODUCER HeightLayerProducer::getInstance()