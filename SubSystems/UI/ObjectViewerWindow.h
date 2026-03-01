#pragma once
#include "../AnalysisObjectManager.h"
#include "FESceneGraphUI.h"

class ObjectViewerWindow
{
	friend class UIInspector;
	SINGLETON_PRIVATE_PART(ObjectViewerWindow)

	bool bVisible = true;
	FESceneGraphUI* SceneGraphUI = nullptr;

	FETexture* VisibilityOnIcon = nullptr;
	FETexture* VisibilityOffIcon = nullptr;
	FETexture* TrashBinIcon = nullptr;
	FETexture* MeshIcon = nullptr;
	FETexture* PointCloudIcon = nullptr;
	FESceneGraphNodeWidget TrashWidget;
	FESceneGraphNodeWidget VisibilityToggleWidget;

	static bool ShouldRenderNode(FENaiveSceneGraphNode* SubTreeRoot);
	static std::string GetDisplayedName(FENaiveSceneGraphNode* SubTreeRoot);
	static bool ShouldShowChildren(FENaiveSceneGraphNode* SubTreeRoot);

	static FETexture* NodeIcon(FENaiveSceneGraphNode* Node);

	static void OnNodeClicked(FENaiveSceneGraphNode* Node, ImGuiMouseButton_ MouseButton);
	static void OnDoubleClickNode(FENaiveSceneGraphNode* Node, ImGuiMouseButton_ MouseButton);
	static void OnNodeSelectionChanged(FENaiveSceneGraphNode* Node, bool bOldState);
public:
	SINGLETON_PUBLIC_PART(ObjectViewerWindow)

	bool IsVisible() const;
	void SetVisible(bool NewValue);

	void Render();

	FEEntity* GetSelectedEntity();
};

#define OBJECT_VIEWER_WINDOW ObjectViewerWindow::GetInstance()