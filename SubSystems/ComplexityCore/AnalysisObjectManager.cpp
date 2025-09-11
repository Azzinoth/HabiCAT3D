#include "AnalysisObjectManager.h"
using namespace FocalEngine;

double MeshGeometryData::GetTotalArea()
{
	return TotalArea;
}

glm::vec3 MeshGeometryData::GetAverageNormal()
{
	return AverageNormal;
}

void MeshGeometryData::UpdateAverageNormal()
{
	AverageNormal = glm::vec3();

	std::vector<double> OriginalAreas;
	double TotalArea = 0.0;
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		const double OriginalArea = GEOMETRY.CalculateTriangleArea(Triangles[i][0], Triangles[i][1], Triangles[i][2]);
		OriginalAreas.push_back(OriginalArea);
		TotalArea += OriginalArea;
	}

	// ******* Getting average normal *******
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		double CurrentTriangleCoef = OriginalAreas[i] / TotalArea;

		AverageNormal += TrianglesNormals[i][0] * static_cast<float>(CurrentTriangleCoef);
		AverageNormal += TrianglesNormals[i][1] * static_cast<float>(CurrentTriangleCoef);
		AverageNormal += TrianglesNormals[i][2] * static_cast<float>(CurrentTriangleCoef);
	}

	AverageNormal = glm::normalize(AverageNormal);
}

AnalysisObjectManager::AnalysisObjectManager() {}
AnalysisObjectManager::~AnalysisObjectManager() {}

void AnalysisObjectManager::InitializeMeshData(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals)
{
	if (CurrentMeshGeometryData != nullptr)
		delete CurrentMeshGeometryData;

	CurrentMeshGeometryData = new MeshGeometryData();

	CurrentMeshGeometryData->Vertices = Vertices;
	CurrentMeshGeometryData->Colors = Colors;
	CurrentMeshGeometryData->UVs = UVs;
	CurrentMeshGeometryData->Tangents = Tangents;
	CurrentMeshGeometryData->Indices = Indices;
	CurrentMeshGeometryData->Normals = Normals;

	std::vector<glm::dvec3> Triangle;
	Triangle.resize(3);
	std::vector<glm::vec3> TriangleNormal;
	TriangleNormal.resize(3);

	for (size_t i = 0; i < Indices.size(); i += 3)
	{
		int VertexPosition = Indices[i] * 3;
		Triangle[0] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 1] * 3;
		Triangle[1] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 2] * 3;
		Triangle[2] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		CurrentMeshGeometryData->Triangles.push_back(Triangle);
		CurrentMeshGeometryData->TrianglesArea.push_back(GEOMETRY.CalculateTriangleArea(Triangle[0], Triangle[1], Triangle[2]));
		CurrentMeshGeometryData->TotalArea += CurrentMeshGeometryData->TrianglesArea.back();

		CurrentMeshGeometryData->TrianglesCentroids.push_back((Triangle[0] + Triangle[1] + Triangle[2]) / 3.0);

		if (!Normals.empty())
		{
			VertexPosition = Indices[i] * 3;
			TriangleNormal[0] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			VertexPosition = Indices[i + 1] * 3;
			TriangleNormal[1] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			VertexPosition = Indices[i + 2] * 3;
			TriangleNormal[2] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			CurrentMeshGeometryData->TrianglesNormals.push_back(TriangleNormal);
		}
	}

	CurrentMeshGeometryData->AABB = FEAABB(Vertices.data(), static_cast<int>(Vertices.size()));
}

void AnalysisObjectManager::AddLoadCallback(std::function<void()> Func)
{
	ClientLoadCallbacks.push_back(Func);
}

void AnalysisObjectManager::ImportOBJ(const char* FileName, bool bForceOneMesh)
{
	FEObjLoader& ObjLoader = FEObjLoader::GetInstance();
	ObjLoader.ForceOneMesh(bForceOneMesh);
	ObjLoader.ForcePositionNormalization(true);
	ObjLoader.UseDoublePrecisionForReadingCoordinates(true);
	ObjLoader.DoubleVertexOnSeams(false);
	ObjLoader.ReadFile(FileName);

	std::vector<FERawOBJData*>* LoadedObjects = ObjLoader.GetLoadedObjects();
	FERawOBJData* FirstObject = LoadedObjects->size() > 0 ? LoadedObjects->at(0) : nullptr;

	if (FirstObject != nullptr)
		InitializeMeshData(FirstObject->DVerC, FirstObject->FColorsC, FirstObject->FTexC, FirstObject->FTanC, FirstObject->FInd, FirstObject->FNorC);

	for (size_t i = 0; i < ClientLoadCallbacks.size(); i++)
	{
		if (ClientLoadCallbacks[i] == nullptr)
			continue;

		ClientLoadCallbacks[i]();
	}
}

