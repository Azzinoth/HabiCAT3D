#pragma once

#include "FESDF.h"
using namespace FocalEngine;

namespace FocalEngine
{
#define DEFAULT_GRID_SIZE 1.25f
#define GRID_VARIANCE 25

	static std::vector<float> SphereJitter = {
		0.0000, 1.0000, -0.0000, 1.430000f,
		1.0000, 0.0000, -0.0000, 1.410000f,
		0.0000, 0.0000, -1.0000, 1.390000f,
		-1.0000, 0.0000, -0.0000, 1.330000f,
		0.0000, 0.0000, 1.0000, 1.400000f,
		0.0000, -1.0000, -0.0000, 1.340000f,
		0.5000, 0.8660, -0.0000, 1.280000f,
		0.8660, 0.5000, -0.0000, 1.290000f,
		0.0000, 0.8660, -0.5000, 1.300000f,
		0.0000, 0.5000, -0.8660, 1.410000f,
		-0.5000, 0.8660, -0.0000, 1.250000f,
		-0.8660, 0.5000, -0.0000, 1.300000f,
		0.0000, 0.8660, 0.5000, 1.270000f,
		0.0000, 0.5000, 0.8660, 1.440000f,
		0.5000, -0.8660, -0.0000, 1.280000f,
		0.8660, -0.5000, -0.0000, 1.270000f,
		0.0000, -0.8660, -0.5000, 1.250000f,
		0.0000, -0.5000, -0.8660, 1.460000f,
		-0.5000, -0.8660, -0.0000, 1.250000f,
		-0.8660, -0.5000, -0.0000, 1.260000f,
		0.0000, -0.8660, 0.5000, 1.410000f,
		0.0000, -0.5000, 0.8660, 1.440000f,
		0.8660, 0.0000, -0.5000, 1.400000f,
		0.5000, 0.0000, -0.8660, 1.390000f,
		-0.5000, 0.0000, -0.8660, 1.410000f,
		-0.8660, 0.0000, -0.5000, 1.420000f,
		-0.8660, 0.0000, 0.5000, 1.250000f,
		-0.5000, 0.0000, 0.8660, 1.280000f,
		0.5000, 0.0000, 0.8660, 1.460000f,
		0.8660, 0.0000, 0.5000, 1.400000f,
		0.5477, 0.6325, -0.5477, 1.270000f,
		-0.5477, 0.6325, -0.5477, 1.270000f,
		-0.5477, 0.6325, 0.5477, 1.390000f,
		0.5477, 0.6325, 0.5477, 1.320000f,
		0.5477, -0.6325, -0.5477, 1.270000f,
		-0.5477, -0.6325, -0.5477, 1.430000f,
		-0.5477, -0.6325, 0.5477, 1.280000f,
		0.5477, -0.6325, 0.5477, 1.300000f,
		0.0000, 0.5000, -0.0000, 1.320000f,
		0.4714, -0.1667, -0.0000, 1.260000f,
		-0.2357, -0.1667, -0.4082, 1.330000f,
		-0.2357, -0.1667, 0.4082, 1.430000f,
		0.2973, 0.4020, -0.0000, 1.470000f,
		0.4781, 0.1463, -0.0000, 1.310000f,
		-0.1487, 0.4020, -0.2575, 1.490000f,
		-0.2391, 0.1463, -0.4140, 1.250000f,
		-0.1487, 0.4020, 0.2575, 1.480000f,
		-0.2391, 0.1463, 0.4140, 1.410000f,
		0.3294, -0.2742, -0.2575, 1.320000f,
		0.0583, -0.2742, -0.4140, 1.480000f,
		0.3294, -0.2742, 0.2575, 1.450000f,
		0.0583, -0.2742, 0.4140, 1.370000f,
		-0.3877, -0.2742, -0.1565, 1.430000f,
		-0.3877, -0.2742, 0.1565, 1.390000f,
		0.2132, 0.2611, -0.3693, 1.370000f,
		-0.4264, 0.2611, -0.0000, 1.480000f,
		0.2132, 0.2611, 0.3693, 1.460000f,
		0.1040, -0.4891, -0.0000, 1.380000f,
	};

