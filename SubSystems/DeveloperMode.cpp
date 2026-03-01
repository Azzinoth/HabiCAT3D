#include "DeveloperMode.h"
using namespace FocalEngine;
#include "UI/NewLayerWindow.h"

DeveloperMode::DeveloperMode() {}
DeveloperMode::~DeveloperMode() {}

void DeveloperMode::Initialize()
{
	APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(DeveloperMode::MouseButtonCallback);
	LAYER_MANAGER.AddActiveLayerChangedCallback(DeveloperMode::OnLayerChange);
	JITTER_MANAGER.SetOnCalculationsEndCallback(DeveloperMode::OnJitterCalculationsEnd);
}

void DeveloperMode::MouseMoveCallback(double XPos, double YPos)
{
	DEVELOPER_MODE.MouseX = XPos;
	DEVELOPER_MODE.MouseY = YPos;
}

void DeveloperMode::MouseButtonCallback(int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (ActiveObject != nullptr && DEVELOPER_MODE.GetDebugGrid() != nullptr)
		{
			if (DEVELOPER_MODE.GetDebugGrid()->RenderingMode != 0)
			{

				DEVELOPER_MODE.GetDebugGrid()->MouseClick(DEVELOPER_MODE.MouseX, DEVELOPER_MODE.MouseY);
				DEVELOPER_MODE.UpdateRenderingMode(DEVELOPER_MODE.GetDebugGrid()->RenderingMode);
			}
		}
	}
}

bool DeveloperMode::IsOn()
{
	return bIsOn;
}

void DeveloperMode::SetIsOn(bool NewValue)
{
	bIsOn = NewValue;
	if (bIsOn)
	{
		JITTER_MANAGER.SetDebugJitterToDoCount(1);
	}
	else
	{
		JITTER_MANAGER.SetDebugJitterToDoCount(-1);
	}
}

MeasurementGrid* DeveloperMode::GetDebugGrid()
{
	return DebugGrid;
}

void DeveloperMode::ClearDebugGrid()
{
	if (DebugGrid != nullptr)
	{
		delete DebugGrid;
		DebugGrid = nullptr;
	}
}

std::vector<GridInitData_Jitter> DeveloperMode::ReadJitterSettingsFromDebugInfo(DataLayerDebugInfo* DebugInfo)
{
	std::vector<GridInitData_Jitter> Result;

	if (DebugInfo == nullptr)
		return Result;

	std::istringstream StringStream(DebugInfo->ToString());
	std::string Line;
	GridInitData_Jitter CurrentData;
	bool bNewData = true;

	while (std::getline(StringStream, Line))
	{
		if (Line.find("ShiftX:") != std::string::npos)
		{
			CurrentData.ShiftX = std::stof(Line.substr(Line.find(":") + 1));
			bNewData = false;
		}
		else if (Line.find("ShiftY:") != std::string::npos)
		{
			CurrentData.ShiftY = std::stof(Line.substr(Line.find(":") + 1));
			bNewData = false;
		}
		else if (Line.find("ShiftZ:") != std::string::npos)
		{
			CurrentData.ShiftZ = std::stof(Line.substr(Line.find(":") + 1));
			bNewData = false;
		}
		else if (Line.find("GridScale:") != std::string::npos)
		{
			CurrentData.GridScale = std::stof(Line.substr(Line.find(":") + 1));
			Result.push_back(CurrentData);
			bNewData = true;
			CurrentData = GridInitData_Jitter();
		}
		else if (bNewData)
		{
			// If it's a new "Jitter" line, skip to the next. If it's other text, it will be ignored.
			continue;
		}
	}

	return Result;
}

