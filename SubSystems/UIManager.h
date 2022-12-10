#pragma once

#include "UIComponents.h"

namespace FocalEngine
{
	class UIManager
	{
	public:
		SINGLETON_PUBLIC_PART(UIManager)

		void ShowTransformConfiguration(std::string Name, FETransformComponent* Transform);
		void ShowCameraTransform();

		void SetCamera(FEBasicCamera* NewCamera);
		void SetMeshShader(FEShader* NewShader);

		void RenderMainWindow(FEMesh* CurrentMesh);

		bool GetWireFrameMode();
		void SetWireFrameMode(bool NewValue);

		bool GetDeveloperMode();
		void SetDeveloperMode(bool NewValue);

		std::string CameraPositionToStr();
		void StrToCameraPosition(std::string Text);

		std::string CameraRotationToStr();
		void StrToCameraRotation(std::string Text);

		void UpdateCurrentMesh(FEMesh* NewMesh);

		float GetAreaToMeasureRugosity();
		void SetAreaToMeasureRugosity(float NewValue);

		int GetRugositySelectionMode();
		void SetRugositySelectionMode(int NewValue);

		bool GetIsModelCamera();
		void SetIsModelCamera(bool NewValue);

		static void(*SwapCameraImpl)(bool);

		bool GetOutputSelectionToFile();
		void SetOutputSelectionToFile(bool NewValue);
	private:
		SINGLETON_PRIVATE_PART(UIManager)

		FEBasicCamera* CurrentCamera = nullptr;
		FEShader* MeshShader = nullptr;

		bool bWireframeMode = false;
		float TimeTookToJitter = 0.0f;

		bool DeveloperMode = false;
		bool bModelCamera = true;
		bool bCloseProgressPopup = false;
		FEMesh* CurrentMesh = nullptr;

		float AreaToMeasureRugosity = 1.0f;
		int RugositySelectionMode = 0;

		FEColorRangeAdjuster RugosityColorRange;

		void RenderLegend();
		void UpdateRugosityRangeSettings();
		void RenderVisualModeWindow();

		FEGraphRender Graph;
		const int StandardGraphBinCount = 128;
		bool bPixelBins = false;
		int CurrentBinCount = StandardGraphBinCount;
		void FillGraphDataPoints(int BinsCount);
		void RenderRugosityHistogram();
		float FillGraphDataPointsTotalTime = 0.0f;
		float SetDataPoints = 0.0f;
		float AreaWithRugositiesTotalTime = 0.0f;
		
		static void OnRugosityCalculationsStart();
		static void OnRugosityCalculationsEnd();
		//void TestCGALVariant();

		bool bOutputSelectionToFile = false;
	};

	#define UI UIManager::getInstance()
}