void AnalysisObjectManager::SaveToRUGFileAskForFilePath()
{
	std::string FilePath;
	FILE_SYSTEM.ShowFileSaveDialog(FilePath, RUGOSITY_SAVE_FILE_FILTER, 1);

	SaveToRUGFile(FilePath);
}

void AnalysisObjectManager::SaveToRUGFile(std::string FilePath)
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

	int Count = static_cast<int>(CurrentMeshGeometryData->Vertices.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshGeometryData->Vertices.data(), sizeof(double) * Count);

	Count = static_cast<int>(CurrentMeshGeometryData->Colors.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshGeometryData->Colors.data(), sizeof(float) * Count);

	Count = static_cast<int>(CurrentMeshGeometryData->UVs.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshGeometryData->UVs.data(), sizeof(float) * Count);

	Count = static_cast<int>(CurrentMeshGeometryData->Normals.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshGeometryData->Normals.data(), sizeof(float) * Count);

	Count = static_cast<int>(CurrentMeshGeometryData->Tangents.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshGeometryData->Tangents.data(), sizeof(float) * Count);

	Count = static_cast<int>(CurrentMeshGeometryData->Indices.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshGeometryData->Indices.data(), sizeof(int) * Count);

	// FIX ME: That portion should not be here.
	/*Count = static_cast<int>(ActiveComplexityMetricInfo->Layers.size());
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

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].ElementsToData.size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].ElementsToData.data(), sizeof(float) * Count);

		Count = ActiveComplexityMetricInfo->Layers[i].DebugInfo != nullptr;
		File.write((char*)&Count, sizeof(int));
		if (Count)
			ActiveComplexityMetricInfo->Layers[i].DebugInfo->ToFile(File);
	}*/

	FEAABB TempAABB(CurrentMeshGeometryData->Vertices.data(), static_cast<int>(CurrentMeshGeometryData->Vertices.size()));
	File.write((char*)&TempAABB.GetMin()[0], sizeof(float));
	File.write((char*)&TempAABB.GetMin()[1], sizeof(float));
	File.write((char*)&TempAABB.GetMin()[2], sizeof(float));

	File.write((char*)&TempAABB.GetMax()[0], sizeof(float));
	File.write((char*)&TempAABB.GetMax()[1], sizeof(float));
	File.write((char*)&TempAABB.GetMax()[2], sizeof(float));

	File.close();
}

void AnalysisObjectManager::InitializePointCloudData(FEPointCloud* PointCloud)
{
	if (CurrentPointCloudGeometryData != nullptr)
		delete CurrentPointCloudGeometryData;

	CurrentPointCloudGeometryData = new PointCloudGeometryData();

	std::vector<FEPointCloudVertex> TemporaryRawData = PointCloud->GetRawData();
	CurrentPointCloudGeometryData->RawPointCloudData.resize(TemporaryRawData.size());
	for (size_t i = 0; i < TemporaryRawData.size(); i++)
	{
		CurrentPointCloudGeometryData->RawPointCloudData[i].X = static_cast<double>(TemporaryRawData[i].X);
		CurrentPointCloudGeometryData->RawPointCloudData[i].Y = static_cast<double>(TemporaryRawData[i].Y);
		CurrentPointCloudGeometryData->RawPointCloudData[i].Z = static_cast<double>(TemporaryRawData[i].Z);

		CurrentPointCloudGeometryData->RawPointCloudData[i].R = TemporaryRawData[i].R;
		CurrentPointCloudGeometryData->RawPointCloudData[i].G = TemporaryRawData[i].G;
		CurrentPointCloudGeometryData->RawPointCloudData[i].B = TemporaryRawData[i].B;
		CurrentPointCloudGeometryData->RawPointCloudData[i].A = TemporaryRawData[i].A;
	}

	CurrentPointCloudGeometryData->PointCloudAABB = PointCloud->GetAABB();
}

FEAABB AnalysisObjectManager::GetPointCloudAABB()
{
	return CurrentPointCloudGeometryData->PointCloudAABB;
}

bool AnalysisObjectManager::HaveAnyData()
{
	return HaveMeshData() || HavePointCloudData();
}

bool AnalysisObjectManager::HaveMeshData()
{
	return CurrentMeshGeometryData != nullptr;
}

bool AnalysisObjectManager::HavePointCloudData()
{
	return CurrentPointCloudGeometryData != nullptr;
}

FEAABB AnalysisObjectManager::GetMeshAABB()
{
	return CurrentMeshGeometryData->AABB;
}