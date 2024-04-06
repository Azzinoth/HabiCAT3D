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
	FEObjLoader& ObjLoader = FEObjLoader::getInstance();
	ObjLoader.ForceOneMesh(bForceOneMesh);
	ObjLoader.ForcePositionNormalization(true);
	ObjLoader.UseDoublePrecisionForReadingCoordinates(true);
	ObjLoader.DoubleVertexOnSeams(false);
	ObjLoader.ReadFile(FileName);

	std::vector<FERawOBJData*>* LoadedObjects = ObjLoader.GetLoadedObjects();
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

	std::fstream File;
	File.open(FilePath, std::ios::out | std::ios::binary);

	// Version of file.
	float version = APP_VERSION;
	File.write((char*)&version, sizeof(float));

	int Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Vertices.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)ActiveComplexityMetricInfo->MeshData.Vertices.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Colors.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)ActiveComplexityMetricInfo->MeshData.Colors.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.UVs.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)ActiveComplexityMetricInfo->MeshData.UVs.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Normals.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)ActiveComplexityMetricInfo->MeshData.Normals.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Tangents.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)ActiveComplexityMetricInfo->MeshData.Tangents.data(), sizeof(float) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Indices.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)ActiveComplexityMetricInfo->MeshData.Indices.data(), sizeof(int) * Count);

	Count = static_cast<int>(ActiveComplexityMetricInfo->Layers.size());
	File.write((char*)&Count, sizeof(int));

	for (size_t i = 0; i < ActiveComplexityMetricInfo->Layers.size(); i++)
	{
		int LayerType = ActiveComplexityMetricInfo->Layers[i].GetType();
		File.write((char*)&LayerType, sizeof(int));

		int LayerIDSize = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetID().size() + 1);
		File.write((char*)&LayerIDSize, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].GetID().c_str(), sizeof(char) * LayerIDSize);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetCaption().size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].GetCaption().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetNote().size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].GetNote().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].TrianglesToData.size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].TrianglesToData.data(), sizeof(float) * Count);

		Count = ActiveComplexityMetricInfo->Layers[i].DebugInfo != nullptr;
		File.write((char*)&Count, sizeof(int));
		if (Count)
			ActiveComplexityMetricInfo->Layers[i].DebugInfo->ToFile(File);
	}

	FEAABB TempAABB(ActiveComplexityMetricInfo->MeshData.Vertices.data(), static_cast<int>(ActiveComplexityMetricInfo->MeshData.Vertices.size()));
	File.write((char*)&TempAABB.GetMin()[0], sizeof(float));
	File.write((char*)&TempAABB.GetMin()[1], sizeof(float));
	File.write((char*)&TempAABB.GetMin()[2], sizeof(float));

	File.write((char*)&TempAABB.GetMax()[0], sizeof(float));
	File.write((char*)&TempAABB.GetMax()[1], sizeof(float));
	File.write((char*)&TempAABB.GetMax()[2], sizeof(float));

	File.close();
}