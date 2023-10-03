#include "NewLayerWindow.h"

NewLayerWindow* NewLayerWindow::Instance = nullptr;

NewLayerWindow::NewLayerWindow()
{
	LayerTypesNames.push_back("Height");
	LayerTypesNames.push_back("Triangles area");
	LayerTypesNames.push_back("Triangles edges");
	LayerTypesNames.push_back("Triangles density");

	LayerTypesNames.push_back("Rugosity");
	LayerTypesNames.push_back("Vector dispersion");
	LayerTypesNames.push_back("Fractal dimension");

	LayerTypesNames.push_back("Compare layers");

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
	const ImVec2 CurrentWinowSize = ImVec2(512, 305);
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
					int OldMode = Mode;
					Mode = i;
					OnModeChanged(OldMode);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();

				if (i == 3 || i == 6)
					ImGui::Separator();
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
			MESH_MANAGER.ActiveMesh->AddLayer(AREA_LAYER_PRODUCER.Calculate(MESH_MANAGER.ActiveMesh));
			LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);

			InternalClose();
			break;
		}
		case 2:
		{
			MESH_MANAGER.ActiveMesh->AddLayer(TRIANGLE_EDGE_LAYER_PRODUCER.Calculate(MESH_MANAGER.ActiveMesh, TrianglesEdgesMode));
			LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);

			InternalClose();
			break;
		}
		case 3:
		{
			if (bRunOnWholeModel)
			{
				TRIANGLE_COUNT_LAYER_PRODUCER.CalculateOnWholeModel(MESH_MANAGER.ActiveMesh);
				MESH_MANAGER.ActiveMesh->HeatMapType = -1;
			}
			else
			{
				TRIANGLE_COUNT_LAYER_PRODUCER.CalculateWithJitterAsync(MESH_MANAGER.ActiveMesh, bSmootherResult);
				MESH_MANAGER.ActiveMesh->HeatMapType = 5;
			}

			InternalClose();
			break;
		}
		case 4:
		{
			if (bRunOnWholeModel)
			{
				RUGOSITY_MANAGER.CalculateOnWholeModel();
				MESH_MANAGER.ActiveMesh->HeatMapType = -1;
			}
			else
			{
				RUGOSITY_MANAGER.CalculateRugorsityWithJitterAsync();
				MESH_MANAGER.ActiveMesh->HeatMapType = 5;
			}

			InternalClose();
			break;
		}
		case 5:
		{
			if (bRunOnWholeModel)
			{
				VECTOR_DISPERSION_LAYER_PRODUCER.CalculateOnWholeModel(MESH_MANAGER.ActiveMesh);
				MESH_MANAGER.ActiveMesh->HeatMapType = -1;
			}
			else
			{
				VECTOR_DISPERSION_LAYER_PRODUCER.CalculateWithJitterAsync(MESH_MANAGER.ActiveMesh, bSmootherResult);
				MESH_MANAGER.ActiveMesh->HeatMapType = 5;
			}

			InternalClose();
			break;
		}
		case 6:
		{
			if (bRunOnWholeModel)
			{
				FRACTAL_DIMENSION_LAYER_PRODUCER.CalculateOnWholeModel(MESH_MANAGER.ActiveMesh);
				MESH_MANAGER.ActiveMesh->HeatMapType = -1;
			}
			else
			{
				FRACTAL_DIMENSION_LAYER_PRODUCER.CalculateWithJitterAsync(MESH_MANAGER.ActiveMesh, bSmootherResult, bFilterFractalDimention);
				MESH_MANAGER.ActiveMesh->HeatMapType = 5;
			}

			InternalClose();
			break;
		}
		case 7:
		{
			MESH_MANAGER.ActiveMesh->AddLayer(COMPARE_LAYER_PRODUCER.Calculate(FirstChoosenLayerIndex, SecondChoosenLayerIndex));
			LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);

			InternalClose();
			break;
		}
	}
}

