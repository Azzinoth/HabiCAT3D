#pragma once
#include "ComplexityMetricInfo.h"
using namespace FocalEngine;

#include "SubSystems/FEObjLoader.h"

namespace FocalEngine
{
	class ComplexityMetricManager
	{
	public:
		SINGLETON_PUBLIC_PART(ComplexityMetricManager)

		ComplexityMetricInfo* ActiveComplexityMetricInfo = nullptr;

		void Init(std::vector<double>& FEVertices, std::vector<int>& FEIndices, std::vector<float>& FENormals);
		void ImportOBJ(const char* FileName, bool bForceOneMesh);

	private:
		SINGLETON_PRIVATE_PART(ComplexityMetricManager)

		//FEMesh* ImportOBJ(const char* FileName, bool bForceOneMesh);
		//FEMesh* LoadRUGMesh(std::string FileName);

		//std::vector<std::function<void()>> ClientLoadCallbacks;
	};

	#define COMPLEXITY_METRIC_MANAGER ComplexityMetricManager::getInstance()
}