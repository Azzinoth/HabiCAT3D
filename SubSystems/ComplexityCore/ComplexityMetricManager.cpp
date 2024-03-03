#include "ComplexityMetricManager.h"
using namespace FocalEngine;

ComplexityMetricManager* ComplexityMetricManager::Instance = nullptr;

ComplexityMetricManager::ComplexityMetricManager() {}
ComplexityMetricManager::~ComplexityMetricManager() {}

void ComplexityMetricManager::Init(std::vector<float>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals)
{
	if (ActiveComplexityMetricInfo != nullptr)
		delete ActiveComplexityMetricInfo;
	
	ActiveComplexityMetricInfo = new ComplexityMetricInfo();
	ActiveComplexityMetricInfo->FillTrianglesData(Vertices, Colors, UVs, Tangents, Indices, Normals);
}

void ComplexityMetricManager::AddLoadCallback(std::function<void()> Func)
{
	ClientLoadCallbacks.push_back(Func);
}

void ComplexityMetricManager::ImportOBJ(const char* FileName, bool bForceOneMesh)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.ForceOneMesh(bForceOneMesh);
	objLoader.ForcePositionNormalization(true);
	objLoader.UseDoublePrecisionForReadingCoordinates(true);
	objLoader.DoubleVertexOnSeams(false);
	objLoader.ReadFile(FileName);

	std::vector<FERawOBJData*>* LoadedObjects = objLoader.GetLoadedObjects();
	FERawOBJData* FirstObject = LoadedObjects->size() > 0 ? LoadedObjects->at(0) : nullptr;

	if (FirstObject != nullptr)
	{
		COMPLEXITY_METRIC_MANAGER.Init(FirstObject->FVerC, FirstObject->fColorsC, FirstObject->FTexC, FirstObject->FTanC, FirstObject->FInd, FirstObject->FNorC);
	}

	for (size_t i = 0; i < ClientLoadCallbacks.size(); i++)
	{
		if (ClientLoadCallbacks[i] == nullptr)
			continue;

		ClientLoadCallbacks[i]();
	}
}

void ComplexityMetricManager::SaveToRUGFileAskForFilePath()
{
	std::string FilePath;
	FILE_SYSTEM.ShowFileSaveDialog(FilePath, RUGOSITY_SAVE_FILE_FILTER, 1);

	SaveToRUGFile(FilePath);
}

void ComplexityMetricManager::SaveToRUGFile(std::string FilePath)
{
	if (FilePath.empty())
		return;

	if (FilePath.find(".rug") == std::string::npos)
		FilePath += ".rug";

	std::fstream file;
	file.open(FilePath, std::ios::out | std::ios::binary);

	// Version of file.
	float version = APP_VERSION;
	file.write((char*)&version, sizeof(float));

	int Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Vertices.size());
	file.write((char*)&Count, sizeof(int));
	file.write((char*)ActiveComplexityMetricInfo->MeshData.Vertices.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Colors.size());
	file.write((char*)&Count, sizeof(int));
	file.write((char*)ActiveComplexityMetricInfo->MeshData.Colors.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.UVs.size());
	file.write((char*)&Count, sizeof(int));
	file.write((char*)ActiveComplexityMetricInfo->MeshData.UVs.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Normals.size());
	file.write((char*)&Count, sizeof(int));
	file.write((char*)ActiveComplexityMetricInfo->MeshData.Normals.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Tangents.size());
	file.write((char*)&Count, sizeof(int));
	file.write((char*)ActiveComplexityMetricInfo->MeshData.Tangents.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Indices.size());
	file.write((char*)&Count, sizeof(int));
	file.write((char*)ActiveComplexityMetricInfo->MeshData.Indices.data(), sizeof(int) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->Layers.size());
	file.write((char*)&Count, sizeof(int));

	for (size_t i = 0; i < ActiveComplexityMetricInfo->Layers.size(); i++)
	{
		int LayerType = ActiveComplexityMetricInfo->Layers[i].GetType();
		file.write((char*)&LayerType, sizeof(int));

		int LayerIDSize = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetID().size() + 1);
		file.write((char*)&LayerIDSize, sizeof(int));
		file.write((char*)ActiveComplexityMetricInfo->Layers[i].GetID().c_str(), sizeof(char) * LayerIDSize);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetCaption().size());
		file.write((char*)&Count, sizeof(int));
		file.write((char*)ActiveComplexityMetricInfo->Layers[i].GetCaption().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetNote().size());
		file.write((char*)&Count, sizeof(int));
		file.write((char*)ActiveComplexityMetricInfo->Layers[i].GetNote().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].TrianglesToData.size());
		file.write((char*)&Count, sizeof(int));
		file.write((char*)ActiveComplexityMetricInfo->Layers[i].TrianglesToData.data(), sizeof(float) * Count);

		Count = ActiveComplexityMetricInfo->Layers[i].DebugInfo != nullptr;
		file.write((char*)&Count, sizeof(int));
		if (Count)
			ActiveComplexityMetricInfo->Layers[i].DebugInfo->ToFile(file);
	}

	FEAABB TempAABB(ActiveComplexityMetricInfo->MeshData.Vertices.data(), static_cast<int>(ActiveComplexityMetricInfo->MeshData.Vertices.size()));
	file.write((char*)&TempAABB.GetMin()[0], sizeof(float));
	file.write((char*)&TempAABB.GetMin()[1], sizeof(float));
	file.write((char*)&TempAABB.GetMin()[2], sizeof(float));

	file.write((char*)&TempAABB.GetMax()[0], sizeof(float));
	file.write((char*)&TempAABB.GetMax()[1], sizeof(float));
	file.write((char*)&TempAABB.GetMax()[2], sizeof(float));

	file.close();
}