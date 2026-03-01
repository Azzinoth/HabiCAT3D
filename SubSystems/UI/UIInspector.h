#pragma once

#include "UICore.h"
#include "../LayerRasterizationManager.h"
#include "NewLayerWindow.h"
#include "ObjectViewerWindow.h"
#include "LoadPhotogrammetryWindow.h"
#include "../ComplexityCore/Layers/LayerManager.h"

class UIInspector
{
	friend class UIManager;
public:
	SINGLETON_PUBLIC_PART(UIInspector)

	void Render(bool bScreenshotMode = false);
	glm::dvec2 CalculateWeightDistributionAtValue(DataLayer* Layer, float Value);

private:
	SINGLETON_PRIVATE_PART(UIInspector)

	bool bVisible = true;

	static void OnLayerChange();

	void RenderSelectedObjectTab();

	char CurrentDistributionEdit[1024];
	glm::vec2 CurrentDistribution = glm::vec2();
	void RenderLayerTab();

	static void OnSelectedImageChangedCallback(COLMAPProject* Project, int ImageID);
};

#define UI_INSPECTOR UIInspector::GetInstance()