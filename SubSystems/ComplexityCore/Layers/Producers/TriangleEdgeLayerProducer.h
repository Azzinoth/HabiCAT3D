#pragma once

#include "../LayerManager.h"
using namespace FocalEngine;

class TriangleEdgeLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(TriangleEdgeLayerProducer)

	DataLayer* Calculate(int Mode);
private:
	SINGLETON_PRIVATE_PART(TriangleEdgeLayerProducer)
};

#define TRIANGLE_EDGE_LAYER_PRODUCER TriangleEdgeLayerProducer::GetInstance()