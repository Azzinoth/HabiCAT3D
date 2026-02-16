#include "LoadPhotogrammetryWindow.h"

LoadPhotogrammetryWindow::LoadPhotogrammetryWindow()
{
	
};

LoadPhotogrammetryWindow::~LoadPhotogrammetryWindow() {};

void LoadPhotogrammetryWindow::Show(std::string FolderPath, COLMAPFoundData FoundData)
{
	bShouldOpen = true;
	this->FolderPath = FolderPath;
	this->FoundData = FoundData;
}

void LoadPhotogrammetryWindow::Close()
{
	bShouldClose = true;
}

void LoadPhotogrammetryWindow::Render()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
	{
		InternalClose();
		return;
	}

	const ImVec2 CurrentWinowSize = ImVec2(512, 180);
	const ImVec2 CurrentWinowPosition = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - CurrentWinowSize.x / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - CurrentWinowSize.y / 2.0f);

	ImGui::SetNextWindowPos(CurrentWinowPosition);
	ImGui::SetNextWindowSize(CurrentWinowSize);
	if (bShouldOpen)
	{
		bShouldOpen = false;
		ImGui::OpenPopup("Load COLMAP Photogrammetry Data");
	}

	bool bValidFolder = FoundData.bCamerasData && FoundData.bImagesData;
	if (ImGui::BeginPopupModal("Load COLMAP Photogrammetry Data", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (bValidFolder)
		{
			ImGui::Text("The selected folder contains the following COLMAP photogrammetry data:");
			ImGui::BeginDisabled();
			ImGui::Checkbox("Load Cameras data", &FoundData.bCamerasData);
			ImGui::Checkbox("Load Images data", &FoundData.bImagesData);
			ImGui::Checkbox("Photos folder found", &FoundData.bPhotos);
			ImGui::EndDisabled();
			ImGui::Checkbox("Load Tie points data", &FoundData.bTiePointsData);
		}
		else
		{
			ImGui::Text("Selected folder does not contain valid COLMAP photogrammetry data.");
			ImGui::Text("Please select a different folder.");
		}
		
		if (!bValidFolder)
		{
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - 120.0f / 2.0f);
			ImGui::SetCursorPosY(CurrentWinowSize.y - 28.0f);
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				InternalClose();
			}
			ImGui::SetItemDefaultFocus();
		}
		else
		{
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
			ImGui::SetCursorPosY(CurrentWinowSize.y - 28.0f);
			if (ImGui::Button("Load", ImVec2(120, 0)))
			{
				COLMAP_DATA_MANAGER.CreateNewProject(ActiveObject->GetID(), FolderPath, FoundData);
				InternalClose();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				InternalClose();
			}
		}

		if (bShouldClose)
			InternalClose();

		ImGui::EndPopup();
	}
}

void LoadPhotogrammetryWindow::InternalClose()
{
	bShouldClose = false;
	ImGui::CloseCurrentPopup();
}