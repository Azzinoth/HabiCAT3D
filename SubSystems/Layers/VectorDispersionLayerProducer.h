#pragma once

#include <numeric>
//#include "../FESDF.h"
#include "../JitterManager.h"
#include "LayerManager.h"
using namespace FocalEngine;

class VectorDispersionLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(VectorDispersionLayerProducer)

	//MeshLayer Calculate(FEMesh* Mesh, int Mode);
	void CalculateTEST(FEMesh* Mesh, int Mode);
private:
	SINGLETON_PRIVATE_PART(VectorDispersionLayerProducer)

	static void OnCalculationsEnd(MeshLayer NewLayer);
};

#define VECTOR_DISPERSION_LAYER_PRODUCER VectorDispersionLayerProducer::getInstance()