#pragma once
#include "FESDF.h"
using namespace FocalEngine;

namespace FocalEngine
{
#define DEFAULT_GRID_SIZE 1.25f
#define GRID_VARIANCE 25

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
		FEBasicCamera* currentCamera = nullptr;

		void MoveRugosityInfoToMesh(SDF* SDF, bool bFinalJitter = true);
		int JitterCounter = 0;
		SDF* calculateSDF(FEMesh* mesh, int dimentions, FEBasicCamera* currentCamera, bool UseJitterExpandedAABB = false);

		float shiftX = 0.0f;
		float shiftY = 0.0f;
		float shiftZ = 0.0f;

		float GridScale = 1.25f;

		bool bWeightedNormals = true;
		bool bNormalizedNormals = true;

		int SDFDimention = 16;
		bool bLastJitter = true;
		int SmoothingFactor = 4;
		int newSDFSeen = 0;
		static void calculateSDFAsync(void* InputData, void* OutputData);
		static void calculateSDFCallback(void* OutputData);
		void RunCreationOfSDFAsync(FEMesh* mesh, bool bJitter = false);
		void calculateRugorsityWithJitterAsyn(FEMesh* mesh, int RugosityLayerIndex = 0);

		std::vector<std::string> dimentionsList;

		float ResolutonInM = 1.0f;
		float LowestResolution = -1.0f;
		float HigestResolution = -1.0f;
		void UpdateCurrentMesh(FEMesh* NewMesh);
		std::vector<std::string> colorSchemesList;

		std::string colorSchemeIndexToString(int index);
		int colorSchemeIndexFromString(std::string name);

		bool GetUseFindSmallestRugosity();
		void SetUseFindSmallestRugosity(bool NewValue);

		bool GetUseCGALVariant();
		void SetUseCGALVariant(bool NewValue);

		glm::dvec2 RugosityAreaDistribution(float RugosityValue);
		double AreaWithRugosities(float MinRugosity, float MaxRugosity);

		float GetLastTimeTookForCalculation();
		float GetMaxRugosityWithOutOutliers(float OutliersPercentage);

		void SetOnRugosityCalculationsStartCallback(void(*Func)(void));
		void SetOnRugosityCalculationsEndCallback(void(*Func)(void));

		std::string GetUsedRugosityAlgorithmName();
		void SetUsedRugosityAlgorithmName(std::string name);
		std::vector<std::string> RugosityAlgorithmList;

		static void ForceOnRugosityCalculationsEnd(FEMesh* Mesh);

		bool IsRugosityInfoReady();
	private:
		SINGLETON_PRIVATE_PART(RugosityManager)

		int RugosityLayerIndex = 0;
		bool bUseFindSmallestRugosity = false;
		bool bUseCGALVariant = false;

		static float LastTimeTookForCalculation;

		static void OnRugosityCalculationsStart(FEMesh* Mesh);
		static void(*OnRugosityCalculationsStartCallbackImpl)(void);

		static void OnRugosityCalculationsEnd(FEMesh* Mesh = nullptr);
		static void(*OnRugosityCalculationsEndCallbackImpl)(void);

		static bool bHaveRugosityInfoReady;

		static FEMesh* CurrentMesh;
	public:
		static std::vector<std::tuple<double, double, int>> RugosityTriangleAreaAndIndex;
	};

	#define RUGOSITY_MANAGER RugosityManager::getInstance()
}