#include "ObjectViewerWindow.h"

ObjectViewerWindow::ObjectViewerWindow() {}

bool ObjectViewerWindow::IsVisible() const
{
	return bVisible;
}

void ObjectViewerWindow::SetVisible(bool NewValue)
{
	bVisible = NewValue;
}

void ObjectViewerWindow::Render()
{
	if (!bVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(ImVec2(2, 20));
	ImGui::SetNextWindowSize(ImVec2(250, 670));
	ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

	std::vector<std::string> ObjectIDs = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectsIDList();
	std::vector<AnalysisObject*> Objects;
	for (size_t i = 0; i < ObjectIDs.size(); i++)
	{
		AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObject(ObjectIDs[i]);
		if (CurrentObject == nullptr)
			continue;

		Objects.push_back(CurrentObject);
	}

	// Render list of objects
	ImGui::BeginListBox("##Objects ListBox", ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 500.0f));

	for (size_t i = 0; i < Objects.size(); i++)
	{
		AnalysisObject* CurrentObject = Objects[i];

		if (ImGui::Selectable(CurrentObject->GetName().c_str(), false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x - 0, 64)))
		{
			ANALYSIS_OBJECT_MANAGER.SetActiveAnalysisObject(CurrentObject->GetID());
		}

		//ImVec2 PostionBeforeDraw = ImGui::GetCursorPos();

		//ImVec2 TextSize = ImGui::CalcTextSize(Layer->GetName().c_str());
		//ImGui::SetCursorPos(PostionBeforeDraw + ImVec2(ImGui::GetContentRegionAvail().x / 2.0f - TextSize.x / 2.0f, 16));

		//if (TerrainLayerRenameIndex == i)
		//{
		//	if (!bLastFrameTerrainLayerRenameEditWasVisible)
		//	{
		//		ImGui::SetKeyboardFocusHere(0);
		//		ImGui::SetFocusID(ImGui::GetID("##newNameTerrainLayerEditor"), FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl());
		//		ImGui::SetItemDefaultFocus();
		//		bLastFrameTerrainLayerRenameEditWasVisible = true;
		//	}

		//	ImGui::SetNextItemWidth(350.0f);
		//	ImGui::SetCursorPos(ImVec2(PostionBeforeDraw.x + 64.0f + (ImGui::GetContentRegionAvail().x - 64.0f) / 2.0f - 350.0f / 2.0f, PostionBeforeDraw.y + 12));
		//	if (ImGui::InputText("##newNameTerrainLayerEditor", TerrainLayerRename, IM_ARRAYSIZE(TerrainLayerRename), ImGuiInputTextFlags_EnterReturnsTrue) ||
		//		ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::IsItemFocused()/*FE_IMGUI_WINDOW_MANAGER.GetCurrentFocusID() != ImGui::GetID("##newNameTerrainLayerEditor")*/)
		//	{
		//		PROJECT_MANAGER.GetCurrent()->SetModified(true);
		//		Layer->SetName(TerrainLayerRename);

		//		TerrainLayerRenameIndex = -1;
		//	}
		//}
		//else
		//{
		//	ImGui::Text(Layer->GetName().c_str());
		//}
		//ImGui::SetCursorPos(PostionBeforeDraw);

		//ImGui::PushID(int(i));
		//if (ImGui::Selectable("##item", SelectedLayer == i ? true : false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x - 0, 64)))
		//{
		//	SelectedLayer = int(i);
		//	TERRAIN_SYSTEM.SetBrushLayerIndex(SelectedLayer);
		//}
		//TerrainChangeLayerMaterialTargets[i]->StickToItem();
		//ImGui::PopID();

		//if (ImGui::IsItemHovered())
		//	HoveredTerrainLayerItem = int(i);

		//ImGui::SetCursorPos(PostionBeforeDraw);
		//ImColor ImageTint = ImGui::IsItemHovered() ? ImColor(1.0f, 1.0f, 1.0f, 0.5f) : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
		//FETexture* PreviewTexture = PREVIEW_MANAGER.GetMaterialPreview(Layer->GetMaterial()->GetObjectID());
		//ImGui::Image(PreviewTexture->GetTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}

	ImGui::EndListBox();

	ImGui::End();
	ImGui::PopStyleVar(2);

}