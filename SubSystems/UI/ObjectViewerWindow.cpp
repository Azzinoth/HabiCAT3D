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

std::string ObjectViewerWindow::ClipTextToWidth(const std::string& Text, float MaxWidth)
{
	std::string Result = Text;
	float TextWidth = ImGui::CalcTextSize(Text.c_str()).x;

	if (TextWidth > MaxWidth)
	{
		const char* EllipsisText = "...";
		float EllipsisWidth = ImGui::CalcTextSize(EllipsisText).x;

		while (!Result.empty() && ImGui::CalcTextSize((Result + EllipsisText).c_str()).x > MaxWidth)
			Result.pop_back();

		Result += EllipsisText;
	}

	return Result;
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
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Objects", nullptr))
	{
		ImVec2 CurrentWindowPosition = ImGui::GetWindowPos();
		ImVec2 CurrentWindowSize = ImGui::GetWindowSize();

		std::vector<std::string> ObjectIDs = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectsIDList();
		std::vector<AnalysisObject*> Objects;
		std::vector<FEPointCloud*> PointClouds;
		std::vector<std::string> PointCloudIDs = RESOURCE_MANAGER.GetPointCloudIDList();
		for (size_t i = 0; i < PointCloudIDs.size(); i++)
		{
			FEPointCloud* CurrentPointCloud = RESOURCE_MANAGER.GetPointCloud(PointCloudIDs[i]);
			if (CurrentPointCloud == nullptr)
				continue;

			PointClouds.push_back(CurrentPointCloud);
		}

		for (size_t i = 0; i < ObjectIDs.size(); i++)
		{
			AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObject(ObjectIDs[i]);
			if (CurrentObject == nullptr)
				continue;

			Objects.push_back(CurrentObject);
		}

		ImGui::BeginListBox("##Objects ListBox", ImVec2(ImGui::GetContentRegionAvail()));
		for (size_t i = 0; i < Objects.size(); i++)
		{
			AnalysisObject* CurrentObject = Objects[i];

			if (ImGui::GetIO().Fonts->Fonts.Size > 0)
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

			ImGui::Image(CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH ? MeshIcon->GetTextureID() : PointCloudIcon->GetTextureID(), ImVec2(32.0f, 32.0f));
			ImGui::SameLine();

			std::string TruncatedName = ClipTextToWidth(CurrentObject->GetName(), ImGui::GetContentRegionAvail().x - 80.0f - 4.0f);
			bool bIsSelected = false;
			if (ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject() != nullptr)
				bIsSelected = CurrentObject->GetID() == ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject()->GetID();
			if (ImGui::Selectable(TruncatedName.c_str(), bIsSelected, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x - 80, 32)))
			{
				ANALYSIS_OBJECT_MANAGER.SetActiveAnalysisObject(CurrentObject->GetID());
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				UI.bModelCamera ? UI.ModelCameraAdjustment(CurrentObject) : UI.FreeCameraAdjustment(CurrentObject);
			}

			float IconSize = ImGui::GetItemRectSize().y - 6.0f;

			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
			FETexture* IconToUse = CurrentObject->IsRenderedInScene() ? VisibilityOnIcon : VisibilityOffIcon;
			if (ImGui::ImageButton(("##VisibilityOnOff" + CurrentObject->GetID()).c_str(), IconToUse->GetTextureID(), ImVec2(IconSize, IconSize)))
				CurrentObject->SetRenderInScene(!CurrentObject->IsRenderedInScene());
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 6.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.4f, 0.4f, 0.3f));
			if (ImGui::ImageButton(("##DeleteObject" + CurrentObject->GetID()).c_str(), TrashBinIcon->GetTextureID(), ImVec2(IconSize, IconSize)))
			{
				ANALYSIS_OBJECT_MANAGER.DeleteAnalysisObject(CurrentObject->GetID());

				ImGui::PopStyleColor(3);
				if (ImGui::GetIO().Fonts->Fonts.Size > 0)
					ImGui::PopFont();

				ImGui::EndListBox();

				ImGui::End();
				ImGui::PopStyleVar(2);

				return;
			}
			
			ImGui::PopStyleColor(3);

			if (ImGui::GetIO().Fonts->Fonts.Size > 0)
				ImGui::PopFont();
		}

		ImGui::EndListBox();
	}

	ImGui::End();
	ImGui::PopStyleVar(2);
}