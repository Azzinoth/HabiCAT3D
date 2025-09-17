#pragma once
#include "../AnalysisObjectManager.h"

class ObjectViewerWindow
{
	SINGLETON_PRIVATE_PART(ObjectViewerWindow)

	bool bVisible = true;

	FETexture* VisibilityOnIcon = nullptr;
	FETexture* VisibilityOffIcon = nullptr;
	FETexture* TrashBinIcon = nullptr;
	FETexture* MeshIcon = nullptr;
	FETexture* PointCloudIcon = nullptr;

	std::string ClipTextToWidth(const std::string& Text, float MaxWidth);
public:
	SINGLETON_PUBLIC_PART(ObjectViewerWindow)

	bool IsVisible() const;
	void SetVisible(bool NewValue);

	void Render();
};

#define OBJECT_VIEWER_WINDOW ObjectViewerWindow::GetInstance()