	static std::vector<float> PseudoRandom64 = {
		0.300000f, 0.100000f, 0.020000f, 1.430000f,
		0.450000f, 0.130000f, 0.090000f, 1.410000f,
		0.410000f, -0.310000f, 0.300000f, 1.390000f,
		0.340000f, 0.130000f, 0.470000f, 1.330000f,
		-0.390000f, 0.060000f, -0.440000f, 1.400000f,
		-0.350000f, -0.020000f, 0.380000f, 1.340000f,
		-0.300000f, 0.480000f, -0.500000f, 1.280000f,
		-0.390000f, 0.060000f, 0.370000f, 1.290000f,
		-0.120000f, 0.270000f, 0.300000f, 1.300000f,
		0.030000f, -0.060000f, -0.290000f, 1.410000f,
		-0.240000f, -0.130000f, -0.130000f, 1.250000f,
		0.220000f, -0.320000f, 0.160000f, 1.300000f,
		-0.360000f, -0.010000f, -0.190000f, 1.270000f,
		-0.200000f, -0.220000f, -0.140000f, 1.440000f,
		0.340000f, -0.100000f, -0.060000f, 1.280000f,
		-0.210000f, 0.290000f, 0.090000f, 1.270000f,
		0.130000f, 0.010000f, 0.180000f, 1.250000f,
		0.090000f, 0.460000f, 0.350000f, 1.460000f,
		-0.500000f, -0.090000f, 0.320000f, 1.250000f,
		-0.200000f, 0.080000f, -0.320000f, 1.260000f,
		-0.390000f, -0.440000f, 0.420000f, 1.410000f,
		0.260000f, -0.040000f, 0.400000f, 1.440000f,
		-0.020000f, -0.010000f, 0.290000f, 1.400000f,
		-0.020000f, -0.370000f, 0.280000f, 1.390000f,
		0.240000f, -0.320000f, -0.290000f, 1.410000f,
		-0.100000f, 0.360000f, -0.110000f, 1.420000f,
		-0.270000f, -0.490000f, 0.470000f, 1.250000f,
		-0.450000f, 0.390000f, -0.310000f, 1.280000f,
		0.440000f, -0.290000f, -0.360000f, 1.460000f,
		0.000000f, -0.050000f, -0.370000f, 1.400000f,
		-0.060000f, 0.120000f, 0.430000f, 1.270000f,
		-0.490000f, 0.410000f, 0.150000f, 1.270000f,
		-0.500000f, 0.370000f, -0.040000f, 1.390000f,
		0.380000f, -0.210000f, 0.040000f, 1.320000f,
		0.280000f, 0.340000f, -0.430000f, 1.270000f,
		-0.410000f, -0.450000f, 0.330000f, 1.430000f,
		-0.180000f, -0.370000f, 0.070000f, 1.280000f,
		-0.110000f, -0.310000f, 0.400000f, 1.300000f,
		-0.060000f, 0.270000f, 0.240000f, 1.320000f,
		-0.390000f, 0.480000f, 0.180000f, 1.260000f,
		-0.390000f, 0.100000f, 0.070000f, 1.330000f,
		-0.260000f, -0.230000f, 0.450000f, 1.430000f,
		0.480000f, 0.080000f, -0.140000f, 1.470000f,
		0.330000f, -0.380000f, -0.350000f, 1.310000f,
		0.180000f, 0.200000f, -0.330000f, 1.490000f,
		-0.230000f, -0.110000f, 0.070000f, 1.250000f,
		0.190000f, -0.230000f, -0.330000f, 1.480000f,
		0.480000f, 0.150000f, 0.290000f, 1.410000f,
		-0.210000f, 0.240000f, 0.180000f, 1.320000f,
		0.450000f, -0.230000f, 0.240000f, 1.480000f,
		0.400000f, 0.370000f, -0.060000f, 1.450000f,
		0.080000f, -0.500000f, 0.200000f, 1.370000f,
		-0.240000f, -0.380000f, 0.440000f, 1.430000f,
		-0.230000f, -0.210000f, 0.240000f, 1.390000f,
		-0.320000f, -0.050000f, 0.330000f, 1.370000f,
		-0.030000f, -0.470000f, -0.320000f, 1.480000f,
		0.320000f, 0.160000f, 0.460000f, 1.460000f,
		0.270000f, 0.430000f, 0.410000f, 1.380000f,
		0.240000f, -0.470000f, -0.190000f, 1.430000f,
		0.480000f, -0.420000f, -0.240000f, 1.490000f,
		-0.400000f, -0.270000f, 0.060000f, 1.450000f,
		0.280000f, 0.430000f, -0.010000f, 1.490000f,
		-0.190000f, 0.110000f, -0.100000f, 1.450000f,
		-0.210000f, -0.040000f, 0.340000f, 1.440000f,
	};

