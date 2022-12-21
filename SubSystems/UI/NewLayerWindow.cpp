#include "NewLayerWindow.h"

NewLayerWindow* NewLayerWindow::Instance = nullptr;

NewLayerWindow::NewLayerWindow()
{
	LayerTypesNames.push_back("Height");
	LayerTypesNames.push_back("Rugosity");
	LayerTypesNames.push_back("Comapare layers");
	LayerTypesNames.push_back("Triangles area");
	LayerTypesNames.push_back("Triangles edges");

	TrianglesEdgesModeNames.push_back("Max triangle edge length");
	TrianglesEdgesModeNames.push_back("Min triangle edge length");
	TrianglesEdgesModeNames.push_back("Mean triangle edge length");
};

NewLayerWindow::~NewLayerWindow() {};

void NewLayerWindow::Show()
{
	FirstChoosenLayerIndex = -1;
	SecondChoosenLayerIndex = -1;

	bShouldOpen = true;
}

void NewLayerWindow::Close()
{
	bShouldClose = true;
}

void NewLayerWindow::Render()
{
	const ImVec2 CurrentWinowSize = ImVec2(512, 256);
	const ImVec2 CurrentWinowPosition = ImVec2(APPLICATION.GetWindowWidth() / 2.0f - CurrentWinowSize.x / 2.0f, APPLICATION.GetWindowHeight() / 2.0f - CurrentWinowSize.y / 2.0f);

	ImGui::SetNextWindowPos(CurrentWinowPosition);
	ImGui::SetNextWindowSize(CurrentWinowSize);
	if (bShouldOpen)
	{
		bShouldOpen = false;
		ImGui::OpenPopup("New Layer");
	}

	if (ImGui::BeginPopupModal("New Layer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
		ImGui::Text("Type of layer: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(190);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
		if (ImGui::BeginCombo("##ChooseTypeOfNewLayer", (LayerTypesNames[Mode]).c_str(), ImGuiWindowFlags_None))
		{
			for (size_t i = 0; i < LayerTypesNames.size(); i++)
			{
				const bool is_selected = LayerTypesNames[Mode] == LayerTypesNames[i];
				if (ImGui::Selectable(LayerTypesNames[i].c_str(), is_selected))
				{
					Mode = i;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();
		RenderSettings();

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		ImGui::SetCursorPosY(CurrentWinowSize.y - 28.0f);
		if (ImGui::Button("Add", ImVec2(120, 0)))
		{
			AddLayer();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			InternalClose();
		}

		if (bShouldClose)
			InternalClose();

		ImGui::EndPopup();
	}
}

std::vector<std::string> NewLayerWindow::GetNewLayerName()
{
	return LayerTypesNames;
}

void NewLayerWindow::InternalClose()
{
	bShouldClose = false;
	ImGui::CloseCurrentPopup();
}

void NewLayerWindow::AddLayer()
{
	switch (Mode)
	{
		case 0:
		{
			MESH_MANAGER.ActiveMesh->AddLayer(HEIGHT_LAYER_PRODUCER.Calculate(MESH_MANAGER.ActiveMesh));
			LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);

			InternalClose();
			break;
		}
		case 1:
		{
			RUGOSITY_MANAGER.JitterToDoCount = 64;
			RUGOSITY_MANAGER.calculateRugorsityWithJitterAsyn();
			MESH_MANAGER.ActiveMesh->HeatMapType = 5;

			InternalClose();
			break;
		}
		case 2:
		{
			MESH_MANAGER.ActiveMesh->AddLayer(COMPARE_LAYER_PRODUCER.Calculate(FirstChoosenLayerIndex, SecondChoosenLayerIndex));
			LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);

			InternalClose();
			break;
		}
		case 3:
		{
			MESH_MANAGER.ActiveMesh->AddLayer(AREA_LAYER_PRODUCER.Calculate(MESH_MANAGER.ActiveMesh));
			LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);

			InternalClose();
			break;
		}
		case 4:
		{
			MESH_MANAGER.ActiveMesh->AddLayer(TRIANGLE_EDGE_LAYER_PRODUCER.Calculate(MESH_MANAGER.ActiveMesh, TrianglesEdgesMode));
			LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);

			InternalClose();
			break;
		}
	}
}

void NewLayerWindow::RenderHeightLayerSettings()
{
	const std::string Text = "No settings available.";

	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize(Text.c_str()).x / 2.0f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize(Text.c_str()).y / 2.0f);
	ImGui::Text(Text.c_str());
}

