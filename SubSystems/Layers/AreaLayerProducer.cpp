#include "AreaLayerProducer.h"
using namespace FocalEngine;

AreaLayerProducer* AreaLayerProducer::Instance = nullptr;

AreaLayerProducer::AreaLayerProducer() {}
AreaLayerProducer::~AreaLayerProducer() {}

MeshLayer AreaLayerProducer::Calculate(FEMesh* Mesh)
{
	MeshLayer Result;

	if (Mesh == nullptr)
		return Result;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < Mesh->Triangles.size(); i++)
	{
		Result.TrianglesToData.push_back(Mesh->TrianglesArea[i]);
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle area"));
	Result.DebugInfo = new MeshLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StarTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	return Result;
}