#pragma once

#include "UIComponents.h"

namespace FocalEngine
{
	class UIManager
	{
	public:
		SINGLETON_PUBLIC_PART(UIManager)

		void showTransformConfiguration(std::string name, FETransformComponent* transform);
		void showCameraTransform();

		void SetCamera(FEBasicCamera* newCamera);
		void SetMeshShader(FEShader* newShader);

		void RenderMainWindow(FEMesh* currentMesh);

		bool GetWireFrameMode();
		void SetWireFrameMode(bool NewValue);

		bool GetDeveloperMode();
		void SetDeveloperMode(bool NewValue);

		std::string CameraPositionToStr();
		void StrToCameraPosition(std::string text);

		std::string CameraRotationToStr();
		void StrToCameraRotation(std::string text);

		void updateCurrentMesh(FEMesh* NewMesh);

		float GetAreaToMeasureRugosity();
		void SetAreaToMeasureRugosity(float NewValue);

		int GetRugositySelectionMode();
		void SetRugositySelectionMode(int NewValue);

		bool GetIsModelCamera();
		void SetIsModelCamera(bool NewValue);

		static void(*SwapCameraImpl)(bool);
	private:
		SINGLETON_PRIVATE_PART(UIManager)

		FEBasicCamera* currentCamera = nullptr;
		FEShader* meshShader = nullptr;

		bool wireframeMode = false;
		float TimeTookToJitter = 0.0f;

		bool DeveloperMode = false;
		bool bModelCamera = true;
		bool bCloseProgressPopup = false;
		FEMesh* currentMesh;

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
		float FillGraphDataPoints_TotalTime = 0.0f;
		float SetDataPoints = 0.0f;
		float AreaWithRugosities_TotalTime = 0.0f;
		
		static void OnRugosityCalculationsStart();
		static void OnRugosityCalculationsEnd();
		//void TestCGALVariant();
	};

	#define UI UIManager::getInstance()
}