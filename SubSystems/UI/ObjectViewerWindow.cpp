#include "ObjectViewerWindow.h"
#include "UIManager.h"

ObjectViewerWindow::ObjectViewerWindow()
{
	if (!APPLICATION.HasConsoleWindow())
	{
		VisibilityOnIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/visibility_on.png", "VisibilityOnIcon");
		VisibilityOffIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/visibility_off.png", "VisibilityOffIcon");
		TrashBinIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/trash_bin.png", "TrashBinIcon");

		MeshIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/mesh.png", "MeshIcon");
		PointCloudIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/point_cloud.png", "PointCloudIcon");

		SceneGraphUI = new FESceneGraphUI();
	}
}

bool ObjectViewerWindow::IsVisible() const
{
	return bVisible;
}

void ObjectViewerWindow::SetVisible(bool NewValue)
{
	bVisible = NewValue;
}

#define ANALISYS_OBJECTS_DEPTH 1
#define PHOTOGRAMMETRY_ANCHOR_DEPTH (ANALISYS_OBJECTS_DEPTH + 1)
#define PHOTOGRAMMETRY_RESOURCES_DEPTH (PHOTOGRAMMETRY_ANCHOR_DEPTH + 1)

bool ObjectViewerWindow::ShouldRenderNode(FENaiveSceneGraphNode* SubTreeRoot)
{
	size_t Depth = SubTreeRoot->GetDepth();
	if (Depth == 0)
		return true;

	FEEntity* CurrentEntity = SubTreeRoot->GetEntity();
	if (CurrentEntity == nullptr)
		return false;

	std::string CurrentEntityName = CurrentEntity->GetName();
	if (CurrentEntityName.find("COLMAPPhysicalCamera_") != std::string::npos)
		return false;

	if (Depth == ANALISYS_OBJECTS_DEPTH)
	{
		AnalysisObject* CurrentAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
		if (CurrentAnalysisObject == nullptr)
			return false;
	}

	if (Depth == ANALISYS_OBJECTS_DEPTH + 1)
	{
		if (CurrentEntityName.find("PhotogrammetryAnchor_") == std::string::npos && CurrentEntityName.find("AnnotationEntity_") == std::string::npos)
			return false;
	}

	if (Depth == PHOTOGRAMMETRY_RESOURCES_DEPTH)
	{
		if (CurrentEntityName.find("COLMAPImages") == std::string::npos &&
			CurrentEntityName.find("Tie Points") == std::string::npos)
			return false;
	}

	return true;
}

std::string ObjectViewerWindow::GetDisplayedName(FENaiveSceneGraphNode* SubTreeRoot)
{
	size_t Depth = SubTreeRoot->GetDepth();

	std::string DisplayedName = SubTreeRoot->GetName();
	FEEntity* CurrentEntity = SubTreeRoot->GetEntity();
	std::string CurrentEntityName = CurrentEntity->GetName();
	if (Depth == ANALISYS_OBJECTS_DEPTH)
	{
		AnalysisObject* CurrentAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
		if (CurrentAnalysisObject != nullptr)
			DisplayedName = CurrentAnalysisObject->GetName();
	}
	if (Depth == ANALISYS_OBJECTS_DEPTH + 1)
	{
		if (CurrentEntityName.find("Annotation") != std::string::npos)
		{
			DisplayedName = "Annotations";
		}
		else 
		{
			DisplayedName = "Photogrammetry";
		}
	}
	if (Depth == PHOTOGRAMMETRY_RESOURCES_DEPTH)
	{
		if (CurrentEntityName.find("COLMAPImages") != std::string::npos)
		{
			DisplayedName = "Images";
		}
		else if (CurrentEntityName.find("Tie Points") != std::string::npos)
		{
			DisplayedName = "Tie Points";
		}
	}

	return DisplayedName;
}

bool ObjectViewerWindow::ShouldShowChildren(FENaiveSceneGraphNode* SubTreeRoot)
{
	size_t Depth = SubTreeRoot->GetDepth();
	if (Depth == PHOTOGRAMMETRY_RESOURCES_DEPTH)
		return false;

	return true;
}

FETexture* ObjectViewerWindow::NodeIcon(FENaiveSceneGraphNode* Node)
{
	FEEntity* CurrentEntity = Node->GetEntity();
	if (CurrentEntity == nullptr)
		return nullptr;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
	if (CurrentObject == nullptr)
		return nullptr;

	return CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH ? OBJECT_VIEWER_WINDOW.MeshIcon : OBJECT_VIEWER_WINDOW.PointCloudIcon;
}

void ObjectViewerWindow::OnDoubleClickNode(FENaiveSceneGraphNode* Node, ImGuiMouseButton_ MouseButton)
{
	if (MouseButton != ImGuiMouseButton_Left)
		return;

	FEEntity* CurrentEntity = Node->GetEntity();
	if (CurrentEntity == nullptr)
		return;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
	if (CurrentObject == nullptr)
		return;

	SETTINGS_WINDOW.FocusCameraOnObject(CurrentObject);
}

AnalysisObject* GetAnalysisObjectFromNode(FENaiveSceneGraphNode* Node)
{
	FEEntity* CurrentEntity = Node->GetEntity();
	if (CurrentEntity == nullptr)
		return nullptr;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
	if (CurrentObject != nullptr)
		return CurrentObject;

	// If we are here, we need to go to parent nodes to find the corresponding analysis object.
	FENaiveSceneGraphNode* CurrentNode = Node->GetParent();
	while (CurrentNode != nullptr)
	{
		CurrentEntity = CurrentNode->GetEntity();
		if (CurrentEntity != nullptr)
		{
			CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
			if (CurrentObject != nullptr)
				return CurrentObject;
		}

		CurrentNode = CurrentNode->GetParent();
	}

	return CurrentObject;
}

