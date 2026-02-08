#pragma once

#include "NewLayerWindow.h"
#include "ObjectViewerWindow.h"
#include "../ComplexityCore/Layers/LayerManager.h"
#include "../LayerRasterizationManager.h"

const COMDLG_FILTERSPEC MODEL_EXPORT_FILE_FILTER[] =
{
	{ L"3D Model file (*.obj)", L"*.obj" }
};

class UIManager
{
	friend class ObjectViewerWindow;
public:
	SINGLETON_PUBLIC_PART(UIManager)

	std::string GetHabiCAT3DVersion();        // "1.0.0"
	std::string GetHabiCAT3DBuildInfo();      // "build 231+52 (dev, ed4c7ce-dirty)"
	std::string GetHabiCAT3DFullVersion();    // "HabiCAT3D 1.0.0 build 231+52 (dev, ed4c7ce-dirty)"
	std::string GetHabiCAT3DBuildTimestamp(); // "20260207232613"
	int GetHabiCAT3DBuildNumber();            // 231

	void ShowTransformConfiguration(std::string Name, FETransformComponent* Transform);
	void ShowCameraTransform();

	void Render(bool bScreenshotMode = false);

	bool GetWireFrameMode();
	void SetWireFrameMode(bool NewValue);

	bool IsInDeveloperMode();
	void SetDeveloperMode(bool NewValue);

	std::string CameraPositionToString();
	void StringToCameraPosition(std::string Text);

	std::string CameraRotationToString();
	void StringToCameraRotation(std::string Text);

	static void OnNewObjectLoaded(AnalysisObject* NewObject);

	float GetRadiusOfAreaToMeasure();
	void SetRadiusOfAreaToMeasure(float NewValue);

	int GetLayerSelectionMode();
	void SetLayerSelectionMode(int NewValue);

	void SetIsModelCamera(bool NewValue, glm::vec3 ModelCameraFocusPoint = glm::vec3(0.0f));
	void SwitchCameraMode(bool bModelCamera, glm::vec3 ModelCameraFocusPoint = glm::vec3(0.0f));

	bool GetOutputSelectionToFile();
	void SetOutputSelectionToFile(bool NewValue);

	bool IsApplyStandardLayoutOnResize() const;
	void SetApplyStandardLayoutOnResize(bool NewValue);
	void ApplyStandardWindowsSizeAndPosition();

	glm::dvec2 CalculateWeightDistributionAtValue(DataLayer* Layer, float Value);

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

	void UpdateMeshSelectedTrianglesRendering();

	void AddOnDebugGridSelectedCellChangedCallback(std::function<void(glm::vec3 SelectedCellIndex)> Callback);

	FEWeightedHistogram* GetHistogramPointer();
private:
	SINGLETON_PRIVATE_PART(UIManager)

	bool bPreviousFrameWindowWasNull = true;

	bool bWireframeMode = false;
	float TimeTookToJitter = 0.0f;

	bool bDeveloperMode = false;
	bool bModelCamera = true;
	//float ModelCameraMouseWheelSensitivity = 0.05f;
	bool bChooseCameraFocusPointMode = false;
	FEEntity* SelectionLinesEntity = nullptr;
	void CleanUpSelectionLinesComponent();

	bool bApplyStandardLayoutOnResize = true;

	std::string NoDataText = "No Data.(Drag & Drop model or point cloud)";

	bool bShouldOpenProgressPopup = false;
	bool bShouldCloseProgressPopup = true;

	char CurrentDistributionEdit[1024];
	glm::vec2 CurrentDistribution = glm::vec2();

	float RadiusOfAreaToMeasure = 1.0f;
	int LayerSelectionMode = 0;

	FEColorRangeAdjuster HeatMapColorRange;

	void RenderLegend(bool bScreenshotMode = false);
	void RenderLayerTabs();

	FEWeightedHistogram Histogram;
	FEArrowScroller HistogramSelectRegionMin;
	FEArrowScroller HistogramSelectRegionMax;
	bool bHistogramSelectRegionMode = false;
	bool bHistogramPixelBins = false;
	void UpdateHistogramData(DataLayer* FromLayer, int NewBinCount);
	void UpdateHistogramData(DataLayer* FirstLayer, DataLayer* SecondLayer, int NewBinCount);
	void RenderHistogramWindow();

	bool bJitterCalculationsInProgress = false;
	static void OnJitterCalculationsStart();
	static void OnJitterCalculationsEnd(DataLayer* NewLayer);

	bool bOutputSelectionToFile = false;

	bool bShouldOpenAboutWindow = false;
	void ShowAboutDialog();
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

	bool IsActiveObjectAndLayerValid();

	float ProgressModalPopupCurrentValue = 0.0f;
	void UpdateProgressModalPopupCurrentValue();

	bool bLayerRasterizationCalculationsInProgress = false;
	static void OnLayerRasterizationCalculationsStart();
	static void OnLayerRasterizationCalculationsEnd();
	void RasterizationSettingsUI();

	void AdjustCameraNearFarPlanes();
	void ModelCameraAdjustment(AnalysisObject* Object = nullptr);
	void FreeCameraAdjustment(AnalysisObject* Object = nullptr);

	void RenderLayerDebugInfo(MeasurementGrid* Grid);
	static void OnDebugGridSelectedCellChanged(glm::vec3 NewSelectedCell);
	std::vector<std::function<void(glm::vec3 SelectedCellIndex)>> ClientOnDebugGridSelectedCellChangedCallbacks;

	static void WindowResizeCallback(int Width, int Height);
};

#define UI UIManager::GetInstance()