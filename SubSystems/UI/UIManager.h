#pragma once

#include "SettingsWindow.h"
#include "UIInspector.h"

class UIManager
{
	friend class ObjectViewerWindow;
public:
	SINGLETON_PUBLIC_PART(UIManager)

	void Render();

	static void OnNewObjectLoaded(AnalysisObject* NewObject);

	bool GetOutputSelectionToFile();
	void SetOutputSelectionToFile(bool NewValue);

	bool IsApplyStandardLayoutOnResize() const;
	void SetApplyStandardLayoutOnResize(bool NewValue);
	void ApplyStandardWindowsSizeAndPosition();

	bool IsProgressModalPopupOpen();

	FEWeightedHistogram* GetHistogramPointer();
private:
	SINGLETON_PRIVATE_PART(UIManager)

	bool bPreviousFrameWindowWasNull = true;
	float TimeTookToJitter = 0.0f;

	bool bApplyStandardLayoutOnResize = true;

	std::string NoDataText = "No Data.(Drag & Drop model or point cloud)";

	bool bShouldOpenProgressPopup = false;
	bool bShouldCloseProgressPopup = true;

	FEColorRangeAdjuster HeatMapColorRange;

	void RenderLegend();
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
	static void OnAnnotationColorChanged(AnnotationData* ChangedAnnotationData, int AnnotationID);

	FETexture* AddNewLayerIcon = nullptr;
	std::vector<std::string> DummyLayers;

	void GetUsableSpaceForLayerList(ImVec2& UsableSpaceStart, ImVec2& UsableSpaceEnd);
	ImVec2 GetLayerListButtonSize(std::string ButtonText);

	int TotalWidthNeededForLayerList(int ButtonUsed);

	bool IsActiveObjectAndLayerValid();

	float ProgressModalPopupCurrentValue = 0.0f;
	void UpdateProgressModalPopupCurrentValue();

	bool bLayerRasterizationCalculationsInProgress = false;
	static void OnLayerRasterizationCalculationsStart();
	static void OnLayerRasterizationCalculationsEnd();

	static void WindowResizeCallback(int Width, int Height);
};

#define UI UIManager::GetInstance()