#pragma once
#include "ComplexityMetricInfo.h"
using namespace FocalEngine;

#include "../EngineInclude.h"

#define APP_VERSION 0.87f

const COMDLG_FILTERSPEC RUGOSITY_LOAD_FILE_FILTER[] =
{
	{ L"Mesh files (*.obj; *.rug)", L"*.obj;*.rug" }
};

const COMDLG_FILTERSPEC RUGOSITY_SAVE_FILE_FILTER[] =
{
	{ L"Rugosity file (*.rug)", L"*.rug" }
};

// FIX ME: Rename it to a analysis manager?
// FIX ME: ComplexityMetricInfo should be directly integrated into this class.
class ComplexityMetricManager
{
public:
	SINGLETON_PUBLIC_PART(ComplexityMetricManager)

	ComplexityMetricInfo* ActiveComplexityMetricInfo = nullptr;

	void Init(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals);
	void ImportOBJ(const char* FileName, bool bForceOneMesh);

	void AddLoadCallback(std::function<void()> Func);
	void SaveToRUGFile(std::string FilePath);
	void SaveToRUGFileAskForFilePath();

	void InitializePointCloudData(FEPointCloud* PointCloud);
	FEAABB GetPointCloudAABB();

	bool HaveAnyData();
	bool HaveMeshData();
	bool HavePointCloudData();
private:
	SINGLETON_PRIVATE_PART(ComplexityMetricManager)

	std::vector<std::function<void()>> ClientLoadCallbacks;
};

#define COMPLEXITY_METRIC_MANAGER ComplexityMetricManager::GetInstance()