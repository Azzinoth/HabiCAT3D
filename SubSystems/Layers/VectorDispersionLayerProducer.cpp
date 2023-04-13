#include "VectorDispersionLayerProducer.h"
using namespace FocalEngine;

VectorDispersionLayerProducer* VectorDispersionLayerProducer::Instance = nullptr;

VectorDispersionLayerProducer::VectorDispersionLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

VectorDispersionLayerProducer::~VectorDispersionLayerProducer() {}


void VectorDispersionLayerProducer::CalculateWithJitterAsync(FEMesh* Mesh, bool bSmootherResult)
{
	if (Mesh == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	auto WorkOnNode = [&](SDFNode* CurrentNode) {
		if (CurrentNode->TrianglesInCell.empty())
			return;

		std::vector<double> NormalX;
		std::vector<double> NormalY;
		std::vector<double> NormalZ;

		for (size_t p = 0; p < CurrentNode->TrianglesInCell.size(); p++)
		{
			std::vector<glm::vec3> CurrentTriangleNormals = MESH_MANAGER.ActiveMesh->TrianglesNormals[CurrentNode->TrianglesInCell[p]];

			for (size_t l = 0; l < CurrentTriangleNormals.size(); l++)
			{
				NormalX.push_back(CurrentTriangleNormals[l][0]);
				NormalY.push_back(CurrentTriangleNormals[l][1]);
				NormalZ.push_back(CurrentTriangleNormals[l][2]);
			}
		}

		double meanX = std::accumulate(NormalX.begin(), NormalX.end(), 0.0) / NormalX.size();
		double meanY = std::accumulate(NormalY.begin(), NormalY.end(), 0.0) / NormalY.size();
		double meanZ = std::accumulate(NormalZ.begin(), NormalZ.end(), 0.0) / NormalZ.size();

		double sumX = std::inner_product(NormalX.begin(), NormalX.end(), NormalX.begin(), 0.0);
		double sumY = std::inner_product(NormalY.begin(), NormalY.end(), NormalY.begin(), 0.0);
		double sumZ = std::inner_product(NormalZ.begin(), NormalZ.end(), NormalZ.begin(), 0.0);

		double DoubleResult = sqrt(sumX / NormalX.size() - meanX * meanX + sumY / NormalY.size() - meanY * meanY + sumZ / NormalZ.size() - meanZ * meanZ);

		CurrentNode->UserData = DoubleResult;
	};

	JITTER_MANAGER.CalculateWithSDFJitterAsync(WorkOnNode, bSmootherResult);
}

void VectorDispersionLayerProducer::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	if (!VECTOR_DISPERSION_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	VECTOR_DISPERSION_LAYER_PRODUCER.bWaitForJitterResult = false;
	MESH_MANAGER.ActiveMesh->AddLayer(NewLayer);
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Vector Dispersion"));
	LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);
}