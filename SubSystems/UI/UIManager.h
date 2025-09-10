#pragma once

#include "NewLayerWindow.h"
#include "../ComplexityCore/Layers/LayerManager.h"
#include "../LayerRasterizationManager.h"

const COMDLG_FILTERSPEC MODEL_EXPORT_FILE_FILTER[] =
{
	{ L"3D Model file (*.obj)", L"*.obj" }
};


class UIManager
{
public:
	SINGLETON_PUBLIC_PART(UIManager)

	void ShowTransformConfiguration(std::string Name, FETransformComponent* Transform);
	void ShowCameraTransform();

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

	void SetIsModelCamera(bool NewValue, glm::vec3 ModelCameraFocusPoint = glm::vec3(0.0f));

	//static void(*SwapCameraImpl)(bool);
	void SwapCamera(bool bModelCamera, glm::vec3 ModelCameraFocusPoint = glm::vec3(0.0f));

	bool GetOutputSelectionToFile();
	void SetOutputSelectionToFile(bool NewValue);

	void ApplyStandardWindowsSizeAndPosition();

	glm::dvec2 LayerValuesAreaDistribution(DataLayer* Layer, float Value);

	float GetAmbientLightFactor();
	void SetAmbientLightFactor(float NewValue);

	MeasurementGrid* GetDebugGrid();
	void UpdateRenderingMode(MeasurementGrid* Grid, int NewRenderingMode);

	bool ShouldTakeScreenshot();
	void SetShouldTakeScreenshot(bool NewValue);

	bool ShouldUseTransparentBackground();
	void SetUseTransparentBackground(bool NewValue);

	bool IsProgressModalPopupOpen();

	bool ExportOBJ(std::string FilePath, int LayerIndex);
private:
	SINGLETON_PRIVATE_PART(UIManager)

		bool bPreviousFrameWindowWasNull = true;

	bool bWireframeMode = false;
	float TimeTookToJitter = 0.0f;

	bool bDeveloperMode = false;
	bool bModelCamera = true;
	//float ModelCameraMouseWheelSensitivity = 0.05f;
	bool bChooseCameraFocusPointMode = false;

	bool bShouldOpenProgressPopup = false;
	bool bShouldCloseProgressPopup = true;

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
	void UpdateHistogramData(DataLayer* FromLayer, int NewBinCount);
	void RenderHistogramWindow();

	bool bJitterCalculationsInProgress = false;
	static void OnJitterCalculationsStart();
	static void OnJitterCalculationsEnd(DataLayer NewLayer);

	bool bOutputSelectionToFile = false;

	bool bShouldOpenAboutWindow = false;
	void OpenAboutWindow();
	void RenderAboutWindow();

	static void OnLayerChange();

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
	MeasurementGrid* DebugGrid = nullptr;
	void InitDebugGrid(size_t JitterIndex);

	bool bNextFrameForScreenshot = false;
	bool bUseTransparentBackground = false;

	bool MeshAndCurrentLayerIsValid();

	float ProgressModalPopupCurrentValue = 0.0f;
	void UpdateProgressModalPopupCurrentValue();

	bool bLayerRasterizationCalculationsInProgress = false;
	static void OnLayerRasterizationCalculationsStart();
	static void OnLayerRasterizationCalculationsEnd();
	void RasterizationSettingsUI();
};

#define UI UIManager::GetInstance()