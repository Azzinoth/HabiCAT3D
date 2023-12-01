#pragma once

#include "NewLayerWindow.h"
#include "../Layers/LayerManager.h"

namespace FocalEngine
{
	class UIManager
	{
	public:
		SINGLETON_PUBLIC_PART(UIManager)

		void ShowTransformConfiguration(std::string Name, FETransformComponent* Transform);
		void ShowCameraTransform();

		void SetCamera(FEBasicCamera* NewCamera);

		void Render(bool bScreenshotMode = false);

		bool GetWireFrameMode();
		void SetWireFrameMode(bool NewValue);

		bool IsInDeveloperMode();
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

		float GetAmbientLightFactor();
		void SetAmbientLightFactor(float NewValue);

		SDF* GetDebugSDF();
		void UpdateRenderingMode(SDF* SDF, int NewRenderingMode);

		bool ShouldTakeScreenshot();
		void SetShouldTakeScreenshot(bool NewValue);

		bool ShouldUseTransparentBackground();
		void SetUseTransparentBackground(bool NewValue);
	private:
		SINGLETON_PRIVATE_PART(UIManager)

		FEBasicCamera* CurrentCamera = nullptr;

		bool bWireframeMode = false;
		float TimeTookToJitter = 0.0f;

		bool bDeveloperMode = false;
		bool bModelCamera = true;

		bool bShouldOpenProgressPopup = false;
		bool bShouldCloseProgressPopup = false;

		float RadiusOfAreaToMeasure = 1.0f;
		int LayerSelectionMode = 0;

		FEColorRangeAdjuster HeatMapColorRange;

		void RenderLegend(bool bScreenshotMode = false);
		void RenderLayerChooseWindow();

		FEWeightedHistogram Histogram;
		FEArrowScroller HistogramSelectRegionMin;
		FEArrowScroller HistogramSelectRegionMax;
		bool bHistogramSelectRegionMode = false;
		bool bHistogramPixelBins = false;
		void UpdateHistogramData(MeshLayer* FromLayer, int NewBinCount);
		void RenderHistogramWindow();
		
		static void OnJitterCalculationsStart();
		static void OnRugosityCalculationsStart();
		static void OnJitterCalculationsEnd(MeshLayer NewLayer);
		static void OnRugosityCalculationsEnd(MeshLayer NewLayer);
		static void OnVectorDispersionCalculationsEnd(MeshLayer NewLayer);
		static void OnFractalDimensionCalculationsEnd(MeshLayer NewLayer);

		bool bOutputSelectionToFile = false;

		bool bShouldOpenAboutWindow = false;
		void OpenAboutWindow();
		void RenderAboutWindow();

		static void OnLayerChange();

		float FindStandardDeviation(std::vector<float> DataPoints);

		FETexture* AddNewLayerIcon = nullptr;
		std::vector<std::string> DummyLayers;

		void GetUsableSpaceForLayerList(ImVec2& UsableSpaceStart, ImVec2& UsableSpaceEnd);
		ImVec2 GetLayerListButtonSize(std::string ButtonText);

		int TotalWidthNeededForLayerList(int ButtonUsed);

		void RenderSettingsWindow();
		void RenderLayerSettingsTab();
		void RenderGeneralSettingsTab();
		void RenderExportTab();

		float AmbientLightFactor = 2.2f;
		int CurrentJitterStepIndexVisualize = 0;
		SDF* DebugSDF = nullptr;
		void InitDebugSDF(size_t JitterIndex);

		bool bNextFrameForScreenshot = false;
		bool bUseTransparentBackground = false;

		bool MeshAndCurrentLayerIsValid();
	};

	#define UI UIManager::getInstance()
}