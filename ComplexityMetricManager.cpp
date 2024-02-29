#include "ComplexityMetricManager.h"
using namespace FocalEngine;

ComplexityMetricManager* ComplexityMetricManager::Instance = nullptr;

ComplexityMetricManager::ComplexityMetricManager() {}
ComplexityMetricManager::~ComplexityMetricManager() {}

void ComplexityMetricManager::Init(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals)
{
	if (ActiveComplexityMetricInfo != nullptr)
		delete ActiveComplexityMetricInfo;
	
	ActiveComplexityMetricInfo = new ComplexityMetricInfo();
	ActiveComplexityMetricInfo->fillTrianglesData(Vertices, Colors, UVs, Tangents, Indices, Normals);
}

void ComplexityMetricManager::AddLoadCallback(std::function<void()> Func)
{
	ClientLoadCallbacks.push_back(Func);
}

void ComplexityMetricManager::ImportOBJ(const char* FileName, bool bForceOneMesh)
{
	ObjLoader& objLoader = ObjLoader::getInstance();
	objLoader.ForceOneMesh(bForceOneMesh);
	objLoader.ForcePositionNormalization(true);
	objLoader.ReadFile(FileName);

	std::vector<RawOBJData*>* LoadedObjects = objLoader.GetLoadedObjects();
	RawOBJData* FirstObject = LoadedObjects->size() > 0 ? LoadedObjects->at(0) : nullptr;

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

	//if (FilePath.empty())
	//	return;

	//if (FilePath.find(".rug") == std::string::npos)
	//	FilePath += ".rug";

	//std::fstream file;
	//file.open(FilePath, std::ios::out | std::ios::binary);

	//// Version of file.
	//float version = APP_VERSION;
	//file.write((char*)&version, sizeof(float));

	//int Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Vertices.size());
	//file.write((char*)&Count, sizeof(int));
	//file.write((char*)ActiveComplexityMetricInfo->MeshData.Vertices.data(), sizeof(float) * Count);

	//Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Colors.size());
	//file.write((char*)&Count, sizeof(int));
	//file.write((char*)ActiveComplexityMetricInfo->MeshData.Colors.data(), sizeof(float) * Count);

	//Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.UVs.size());
	//file.write((char*)&Count, sizeof(int));
	//file.write((char*)ActiveComplexityMetricInfo->MeshData.UVs.data(), sizeof(float) * Count);

	//Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Normals.size());
	//file.write((char*)&Count, sizeof(int));
	//file.write((char*)ActiveComplexityMetricInfo->MeshData.Normals.data(), sizeof(float) * Count);

	//Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Tangents.size());
	//file.write((char*)&Count, sizeof(int));
	//file.write((char*)ActiveComplexityMetricInfo->MeshData.Tangents.data(), sizeof(float) * Count);

	//Count = static_cast<int>(ActiveComplexityMetricInfo->MeshData.Indices.size());
	//file.write((char*)&Count, sizeof(int));
	//file.write((char*)ActiveComplexityMetricInfo->MeshData.Indices.data(), sizeof(int) * Count);

	//Count = static_cast<int>(ActiveComplexityMetricInfo->Layers.size());
	//file.write((char*)&Count, sizeof(int));

	//for (size_t i = 0; i < ActiveComplexityMetricInfo->Layers.size(); i++)
	//{
	//	int LayerType = ActiveComplexityMetricInfo->Layers[i].GetType();
	//	file.write((char*)&LayerType, sizeof(int));

	//	int LayerIDSize = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetID().size() + 1);
	//	file.write((char*)&LayerIDSize, sizeof(int));
	//	file.write((char*)ActiveComplexityMetricInfo->Layers[i].GetID().c_str(), sizeof(char) * LayerIDSize);

	//	Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetCaption().size());
	//	file.write((char*)&Count, sizeof(int));
	//	file.write((char*)ActiveComplexityMetricInfo->Layers[i].GetCaption().c_str(), sizeof(char) * Count);

	//	Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetNote().size());
	//	file.write((char*)&Count, sizeof(int));
	//	file.write((char*)ActiveComplexityMetricInfo->Layers[i].GetNote().c_str(), sizeof(char) * Count);

	//	Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].TrianglesToData.size());
	//	file.write((char*)&Count, sizeof(int));
	//	file.write((char*)ActiveComplexityMetricInfo->Layers[i].TrianglesToData.data(), sizeof(float) * Count);

	//	Count = ActiveComplexityMetricInfo->Layers[i].DebugInfo != nullptr;
	//	file.write((char*)&Count, sizeof(int));
	//	if (Count)
	//		ActiveComplexityMetricInfo->Layers[i].DebugInfo->ToFile(file);
	//}

	//FEAABB TempAABB(ActiveComplexityMetricInfo->MeshData.Vertices.data(), static_cast<int>(ActiveComplexityMetricInfo->MeshData.Vertices.size()));
	//file.write((char*)&TempAABB.getMin()[0], sizeof(float));
	//file.write((char*)&TempAABB.getMin()[1], sizeof(float));
	//file.write((char*)&TempAABB.getMin()[2], sizeof(float));

	//file.write((char*)&TempAABB.getMax()[0], sizeof(float));
	//file.write((char*)&TempAABB.getMax()[1], sizeof(float));
	//file.write((char*)&TempAABB.getMax()[2], sizeof(float));

	//file.close();
}

