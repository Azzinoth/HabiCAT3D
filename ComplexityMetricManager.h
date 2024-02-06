#pragma once
#include "ComplexityMetricInfo.h"
using namespace FocalEngine;

#include "SubSystems/FEObjLoader.h"

#define APP_VERSION 0.62

namespace FocalEngine
{
	const COMDLG_FILTERSPEC RUGOSITY_LOAD_FILE_FILTER[] =
	{
		{ L"Mesh files (*.obj; *.rug)", L"*.obj;*.rug" }
	};

	const COMDLG_FILTERSPEC RUGOSITY_SAVE_FILE_FILTER[] =
	{
		{ L"Rugosity file (*.rug)", L"*.rug" }
	};

	class ComplexityMetricManager
	{
	public:
		SINGLETON_PUBLIC_PART(ComplexityMetricManager)

		ComplexityMetricInfo* ActiveComplexityMetricInfo = nullptr;

		void Init(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals);
		void ImportOBJ(const char* FileName, bool bForceOneMesh);

		void AddLoadCallback(std::function<void()> Func);
		void SaveToRUGFile();
	private:
		SINGLETON_PRIVATE_PART(ComplexityMetricManager)

		//FEMesh* ImportOBJ(const char* FileName, bool bForceOneMesh);
		//FEMesh* LoadRUGMesh(std::string FileName);

		std::vector<std::function<void()>> ClientLoadCallbacks;
	};

	#define COMPLEXITY_METRIC_MANAGER ComplexityMetricManager::getInstance()
}