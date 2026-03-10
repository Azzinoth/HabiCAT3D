#pragma once

#include "UICore.h"
#include "../LayerRasterizationManager.h"
#include "NewLayerWindow.h"
#include "ObjectViewerWindow.h"
#include "LoadPhotogrammetryWindow.h"
#include "../ComplexityCore/Layers/LayerManager.h"
#include "../SubSystems/AnnotationManager.h"

const COMDLG_FILTERSPEC MODEL_EXPORT_FILE_FILTER[] =
{
	{ L"3D Model file (*.obj)", L"*.obj" }
};

class UIInspector
{
	friend class UIManager;
public:
	SINGLETON_PUBLIC_PART(UIInspector)

	void Render(bool bScreenshotMode = false);
	glm::dvec2 CalculateWeightDistributionAtValue(DataLayer* Layer, float Value);

	void UpdateMeshSelectedTrianglesRendering();

	// Geometry selection.
	float GetRadiusOfAreaToSelect();
	void SetRadiusOfAreaToSelect(float NewValue);

	int GetMeshSelectionMode();
	void SetMeshSelectionMode(int NewValue);

	// Export stuff.
	bool ExportOBJ(std::string FilePath, int LayerIndex);

	bool ShouldTakeScreenshot();
	void SetShouldTakeScreenshot(bool NewValue);

	bool ShouldUseTransparentBackground();
	void SetUseTransparentBackground(bool NewValue);
private:
	SINGLETON_PRIVATE_PART(UIInspector)

	bool bVisible = true;

	static void OnLayerChange();

	void RenderSelectedObjectTab();

	char CurrentDistributionEdit[1024];
	glm::vec2 CurrentDistribution = glm::vec2();
	void RenderLayerTab();

	bool bNextFrameForScreenshot = false;
	bool bUseTransparentBackground = false;
	void RenderExportTab();

	static void OnSelectedImageChangedCallback(COLMAPProject* Project, int ImageID);

	static void MouseButtonCallback(int Button, int Action, int Mods);

	// Geometry selection.
	FEEntity* SelectionLinesEntity = nullptr;
	float RadiusOfAreaToSelect = 1.0f;
	int MeshSelectionMode = 0;

	void CleanUpSelectionLinesComponent();
	static void OnObjectLoad(AnalysisObject* NewObject);
	static void OnActiveObjectChange(AnalysisObject* NewActiveObject);

	// Export stuff.
	void OutputSelectedAreaInfoToFile();

	void RasterizationSettingsUI();
};

#define UI_INSPECTOR UIInspector::GetInstance()