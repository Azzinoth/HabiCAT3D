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

		void RenderMainWindow();
		void Render();

		bool GetWireFrameMode();
		void SetWireFrameMode(bool NewValue);

		bool GetDeveloperMode();
		void SetDeveloperMode(bool NewValue);

		std::string CameraPositionToStr();
		void StrToCameraPosition(std::string Text);

		std::string CameraRotationToStr();
		void StrToCameraRotation(std::string Text);

		static void OnMeshUpdate();

		float GetRadiusOfAreaToMeasure();
		void SetRadiusOfAreaToMeasure(float NewValue);

		int GetLayerSelectionMode();
		void SetLayerSelectionMode(int NewValue);

		bool GetIsModelCamera();
		void SetIsModelCamera(bool NewValue);

		static void(*SwapCameraImpl)(bool);

		bool GetOutputSelectionToFile();
		void SetOutputSelectionToFile(bool NewValue);

		void ApplyStandardWindowsSizeAndPosition();

		glm::dvec2 LayerValuesAreaDistribution(MeshLayer* Layer, float Value);
	private:
		SINGLETON_PRIVATE_PART(UIManager)

		FEBasicCamera* CurrentCamera = nullptr;

		bool bWireframeMode = false;
		float TimeTookToJitter = 0.0f;

		bool DeveloperMode = false;
		bool bModelCamera = true;
		bool bCloseProgressPopup = false;

		float RadiusOfAreaToMeasure = 1.0f;
		int LayerSelectionMode = 0;

		FEColorRangeAdjuster HeatMapColorRange;

		void RenderLegend();
		void RenderLayerChooseWindow();

		FEGraphRender Graph;
		const int StandardGraphBinCount = 128;
		bool bPixelBins = false;
		int CurrentBinCount = StandardGraphBinCount;
		void FillGraphDataPoints(int BinsCount);
		void RenderHistogram();
		float FillGraphDataPointsTotalTime = 0.0f;
		float SetDataPoints = 0.0f;
		float AreaWithRugositiesTotalTime = 0.0f;
		
		static void OnRugosityCalculationsStart();
		static void OnRugosityCalculationsEnd(MeshLayer NewLayer);

		bool bOutputSelectionToFile = false;

		void RenderDeveloperModeMainWindow();
		void RenderUserModeMainWindow();

		bool bShouldOpenAboutWindow = false;
		void OpenAboutWindow();
		void RenderAboutWindow();

		void AfterLayerChange();

		float FindStandardDeviation(std::vector<float> DataPoints);
	};

	#define UI UIManager::getInstance()
}