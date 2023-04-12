#pragma once
//#include "FESDF.h"
#include "JitterManager.h"
using namespace FocalEngine;

namespace FocalEngine
{
#define DEFAULT_GRID_SIZE 1.25f
#define GRID_VARIANCE 25

	static std::vector<glm::vec3> SphereVectors = {
		glm::vec3(0.0000, 1.0000, -0.0000),
		glm::vec3(0.5257, 0.8507, -0.0000),
		glm::vec3(0.1625, 0.8507, -0.5000),
		glm::vec3(-0.4253, 0.8507, -0.3090),
		glm::vec3(-0.4253, 0.8507, 0.3090),
		glm::vec3(0.1625, 0.8507, 0.5000),
		glm::vec3(0.8944, 0.4472, -0.0000),
		glm::vec3(0.2764, 0.4472, -0.8507),
		glm::vec3(-0.7236, 0.4472, -0.5257),
		glm::vec3(-0.7236, 0.4472, 0.5257),
		glm::vec3(0.2764, 0.4472, 0.8507),
		glm::vec3(0.6882, 0.5257, -0.5000),
		glm::vec3(-0.2629, 0.5257, -0.8090),
		glm::vec3(-0.8507, 0.5257, 0.0000),
		glm::vec3(-0.2629, 0.5257, 0.8090),
		glm::vec3(0.6882, 0.5257, 0.5000),
		glm::vec3(0.9511, 0.0000, -0.3090),
		glm::vec3(0.5878, 0.0000, -0.8090),
		glm::vec3(-0.0000, 0.0000, -1.0000),
		glm::vec3(-0.5878, 0.0000, -0.8090),
		glm::vec3(-0.9511, 0.0000, -0.3090),
		glm::vec3(-0.9511, 0.0000, 0.3090),
		glm::vec3(-0.5878, 0.0000, 0.8090),
		glm::vec3(0.0000, 0.0000, 1.0000),
		glm::vec3(0.5878, 0.0000, 0.8090),
		glm::vec3(0.9511, 0.0000, 0.3090),
		glm::vec3(0.2733, 0.9619, -0.0000),
		glm::vec3(0.0844, 0.9619, -0.2599),
		glm::vec3(-0.2211, 0.9619, -0.1606),
		glm::vec3(-0.2211, 0.9619, 0.1606),
		glm::vec3(0.0844, 0.9619, 0.2599),
		glm::vec3(0.3618, 0.8944, -0.2629),
		glm::vec3(-0.1382, 0.8944, -0.4253),
		glm::vec3(-0.4472, 0.8944, 0.0000),
		glm::vec3(-0.1382, 0.8944, 0.4253),
		glm::vec3(0.3618, 0.8944, 0.2629),
		glm::vec3(0.7382, 0.6746, -0.0000),
		glm::vec3(0.2281, 0.6746, -0.7020),
		glm::vec3(-0.5972, 0.6746, -0.4339),
		glm::vec3(-0.5972, 0.6746, 0.4339),
		glm::vec3(0.2281, 0.6746, 0.7020),
		glm::vec3(0.6382, 0.7236, -0.2629),
		glm::vec3(-0.0528, 0.7236, -0.6882),
		glm::vec3(-0.6708, 0.7236, -0.1625),
		glm::vec3(-0.3618, 0.7236, 0.5878),
		glm::vec3(0.4472, 0.7236, 0.5257),
		glm::vec3(0.6382, 0.7236, 0.2629),
		glm::vec3(0.4472, 0.7236, -0.5257),
		glm::vec3(-0.3618, 0.7236, -0.5878),
		glm::vec3(-0.6708, 0.7236, 0.1625),
		glm::vec3(-0.0528, 0.7236, 0.6882),
		glm::vec3(0.8226, 0.5057, -0.2599),
		glm::vec3(0.0070, 0.5057, -0.8627),
		glm::vec3(-0.8183, 0.5057, -0.2733),
		glm::vec3(-0.5128, 0.5057, 0.6938),
		glm::vec3(0.5014, 0.5057, 0.7020),
		glm::vec3(0.8226, 0.5057, 0.2599),
		glm::vec3(0.5014, 0.5057, -0.7020),
		glm::vec3(-0.5128, 0.5057, -0.6938),
		glm::vec3(-0.8183, 0.5057, 0.2733),
		glm::vec3(0.0070, 0.5057, 0.8627),
		glm::vec3(0.9593, 0.2325, -0.1606),
		glm::vec3(0.8618, 0.2764, -0.4253),
		glm::vec3(0.6708, 0.2764, -0.6882),
		glm::vec3(0.4492, 0.2325, -0.8627),
		glm::vec3(0.1437, 0.2325, -0.9619),
		glm::vec3(-0.1382, 0.2764, -0.9511),
		glm::vec3(-0.4472, 0.2764, -0.8507),
		glm::vec3(-0.6816, 0.2325, -0.6938),
		glm::vec3(-0.8705, 0.2325, -0.4339),
		glm::vec3(-0.9472, 0.2764, -0.1625),
		glm::vec3(-0.9472, 0.2764, 0.1625),
		glm::vec3(-0.8705, 0.2325, 0.4339),
		glm::vec3(-0.6816, 0.2325, 0.6938),
		glm::vec3(-0.4472, 0.2764, 0.8507),
		glm::vec3(-0.1382, 0.2764, 0.9511),
		glm::vec3(0.1437, 0.2325, 0.9619),
		glm::vec3(0.4492, 0.2325, 0.8627),
		glm::vec3(0.6708, 0.2764, 0.6882),
		glm::vec3(0.8618, 0.2764, 0.4253),
		glm::vec3(0.9593, 0.2325, 0.1606),
		glm::vec3(0.8090, 0.0000, -0.5878),
		glm::vec3(0.3090, 0.0000, -0.9511),
		glm::vec3(-0.3090, 0.0000, -0.9511),
		glm::vec3(-0.8090, 0.0000, -0.5878),
		glm::vec3(-1.0000, 0.0000, 0.0000),
		glm::vec3(-0.8090, 0.0000, 0.5878),
		glm::vec3(-0.3090, 0.0000, 0.9511),
		glm::vec3(0.3090, 0.0000, 0.9511),
		glm::vec3(0.8090, 0.0000, 0.5878),
		glm::vec3(1.0000, 0.0000, -0.0000)
	};

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

		static void CalculateOneNodeRugosity(SDFNode* CurrentNode);

		uint64_t StartTime;
	};

	#define RUGOSITY_MANAGER RugosityManager::getInstance()
}