#include "UIComponents.h"
#include "../Layers/CompareLayerProducer.h"

class NewLayerWindow
{
	SINGLETON_PRIVATE_PART(NewLayerWindow)

	bool bShouldOpen = false;
	bool bShouldClose = false;

	int Mode = 0;
	std::vector<std::string> LayerTypesNames;

	void InternalClose();
	void AddLayer();

	void RenderHeightLayerSettings();
	void RenderRugosityLayerSettings();
	void RenderCompareLayerSettings();
	int FirstChoosenLayerIndex = -1;
	int SecondChoosenLayerIndex = -1;

	void RenderSettings();
public:
	SINGLETON_PUBLIC_PART(NewLayerWindow)

	void Show();
	void Close();
	void Render();

	std::vector<std::string> GetNewLayerName();
};

#define NEW_LAYER_WINDOW NewLayerWindow::getInstance()