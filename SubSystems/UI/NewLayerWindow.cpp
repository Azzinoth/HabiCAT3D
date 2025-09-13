#include "NewLayerWindow.h"

NewLayerWindow::NewLayerWindow()
{
	TrianglesEdgesModeNames.push_back("Max triangle edge length");
	TrianglesEdgesModeNames.push_back("Min triangle edge length");
	TrianglesEdgesModeNames.push_back("Mean triangle edge length");

	LayerTypeToName[LAYER_TYPE::HEIGHT] = "Height";
	LayerTypeToName[LAYER_TYPE::TRIANGLE_AREA] = "Triangle area";
	LayerTypeToName[LAYER_TYPE::TRIANGLE_EDGE] = "Triangle edges";
	LayerTypeToName[LAYER_TYPE::TRIANGLE_DENSITY] = "Triangle density";

	LayerTypeToName[LAYER_TYPE::RUGOSITY] = "Rugosity";
	LayerTypeToName[LAYER_TYPE::VECTOR_DISPERSION] = "Vector dispersion";
	LayerTypeToName[LAYER_TYPE::FRACTAL_DIMENSION] = "Fractal dimension";

	LayerTypeToName[LAYER_TYPE::COMPARE] = "Compare layers";

	LayerTypeToName[LAYER_TYPE::POINT_DENSITY] = "Point density";
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
	CheckAvailableDataSources();

	const ImVec2 CurrentWinowSize = ImVec2(512, 420);
	const ImVec2 CurrentWinowPosition = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - CurrentWinowSize.x / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - CurrentWinowSize.y / 2.0f);

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
		if (ImGui::BeginCombo("##ChooseTypeOfNewLayer", (LayerTypeToName[SelectedLayerType]).c_str(), ImGuiWindowFlags_None))
		{
			for (size_t i = 0; i < AvailableLayerTypes.size(); i++)
			{
				const bool bIsSelected = LayerTypeToName[SelectedLayerType] == LayerTypeToName[AvailableLayerTypes[i]];
				if (ImGui::Selectable(LayerTypeToName[AvailableLayerTypes[i]].c_str(), bIsSelected))
				{
					LAYER_TYPE OldLayerType = SelectedLayerType;
					SelectedLayerType = AvailableLayerTypes[i];
					CurrentDataSource = DataLayer::GetDataSourceTypeForLayerType(SelectedLayerType);
					OnLayerTypeChanged(OldLayerType);
				}

				if (bIsSelected)
					ImGui::SetItemDefaultFocus();

				if (AvailableLayerTypes[i] == LAYER_TYPE::TRIANGLE_DENSITY || AvailableLayerTypes[i] == LAYER_TYPE::FRACTAL_DIMENSION)
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

void NewLayerWindow::InternalClose()
{
	bShouldClose = false;
	ImGui::CloseCurrentPopup();
}

void NewLayerWindow::AddLayer()
{
	switch (SelectedLayerType)
	{
		case LAYER_TYPE::HEIGHT:
		{
			LAYER_MANAGER.AddLayer(HEIGHT_LAYER_PRODUCER.Calculate());
			LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(LAYER_MANAGER.Layers.size() - 1));

			InternalClose();
			break;
		}
		case LAYER_TYPE::TRIANGLE_AREA:
		{
			LAYER_MANAGER.AddLayer(AREA_LAYER_PRODUCER.Calculate());
			LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(LAYER_MANAGER.Layers.size() - 1));

			InternalClose();
			break;
		}
		case LAYER_TYPE::TRIANGLE_EDGE:
		{
			LAYER_MANAGER.AddLayer(TRIANGLE_EDGE_LAYER_PRODUCER.Calculate(TrianglesEdgesMode));
			LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(LAYER_MANAGER.Layers.size() - 1));

			InternalClose();
			break;
		}
		case LAYER_TYPE::TRIANGLE_DENSITY:
		{
			if (bRunOnWholeModel)
			{
				TRIANGLE_COUNT_LAYER_PRODUCER.CalculateOnWholeModel();
				SCENE_RESOURCES.SetHeatMapType(-1);
			}
			else
			{
				TRIANGLE_COUNT_LAYER_PRODUCER.CalculateWithJitterAsync(bSmootherResult);
				SCENE_RESOURCES.SetHeatMapType(5);
			}

			InternalClose();
			break;
		}
		case LAYER_TYPE::RUGOSITY:
		{
			if (bRunOnWholeModel)
			{
				RUGOSITY_LAYER_PRODUCER.CalculateOnWholeModel();
				SCENE_RESOURCES.SetHeatMapType(-1);
			}
			else
			{
				RUGOSITY_LAYER_PRODUCER.CalculateWithJitterAsync();
				SCENE_RESOURCES.SetHeatMapType(5);
			}

			InternalClose();
			break;
		}
		case LAYER_TYPE::VECTOR_DISPERSION:
		{
			if (bRunOnWholeModel)
			{
				VECTOR_DISPERSION_LAYER_PRODUCER.CalculateOnWholeModel();
				SCENE_RESOURCES.SetHeatMapType(-1);
			}
			else
			{
				VECTOR_DISPERSION_LAYER_PRODUCER.CalculateWithJitterAsync(bSmootherResult);
				SCENE_RESOURCES.SetHeatMapType(5);
			}

			InternalClose();
			break;
		}
		case LAYER_TYPE::FRACTAL_DIMENSION:
		{
			if (bRunOnWholeModel)
			{
				FRACTAL_DIMENSION_LAYER_PRODUCER.CalculateOnWholeModel();
				SCENE_RESOURCES.SetHeatMapType(-1);
			}
			else
			{
				FRACTAL_DIMENSION_LAYER_PRODUCER.CalculateWithJitterAsync(bSmootherResult);
				SCENE_RESOURCES.SetHeatMapType(5);
			}

			InternalClose();
			break;
		}
		case LAYER_TYPE::COMPARE:
		{
			LAYER_MANAGER.AddLayer(COMPARE_LAYER_PRODUCER.Calculate(FirstChoosenLayerIndex, SecondChoosenLayerIndex));
			LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(LAYER_MANAGER.Layers.size() - 1));
			SCENE_RESOURCES.SetHeatMapType(6);

			InternalClose();
			break;
		}
		case LAYER_TYPE::POINT_DENSITY:
		{
			POINT_DENSITY_LAYER_PRODUCER.CalculateWithJitterAsync(bSmootherResult);
			
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
		ImGui::RadioButton(("Large (Grid size - " + std::to_string(JITTER_MANAGER.GetHighestPossibleResolution()) + " m)").c_str(), &FeaturesSizeSelectionMode, 1);
		ImGui::RadioButton("Custom", &FeaturesSizeSelectionMode, 3);

		if (FeaturesSizeSelectionMode == 0)
		{
			JITTER_MANAGER.SetResolutionInM(JITTER_MANAGER.GetLowestPossibleResolution());
		}
		else if (FeaturesSizeSelectionMode == 1)
		{
			JITTER_MANAGER.SetResolutionInM(JITTER_MANAGER.GetHighestPossibleResolution());
		}
		else
		{
			ImGui::Text(("For current mesh \n Min value : "
				+ std::to_string(JITTER_MANAGER.GetLowestPossibleResolution())
				+ " m \n Max value : "
				+ std::to_string(JITTER_MANAGER.GetHighestPossibleResolution()) + " m").c_str());

			ImGui::SetNextItemWidth(128);
			float TempResolution = JITTER_MANAGER.GetResolutionInM();
			ImGui::DragFloat("##ResolutionInM", &TempResolution, 0.01f);
			JITTER_MANAGER.SetResolutionInM(TempResolution);
		}

		ImGui::Text("Jitter quality(higher quality, slower calculations): ");
		ImGui::SetNextItemWidth(70);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
		auto JitterList = JITTER_MANAGER.GetJitterVectorSetNames();
		if (ImGui::BeginCombo("##Jitter quality", (JITTER_MANAGER.GetCurrentJitterVectorSetName()).c_str(), ImGuiWindowFlags_None))
		{
			for (size_t i = 0; i < JitterList.size(); i++)
			{
				bool is_selected = (JITTER_MANAGER.GetCurrentJitterVectorSetName() == JitterList[i]);
				if (ImGui::Selectable(JitterList[i].c_str(), is_selected))
				{
					JITTER_MANAGER.SetCurrentJitterVectorSetName(JitterList[i]);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
}

void NewLayerWindow::RenderNoSettingsAvailable()
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
				TrianglesEdgesMode = static_cast<int>(i);
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
	if (ImGui::BeginCombo("##ChooseRugosityAlgorithm", (RUGOSITY_LAYER_PRODUCER.GetUsedRugosityAlgorithmName()).c_str(), ImGuiWindowFlags_None))
	{
		std::vector<std::string> AlgorithmList = RUGOSITY_LAYER_PRODUCER.GetRugosityAlgorithmList();
		for (size_t i = 0; i < AlgorithmList.size(); i++)
		{
			bool is_selected = (RUGOSITY_LAYER_PRODUCER.GetUsedRugosityAlgorithmName() == AlgorithmList[i]);
			if (ImGui::Selectable(AlgorithmList[i].c_str(), is_selected))
			{
				RUGOSITY_LAYER_PRODUCER.SetUsedRugosityAlgorithmName(AlgorithmList[i]);
				RUGOSITY_LAYER_PRODUCER.SetDeleteOutliers(true);
				if (i != 0)
					RUGOSITY_LAYER_PRODUCER.SetDeleteOutliers(false);
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (RUGOSITY_LAYER_PRODUCER.GetUsedRugosityAlgorithmName() == "Min Rugosity(default)")
	{
		ImGui::Text("Orientation set(advanced option): ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(190);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
		if (ImGui::BeginCombo("##OrientationSet", (RUGOSITY_LAYER_PRODUCER.GetOrientationSetForMinRugosityName()).c_str(), ImGuiWindowFlags_None))
		{
			std::vector<std::string> OrientationSetNames = RUGOSITY_LAYER_PRODUCER.GetOrientationSetNamesForMinRugosityList();
			for (size_t i = 0; i < OrientationSetNames.size(); i++)
			{
				bool is_selected = (RUGOSITY_LAYER_PRODUCER.GetOrientationSetForMinRugosityName() == OrientationSetNames[i]);
				if (ImGui::Selectable(OrientationSetNames[i].c_str(), is_selected))
				{
					RUGOSITY_LAYER_PRODUCER.SetOrientationSetForMinRugosityName(OrientationSetNames[i]);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	if (!bRunOnWholeModel)
	{
		bool bTempBool = RUGOSITY_LAYER_PRODUCER.ShouldDeletedOutlier();
		ImGui::Checkbox("Delete outliers", &bTempBool);
		RUGOSITY_LAYER_PRODUCER.SetDeleteOutliers(bTempBool);
	}
	
	bool TempBool = RUGOSITY_LAYER_PRODUCER.GetIsUsingUniqueProjectedArea();
	ImGui::Checkbox("Unique projected area (very slow).", &TempBool);
	RUGOSITY_LAYER_PRODUCER.SetIsUsingUniqueProjectedArea(TempBool);

	if (TempBool)
	{
		TempBool = RUGOSITY_LAYER_PRODUCER.GetIsUsingUniqueProjectedAreaApproximation();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 15);
		ImGui::Checkbox("Enable Approximation (Speeds Up by Over 100x)", &TempBool);
		RUGOSITY_LAYER_PRODUCER.SetIsUsingUniqueProjectedAreaApproximation(TempBool);
	}

	TempBool = RUGOSITY_LAYER_PRODUCER.ShouldCalculateStandardDeviation();
	ImGui::Checkbox("Add standard deviation layer.", &TempBool);
	RUGOSITY_LAYER_PRODUCER.SetCalculateStandardDeviation(TempBool);

	RenderCellSizeSettings();
}

void NewLayerWindow::RenderVectorDispersionSettings()
{
	bool TempBool = VECTOR_DISPERSION_LAYER_PRODUCER.GetShouldCalculateStandardDeviation();
	ImGui::Checkbox("Add standard deviation layer.", &TempBool);
	VECTOR_DISPERSION_LAYER_PRODUCER.SetShouldCalculateStandardDeviation(TempBool);

	RenderCellSizeSettings();
}

void NewLayerWindow::RenderFractalDimentionSettings()
{
	bool TempBool = FRACTAL_DIMENSION_LAYER_PRODUCER.GetShouldFilterFractalDimensionValues();
	ImGui::Checkbox("Filter FD outliers", &TempBool);
	FRACTAL_DIMENSION_LAYER_PRODUCER.SetShouldFilterFractalDimensionValues(TempBool);

	TempBool = FRACTAL_DIMENSION_LAYER_PRODUCER.GetShouldCalculateStandardDeviation();
	ImGui::Checkbox("Add standard deviation layer.", &TempBool);
	FRACTAL_DIMENSION_LAYER_PRODUCER.SetShouldCalculateStandardDeviation(TempBool);

	RenderCellSizeSettings();
}

void NewLayerWindow::RenderCompareLayerSettings()
{
	if (LAYER_MANAGER.Layers.size() < 2)
	{
		const std::string Text = "To compare layers, you should have at least two layers.";

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize(Text.c_str()).x / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize(Text.c_str()).y / 2.0f);
		ImGui::Text(Text.c_str());

		return;
	}

	std::string FirstString = "Choose layer";
	if (FirstChoosenLayerIndex != -1)
		FirstString = LAYER_MANAGER.Layers[FirstChoosenLayerIndex].GetCaption();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	ImGui::Text("First layer: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(190);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::BeginCombo("##ChooseFirstLayer", FirstString.c_str(), ImGuiWindowFlags_None))
	{
		for (size_t i = 0; i < LAYER_MANAGER.Layers.size(); i++)
		{
			bool is_selected = (i == FirstChoosenLayerIndex);
			if (ImGui::Selectable(LAYER_MANAGER.Layers[i].GetCaption().c_str(), is_selected))
			{
				FirstChoosenLayerIndex = static_cast<int>(i);
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	std::string SecondString = "Choose layer";
	if (SecondChoosenLayerIndex != -1)
		SecondString = LAYER_MANAGER.Layers[SecondChoosenLayerIndex].GetCaption();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	ImGui::Text("Second layer: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(190);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::BeginCombo("##ChooseSecondLayer", SecondString.c_str(), ImGuiWindowFlags_None))
	{
		for (size_t i = 0; i < LAYER_MANAGER.Layers.size(); i++)
		{
			bool is_selected = (i == SecondChoosenLayerIndex);
			if (ImGui::Selectable(LAYER_MANAGER.Layers[i].GetCaption().c_str(), is_selected))
			{
				SecondChoosenLayerIndex = static_cast<int>(i);
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

void NewLayerWindow::RenderPointDensitySettings()
{
	RenderCellSizeSettings();
}

void NewLayerWindow::RenderSettings()
{
	switch (SelectedLayerType)
	{
		case LAYER_TYPE::HEIGHT:
		{
			RenderNoSettingsAvailable();
			break;
		}
		case LAYER_TYPE::TRIANGLE_AREA:
		{
			RenderNoSettingsAvailable();
			break;
		}
		case LAYER_TYPE::TRIANGLE_EDGE:
		{
			RenderTrianglesEdgesLayerSettings();
			break;
		}
		case LAYER_TYPE::TRIANGLE_DENSITY:
		{
			RenderTriangleDensityLayerSettings();
			break;
		}
		case LAYER_TYPE::RUGOSITY:
		{
			RenderRugosityLayerSettings();
			break;
		}
		case LAYER_TYPE::VECTOR_DISPERSION:
		{
			RenderVectorDispersionSettings();
			break;
		}
		case LAYER_TYPE::FRACTAL_DIMENSION:
		{
			RenderFractalDimentionSettings();
			break;
		}
		case LAYER_TYPE::COMPARE:
		{
			RenderCompareLayerSettings();
			break;
		}
		case LAYER_TYPE::POINT_DENSITY:
		{
			RenderPointDensitySettings();
			break;
		}
	}
}

void NewLayerWindow::OnLayerTypeChanged(LAYER_TYPE OldLayerType)
{
	switch (SelectedLayerType)
	{
		case LAYER_TYPE::HEIGHT:
		{
			break;
		}
		case LAYER_TYPE::TRIANGLE_AREA:
		{
			break;
		}
		case LAYER_TYPE::TRIANGLE_EDGE:
		{
			break;
		}
		case LAYER_TYPE::TRIANGLE_DENSITY:
		{
			break;
		}
		case LAYER_TYPE::RUGOSITY:
		{
			break;
		}
		case LAYER_TYPE::VECTOR_DISPERSION:
		{
			break;
		}
		case LAYER_TYPE::FRACTAL_DIMENSION:
		{
			FeaturesSizeSelectionMode = 3;
			double StartingResolution = JITTER_MANAGER.GetLowestPossibleResolution() + (JITTER_MANAGER.GetHighestPossibleResolution() - JITTER_MANAGER.GetLowestPossibleResolution()) / 2.0f;
			JITTER_MANAGER.SetResolutionInM(static_cast<float>(StartingResolution));

			break;
		}
		case LAYER_TYPE::COMPARE:
		{
			break;
		}
	}
}

void NewLayerWindow::CheckAvailableDataSources()
{
	AvailableDataSources.clear();
	AvailableLayerTypes.clear();

	if (ANALYSIS_OBJECT_MANAGER.HaveMeshData())
	{
		AvailableDataSources.push_back(DATA_SOURCE_TYPE::MESH);
		AvailableLayerTypes.push_back(LAYER_TYPE::HEIGHT);
		AvailableLayerTypes.push_back(LAYER_TYPE::TRIANGLE_AREA);
		AvailableLayerTypes.push_back(LAYER_TYPE::TRIANGLE_EDGE);
		AvailableLayerTypes.push_back(LAYER_TYPE::TRIANGLE_DENSITY);
		AvailableLayerTypes.push_back(LAYER_TYPE::RUGOSITY);
		AvailableLayerTypes.push_back(LAYER_TYPE::VECTOR_DISPERSION);
		AvailableLayerTypes.push_back(LAYER_TYPE::FRACTAL_DIMENSION);
		AvailableLayerTypes.push_back(LAYER_TYPE::COMPARE);
	}
		
	if (ANALYSIS_OBJECT_MANAGER.HavePointCloudData())
	{
		AvailableDataSources.push_back(DATA_SOURCE_TYPE::POINT_CLOUD);
		AvailableLayerTypes.push_back(LAYER_TYPE::POINT_DENSITY);
	}
}