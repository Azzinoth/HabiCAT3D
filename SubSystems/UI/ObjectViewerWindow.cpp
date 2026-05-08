#include "ObjectViewerWindow.h"
#include "UIManager.h"
using namespace SceneGraphUI;

ObjectViewerWindow::ObjectViewerWindow()
{
	if (!APPLICATION.HasConsoleWindow())
	{
		VisibilityOnIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/visibility_on.png", "VisibilityOnIcon");
		VisibilityOffIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/visibility_off.png", "VisibilityOffIcon");
		TrashBinIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/trash_bin.png", "TrashBinIcon");

		MeshIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/mesh.png", "MeshIcon");
		PointCloudIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/point_cloud.png", "PointCloudIcon");

		GraphBackend = new HabiCATGraphBackend();
		SceneGraphUI = new TreeView(GraphBackend);
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

bool ObjectViewerWindow::ShouldRenderNode(SceneGraphUI::NodeHandle SubTreeRoot)
{
	FENaiveSceneGraphNode* CurrentNode = SubTreeRoot.As<FENaiveSceneGraphNode>();
	size_t Depth = CurrentNode->GetDepth();
	if (Depth == 0)
		return true;

	FEEntity* CurrentEntity = CurrentNode->GetEntity();
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

bool ObjectViewerWindow::ShouldShowChildren(SceneGraphUI::NodeHandle SubTreeRoot)
{
	FENaiveSceneGraphNode* CurrentNode = SubTreeRoot.As<FENaiveSceneGraphNode>();
	size_t Depth = CurrentNode->GetDepth();
	if (Depth == PHOTOGRAMMETRY_RESOURCES_DEPTH)
		return false;

	return true;
}

ImTextureID ObjectViewerWindow::NodeIcon(SceneGraphUI::NodeHandle Node)
{
	FENaiveSceneGraphNode* CurrentNode = Node.As<FENaiveSceneGraphNode>();
	FEEntity* CurrentEntity = CurrentNode->GetEntity();
	if (CurrentEntity == nullptr)
		return 0;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
	if (CurrentObject == nullptr)
		return 0;

	return CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH ? OBJECT_VIEWER_WINDOW.MeshIcon->GetTextureID() : OBJECT_VIEWER_WINDOW.PointCloudIcon->GetTextureID();
}

void ObjectViewerWindow::OnDoubleClickNode(SceneGraphUI::NodeHandle Node, ImGuiMouseButton_ MouseButton)
{
	if (MouseButton != ImGuiMouseButton_Left)
		return;

	FENaiveSceneGraphNode* CurrentNode = Node.As<FENaiveSceneGraphNode>();
	FEEntity* CurrentEntity = CurrentNode->GetEntity();
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

void ObjectViewerWindow::OnNodeClicked(SceneGraphUI::NodeHandle Node, ImGuiMouseButton_ MouseButton)
{
	if (MouseButton != ImGuiMouseButton_Left)
		return;

	FENaiveSceneGraphNode* CurrentNode = Node.As<FENaiveSceneGraphNode>();
	FEEntity* CurrentEntity = CurrentNode->GetEntity();
	if (CurrentEntity == nullptr)
		return;

	AnalysisObject* CurrentObject = GetAnalysisObjectFromNode(CurrentNode);
	if (CurrentObject != nullptr)
		ANALYSIS_OBJECT_MANAGER.SetActiveAnalysisObject(CurrentObject->GetID());
}

void ObjectViewerWindow::OnNodeSelectionChanged(SceneGraphUI::NodeHandle Node, bool bOldState)
{
	FENaiveSceneGraphNode* CurrentNode = Node.As<FENaiveSceneGraphNode>();
	FEEntity* CurrentEntity = CurrentNode->GetEntity();
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
		SceneGraphUI->SetNodeChildrenVisiblePredicate(ObjectViewerWindow::ShouldShowChildren);

		SceneGraphUI->SetNodeIconProvider(ObjectViewerWindow::NodeIcon);

		SceneGraphUI->AddOnNodeClickedCallback(ObjectViewerWindow::OnNodeClicked);
		SceneGraphUI->AddOnNodeDoubleClickedCallback(ObjectViewerWindow::OnDoubleClickNode);
		SceneGraphUI->AddOnNodeSelectionChangedCallback(ObjectViewerWindow::OnNodeSelectionChanged);
		
		TrashWidget.Icon = TrashBinIcon->GetTextureID();
		TrashWidget.bIsInteractive = true;
		TrashWidget.OnClickCallback = [](SceneGraphUI::NodeHandle Node) {
			FENaiveSceneGraphNode* CurrentNode = Node.As<FENaiveSceneGraphNode>();
			FEEntity* CurrentEntity = CurrentNode->GetEntity();
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

		
		VisibilityToggleWidget.Icon = VisibilityOnIcon->GetTextureID();
		VisibilityToggleWidget.DynamicIconProvider = [this](SceneGraphUI::NodeHandle Node) -> ImTextureID {
			FENaiveSceneGraphNode* CurrentNode = Node.As<FENaiveSceneGraphNode>();
			FEEntity* CurrentEntity = CurrentNode->GetEntity();
			if (CurrentEntity == nullptr)
				return 0;

			bool bIsVisible = CurrentEntity->IsVisible();
			return bIsVisible ? VisibilityOnIcon->GetTextureID() : VisibilityOffIcon->GetTextureID();
		};

		VisibilityToggleWidget.bIsInteractive = true;
		VisibilityToggleWidget.OnClickCallback = [](SceneGraphUI::NodeHandle Node) {
			FENaiveSceneGraphNode* CurrentNode = Node.As<FENaiveSceneGraphNode>();
			FEEntity* CurrentEntity = CurrentNode->GetEntity();
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

		GraphBackend->SetSceneID(MAIN_SCENE_MANAGER.GetMainScene()->GetObjectID());
		SceneGraphUI->Render(SceneGraphUI::NodeHandle(MAIN_SCENE_MANAGER.GetMainScene()->SceneGraph.GetRoot(), GraphBackend), false);
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

void ObjectViewerWindow::SetNodeSelected(FENaiveSceneGraphNode* Node, bool bSelected)
{
	SceneGraphUI->SetNodeSelected(SceneGraphUI::NodeHandle(Node, GraphBackend), bSelected);
}

void ObjectViewerWindow::ExpandToNode(FENaiveSceneGraphNode* Node)
{
	SceneGraphUI->ExpandToNode(SceneGraphUI::NodeHandle(Node, GraphBackend));
}