void NewLayerWindow::RenderCellSizeSettings()
{
	//ImGui::Checkbox("Smoother results", &bSmootherResult);

	ImGui::Checkbox("Single value", &bRunOnWholeModel);
	if (!bRunOnWholeModel)
	{
		ImGui::Text("Grid size:");
		ImGui::RadioButton(("Small (Grid size - " + std::to_string(JITTER_MANAGER.GetLowestPossibleResolution()) + " m)").c_str(), &FeaturesSizeSelectionMode, 0);
		ImGui::RadioButton(("Large (Grid size - " + std::to_string(JITTER_MANAGER.GetHigestPossibleResolution()) + " m)").c_str(), &FeaturesSizeSelectionMode, 1);
		ImGui::RadioButton("Custom", &FeaturesSizeSelectionMode, 3);

		if (FeaturesSizeSelectionMode == 0)
		{
			JITTER_MANAGER.SetResolutonInM(JITTER_MANAGER.GetLowestPossibleResolution());
		}
		else if (FeaturesSizeSelectionMode == 1)
		{
			JITTER_MANAGER.SetResolutonInM(JITTER_MANAGER.GetHigestPossibleResolution());
		}
		else
		{
			ImGui::Text(("For current mesh \n Min value : "
				+ std::to_string(JITTER_MANAGER.GetLowestPossibleResolution())
				+ " m \n Max value : "
				+ std::to_string(JITTER_MANAGER.GetHigestPossibleResolution()) + " m").c_str());

			ImGui::SetNextItemWidth(128);
			float TempResoluton = JITTER_MANAGER.GetResolutonInM();
			ImGui::DragFloat("##ResolutonInM", &TempResoluton, 0.01f);
			JITTER_MANAGER.SetResolutonInM(TempResoluton);
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

void NewLayerWindow::RenderTriangleDensityLayerSettings()
{
	RenderCellSizeSettings();
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

	if (!bRunOnWholeModel)
		ImGui::Checkbox("Delete outliers", &RUGOSITY_MANAGER.bDeleteOutliers);
	ImGui::Checkbox("Unique projected area (very slow).", &RUGOSITY_MANAGER.bOverlapAware);

	RenderCellSizeSettings();
}

void NewLayerWindow::RenderVectorDispersionSettings()
{
	RenderCellSizeSettings();
}

void NewLayerWindow::RenderFractalDimentionSettings()
{
	ImGui::Checkbox("Filter FD outliers", &bFilterFractalDimention);
	RenderCellSizeSettings();
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
			RenderAreaLayerSettings();
			break;
		}
		case 2:
		{
			RenderTrianglesEdgesLayerSettings();
			break;
		}
		case 3:
		{
			RenderTriangleDensityLayerSettings();
			break;
		}
		case 4:
		{
			RenderRugosityLayerSettings();
			break;
		}
		
		case 5:
		{
			RenderVectorDispersionSettings();
			break;
		}
		case 6:
		{
			RenderFractalDimentionSettings();
			break;
		}
		case 7:
		{
			RenderCompareLayerSettings();
			break;
		}
	}
}

void NewLayerWindow::OnModeChanged(int OldMode)
{
	switch (Mode)
	{
		case 0:
		{
			//RenderHeightLayerSettings();
			break;
		}
		case 1:
		{
			//RenderAreaLayerSettings();
			break;
		}
		case 2:
		{
			//RenderTrianglesEdgesLayerSettings();
			break;
		}
		case 3:
		{
			//RenderTriangleDensityLayerSettings();
			break;
		}
		case 4:
		{
			//RenderRugosityLayerSettings();
			break;
		}

		case 5:
		{
			//RenderVectorDispersionSettings();
			break;
		}
		case 6:
		{
			FeaturesSizeSelectionMode = 3;
			double StartingResolution = JITTER_MANAGER.GetLowestPossibleResolution() + (JITTER_MANAGER.GetHigestPossibleResolution() - JITTER_MANAGER.GetLowestPossibleResolution()) / 2.0f;
			JITTER_MANAGER.SetResolutonInM(StartingResolution);

			break;
		}
		case 7:
		{
			//RenderCompareLayerSettings();
			break;
		}
	}
}