void DeveloperMode::InitDebugGrid(size_t JitterIndex)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	if (LAYER_MANAGER.GetActiveLayer() == nullptr)
		return;

	std::vector<GridInitData_Jitter> UsedSettings;
	UsedSettings = ReadJitterSettingsFromDebugInfo(LAYER_MANAGER.GetActiveLayer()->DebugInfo);

	if (JitterIndex >= UsedSettings.size())
		return;

	if (UsedSettings.size() == 0)
		return;

	if (JitterIndex < 0 || JitterIndex >= UsedSettings.size())
		JitterIndex = UsedSettings.size() - 1;

	// We are working with jitter manager
	// that means that layer should have this info.
	float CurrentLayerResolutionInM = 0.0f;
	DataLayer* Layer = LAYER_MANAGER.GetActiveLayer();
	for (size_t i = 0; i < Layer->DebugInfo->Entries.size(); i++)
	{
		if (Layer->DebugInfo->Entries[i].Name == "Resolution used")
		{
			std::string Data = Layer->DebugInfo->Entries[i].RawData;
			Data.erase(Data.begin() + Data.find(" m."), Data.end());
			CurrentLayerResolutionInM = static_cast<float>(atof(Data.c_str()));
			break;
		}
	}

	if (CurrentLayerResolutionInM <= 0.0f && CurrentLayerResolutionInM != -1.0f)
		return;

	delete DebugGrid;
	DebugGrid = new MeasurementGrid();
	DebugGrid->AddOnSelectedCellChangedCallback(OnDebugGridSelectedCellChanged);

	GridInitData_Jitter* CurrentSettings = &UsedSettings[JitterIndex];
	FEAABB FinalAABB = JITTER_MANAGER.GetAABBForJitteredGrid(CurrentSettings, CurrentLayerResolutionInM);

	DebugGrid->Init(FinalAABB, CurrentLayerResolutionInM);
	ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH ? DebugGrid->FillCellsWithTriangleInfo() : DebugGrid->FillCellsWithPointInfo();
	DebugGrid->bFullyLoaded = true;
}

void DeveloperMode::AddOnDebugGridSelectedCellChangedCallback(std::function<void(glm::vec3 SelectedCellIndex)> Callback)
{
	ClientOnDebugGridSelectedCellChangedCallbacks.push_back(Callback);
}

void DeveloperMode::OnDebugGridSelectedCellChanged(glm::vec3 NewSelectedCell)
{
	for (const auto& Callback : DEVELOPER_MODE.ClientOnDebugGridSelectedCellChangedCallbacks)
		Callback(NewSelectedCell);
}

