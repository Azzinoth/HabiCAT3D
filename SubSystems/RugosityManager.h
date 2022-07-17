#pragma once
#include "FESDF.h"
using namespace FocalEngine;

namespace FocalEngine
{
	struct SDFInitData
	{
		FEMesh* mesh = nullptr;
		int dimentions = 4;
		bool UseJitterExpandedAABB = false;

		float shiftX = 0.0f;
		float shiftY = 0.0f;
		float shiftZ = 0.0f;

		float GridScale = 2.5f;
	};

	class RugosityManager
	{
	public:
		SINGLETON_PUBLIC_PART(RugosityManager)

		SDF* currentSDF = nullptr;
		FEFreeCamera* currentCamera = nullptr;

		void MoveRugosityInfoToMesh(SDF* SDF, bool bFinalJitter = true);
		int JitterCounter = 0;
		SDF* calculateSDF(FEMesh* mesh, int dimentions, FEFreeCamera* currentCamera, bool UseJitterExpandedAABB = false);

		float shiftX = 0.0f;
		float shiftY = 0.0f;
		float shiftZ = 0.0f;

		float GridScale = 2.5f;

		bool bWeightedNormals = false;
		bool bNormalizedNormals = false;

		int SDFDimention = 16;
		bool bLastJitter = true;
		int SmoothingFactor = 4;
		int newSDFSeen = 0;
		static void calculateSDFAsync(void* InputData, void* OutputData);
		static void calculateSDFCallback(void* OutputData);
		void RunCreationOfSDFAsync(FEMesh* mesh, bool bJitter = false);
		void calculateRugorsityWithJitterAsyn(FEMesh* mesh);
	private:
		SINGLETON_PRIVATE_PART(RugosityManager)


	};

	#define RUGOSITY_MANAGER RugosityManager::getInstance()
}