#pragma once
#include "../AnalysisObjectManager.h"
#include "../FESceneGraphUI/TreeView.h"
#include "HabiCATGraphBackend.h"

class ObjectViewerWindow
{
	friend class UIInspector;
	friend class UIManager;
	SINGLETON_PRIVATE_PART(ObjectViewerWindow)

	bool bVisible = true;
	SceneGraphUI::TreeView* SceneGraphUI = nullptr;
	HabiCATGraphBackend* GraphBackend = nullptr;

	FETexture* VisibilityOnIcon = nullptr;
	FETexture* VisibilityOffIcon = nullptr;
	FETexture* TrashBinIcon = nullptr;
	FETexture* MeshIcon = nullptr;
	FETexture* PointCloudIcon = nullptr;
	SceneGraphUI::NodeWidget TrashWidget;
	SceneGraphUI::NodeWidget VisibilityToggleWidget;

	static bool ShouldRenderNode(SceneGraphUI::NodeHandle SubTreeRoot);
	static bool ShouldShowChildren(SceneGraphUI::NodeHandle SubTreeRoot);

	static ImTextureID NodeIcon(SceneGraphUI::NodeHandle Node);

	static void OnNodeClicked(SceneGraphUI::NodeHandle Node, ImGuiMouseButton_ MouseButton);
	static void OnDoubleClickNode(SceneGraphUI::NodeHandle Node, ImGuiMouseButton_ MouseButton);
	static void OnNodeSelectionChanged(SceneGraphUI::NodeHandle Node, bool bOldState);
public:
	SINGLETON_PUBLIC_PART(ObjectViewerWindow)

	bool IsVisible() const;
	void SetVisible(bool NewValue);

	void Render();

	FEEntity* GetSelectedEntity();
	
	void SetNodeSelected(FENaiveSceneGraphNode* Node, bool bSelected);
	void ExpandToNode(FENaiveSceneGraphNode* Node);
};

#define OBJECT_VIEWER_WINDOW ObjectViewerWindow::GetInstance()