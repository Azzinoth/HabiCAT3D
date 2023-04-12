#pragma once

#include "FESDF.h"
using namespace FocalEngine;

namespace FocalEngine
{
	struct SDFInitData_Jitter
	{
		FEMesh* Mesh = nullptr;
		int Dimentions = 4;

		float ShiftX = 0.0f;
		float ShiftY = 0.0f;
		float ShiftZ = 0.0f;

		float GridScale = 2.5f;
	};

	//class SDFNode;
	//class SDF;

	class JitterManager
	{
	public:
		SINGLETON_PUBLIC_PART(JitterManager)

		void CalculateWithSDFJitterAsync(std::function<void(SDFNode* currentNode)> Func);
		void SetOnCalculationsEndCallback(std::function<void(MeshLayer CurrentMeshLayer)> Func/*void(*Func)(MeshLayer)*/);
	private:
		SINGLETON_PRIVATE_PART(JitterManager)

		SDF* currentSDF = nullptr;
		std::function<void(SDFNode* currentNode)> CurrentFunc;

		int JitterDoneCount = 0;
		int JitterToDoCount = 4;

		float ResolutonInM = 1.0f;
		int SDFDimention = 16;

		float ShiftX = 0.0f;
		float ShiftY = 0.0f;
		float ShiftZ = 0.0f;

		float GridScale = 1.25f;

		std::vector<std::vector<float>> PerJitterResult;
		std::vector<float> Result;

		void RunCreationOfSDFAsync();
		static void RunCalculationOnSDFAsync(void* InputData, void* OutputData);
		static void AfterCalculationFinishSDFCallback(void* OutputData);
		void MoveResultDataFromSDF(SDF* SDF);

		static void OnCalculationsEnd();
		std::function<void(MeshLayer CurrentMeshLayer)> OnCalculationsEndCallbackImpl;
		//static void(*OnCalculationsEndCallbackImpl)(MeshLayer);

		static float LastTimeTookForCalculation;
		uint64_t StartTime;
	};

	#define JITTER_MANAGER JitterManager::getInstance()
}