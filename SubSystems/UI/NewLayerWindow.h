#include "UIWeightedHistogram.h"

#include "../ComplexityCore/Layers/Producers/HeightLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/AreaLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/TriangleEdgeLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/CompareLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/VectorDispersionLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/FractalDimensionLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/TriangleCountLayerProducer.h"

class NewLayerWindow
{
	SINGLETON_PRIVATE_PART(NewLayerWindow)

	bool bShouldOpen = false;
	bool bShouldClose = false;
	bool bSmootherResult = false;
	bool bRunOnWholeModel = false;

	int Mode = 0;
	int FeaturesSizeSelectionMode = 0;
	std::vector<std::string> LayerTypesNames;

	int FirstChoosenLayerIndex = -1;
	int SecondChoosenLayerIndex = -1;

	void InternalClose();
	void AddLayer();

	void RenderCellSizeSettings();

	void RenderHeightLayerSettings();
	void RenderAreaLayerSettings();
	void RenderTrianglesEdgesLayerSettings();
	std::vector<std::string> TrianglesEdgesModeNames;
	int TrianglesEdgesMode = 0;
	void RenderTriangleDensityLayerSettings();

	void RenderRugosityLayerSettings();
	void RenderVectorDispersionSettings();
	void RenderFractalDimentionSettings();

	void RenderCompareLayerSettings();

	void RenderSettings();
	void OnModeChanged(int OldMode);
public:
	SINGLETON_PUBLIC_PART(NewLayerWindow)

	void Show();
	void Close();
	void Render();

	std::vector<std::string> GetNewLayerName();
};

#define NEW_LAYER_WINDOW NewLayerWindow::GetInstance()