void ComplexityMetricManager::SaveToRUGFile(std::string FilePath)
{
	//std::string FilePath;
	//FILE_SYSTEM.ShowFileSaveDialog(FilePath, RUGOSITY_SAVE_FILE_FILTER, 1);

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

//FEMesh* MeshManager::LoadRUGMesh(std::string FileName)
//{
//	std::fstream File;
//
//	File.open(FileName, std::ios::in | std::ios::binary);
//	const std::streamsize FileSize = File.tellg();
//	if (FileSize < 0)
//	{
//		LOG.Add(std::string("Can't load file: ") + FileName + " in function LoadRUGMesh.");
//		return nullptr;
//	}
//
//	char* Buffer = new char[4];
//	long long ArraySize = 0;
//
//	// version of FEMesh file type
//	File.read(Buffer, 4);
//	const float Version = *(float*)Buffer;
//	if (Version > APP_VERSION && abs(Version - APP_VERSION) > 0.0001)
//	{
//		LOG.Add(std::string("Can't load file: ") + FileName + " in function LoadRUGMesh. File was created in different version of application!");
//		return nullptr;
//	}
//
//	File.read(Buffer, 4);
//	const int VertexCount = *(int*)Buffer;
//	ArraySize = long long(VertexCount) * long long(4);
//	char* VertexBuffer = new char[ArraySize];
//	File.read(VertexBuffer, ArraySize);
//
//	File.read(Buffer, 4);
//	const int ColorCount = *(int*)Buffer;
//	char* ColorBuffer = nullptr;
//	if (ColorCount != 0)
//	{
//		ArraySize = long long(ColorCount) * long long(4);
//		ColorBuffer = new char[ArraySize];
//		File.read(ColorBuffer, ArraySize);
//	}
//
//	File.read(Buffer, 4);
//	const int TexCout = *(int*)Buffer;
//	ArraySize = long long(TexCout) * long long(4);
//	char* TexBuffer = new char[ArraySize];
//	File.read(TexBuffer, ArraySize);
//
//	File.read(Buffer, 4);
//	const int NormCout = *(int*)Buffer;
//	ArraySize = long long(NormCout) * long long(4);
//	char* NormBuffer = new char[ArraySize];
//	File.read(NormBuffer, ArraySize);
//
//	File.read(Buffer, 4);
//	const int TangCout = *(int*)Buffer;
//	ArraySize = long long(TangCout) * long long(4);
//	char* TangBuffer = new char[ArraySize];
//	File.read(TangBuffer, ArraySize);
//
//	File.read(Buffer, 4);
//	const int IndexCout = *(int*)Buffer;
//	ArraySize = long long(IndexCout) * long long(4);
//	char* IndexBuffer = new char[ArraySize];
//	File.read(IndexBuffer, ArraySize);
//
//	File.read(Buffer, 4);
//	const int LayerCount = *(int*)Buffer;
//	std::vector<MeshLayer> Layers;
//	Layers.resize(LayerCount);
//
//	for (size_t i = 0; i < Layers.size(); i++)
//	{
//		if (Version >= 0.55)
//		{
//			File.read(Buffer, 4);
//			const int LayerType = *(int*)Buffer;
//			Layers[i].SetType(LAYER_TYPE(LayerType));
//		}
//
//		Layers[i].SetCaption(FILE_SYSTEM.ReadFEString(File));
//		Layers[i].SetNote(FILE_SYSTEM.ReadFEString(File));
//
//		// TrianglesToData
//		File.read(Buffer, 4);
//		const int TrianglesToDataCout = *(int*)Buffer;
//		std::vector<float> TrianglesData;
//		Layers[i].TrianglesToData.resize(TrianglesToDataCout);
//		File.read((char*)Layers[i].TrianglesToData.data(), TrianglesToDataCout * 4);
//
//		// Debug info.
//		File.read(Buffer, 4);
//		const int DebugInfoPresent = *(int*)Buffer;
//		if (DebugInfoPresent)
//		{
//			Layers[i].DebugInfo = new MeshLayerDebugInfo();
//			Layers[i].DebugInfo->FromFile(File);
//		}
//	}
//
//	FEAABB MeshAABB;
//
//	glm::vec3 Min;
//	File.read(Buffer, 4);
//	Min.x = *(float*)Buffer;
//	File.read(Buffer, 4);
//	Min.y = *(float*)Buffer;
//	File.read(Buffer, 4);
//	Min.z = *(float*)Buffer;
//
//	glm::vec3 Max;
//	File.read(Buffer, 4);
//	Max.x = *(float*)Buffer;
//	File.read(Buffer, 4);
//	Max.y = *(float*)Buffer;
//	File.read(Buffer, 4);
//	Max.z = *(float*)Buffer;
//
//	MeshAABB = FEAABB(Min, Max);
//
//	File.close();
//
//	FEMesh* NewMesh = RawDataToMesh((float*)VertexBuffer, VertexCount,
//		(float*)ColorBuffer, ColorCount,
//		(float*)TexBuffer, TexCout,
//		(float*)NormBuffer, NormCout,
//		(float*)TangBuffer, TangCout,
//		(int*)IndexBuffer, IndexCout,
//		nullptr, 0, 0, "");
//
//	std::vector<double> FEVertices;
//	FEVertices.resize(VertexCount);
//	for (size_t i = 0; i < VertexCount; i++)
//	{
//		FEVertices[i] = ((float*)VertexBuffer)[i];
//	}
//
//	std::vector<int> FEIndices;
//	FEIndices.resize(IndexCout);
//	for (size_t i = 0; i < IndexCout; i++)
//	{
//		FEIndices[i] = ((int*)IndexBuffer)[i];
//	}
//
//	std::vector<float> FENormals;
//	FENormals.resize(NormCout);
//	for (size_t i = 0; i < NormCout; i++)
//	{
//		FENormals[i] = ((float*)NormBuffer)[i];
//	}
//
//	COMPLEXITY_METRIC_MANAGER.Init(FEVertices, FEIndices, FENormals);
//
//	delete[] Buffer;
//	delete[] VertexBuffer;
//	delete[] TexBuffer;
//	delete[] NormBuffer;
//	delete[] TangBuffer;
//	delete[] IndexBuffer;
//
//	NewMesh->AABB = MeshAABB;
//
//	for (size_t i = 0; i < Layers.size(); i++)
//	{
//		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(Layers[i]);
//	}
//
//	return NewMesh;
//}