	struct SDFInitData_Jitter
	{
		float ShiftX = 0.0f;
		float ShiftY = 0.0f;
		float ShiftZ = 0.0f;

		float GridScale = 2.5f;
	};

	class JitterManager
	{
		friend class UIManager;
	public:
		SINGLETON_PUBLIC_PART(JitterManager)

		static void OnMeshUpdate();

		void CalculateWithSDFJitterAsync(std::function<void(SDFNode* currentNode)> Func, bool bSmootherResult = false);
		void SetOnCalculationsStartCallback(std::function<void()> Func);
		void SetOnCalculationsEndCallback(std::function<void(MeshLayer CurrentMeshLayer)> Func);

		float GetResolutonInM();
		void SetResolutonInM(float NewResolutonInM);

		float GetLowestPossibleResolution();
		float GetHigestPossibleResolution();

		int GetJitterDoneCount();
		int GetJitterToDoCount();

		std::vector<std::vector<float>> GetPerJitterResult();

		SDF* GetLastUsedSDF();
		std::vector<SDFInitData_Jitter> GetLastUsedJitterSettings();

		/**
		* @brief Sets the function that determines whether a calculated value should be ignored.
		*
		* This function is called for each calculated value in each jitter. If it returns true, the value is ignored.
		* If it returns false, the value is used to calculate all jitter combined result array.
		* 
		* After each calculation, this function will reset to ensure the default behavior is applied.
		*
		* @param Func The desired function.
		*/
		void SetIgnoreValueFunction(std::function<bool(float Value)> Func);

		/**
		* @brief Sets the fallback value for the result array.
		*
		* This value is used for an element in the result array only if all its
		* calculated values are ignored. By providing a fallback, we ensure that
		* the result array has a consistent value in such scenarios, preventing
		* potential issues from uninitialized or unpredictable data.
		* 
		* After each calculation, this value will be reset to 1.0f to ensure that the default behavior is used.
		*
		* @param NewValue The desired fallback value.
		*/
		void SetFallbackValue(float NewValue);

	private:
		SINGLETON_PRIVATE_PART(JitterManager)

		SDF* LastUsedSDF = nullptr;
		std::vector<SDFInitData_Jitter> LastUsedJitterSettings;
		std::function<void(SDFNode* currentNode)> CurrentFunc;

		int JitterDoneCount = 0;
		int JitterToDoCount = 4;
		int DebugJitterToDoCount = -1;
		int GetDebugJitterToDoCount();
		void SetDebugJitterToDoCount(int NewValue);

		float ResolutonInM = 1.0f;
		float LowestPossibleResolution = -1.0f;
		float HigestPossibleResolution = -1.0f;

		float ShiftX = 0.0f;
		float ShiftY = 0.0f;
		float ShiftZ = 0.0f;

		float GridScale = 1.25f;

		std::vector<float> Result;
		std::vector<std::vector<float>> PerJitterResult;
		std::vector<int> CorrectValuesCounters;
		float FallbackValue = 1.0f;
		std::function<bool(float Value)> IgnoreValueFunc = nullptr;

		void RunCreationOfSDFAsync();
		static void RunCalculationOnSDFAsync(void* InputData, void* OutputData);
		static void AfterCalculationFinishSDFCallback(void* OutputData);
		void MoveResultDataFromSDF(SDF* SDF);

		static void OnCalculationsStart();
		static void OnCalculationsEnd();

		std::vector <std::function<void()>> OnCalculationsStartCallbacks;
		std::vector<std::function<void(MeshLayer CurrentMeshLayer)>> OnCalculationsEndCallbacks;

		float LastTimeTookForCalculation;
		uint64_t StartTime;
	};

	#define JITTER_MANAGER JitterManager::getInstance()
}