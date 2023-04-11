#pragma once

#include <numeric>
#include "../FESDF.h"
#include "LayerManager.h"
using namespace FocalEngine;

class VectorDispersionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(VectorDispersionLayerProducer)

	MeshLayer Calculate(FEMesh* Mesh, int Mode);
private:
	SINGLETON_PRIVATE_PART(VectorDispersionLayerProducer)
};

#define VECTOR_DISPERSION_LAYER_PRODUCER VectorDispersionLayerProducer::getInstance()