#pragma once
#include "../../EngineInclude.h"
using namespace FocalEngine;

#define APP_VERSION 0.91f

const COMDLG_FILTERSPEC RUGOSITY_LOAD_FILE_FILTER[] =
{
	{ L"Mesh files (*.obj; *.rug)", L"*.obj;*.rug" }
};

const COMDLG_FILTERSPEC RUGOSITY_SAVE_FILE_FILTER[] =
{
	{ L"Rugosity file (*.rug)", L"*.rug" }
};

class MeshGeometryData
{
	friend class AnalysisObjectManager;

	FEAABB AABB;
	double TotalArea = 0.0;
	glm::vec3 AverageNormal = glm::vec3();
public:
	double GetTotalArea();

	std::vector<double> Vertices;
	std::vector<float> Colors;
	std::vector<float> UVs;
	std::vector<float> Tangents;
	std::vector<int> Indices;
	std::vector<float> Normals;

	std::vector<int> TriangleSelected;

	std::vector<std::vector<glm::dvec3>> Triangles;
	std::vector<double> TrianglesArea;

	std::vector<std::vector<glm::vec3>> TrianglesNormals;
	std::vector<glm::dvec3> TrianglesCentroids;

	glm::vec3 GetAverageNormal();
	void UpdateAverageNormal();

	std::string FileName;
	FETransformComponent* Position = new FETransformComponent();
};

struct PointCloudGeometryData
{
	std::vector<FEPointCloudVertexDouble> RawPointCloudData;
	FEAABB PointCloudAABB;
};

class AnalysisObjectManager
{
public:
	SINGLETON_PUBLIC_PART(AnalysisObjectManager)

	MeshGeometryData* CurrentMeshGeometryData = nullptr;
	PointCloudGeometryData* CurrentPointCloudGeometryData = nullptr;

	void InitializeMeshData(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals);
	void ImportOBJ(const char* FileName, bool bForceOneMesh);
	FEAABB GetMeshAABB();

	void AddLoadCallback(std::function<void()> Func);
	void SaveToRUGFile(std::string FilePath);
	void SaveToRUGFileAskForFilePath();

	void InitializePointCloudData(FEPointCloud* PointCloud);
	FEAABB GetPointCloudAABB();

	bool HaveAnyData();
	bool HaveMeshData();
	bool HavePointCloudData();
private:
	SINGLETON_PRIVATE_PART(AnalysisObjectManager)

	std::vector<std::function<void()>> ClientLoadCallbacks;
};

#define ANALYSIS_OBJECT_MANAGER AnalysisObjectManager::GetInstance()