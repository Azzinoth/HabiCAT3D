#pragma once
#include "FESDF.h"
using namespace FocalEngine;

namespace FocalEngine
{
#define DEFAULT_GRID_SIZE 1.25f
#define GRID_VARIANCE 25

	struct SDFInitData
	{
		FEMesh* Mesh = nullptr;
		int Dimentions = 4;
		bool UseJitterExpandedAABB = false;

		float ShiftX = 0.0f;
		float ShiftY = 0.0f;
		float ShiftZ = 0.0f;

		float GridScale = 2.5f;
	};

	class RugosityManager
	{
	public:
		SINGLETON_PUBLIC_PART(RugosityManager)

		SDF* currentSDF = nullptr;
		FEBasicCamera* currentCamera = nullptr;

		bool bWeightedNormals = true;
		bool bNormalizedNormals = true;
		bool bDeleteOutliers = true;

		int SDFDimention = 16;

		int JitterDoneCount = 0;
		int JitterToDoCount = 4;

		static void calculateSDFAsync(void* InputData, void* OutputData);
		static void calculateSDFCallback(void* OutputData);
		void RunCreationOfSDFAsync(bool bJitter = false);
		void CalculateRugorsityWithJitterAsync(int RugosityLayerIndex = 0);

		std::vector<std::string> dimentionsList;

		float ResolutonInM = 1.0f;
		float LowestResolution = -1.0f;
		float HigestResolution = -1.0f;
		static void OnMeshUpdate();
		std::vector<std::string> colorSchemesList;

		std::string colorSchemeIndexToString(int index);
		int colorSchemeIndexFromString(std::string name);

		bool GetUseFindSmallestRugosity();
		void SetUseFindSmallestRugosity(bool NewValue);

		bool GetUseCGALVariant();
		void SetUseCGALVariant(bool NewValue);

		float GetLastTimeTookForCalculation();

		void SetOnRugosityCalculationsStartCallback(void(*Func)(void));
		void SetOnRugosityCalculationsEndCallback(void(*Func)(MeshLayer));

		std::string GetUsedRugosityAlgorithmName();
		void SetUsedRugosityAlgorithmName(std::string name);
		std::vector<std::string> RugosityAlgorithmList;

		std::vector<std::vector<float>> PerJitterResult;
		std::vector<float> Result;

		/*bool bTestJitter = false;
		bool bTestSphereJitter = false;*/
	private:
		SINGLETON_PRIVATE_PART(RugosityManager)

		SDF* calculateSDF(int dimentions, FEBasicCamera* currentCamera, bool UseJitterExpandedAABB = false);
		void MoveRugosityInfoFromSDF(SDF* SDF);

		float shiftX = 0.0f;
		float shiftY = 0.0f;
		float shiftZ = 0.0f;

		float GridScale = 1.25f;

		int RugosityLayerIndex = 0;
		bool bUseFindSmallestRugosity = false;
		bool bUseCGALVariant = false;

		static float LastTimeTookForCalculation;

		static void OnRugosityCalculationsStart();
		static void(*OnRugosityCalculationsStartCallbackImpl)(void);

		static void OnRugosityCalculationsEnd();
		static void(*OnRugosityCalculationsEndCallbackImpl)(MeshLayer);

		uint64_t StartTime;
	};

	#define RUGOSITY_MANAGER RugosityManager::getInstance()
}