#include "VectorDispersionLayerProducer.h"
using namespace FocalEngine;

VectorDispersionLayerProducer* VectorDispersionLayerProducer::Instance = nullptr;

VectorDispersionLayerProducer::VectorDispersionLayerProducer() {}
VectorDispersionLayerProducer::~VectorDispersionLayerProducer() {}

double calculateVectorDispersion(const std::vector<std::vector<double>>& surfaceNormals) 
{
	std::vector<double> x(surfaceNormals.size());
	std::vector<double> y(surfaceNormals.size());
	std::vector<double> z(surfaceNormals.size());

	for (int i = 0; i < surfaceNormals.size(); ++i) 
	{
		x[i] = surfaceNormals[i][0];
		y[i] = surfaceNormals[i][1];
		z[i] = surfaceNormals[i][2]; 
	}
	
	double meanX = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
	double meanY = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
	double meanZ = std::accumulate(z.begin(), z.end(), 0.0) / z.size();

	double sumX = std::inner_product(x.begin(), x.end(), x.begin(), 0.0);
	double sumY = std::inner_product(y.begin(), y.end(), y.begin(), 0.0);
	double sumZ = std::inner_product(z.begin(), z.end(), z.begin(), 0.0);
	
	return sqrt(sumX / x.size() - meanX * meanX + sumY / y.size() - meanY * meanY + sumZ / z.size() - meanZ * meanZ);
}

//CurrentNode->Rugosity = DoubleResult;

void VectorDispersionLayerProducer::CalculateTEST(FEMesh* Mesh, int Mode)
{
	MeshLayer Result;

	if (Mesh == nullptr)
		return;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	//SDF* CurrentSDF = new SDF();
	//FEAABB finalAABB = Mesh->AABB;
	//int SDFDimention = 16;

	//const float cellSize = finalAABB.getSize() / SDFDimention;

	//const glm::vec3 center = Mesh->AABB.getCenter() + glm::vec3(0.0f/*Input->ShiftX, Input->ShiftY, Input->ShiftZ*/) * cellSize;
	//const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	//finalAABB = SDFAABB;

	//float ResolutonInM = 1.0f;
	//CurrentSDF->Init(0/*Input->dimentions*/, finalAABB, nullptr, ResolutonInM);
	//CurrentSDF->FillCellsWithTriangleInfo();

	//Result.TrianglesToData.resize(Mesh->Triangles.size());

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

		CurrentNode->Rugosity = DoubleResult;
	};

	JITTER_MANAGER.SetOnCalculationsEndCallback(OnCalculationsEnd);
	JITTER_MANAGER.CalculateWithSDFJitterAsync(WorkOnNode);

	/*CurrentSDF->RunOnAllNodes(WorkOnNode);

	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Vector Dispersion"));
	Result.DebugInfo = new MeshLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StarTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));*/
}

//MeshLayer VectorDispersionLayerProducer::Calculate(FEMesh* Mesh, int Mode)
//{
//	MeshLayer Result;
//
//	if (Mesh == nullptr)
//		return Result;
//
//	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
//
//	SDF* CurrentSDF = new SDF();
//	FEAABB finalAABB = Mesh->AABB;
//	int SDFDimention = 16;
//
//	const float cellSize = finalAABB.getSize() / SDFDimention;
//
//	const glm::vec3 center = Mesh->AABB.getCenter() + glm::vec3(0.0f/*Input->ShiftX, Input->ShiftY, Input->ShiftZ*/) * cellSize;
//	const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
//	finalAABB = SDFAABB;
//
//	float ResolutonInM = 1.0f;
//	CurrentSDF->Init(0/*Input->dimentions*/, finalAABB, nullptr, ResolutonInM);
//	CurrentSDF->FillCellsWithTriangleInfo();
//
//	Result.TrianglesToData.resize(Mesh->Triangles.size());
//
//	auto WorkOnNode = [&](SDFNode* CurrentNode){
//		if (CurrentNode->TrianglesInCell.empty())
//			return;
//
//		std::vector<double> NormalX;
//		std::vector<double> NormalY;
//		std::vector<double> NormalZ;
//
//		for (size_t p = 0; p < CurrentNode->TrianglesInCell.size(); p++)
//		{
//			std::vector<glm::vec3> CurrentTriangleNormals = MESH_MANAGER.ActiveMesh->TrianglesNormals[CurrentNode->TrianglesInCell[p]];
//
//			for (size_t l = 0; l < CurrentTriangleNormals.size(); l++)
//			{
//				NormalX.push_back(CurrentTriangleNormals[l][0]);
//				NormalY.push_back(CurrentTriangleNormals[l][1]);
//				NormalZ.push_back(CurrentTriangleNormals[l][2]);
//			}
//		}
//
//		double meanX = std::accumulate(NormalX.begin(), NormalX.end(), 0.0) / NormalX.size();
//		double meanY = std::accumulate(NormalY.begin(), NormalY.end(), 0.0) / NormalY.size();
//		double meanZ = std::accumulate(NormalZ.begin(), NormalZ.end(), 0.0) / NormalZ.size();
//
//		double sumX = std::inner_product(NormalX.begin(), NormalX.end(), NormalX.begin(), 0.0);
//		double sumY = std::inner_product(NormalY.begin(), NormalY.end(), NormalY.begin(), 0.0);
//		double sumZ = std::inner_product(NormalZ.begin(), NormalZ.end(), NormalZ.begin(), 0.0);
//
//		double DoubleResult = sqrt(sumX / NormalX.size() - meanX * meanX + sumY / NormalY.size() - meanY * meanY + sumZ / NormalZ.size() - meanZ * meanZ);
//
//		for (size_t p = 0; p < CurrentNode->TrianglesInCell.size(); p++)
//		{
//			int TriangleIndex = CurrentNode->TrianglesInCell[p];
//
//			Result.TrianglesToData[TriangleIndex] = DoubleResult;
//		}
//	};
//
//	CurrentSDF->RunOnAllNodes(WorkOnNode);
//	
//	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Vector Dispersion"));
//	Result.DebugInfo = new MeshLayerDebugInfo();
//
//	Result.DebugInfo->AddEntry("Start time", StarTime);
//	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
//
//	return Result;
//}

void VectorDispersionLayerProducer::OnCalculationsEnd(MeshLayer NewLayer)
{
	MESH_MANAGER.ActiveMesh->AddLayer(NewLayer);
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Vector Dispersion"));
	LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);
}