void DeveloperMode::ShowLayerDebugUI()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();

	if (DEVELOPER_MODE.IsOn() && LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		if (DEVELOPER_MODE.GetDebugGrid() == nullptr)
			DEVELOPER_MODE.InitDebugGrid(JITTER_MANAGER.GetJitterToDoCount() - 1);

		MeasurementGrid* DebugGrid = DEVELOPER_MODE.GetDebugGrid();
		if (DebugGrid != nullptr)
		{
			std::vector<GridInitData_Jitter> UsedSettings;
			UsedSettings = DEVELOPER_MODE.ReadJitterSettingsFromDebugInfo(LAYER_MANAGER.GetActiveLayer()->DebugInfo);

			if (UsedSettings.size() > 0)
			{
				if (CurrentJitterStepIndexVisualize < 0 || CurrentJitterStepIndexVisualize >= UsedSettings.size())
					CurrentJitterStepIndexVisualize = static_cast<int>(UsedSettings.size() - 1);

				ImGui::Text("Individual jitter steps: ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(190);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
				if (ImGui::BeginCombo("##ChooseJitterStep", std::to_string(CurrentJitterStepIndexVisualize).c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < UsedSettings.size(); i++)
					{
						bool bIsSelected = (CurrentJitterStepIndexVisualize == i);
						if (ImGui::Selectable(std::to_string(i).c_str(), bIsSelected))
						{
							CurrentJitterStepIndexVisualize = static_cast<int>(i);
							int LastGridRendetingMode = DebugGrid->RenderingMode;

							DEVELOPER_MODE.InitDebugGrid(CurrentJitterStepIndexVisualize);

							DebugGrid->RenderingMode = LastGridRendetingMode;
							if (DebugGrid->RenderingMode == 1)
								UpdateRenderingMode(1);
						}

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				std::string JitterInfo = "ShiftX: " + std::to_string(UsedSettings[CurrentJitterStepIndexVisualize].ShiftX);
				JitterInfo += " ShiftY: " + std::to_string(UsedSettings[CurrentJitterStepIndexVisualize].ShiftY);
				JitterInfo += " ShiftZ: " + std::to_string(UsedSettings[CurrentJitterStepIndexVisualize].ShiftZ);
				JitterInfo += " GridScale: " + std::to_string(UsedSettings[CurrentJitterStepIndexVisualize].GridScale);
				ImGui::Text(JitterInfo.c_str());
			}

			ImGui::Text("Visualization of Grid:");

			int TempRenderingMode = DebugGrid->RenderingMode;
			if (ImGui::RadioButton("Do not draw", &TempRenderingMode, 0))
			{
				UpdateRenderingMode(0);
			}

			std::string CurrentGeometryType = ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH ? "triangles" : "points";
			if (ImGui::RadioButton(("Show cells with " + CurrentGeometryType).c_str(), &TempRenderingMode, 1))
			{
#ifdef NEW_LINES
				InitDebugGrid(CurrentJitterStepIndexVisualize);
#endif
				UpdateRenderingMode(1);
			}

			if (ImGui::RadioButton("Show all cells", &TempRenderingMode, 2))
			{
				UpdateRenderingMode(2);
			}

#ifdef NEW_LINES
			if (DebugGrid->RenderingMode == 1)
			{
				DebugGrid->AddLinesOfGrid();
			}
#endif

			DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
			if (CurrentLayer == nullptr)
				return;

			if (CurrentLayer->GetType() == LAYER_TYPE::UNKNOWN)
				return;

			switch (CurrentLayer->GetType())
			{
				case LAYER_TYPE::RUGOSITY:
				{
					RUGOSITY_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(DebugGrid);
					break;
				}

				case LAYER_TYPE::VECTOR_DISPERSION:
				{
					VECTOR_DISPERSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(DebugGrid);
					break;
				}

				case LAYER_TYPE::FRACTAL_DIMENSION:
				{
					FRACTAL_DIMENSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(DebugGrid);
					break;
				}

				case LAYER_TYPE::POINT_DENSITY:
				{
					POINT_DENSITY_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(DebugGrid);
					break;
				}

				case LAYER_TYPE::STRUCTURAL_ROUGHNESS:
				{
					STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(DebugGrid);
					break;
				}

				default:
					break;
			}
		}
	}
}

void DeveloperMode::UpdateRenderingMode(int NewRenderingMode)
{
	if (DebugGrid == nullptr)
		return;

	if (NewRenderingMode < 0)
		return;

	if (DebugGrid->RenderingMode != NewRenderingMode)
		DebugGrid->ClearSelection();

	DebugGrid->RenderingMode = NewRenderingMode;
	DebugGrid->UpdateLineRepresentation();
}

void DeveloperMode::OnLayerChange()
{
	if (DEVELOPER_MODE.GetDebugGrid() != nullptr)
	{
		DEVELOPER_MODE.InitDebugGrid(DEVELOPER_MODE.CurrentJitterStepIndexVisualize);
		DEVELOPER_MODE.UpdateRenderingMode(DEVELOPER_MODE.GetDebugGrid()->RenderingMode);
	}
}

void DeveloperMode::OnJitterCalculationsEnd(DataLayer* NewLayer)
{
	DEVELOPER_MODE.CurrentJitterStepIndexVisualize = static_cast<int>(JITTER_MANAGER.GetLastUsedJitterSettings().size() - 1);
}