void ObjectViewerWindow::OnNodeClicked(FENaiveSceneGraphNode* Node, ImGuiMouseButton_ MouseButton)
{
	if (MouseButton != ImGuiMouseButton_Left)
		return;

	FEEntity* CurrentEntity = Node->GetEntity();
	if (CurrentEntity == nullptr)
		return;

	AnalysisObject* CurrentObject = GetAnalysisObjectFromNode(Node);
	if (CurrentObject != nullptr)
		ANALYSIS_OBJECT_MANAGER.SetActiveAnalysisObject(CurrentObject->GetID());
}

void ObjectViewerWindow::OnNodeSelectionChanged(FENaiveSceneGraphNode* Node, bool bOldState)
{
	FEEntity* CurrentEntity = Node->GetEntity();
	if (CurrentEntity == nullptr)
		return;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
	if (CurrentObject == nullptr)
	{

	}
	else
	{
		ANALYSIS_OBJECT_MANAGER.SetActiveAnalysisObject(CurrentObject->GetID());
	}
}

void ObjectViewerWindow::Render()
{
	if (!bVisible)
		return;

	static bool bFirstTime = true;
	if (bFirstTime)
	{
		ImGui::SetNextWindowPos(ImVec2(155, 110), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);

		bFirstTime = false;

		SceneGraphUI->SetNodeRenderPredicate(ObjectViewerWindow::ShouldRenderNode);
		SceneGraphUI->SetNodeDisplayNameProvider(ObjectViewerWindow::GetDisplayedName);
		SceneGraphUI->SetNodeChildrenVisiblePredicate(ObjectViewerWindow::ShouldShowChildren);

		SceneGraphUI->SetNodeIconProvider(ObjectViewerWindow::NodeIcon);

		SceneGraphUI->AddOnNodeClickedCallback(ObjectViewerWindow::OnNodeClicked);
		SceneGraphUI->AddOnNodeDoubleClickedCallback(ObjectViewerWindow::OnDoubleClickNode);
		SceneGraphUI->AddOnNodeSelectionChangedCallback(ObjectViewerWindow::OnNodeSelectionChanged);
		
		TrashWidget.Icon = TrashBinIcon;
		TrashWidget.bIsInteractive = true;
		TrashWidget.OnClickCallback = [](FENaiveSceneGraphNode* Node) {
			FEEntity* CurrentEntity = Node->GetEntity();
			if (CurrentEntity == nullptr)
				return;

			AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
			if (CurrentObject != nullptr)
			{
				ANALYSIS_OBJECT_MANAGER.DeleteAnalysisObject(CurrentObject->GetID());
			}

			AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByEntityID(CurrentEntity->GetObjectID());
			if (CurrentAnnotationData != nullptr)
			{
				AnalysisObject* AnnotatedObject = CurrentAnnotationData->GetAnalysisObject();
				ANNOTATION_MANAGER.RemoveAnnotationFromAnalysisObject(AnnotatedObject->GetID());
			}
		};
		TrashWidget.bIsVisibleByDefault = true;
		TrashWidget.TooltipText = "Delete";

		SceneGraphUI->AddNodeWidget(TrashWidget);

		
		VisibilityToggleWidget.Icon = VisibilityOnIcon;
		VisibilityToggleWidget.DynamicIconProvider = [this](FENaiveSceneGraphNode* Node) -> FETexture* {
			FEEntity* CurrentEntity = Node->GetEntity();
			if (CurrentEntity == nullptr)
				return nullptr;

			bool bIsVisible = CurrentEntity->IsVisible();
			return bIsVisible ? VisibilityOnIcon : VisibilityOffIcon;
		};

		VisibilityToggleWidget.bIsInteractive = true;
		VisibilityToggleWidget.OnClickCallback = [](FENaiveSceneGraphNode* Node) {
			FEEntity* CurrentEntity = Node->GetEntity();
			if (CurrentEntity == nullptr)
				return;

			bool bIsVisible = CurrentEntity->IsVisible();
			CurrentEntity->SetVisible(!bIsVisible);
		};
		VisibilityToggleWidget.bIsVisibleByDefault = true;
		VisibilityToggleWidget.TooltipText = "Show/Hide";

		SceneGraphUI->AddNodeWidget(VisibilityToggleWidget);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Objects", nullptr))
	{
		ImVec2 CurrentWindowPosition = ImGui::GetWindowPos();
		ImVec2 CurrentWindowSize = ImGui::GetWindowSize();

		static bool bSceneGraphDebugMode = false;
		if (ImGui::Checkbox("Scene Graph Debug Mode", &bSceneGraphDebugMode))
			SceneGraphUI->SetDebugMode(bSceneGraphDebugMode);

		SceneGraphUI->Render(MAIN_SCENE_MANAGER.GetMainScene()->SceneGraph.GetRoot(), false);
	}

	ImGui::End();
	ImGui::PopStyleVar(2);
}

FEEntity* ObjectViewerWindow::GetSelectedEntity()
{
	std::vector<std::string> SelectedNodes = SceneGraphUI->GetSelectedNodeIDs();
	// Although SceneGraphUI support multiple selection, we use default single selection mode.
	if (SelectedNodes.empty())
		return nullptr;

	FENaiveSceneGraphNode* SelectedNode = MAIN_SCENE_MANAGER.GetMainScene()->SceneGraph.GetNodeByID(SelectedNodes[0]);
	if (SelectedNode == nullptr)
		return nullptr;

	return SelectedNode->GetEntity();
}