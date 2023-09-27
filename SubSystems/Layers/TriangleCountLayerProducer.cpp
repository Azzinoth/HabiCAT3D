#include "TriangleCountLayerProducer.h"
using namespace FocalEngine;

TriangleCountLayerProducer* TriangleCountLayerProducer::Instance = nullptr;

TriangleCountLayerProducer::TriangleCountLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

TriangleCountLayerProducer::~TriangleCountLayerProducer() {}

void TriangleCountLayerProducer::CalculateWithJitterAsync(FEMesh* Mesh, bool bSmootherResult)
{
	if (Mesh == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](SDFNode* CurrentNode) {
		if (CurrentNode->TrianglesInCell.empty())
			return;

		CurrentNode->UserData = CurrentNode->TrianglesInCell.size();
	};

	JITTER_MANAGER.CalculateWithSDFJitterAsync(WorkOnNode, bSmootherResult);
}

void TriangleCountLayerProducer::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	if (!TRIANGLE_COUNT_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	TRIANGLE_COUNT_LAYER_PRODUCER.bWaitForJitterResult = false;
	MESH_MANAGER.ActiveMesh->AddLayer(NewLayer);
	MESH_MANAGER.ActiveMesh->Layers.back().SetType(LAYER_TYPE::VECTOR_DISPERSION);
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangles density"));
	LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);
}

void TriangleCountLayerProducer::CalculateOnWholeModel(FEMesh* Mesh)
{
	if (Mesh == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](SDFNode* CurrentNode) {
		if (CurrentNode->TrianglesInCell.empty())
			return;

		CurrentNode->UserData = CurrentNode->TrianglesInCell.size();
	};

	JITTER_MANAGER.CalculateOnWholeModel(WorkOnNode);
}