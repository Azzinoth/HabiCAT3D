#include "UIWeightedHistogram.h"

#include "../ComplexityCore/Layers/Producers/HeightLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/AreaLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/TriangleEdgeLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/CompareLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/VectorDispersionLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/FractalDimensionLayerProducer.h"
#include "../ComplexityCore/Layers/Producers/TriangleCountLayerProducer.h"

#include "../ComplexityCore/Layers/Producers/PointDensityLayerProducer.h"

class NewLayerWindow
{
	SINGLETON_PRIVATE_PART(NewLayerWindow)

	bool bShouldOpen = false;
	bool bShouldClose = false;
	bool bSmootherResult = false;
	bool bRunOnWholeModel = false;

	std::vector<DATA_SOURCE_TYPE> AvailableDataSources;
	std::vector<LAYER_TYPE> AvailableLayerTypes;
	std::unordered_map<LAYER_TYPE, std::string> LayerTypeToName;
	void CheckAvailableDataSources();

	DATA_SOURCE_TYPE CurrentDataSource = DATA_SOURCE_TYPE::MESH;
	LAYER_TYPE SelectedLayerType = LAYER_TYPE::UNKNOWN;
	
	int FeaturesSizeSelectionMode = 0;
	int FirstChoosenLayerIndex = -1;
	int SecondChoosenLayerIndex = -1;

	void InternalClose();
	void AddLayer();

	void RenderCellSizeSettings();

	void RenderNoSettingsAvailable();
	void RenderTrianglesEdgesLayerSettings();
	std::vector<std::string> TrianglesEdgesModeNames;
	int TrianglesEdgesMode = 0;
	void RenderTriangleDensityLayerSettings();

	void RenderRugosityLayerSettings();
	void RenderVectorDispersionSettings();
	void RenderFractalDimentionSettings();

	void RenderCompareLayerSettings();

	void RenderPointDensitySettings();

	void RenderSettings();
	void OnLayerTypeChanged(LAYER_TYPE OldLayerType);
public:
	SINGLETON_PUBLIC_PART(NewLayerWindow)

	void Show();
	void Close();
	void Render();
};

#define NEW_LAYER_WINDOW NewLayerWindow::GetInstance()