void NewLayerWindow::RenderRugosityLayerSettings()
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	ImGui::Text("Rugosity algorithm: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(190);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::BeginCombo("##ChooseRugosityAlgorithm", (RUGOSITY_MANAGER.GetUsedRugosityAlgorithmName()).c_str(), ImGuiWindowFlags_None))
	{
		for (size_t i = 0; i < RUGOSITY_MANAGER.RugosityAlgorithmList.size(); i++)
		{
			bool is_selected = (RUGOSITY_MANAGER.GetUsedRugosityAlgorithmName() == RUGOSITY_MANAGER.RugosityAlgorithmList[i]);
			if (ImGui::Selectable(RUGOSITY_MANAGER.RugosityAlgorithmList[i].c_str(), is_selected))
			{
				RUGOSITY_MANAGER.SetUsedRugosityAlgorithmName(RUGOSITY_MANAGER.RugosityAlgorithmList[i]);
				RUGOSITY_MANAGER.bDeleteOutliers = true;
				if (i != 0)
					RUGOSITY_MANAGER.bDeleteOutliers = false;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Checkbox("Delete outliers", &RUGOSITY_MANAGER.bDeleteOutliers);

	ImGui::Text("Grid size:");
	static int SmallScaleFeatures = 0;
	ImGui::RadioButton(("Small (Grid size - " + std::to_string(RUGOSITY_MANAGER.LowestResolution) + " m)").c_str(), &SmallScaleFeatures, 0);
	ImGui::RadioButton(("Large (Grid size - " + std::to_string(RUGOSITY_MANAGER.HigestResolution) + " m)").c_str(), &SmallScaleFeatures, 1);
	ImGui::RadioButton("Custom", &SmallScaleFeatures, 3);

	if (SmallScaleFeatures == 0)
	{
		RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.LowestResolution;
	}
	else if (SmallScaleFeatures == 1)
	{
		RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.HigestResolution;
	}
	else
	{
		ImGui::Text(("For current mesh \n Min value : "
			+ std::to_string(RUGOSITY_MANAGER.LowestResolution)
			+ " m \n Max value : "
			+ std::to_string(RUGOSITY_MANAGER.HigestResolution) + " m").c_str());

		ImGui::SetNextItemWidth(128);
		ImGui::DragFloat("##ResolutonInM", &RUGOSITY_MANAGER.ResolutonInM, 0.01f);

		if (RUGOSITY_MANAGER.ResolutonInM < RUGOSITY_MANAGER.LowestResolution)
			RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.LowestResolution;

		if (RUGOSITY_MANAGER.ResolutonInM > RUGOSITY_MANAGER.HigestResolution)
			RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.HigestResolution;
	}
}

void NewLayerWindow::RenderCompareLayerSettings()
{
	if (MESH_MANAGER.ActiveMesh->Layers.size() < 2)
	{
		const std::string Text = "To compare layers, you should have at least two layers.";

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize(Text.c_str()).x / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize(Text.c_str()).y / 2.0f);
		ImGui::Text(Text.c_str());

		return;
	}

	std::string FirstString = "Choose layer";
	if (FirstChoosenLayerIndex != -1)
		FirstString = MESH_MANAGER.ActiveMesh->Layers[FirstChoosenLayerIndex].GetCaption();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	ImGui::Text("First layer: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(190);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::BeginCombo("##ChooseFirstLayer", FirstString.c_str(), ImGuiWindowFlags_None))
	{
		for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->Layers.size(); i++)
		{
			bool is_selected = (i == FirstChoosenLayerIndex);
			if (ImGui::Selectable(MESH_MANAGER.ActiveMesh->Layers[i].GetCaption().c_str(), is_selected))
			{
				FirstChoosenLayerIndex = i;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	std::string SecondString = "Choose layer";
	if (SecondChoosenLayerIndex != -1)
		SecondString = MESH_MANAGER.ActiveMesh->Layers[SecondChoosenLayerIndex].GetCaption();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	ImGui::Text("Second layer: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(190);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::BeginCombo("##ChooseSecondLayer", SecondString.c_str(), ImGuiWindowFlags_None))
	{
		for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->Layers.size(); i++)
		{
			bool is_selected = (i == SecondChoosenLayerIndex);
			if (ImGui::Selectable(MESH_MANAGER.ActiveMesh->Layers[i].GetCaption().c_str(), is_selected))
			{
				SecondChoosenLayerIndex = i;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	bool TempBool = COMPARE_LAYER_PRODUCER.GetShouldNormalize();
	ImGui::Checkbox("Normalize before compare", &TempBool);
	COMPARE_LAYER_PRODUCER.SetShouldNormalize(TempBool);
}

void NewLayerWindow::RenderAreaLayerSettings()
{
	const std::string Text = "No settings available.";

	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize(Text.c_str()).x / 2.0f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize(Text.c_str()).y / 2.0f);
	ImGui::Text(Text.c_str());
}

void NewLayerWindow::RenderTrianglesEdgesLayerSettings()
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	ImGui::Text("Choose in what information you are interested: ");
	ImGui::SetNextItemWidth(240);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::BeginCombo("##ChooseOptionLayer", TrianglesEdgesModeNames[TrianglesEdgesMode].c_str(), ImGuiWindowFlags_None))
	{
		for (size_t i = 0; i < TrianglesEdgesModeNames.size(); i++)
		{
			bool is_selected = (i == TrianglesEdgesMode);
			if (ImGui::Selectable(TrianglesEdgesModeNames[i].c_str(), is_selected))
			{
				TrianglesEdgesMode = i;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void NewLayerWindow::RenderSettings()
{
	switch (Mode)
	{
		case 0:
		{
			RenderHeightLayerSettings();
			break;
		}
		case 1:
		{
			RenderRugosityLayerSettings();
			break;
		}
		case 2:
		{
			RenderCompareLayerSettings();
			break;
		}
		case 3:
		{
			RenderAreaLayerSettings();
			break;
		}
		case 4:
		{
			RenderTrianglesEdgesLayerSettings();
			break;
		}
	}
}