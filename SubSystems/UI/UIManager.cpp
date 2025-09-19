#include "UIManager.h"
using namespace FocalEngine;

UIManager::UIManager()
{
	HeatMapColorRange.SetPosition(ImVec2(0, 20));

	Histogram.SetSize(ImVec2(300, 180));
	Histogram.SetPosition(ImVec2(20, 60));

	HistogramSelectRegionMin.SetColor(ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
	HistogramSelectRegionMin.SetOrientation(true);

	HistogramSelectRegionMax.SetColor(ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
	HistogramSelectRegionMax.SetOrientation(true);

	JITTER_MANAGER.SetOnCalculationsStartCallback(OnJitterCalculationsStart);
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);

	ANALYSIS_OBJECT_MANAGER.AddOnLoadCallback(UIManager::OnNewObjectLoaded);
	LAYER_MANAGER.AddActiveLayerChangedCallback(UIManager::OnLayerChange);

	LAYER_RASTERIZATION_MANAGER.SetOnCalculationsStartCallback(OnLayerRasterizationCalculationsStart);
	LAYER_RASTERIZATION_MANAGER.SetOnCalculationsEndCallback(OnLayerRasterizationCalculationsEnd);

	AddNewLayerIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/AddNewLayer.png");
}

UIManager::~UIManager() {}

std::string TruncateAfterDot(std::string FloatingPointNumber, const int DigitCount = 2)
{
	int Count = 0;
	bool WasFound = false;
	for (size_t i = 0; i < FloatingPointNumber.size(); i++)
	{
		if (FloatingPointNumber[i] == '.')
		{
			WasFound = true;
			continue;
		}

		if (WasFound)
		{
			if (DigitCount == Count)
			{
				std::string Result = FloatingPointNumber.substr(0, i);
				return Result;
			}
			Count++;
		}
	}

	return FloatingPointNumber;
}

void ShowToolTip(const char* Text)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(Text);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void UIManager::ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform)
{
	static float EditWidth = 70.0f;
	bool bModified = false;
	// ********************* POSITION *********************
	glm::vec3 Position = Transform->GetPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X pos : ") + Name).c_str(), &Position[0], 0.1f))
		bModified = true;
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y pos : ") + Name).c_str(), &Position[1], 0.1f))
		bModified = true;
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z pos : ") + Name).c_str(), &Position[2], 0.1f))
		bModified = true;
	ShowToolTip("Z position");

	if (bModified)
		Transform->SetPosition(Position);

	bModified = false;

	// ********************* WORLD POSITION *********************
	glm::vec3 WorldPosition = Transform->GetPosition(FE_WORLD_SPACE);
	ImGui::Text("World Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X pos : ") + Name).c_str(), &WorldPosition[0], 0.1f))
		bModified = true;
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y pos : ") + Name).c_str(), &WorldPosition[1], 0.1f))
		bModified = true;
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z pos : ") + Name).c_str(), &WorldPosition[2], 0.1f))
		bModified = true;
	ShowToolTip("Z position");

	if (bModified)
		Transform->SetPosition(WorldPosition, FE_WORLD_SPACE);

	bModified = false;

	// ********************* ROTATION *********************
	glm::vec3 Rotation = Transform->GetRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X rot : ") + Name).c_str(), &Rotation[0], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y rot : ") + Name).c_str(), &Rotation[1], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z rot : ") + Name).c_str(), &Rotation[2], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Z rotation");

	if (bModified)
		Transform->SetRotation(Rotation);

	bModified = false;

	// ********************* WORLD ROTATION *********************
	glm::vec3 WorldRotation = Transform->GetRotation(FE_WORLD_SPACE);
	ImGui::Text("World Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X rot : ") + Name).c_str(), &WorldRotation[0], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y rot : ") + Name).c_str(), &WorldRotation[1], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z rot : ") + Name).c_str(), &WorldRotation[2], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Z rotation");

	if (bModified)
		Transform->SetRotation(WorldRotation, FE_WORLD_SPACE);

	bModified = false;

	// ********************* SCALE *********************
	bool bUniformScaling = Transform->IsUniformScalingSet();
	ImGui::Checkbox("Uniform scaling", &bUniformScaling);
	Transform->SetUniformScaling(bUniformScaling);

	glm::vec3 Scale = Transform->GetScale();
	float ScaleChangeSpeed = Scale.x * 0.01f;
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X scale : ") + Name).c_str(), &Scale[0], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[1] = Scale[0];
			Scale[2] = Scale[0];
		}
	}
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y scale : ") + Name).c_str(), &Scale[1], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[0] = Scale[1];
			Scale[2] = Scale[1];
		}
	}
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z scale : ") + Name).c_str(), &Scale[2], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[0] = Scale[2];
			Scale[1] = Scale[2];
		}
	}
	ShowToolTip("Z scale");

	if (bModified)
		Transform->SetScale(Scale);

	bModified = false;

	// ********************* WORLD SCALE *********************
	glm::vec3 WorldScale = Transform->GetScale(FE_WORLD_SPACE);
	ScaleChangeSpeed = WorldScale.x * 0.01f;
	ImGui::Text("World Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X scale : ") + Name).c_str(), &WorldScale[0], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[1] = WorldScale[0];
			WorldScale[2] = WorldScale[0];
		}
	}
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y scale : ") + Name).c_str(), &WorldScale[1], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[0] = WorldScale[1];
			WorldScale[2] = WorldScale[1];
		}
	}
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z scale : ") + Name).c_str(), &WorldScale[2], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[0] = WorldScale[2];
			WorldScale[1] = WorldScale[2];
		}
	}
	ShowToolTip("Z scale");

	if (bModified)
		Transform->SetScale(WorldScale, FE_WORLD_SPACE);
}

void UIManager::ShowCameraTransform()
{
	if (!bModelCamera)
	{
		// ********* POSITION *********
		glm::vec3 CameraPosition = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE);

		ImGui::Text("Position : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##X pos", &CameraPosition[0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Y pos", &CameraPosition[1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Z pos", &CameraPosition[2], 0.1f);

		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().SetPosition(CameraPosition, FE_WORLD_SPACE);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Copy##Position"))
			APPLICATION.SetClipboardText(CameraPositionToString());

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Paste##Position"))
			StringToCameraPosition(APPLICATION.GetClipboardText());

		// ********* ROTATION *********
		glm::vec3 CameraRotation = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetRotation(FE_WORLD_SPACE);

		ImGui::Text("Rotation : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##X rot", &CameraRotation[0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Y rot", &CameraRotation[1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Z rot", &CameraRotation[2], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Copy##Rotation"))
			APPLICATION.SetClipboardText(CameraRotationToString());

		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().SetRotation(CameraRotation, FE_WORLD_SPACE);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Paste##Rotation"))
			StringToCameraRotation(APPLICATION.GetClipboardText());
		
		float NearPlane = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().GetNearPlane();
		ImGui::Text("Near plane: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Near plane", &NearPlane, 0.01f, 0.01f, 100.0f);
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetNearPlane(NearPlane);

		float FarPlane = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().GetFarPlane();
		ImGui::Text("Far plane: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Far plane", &FarPlane, 0.01f, 0.01f, 100000.0f);
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetFarPlane(FarPlane);

		FENativeScriptComponent& NativeScriptComponent = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FENativeScriptComponent>();
		float CameraSpeed = 0.0f;
		NativeScriptComponent.GetVariableValue<float>("MovementSpeed", CameraSpeed);
		ImGui::Text("Camera speed: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Camera_speed", &CameraSpeed, 0.01f, 0.01f, 100.0f);
		NativeScriptComponent.SetVariableValue("MovementSpeed", CameraSpeed);

		if (bDeveloperMode)
		{
			ImGui::SameLine();
			ImGui::Text(("Thread count: " + std::to_string(THREAD_POOL.GetThreadCount())).c_str());
		}
	}
	else
	{
		if (bDeveloperMode)
		{
			ImGui::Text(("Thread count: " + std::to_string(THREAD_POOL.GetThreadCount())).c_str());
		}
	}
}

bool UIManager::GetWireFrameMode()
{
	return bWireframeMode;
}

void UIManager::SetWireFrameMode(const bool NewValue)
{
	bWireframeMode = NewValue;
}

bool UIManager::IsInDeveloperMode()
{
	return bDeveloperMode;
}

void UIManager::SetDeveloperMode(const bool NewValue)
{
	bDeveloperMode = NewValue;

	if (bDeveloperMode)
	{
		JITTER_MANAGER.SetDebugJitterToDoCount(1);
	}
	else
	{
		JITTER_MANAGER.SetDebugJitterToDoCount(-1);
	}
}

void UIManager::Render(bool bScreenshotMode)
{
	if (APPLICATION.GetMainWindow() == nullptr)
		return;
	
	if (bPreviousFrameWindowWasNull)
		bPreviousFrameWindowWasNull = false;

	if (bScreenshotMode)
	{
		RenderLegend(bScreenshotMode);
		return;
	}

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Load..."))
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileOpenDialog(FilePath, RUGOSITY_LOAD_FILE_FILTER, 1);

				if (!FilePath.empty())
					ANALYSIS_OBJECT_MANAGER.LoadResource(FilePath);
			}

			size_t ObjectCount = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount();
			if (ObjectCount == 0)
				ImGui::BeginDisabled();

			if (ImGui::MenuItem("Save..."))
				ANALYSIS_OBJECT_MANAGER.SaveToRUGFileAskForFilePath();

			if (ObjectCount == 0)
				ImGui::EndDisabled();

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				APPLICATION.Close();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Info"))
		{
			if (ImGui::MenuItem("About..."))
				ShowAboutDialog();

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	OBJECT_VIEWER_WINDOW.Render();
	RenderSettingsWindow();
	RenderLegend();
	RenderLayerTabs();
	RenderHistogramWindow();
	RenderAboutWindow();

	NEW_LAYER_WINDOW.Render();

	if (UI.bShouldOpenProgressPopup)
	{
		UI.bShouldOpenProgressPopup = false;
		ImGui::OpenPopup("Calculating...");
	}

	ImGui::SetNextWindowSize(ImVec2(300, 65));
	if (ImGui::BeginPopupModal("Calculating...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		int WindowW = 0;
		int WindowH = 0;
		APPLICATION.GetMainWindow()->GetSize(&WindowW, &WindowH);

		ImGui::SetWindowPos(ImVec2(WindowW / 2.0f - ImGui::GetWindowWidth() / 2.0f, WindowH / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		UpdateProgressModalPopupCurrentValue();
		std::string ProgressText = "Progress: " + std::to_string(ProgressModalPopupCurrentValue * 100.0f);
		ProgressText += " %%";
		ImGui::SetCursorPosX(90);
		ImGui::Text(ProgressText.c_str());

		std::string TimeToFinish = "Time left: " + TIME.TimeToFormattedString(JITTER_MANAGER.ApproximateTimeToFinishInMS, FE_TIME_RESOLUTION_MILLISECONDS);
		int TextWidth = static_cast<int>(ImGui::CalcTextSize(TimeToFinish.c_str()).x);
		ImGui::SetCursorPosX(300 / 2.0f - TextWidth / 2.0f);
		ImGui::Text(TimeToFinish.c_str());

		if (bShouldCloseProgressPopup)
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

std::string UIManager::CameraPositionToString()
{
	const glm::vec3 CameraPosition = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE);
	return "( X:" + std::to_string(CameraPosition.x) + " Y:" + std::to_string(CameraPosition.y) + " Z:" + std::to_string(CameraPosition.z) + " )";
}

void UIManager::StringToCameraPosition(std::string Text)
{
	size_t StartPosition = Text.find("( X:");
	if (StartPosition == std::string::npos)
		return;

	size_t EndPosition = Text.find(" Y:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("( X:") < 0 ||
		StartPosition + strlen("( X:") + EndPosition - (StartPosition + strlen("( X:")) >= Text.size())
		return;

	std::string temp = Text.substr(StartPosition + strlen("( X:"), EndPosition - (StartPosition + strlen("( X:")));

	if (temp.empty())
		return;

	const float X = float(atof(temp.c_str()));

	StartPosition = Text.find("Y:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = Text.find(" Z:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Y:") < 0 ||
		StartPosition + strlen("Y:") + EndPosition - (StartPosition + strlen("Y:")) >= Text.size())
		return;

	temp = Text.substr(StartPosition + strlen("Y:"), EndPosition - (StartPosition + strlen("Y:")));

	if (temp.empty())
		return;

	const float Y = float(atof(temp.c_str()));

	StartPosition = Text.find("Z:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = Text.find(" )");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Z:") < 0 ||
		StartPosition + strlen("Z:") + EndPosition - (StartPosition + strlen("Z:")) >= Text.size())
		return;

	temp = Text.substr(StartPosition + strlen("Z:"), EndPosition - (StartPosition + strlen("Z:")));

	if (temp.empty())
		return;

	const float Z = float(atof(temp.c_str()));

	MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().SetPosition(glm::vec3(X, Y, Z), FE_WORLD_SPACE);
}

std::string UIManager::CameraRotationToString()
{
	const glm::vec3 CameraRotation = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetRotation(FE_WORLD_SPACE);
	return "( X:" + std::to_string(CameraRotation.x) + " Y:" + std::to_string(CameraRotation.y) + " Z:" + std::to_string(CameraRotation.z) + " )";
}

void UIManager::StringToCameraRotation(std::string Text)
{
	size_t StartPosition = Text.find("( X:");
	if (StartPosition == std::string::npos)
		return;

	size_t EndPosition = Text.find(" Y:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("( X:") < 0 ||
		StartPosition + strlen("( X:") + EndPosition - (StartPosition + strlen("( X:")) >= Text.size())
		return;

	std::string temp = Text.substr(StartPosition + strlen("( X:"), EndPosition - (StartPosition + strlen("( X:")));

	if (temp.empty())
		return;

	const float X = float(atof(temp.c_str()));

	StartPosition = Text.find("Y:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = Text.find(" Z:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Y:") < 0 ||
		StartPosition + strlen("Y:") + EndPosition - (StartPosition + strlen("Y:")) >= Text.size())
		return;

	temp = Text.substr(StartPosition + strlen("Y:"), EndPosition - (StartPosition + strlen("Y:")));

	if (temp.empty())
		return;

	const float Y = float(atof(temp.c_str()));

	StartPosition = Text.find("Z:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = Text.find(" )");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Z:") < 0 ||
		StartPosition + strlen("Z:") + EndPosition - (StartPosition + strlen("Z:")) >= Text.size())
		return;

	temp = Text.substr(StartPosition + strlen("Z:"), EndPosition - (StartPosition + strlen("Z:")));

	if (temp.empty())
		return;

	const float Z = float(atof(temp.c_str()));

	MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().SetRotation(glm::vec3(X, Y, Z), FE_WORLD_SPACE);
}

void UIManager::OnNewObjectLoaded(AnalysisObject* NewObject)
{
	UI.AdjustCameraNearFarPlanes();
	UI.bModelCamera ? UI.ModelCameraAdjustment() : UI.FreeCameraAdjustment();

	LINE_RENDERER.ClearAll();
	LINE_RENDERER.SyncWithGPU();

	UI.Histogram.Clear();
	UI.HeatMapColorRange.Clear();

	LAYER_RASTERIZATION_MANAGER.ClearAllData();
	float ResolutionInMeters = LAYER_RASTERIZATION_MANAGER.GetResolutionInMetersBasedOnResolutionInPixels(512);
	if (ResolutionInMeters > 0.0f)
		LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(ResolutionInMeters);
}

float UIManager::GetRadiusOfAreaToMeasure()
{
	return RadiusOfAreaToMeasure;
}

void UIManager::SetRadiusOfAreaToMeasure(const float NewValue)
{
	RadiusOfAreaToMeasure = NewValue;
}

int UIManager::GetLayerSelectionMode()
{
	return LayerSelectionMode;
}

void UIManager::SetLayerSelectionMode(const int NewValue)
{
	LayerSelectionMode = NewValue;
}

void UIManager::OnJitterCalculationsStart()
{
	UI.bJitterCalculationsInProgress = true;
	UI.bShouldCloseProgressPopup = false;
	UI.bShouldOpenProgressPopup = true;
}

void UIManager::OnJitterCalculationsEnd(DataLayer* NewLayer)
{
	UI.bShouldCloseProgressPopup = true;
	UI.CurrentJitterStepIndexVisualize = static_cast<int>(JITTER_MANAGER.GetLastUsedJitterSettings().size() - 1);
	UI.bJitterCalculationsInProgress = false;
}

static auto CompareColormapValue = [](float Value) {

	Value = Value * 2.0f - 1.0f;

	static auto Mix = [](glm::vec3 FirstColor, glm::vec3 SecondColor, float Factor) {
		return glm::vec3(FirstColor.x + (SecondColor.x - FirstColor.x) * Factor,
						 FirstColor.y + (SecondColor.y - FirstColor.y) * Factor,
						 FirstColor.z + (SecondColor.z - FirstColor.z) * Factor);
	};

	// Define the colors
	glm::vec3 ColorNegative = glm::vec3(0.0, 0.0, 1.0); // Blue for negative
	glm::vec3 ColorNeutral = glm::vec3(1.0, 1.0, 1.0);  // White for zero
	glm::vec3 ColorPositive = glm::vec3(1.0, 0.0, 0.0); // Red for positive

	glm::vec3 FinalColor;
	// Interpolate between the colors based on the factor
	if (Value < 0)
	{
		// Interpolate between blue and white for negative values
		FinalColor = Mix(ColorNeutral, ColorNegative, -Value);
	}
	else
	{
		// Interpolate between white and red for positive values
		FinalColor = Mix(ColorNeutral, ColorPositive, Value);
	}

	return glm::vec3(FinalColor.x, FinalColor.y, FinalColor.z);
};

static auto RainbowScaledColor = [](float Value) {
	Value = 1.0f - Value;
	Value *= 6.0f;
	const int sextant = int(Value);
	const float vsf = Value - sextant;
	const float mid1 = vsf;
	const float mid2 = 1.0f - vsf;

	glm::vec3 result = glm::vec3(1, 0, 0);

	switch (sextant)
	{
	case 0:
		result.x = 1;
		result.y = 0;
		result.z = 0;
		break;
	case 1:
		result.x = 1;
		result.y = mid1;
		result.z = 0;
		break;
	case 2:
		result.x = mid2;
		result.y = 1;
		result.z = 0;
		break;
	case 3:
		result.x = 0;
		result.y = 1;
		result.z = mid1;
		break;
	case 4:
		result.x = 0;
		result.y = mid2;
		result.z = 1;
		break;
	case 5:
		result.x = mid1;
		result.y = 0;
		result.z = 1;
		break;
	}

	return ImColor(int(result.x * 255), int(result.y * 255), int(result.z * 255), 255);
};

void UIManager::RenderLegend(bool bScreenshotMode)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(ImVec2(2, 20));
	ImGui::SetNextWindowSize(ImVec2(150, 670));
	ImGui::Begin("Heat map legend", nullptr,
									ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoCollapse |
									ImGuiWindowFlags_NoScrollbar |
									(bScreenshotMode ? ImGuiWindowFlags_NoBackground : ImGuiWindowFlags_None) |
									(bScreenshotMode ? ImGuiWindowFlags_NoTitleBar : ImGuiWindowFlags_None));

	if (HeatMapColorRange.GetColorRangeFunction() == nullptr)
		HeatMapColorRange.SetColorRangeFunction(GetTurboColorMap);

	static bool bLastFrameActiveLayerWasValid = false;
	if (bLastFrameActiveLayerWasValid && LAYER_MANAGER.GetActiveLayer() == nullptr)
		HeatMapColorRange.Clear();

	bLastFrameActiveLayerWasValid = LAYER_MANAGER.GetActiveLayer() != nullptr;

	if (bScreenshotMode && IsActiveObjectAndLayerValid())
	{
		DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
		if (CurrentLayer->GetMin() == CurrentLayer->GetMax())
		{
			HeatMapColorRange.Legend.SetDummyValues();
		}
		else
		{
			HeatMapColorRange.Legend.Clear();
			HeatMapColorRange.Legend.SetCaption(1.0f, TruncateAfterDot(std::to_string(CurrentLayer->GetMin() + (CurrentLayer->GetMax() - CurrentLayer->GetMin()) * HeatMapColorRange.GetSliderValue())));
			const float MiddleOfUsedRange = (HeatMapColorRange.GetSliderValue() + CurrentLayer->MinVisible / CurrentLayer->GetMax()) / 2.0f;
			HeatMapColorRange.Legend.SetCaption(0.0f, TruncateAfterDot(std::to_string(CurrentLayer->MinVisible)));
		}
	}

	HeatMapColorRange.Render(bScreenshotMode);

	if (bScreenshotMode)
	{
		ImGui::End();
		ImGui::PopStyleVar(2);
		return;
	}

	static char CurrentRugosityMax[1024];
	static float LastValue = HeatMapColorRange.GetSliderValue();
	if (IsActiveObjectAndLayerValid())
	{
		DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
		if (CurrentLayer->GetMin() == CurrentLayer->GetMax())
		{
			HeatMapColorRange.Legend.SetDummyValues();
		}
		else
		{
			if (abs(CurrentLayer->GetMax()) < 100000 && LastValue != HeatMapColorRange.GetSliderValue())
			{
				LastValue = HeatMapColorRange.GetSliderValue();
				strcpy_s(CurrentRugosityMax, TruncateAfterDot(std::to_string(CurrentLayer->GetMin() + (CurrentLayer->GetMax() - CurrentLayer->GetMin()) * HeatMapColorRange.GetSliderValue())).c_str());
			}

			HeatMapColorRange.Legend.Clear();
			HeatMapColorRange.Legend.SetCaption(1.0f, "max: " + TruncateAfterDot(std::to_string(CurrentLayer->GetMax())));

			HeatMapColorRange.Legend.SetCaption(HeatMapColorRange.GetSliderValue(), /*"current: " +*/ TruncateAfterDot(std::to_string(CurrentLayer->GetMin() + (CurrentLayer->GetMax() - CurrentLayer->GetMin()) * HeatMapColorRange.GetSliderValue())));

			const float MiddleOfUsedRange = (HeatMapColorRange.GetSliderValue() + CurrentLayer->MinVisible / CurrentLayer->GetMax()) / 2.0f;
			HeatMapColorRange.Legend.SetCaption(0.0f, "min: " + TruncateAfterDot(std::to_string(CurrentLayer->MinVisible)));

			CurrentLayer->MaxVisible = CurrentLayer->GetMin() + (CurrentLayer->GetMax() - CurrentLayer->GetMin()) * HeatMapColorRange.GetSliderValue();
		}
	}

	if (!IsActiveObjectAndLayerValid())
		ImGui::BeginDisabled();

	ImGui::SetCursorPosX(10);
	ImGui::SetCursorPosY(642);
	ImGui::SetNextItemWidth(62);
	if (ImGui::InputText("##CurrentRugosityMax", CurrentRugosityMax, IM_ARRAYSIZE(CurrentRugosityMax), ImGuiInputTextFlags_EnterReturnsTrue) ||
		ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##CurrentRugosityMax"))
	{
		
	}

	ImGui::SameLine();
	if (ImGui::Button("Set", ImVec2(62, 19)))
	{
		DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();

		float NewValue = float(atof(CurrentRugosityMax));
		if (NewValue < ActiveLayer->GetMin())
			NewValue = ActiveLayer->GetMin();

		HeatMapColorRange.SetSliderValue((NewValue - ActiveLayer->GetMin()) / float(ActiveLayer->GetMax() - ActiveLayer->GetMin()));
	}

	if (!IsActiveObjectAndLayerValid())
		ImGui::EndDisabled();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

void UIManager::GetUsableSpaceForLayerList(ImVec2& UsableSpaceStart, ImVec2& UsableSpaceEnd)
{
	ImGuiWindow* SettingsWindow = ImGui::FindWindowByName("Settings");
	ImGuiWindow* LegendWindow = ImGui::FindWindowByName("Heat map legend");

	UsableSpaceStart = ImVec2(0.0f, 0.0f);
	UsableSpaceEnd = ImVec2(static_cast<float>(APPLICATION.GetMainWindow()->GetWidth()), static_cast<float>(APPLICATION.GetMainWindow()->GetHeight()));
	if (SettingsWindow != nullptr && LegendWindow != nullptr)
	{
		UsableSpaceStart.x = LegendWindow->Pos.x + LegendWindow->SizeFull.x;
		UsableSpaceStart.y = 20;

		UsableSpaceEnd.x = SettingsWindow->Pos.x;
		UsableSpaceEnd.y = static_cast<float>(APPLICATION.GetMainWindow()->GetHeight() - 20);
	}
}

ImVec2 UIManager::GetLayerListButtonSize(std::string ButtonText)
{
	ImVec2 Result;

	ImVec2 TextSize = ImGui::CalcTextSize(ButtonText.c_str());
	Result.x = TextSize.x + GImGui->Style.FramePadding.x * 2.0f;
	Result.y = TextSize.y + GImGui->Style.FramePadding.y * 2.0f;

	return TextSize;
}

int UIManager::TotalWidthNeededForLayerList(int ButtonUsed)
{
	int Result = 0;
	const int ButtonSpacing = 6;
	const int FirstLastButtonPadding = 4;

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return Result;

	ButtonUsed = static_cast<int>(std::min(size_t(ButtonUsed), ActiveObject->Layers.size()));

	if (ButtonUsed == 0)
		return Result;

	Result += static_cast<int>(GetLayerListButtonSize("No Layer").x + 16);
	for (int i = 0; i < ButtonUsed; i++)
	{
		Result += static_cast<int>(GetLayerListButtonSize(ActiveObject->Layers[i]->GetCaption()).x + ButtonSpacing * 2.0f + 4);
	}

	Result += static_cast<int>(FirstLastButtonPadding * 2.0f);
	
	return Result;
}

void UIManager::RenderLayerTabs()
{
	static int RowCount = 1;
	const int ButtonSpacing = 6;
	const int FirstLastButtonPadding = 4;
	const int RowHeight = 26;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImVec2 UsableSpaceStart, UsableSpaceEnd;
	GetUsableSpaceForLayerList(UsableSpaceStart, UsableSpaceEnd);

	int UsableSpaceCenter = static_cast<int>(UsableSpaceStart.x + (UsableSpaceEnd.x - UsableSpaceStart.x) / 2.0f);
	int UsableSpaceWidth = static_cast<int>(UsableSpaceEnd.x - UsableSpaceStart.x);

	int TotalWidthNeeded = 0;
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject != nullptr)
		TotalWidthNeeded = TotalWidthNeededForLayerList(static_cast<int>(ActiveObject->Layers.size()));

	if (TotalWidthNeeded == 0)
	{
		if (ActiveObject == nullptr)
		{
			TotalWidthNeeded = static_cast<int>(ImGui::CalcTextSize(NoDataText.c_str()).x + 18);
		}
		else
		{
			TotalWidthNeeded = static_cast<int>(GetLayerListButtonSize("No Layer").x + 16);
			TotalWidthNeeded += static_cast<int>(FirstLastButtonPadding * 2.0f);
		}
	}
	else if (TotalWidthNeeded > UsableSpaceWidth - 10.0f)
	{
		TotalWidthNeeded = static_cast<int>(UsableSpaceWidth - 10.0f);
	}

	const float CurrentWindowW = static_cast<float>(TotalWidthNeeded);
	const float CurrentWindowH = 6.0f + RowHeight * RowCount;

	ImVec2 LayerListWindowPosition = ImVec2(UsableSpaceCenter - CurrentWindowW / 2.0f, 21);
	ImGui::SetNextWindowPos(LayerListWindowPosition);
	ImGui::SetNextWindowSize(ImVec2(CurrentWindowW, CurrentWindowH));
	ImGui::Begin("Layers", nullptr, ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoCollapse |
									ImGuiWindowFlags_NoScrollbar |
									ImGuiWindowFlags_NoTitleBar);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
	if (ActiveObject == nullptr)
	{
		ImVec2 TextSize = ImGui::CalcTextSize(NoDataText.c_str());

		ImGui::SetCursorPos(ImVec2(CurrentWindowW / 2.0f - TextSize.x / 2.0f, CurrentWindowH / 2.0f - TextSize.y / 2.0f));
		ImGui::Text(NoDataText.c_str());

		ImGui::PopStyleVar(3);
		ImGui::End();

		return;
	}

	int CurrentRow = 0;
	int PreviousButtonWidth = 0;
	int YPosition = static_cast<int>(ImGui::GetCursorPosY() + 7);
	for (int i = -1; i < int(ActiveObject->Layers.size()); i++)
	{
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 0.6f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.5f, 0.7f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.0f, 1.6f, 0.6f));

		if (LAYER_MANAGER.GetActiveLayerIndex() == i)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(0.0f, 1.0f, 0.5f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1.0f, 1.6f, 1.6f));
		}

		if (i == -1)
		{
			ImGui::SetCursorPosY(static_cast<float>(YPosition));
			if (ImGui::Button("No Layer"))
				ActiveObject->ClearActiveLayer();
		}
		else
		{
			if (ImGui::GetCursorPosX() + GetLayerListButtonSize(ActiveObject->Layers[i]->GetCaption()).x + ButtonSpacing * 2.0f + 4 > (UsableSpaceWidth - 10))
			{
				ImGui::SetCursorPosX(FirstLastButtonPadding * 2.0f);
				CurrentRow++;
			}

			ImGui::SetCursorPosY(static_cast<float>(YPosition + CurrentRow * RowHeight));
			if (ImGui::Button((ActiveObject->Layers[i]->GetCaption() + "##" + std::to_string(i)).c_str()))
				ActiveObject->SetActiveLayer(ActiveObject->Layers[i]->GetID());
		}
		
		ImGui::PopStyleColor(4);
	}

	RowCount = CurrentRow + 1;

	ImGui::PopStyleVar(3);
	ImGui::End();

	LayerListWindowPosition.x += CurrentWindowW / 2.0f - 48 / 2.0f;
	LayerListWindowPosition.y += CurrentWindowH;
	ImGui::SetNextWindowPos(LayerListWindowPosition);
	ImGui::SetNextWindowSize(ImVec2(48, 48));
	if (ImGui::Begin("AddNewLayerIconWindow", nullptr, ImGuiWindowFlags_NoMove |
													   ImGuiWindowFlags_NoResize |
													   ImGuiWindowFlags_NoCollapse |
													   ImGuiWindowFlags_NoScrollbar |
													   ImGuiWindowFlags_NoTitleBar |
	                                                   ImGuiWindowFlags_NoBackground))
	{

		ImVec4 Transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, Transparent);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		if (ImGui::ImageButton("AddNewLayerButton", AddNewLayerIcon->GetTextureID(), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1)))
		{
			NEW_LAYER_WINDOW.Show();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Add new layer");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
	ImGui::End();
}

void UIManager::SwitchCameraMode(bool bModelCamera, glm::vec3 ModelCameraFocusPoint)
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr)
		return;

	if (bModelCamera)
	{
		std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Model view camera prefab");
		if (CameraPrefab.empty())
			return;

		std::vector<std::string> EntitiesIDList = CameraPrefab[0]->GetScene()->GetEntityIDList();
		if (EntitiesIDList.empty())
			return;

		for (size_t i = 0; i < EntitiesIDList.size(); i++)
		{
			FEEntity* CurrentEntity = CameraPrefab[0]->GetScene()->GetEntity(EntitiesIDList[i]);
			if (CurrentEntity == nullptr)
				continue;

			if (CurrentEntity->HasComponent<FECameraComponent>() && CurrentEntity->HasComponent<FENativeScriptComponent>())
			{
				CameraEntity->RemoveComponent<FENativeScriptComponent>();
				CameraEntity->AddComponent<FENativeScriptComponent>();
				NATIVE_SCRIPT_SYSTEM.InitializeScriptComponent(CameraEntity, CurrentEntity->GetComponent<FENativeScriptComponent>().GetModuleID(), "ModelViewCameraController");
				FENativeScriptComponent& NativeScriptComponent = CameraEntity->GetComponent<FENativeScriptComponent>();
				NativeScriptComponent.SetVariableValue("TargetPosition", ModelCameraFocusPoint);

				CameraEntity->GetComponent<FECameraComponent>().SetActive(false);
				return;
			}
		}
	}
	else
	{
		std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Free camera prefab");
		if (CameraPrefab.empty())
			return;

		std::vector<std::string> EntitiesIDList = CameraPrefab[0]->GetScene()->GetEntityIDList();
		if (EntitiesIDList.empty())
			return;

		for (size_t i = 0; i < EntitiesIDList.size(); i++)
		{
			FEEntity* CurrentEntity = CameraPrefab[0]->GetScene()->GetEntity(EntitiesIDList[i]);
			if (CurrentEntity == nullptr)
				continue;

			if (CurrentEntity->HasComponent<FECameraComponent>() && CurrentEntity->HasComponent<FENativeScriptComponent>())
			{
				CameraEntity->RemoveComponent<FENativeScriptComponent>();
				CameraEntity->AddComponent<FENativeScriptComponent>();
				NATIVE_SCRIPT_SYSTEM.InitializeScriptComponent(CameraEntity, CurrentEntity->GetComponent<FENativeScriptComponent>().GetModuleID(), "FreeCameraController");

				CameraEntity->GetComponent<FECameraComponent>().SetActive(false);
				return;
			}
		}
	}
}

void UIManager::AdjustCameraNearFarPlanes()
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr)
		return;

	if (ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() == 0)
		return;

	FEAABB AllObjectsAABB = ANALYSIS_OBJECT_MANAGER.GetAllObjectsAABB();

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.SetNearPlane(AllObjectsAABB.GetLongestAxisLength() * 0.001f);
	CameraComponent.SetFarPlane(AllObjectsAABB.GetLongestAxisLength() * 5.0f);
}

void UIManager::ModelCameraAdjustment(AnalysisObject* Object)
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr)
		return;

	if (ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() == 0)
		return;

	FEAABB AABBToWorkWith = Object == nullptr ? ANALYSIS_OBJECT_MANAGER.GetAllObjectsAABB() : Object->GetAnalysisData()->GetAABB();

	FENativeScriptComponent& NativeScriptComponent = CameraEntity->GetComponent<FENativeScriptComponent>();
	NativeScriptComponent.SetVariableValue("DistanceToModel", AABBToWorkWith.GetLongestAxisLength() * 1.5f);
	NativeScriptComponent.SetVariableValue("MouseWheelSensitivity", AABBToWorkWith.GetLongestAxisLength() * 0.1f);
}

void UIManager::FreeCameraAdjustment(AnalysisObject* Object)
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr)
		return;

	if (ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() == 0)
		return;

	FEAABB AABBToWorkWith = Object == nullptr ? ANALYSIS_OBJECT_MANAGER.GetAllObjectsAABB() : Object->GetAnalysisData()->GetAABB();

	FETransformComponent& TransformComponent = CameraEntity->GetComponent<FETransformComponent>();
	TransformComponent.SetPosition(glm::vec3(0.0f, 0.0f, AABBToWorkWith.GetLongestAxisLength() * 1.5f));
	TransformComponent.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

	FENativeScriptComponent& NativeScriptComponent = CameraEntity->GetComponent<FENativeScriptComponent>();
	NativeScriptComponent.SetVariableValue("MovementSpeed", AABBToWorkWith.GetLongestAxisLength() / 5.0f);
}

void UIManager::SetIsModelCamera(const bool NewValue, glm::vec3 ModelCameraFocusPoint)
{
	bChooseCameraFocusPointMode = false;

	SwitchCameraMode(NewValue, ModelCameraFocusPoint);
	AdjustCameraNearFarPlanes();
	NewValue ? ModelCameraAdjustment() : FreeCameraAdjustment();

	bModelCamera = NewValue;
}

void UIManager::UpdateHistogramData(DataLayer* FromLayer, int NewBinCount)
{
	std::vector<double> Values;
	std::vector<double> Weights;

	if (FromLayer->ValueWeightAndIndex.empty())
		return;

	for (const auto& Tuple : FromLayer->ValueWeightAndIndex)
	{
		Values.push_back(std::get<0>(Tuple));
		Weights.push_back(std::get<1>(Tuple));
	}

	Histogram.FillDataBins(Values, Weights, NewBinCount);
}

void UIManager::RenderHistogramWindow()
{
	bool bLayerWithOneValue = false;
	if (IsActiveObjectAndLayerValid())
	{
		if (LAYER_MANAGER.GetActiveLayer()->GetMin() == LAYER_MANAGER.GetActiveLayer()->GetMax())
			bLayerWithOneValue = true;
	}

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	// FIX ME: Should it work only with meshes?
	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());

	static float LastWindowW = 0.0f;
	static float LastWindowH = 0.0f;
	static float Epsilon = 0.001f;

	const ImGuiWindow* HistogramWindow = ImGui::FindWindowByName("Histogram");
	if (HistogramWindow != nullptr)
	{
		if (std::abs(LastWindowW - HistogramWindow->SizeFull.x) > Epsilon || std::abs(LastWindowH - HistogramWindow->SizeFull.y) > Epsilon)
		{
			Histogram.SetSize(ImVec2(HistogramWindow->SizeFull.x - 40, HistogramWindow->SizeFull.y - Histogram.GetPosition().y - 50.0f));

			HistogramSelectRegionMin.SetAvailableRange(Histogram.GetSize().x - 1);
			HistogramSelectRegionMax.SetAvailableRange(Histogram.GetSize().x - 1);

			HistogramSelectRegionMin.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMin.GetRangePosition(), 0.0f));
			HistogramSelectRegionMax.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMax.GetRangePosition(), 0.0f));

			if (ActiveMesh != nullptr &&
				bHistogramPixelBins &&
				LAYER_MANAGER.GetActiveLayerIndex() != -1)
			{
				UpdateHistogramData(LAYER_MANAGER.GetActiveLayer(), Histogram.GetCurrentBinCount());
			}

			LastWindowW = HistogramWindow->SizeFull.x;
			LastWindowH = HistogramWindow->SizeFull.y;
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Histogram", nullptr))
	{
		if (!IsActiveObjectAndLayerValid() || bLayerWithOneValue)
			ImGui::BeginDisabled();
		
		ImGui::SetCursorPos(ImVec2(12.0f, 30.0f));
		if (ImGui::Checkbox("Select region mode", &bHistogramSelectRegionMode))
		{
			if (ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
			{
				Histogram.SetPosition(ImVec2(Histogram.GetPosition().x, bHistogramSelectRegionMode ? 80.0f : 60.0f));
				Histogram.SetSize(ImVec2(HistogramWindow->SizeFull.x - 40, HistogramWindow->SizeFull.y - Histogram.GetPosition().y - 50.0f));

				if (bHistogramSelectRegionMode)
				{
					HistogramSelectRegionMin.SetStartPosition(Histogram.GetPosition());
					HistogramSelectRegionMin.SetAvailableRange(Histogram.GetSize().x - 1);

					HistogramSelectRegionMax.SetStartPosition(Histogram.GetPosition() + ImVec2(1.0f, 0.0f));
					HistogramSelectRegionMax.SetAvailableRange(Histogram.GetSize().x - 1);
					HistogramSelectRegionMax.SetRangePosition(1.0f);
					HistogramSelectRegionMax.SetPixelPosition(ImVec2(Histogram.GetSize().x, 0.0f));
				}
				else
				{
					HistogramSelectRegionMin.SetRangePosition(0.0f);
					HistogramSelectRegionMin.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMin.GetRangePosition(), 0.0f));

					HistogramSelectRegionMax.SetRangePosition(1.0f);
					HistogramSelectRegionMax.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMax.GetRangePosition(), 0.0f));

					LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMin(0.0f);
					LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMax(0.0f);
				}
			}
		}

		if (IsActiveObjectAndLayerValid() && !bLayerWithOneValue)
			Histogram.Render();

		if (bHistogramSelectRegionMode && ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
		{
			HistogramSelectRegionMin.Render();
			HistogramSelectRegionMax.Render();

			if (HistogramSelectRegionMin.GetRangePosition() + 0.001f >= HistogramSelectRegionMax.GetRangePosition())
			{
				HistogramSelectRegionMin.SetRangePosition(HistogramSelectRegionMax.GetRangePosition() - 0.001f);
				HistogramSelectRegionMin.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMin.GetRangePosition(), 0.0f));
			}

			if (HistogramSelectRegionMax.GetRangePosition() - 0.001f < HistogramSelectRegionMin.GetRangePosition())
			{
				HistogramSelectRegionMax.SetRangePosition(HistogramSelectRegionMax.GetRangePosition() + 0.001f);
				HistogramSelectRegionMax.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMax.GetRangePosition(), 0.0f));
			}

			// Render a text about what percentage of the area is selected
			float MinValueSelected = LAYER_MANAGER.GetActiveLayer()->GetMin() + (LAYER_MANAGER.GetActiveLayer()->GetMax() - LAYER_MANAGER.GetActiveLayer()->GetMin()) * HistogramSelectRegionMin.GetRangePosition();
			float MaxValueSelected = LAYER_MANAGER.GetActiveLayer()->GetMin() + (LAYER_MANAGER.GetActiveLayer()->GetMax() - LAYER_MANAGER.GetActiveLayer()->GetMin()) * HistogramSelectRegionMax.GetRangePosition();

			glm::vec2 MinValueDistribution = CalculateAreaDistributionAtValue(LAYER_MANAGER.GetActiveLayer(), MinValueSelected);
			glm::vec2 MaxValueDistribution = CalculateAreaDistributionAtValue(LAYER_MANAGER.GetActiveLayer(), MaxValueSelected);
			AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
			float PercentageOfAreaSelected = 0.0f;
			if (ActiveObject != nullptr)
			{
				MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
				if (CurrentMeshAnalysisData != nullptr)
				{
					PercentageOfAreaSelected = static_cast<float>((MaxValueDistribution.x / CurrentMeshAnalysisData->GetTotalArea() * 100.0) - (MinValueDistribution.x / CurrentMeshAnalysisData->GetTotalArea() * 100.0));
				}
			}

			ImGui::SetCursorPos(ImVec2(200.0f, 33.0f));
			std::string CurrentText = "Selected area: " + TruncateAfterDot(std::to_string(PercentageOfAreaSelected), 3) + " %%";
			ImGui::Text(CurrentText.c_str());

			// Render text that corresponds to the min value
			ImGui::SetCursorPos(Histogram.GetPosition() + HistogramSelectRegionMin.GetPixelPosition() - ImVec2(HistogramSelectRegionMin.GetSize() * 0.90f, HistogramSelectRegionMin.GetSize() * 1.65f));
			std::string MinValue = TruncateAfterDot(std::to_string(MinValueSelected), 2);
			ImGui::Text(MinValue.c_str());

			// Line that corresponds to the min value
			ImVec2 ArrowPosition = HistogramWindow->Pos + Histogram.GetPosition() + HistogramSelectRegionMin.GetPixelPosition();
			ImGui::GetWindowDrawList()->AddRectFilled(ArrowPosition - ImVec2(1.0f, 0.0f),
													  ArrowPosition + ImVec2(1.0f, Histogram.GetSize().y - 1.0f),
													  ImColor(56.0f / 255.0f, 205.0f / 255.0f, 137.0f / 255.0f, 165.0f / 255.0f));

			// Render text that corresponds to the min value
			ImGui::SetCursorPos(Histogram.GetPosition() + HistogramSelectRegionMax.GetPixelPosition() - ImVec2(HistogramSelectRegionMax.GetSize() * 0.90f, HistogramSelectRegionMax.GetSize() * 1.65f));
			std::string MaxValue = TruncateAfterDot(std::to_string(MaxValueSelected), 2);
			ImGui::Text(MaxValue.c_str());

			// Line that corresponds to the max value
			ArrowPosition = HistogramWindow->Pos + Histogram.GetPosition() + HistogramSelectRegionMax.GetPixelPosition();
			ImGui::GetWindowDrawList()->AddRectFilled(ArrowPosition - ImVec2(1.0f, 0.0f),
													  ArrowPosition + ImVec2(1.0f, Histogram.GetSize().y - 1.0f),
													  ImColor(156.0f / 255.0f, 105.0f / 255.0f, 137.0f / 255.0f, 165.0f / 255.0f));
			
			DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
			if (CurrentLayer != nullptr)
			{
				CurrentLayer->SetSelectedRangeMin(HistogramSelectRegionMin.GetRangePosition());
				CurrentLayer->SetSelectedRangeMax(HistogramSelectRegionMax.GetRangePosition());
			}
		}

		bool bInterpolate = Histogram.IsUsingInterpolation();
		if (HistogramWindow != nullptr)
			ImGui::SetCursorPos(ImVec2(10.0f, Histogram.GetPosition().y + Histogram.GetSize().y + 20.0f));

		if (ImGui::Checkbox("Interpolate", &bInterpolate))
			Histogram.SetIsUsingInterpolation(bInterpolate);
		
		if (HistogramWindow != nullptr)
			ImGui::SetCursorPos(ImVec2(130.0f, Histogram.GetPosition().y + Histogram.GetSize().y + 20.0f));
		if (ImGui::Checkbox("BinCount = Pixels", &bHistogramPixelBins))
		{
			int NewBinCount = 128;
			if (bHistogramPixelBins)
				NewBinCount = static_cast<int>(HistogramWindow->SizeFull.x - 20);

			UpdateHistogramData(LAYER_MANAGER.GetActiveLayer(), NewBinCount);
		}

		static char CurrentBinCountChar[1024] = "128";
		if (!bHistogramPixelBins)
		{
			ImGui::SetCursorPosX(290.0f);
			if (HistogramWindow != nullptr)
				ImGui::SetCursorPosY(Histogram.GetPosition().y + Histogram.GetSize().y + 20.0f);
			ImGui::SetNextItemWidth(62);
			if (ImGui::InputText("##BinCount", CurrentBinCountChar, IM_ARRAYSIZE(CurrentBinCountChar), ImGuiInputTextFlags_EnterReturnsTrue) ||
				ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##CurrentBinCountChar"))
			{
				int TempInt = atoi(CurrentBinCountChar);
				if (TempInt <= 0)
					TempInt = 1;

				if (HistogramWindow != nullptr)
				{
					if (TempInt > HistogramWindow->SizeFull.x - 20)
						TempInt = static_cast<int>(HistogramWindow->SizeFull.x - 20);
				}

				if (Histogram.GetCurrentBinCount() != TempInt)
				{
					if (ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
						UpdateHistogramData(LAYER_MANAGER.GetActiveLayer(), TempInt);
				}
			}
		}

		if (!IsActiveObjectAndLayerValid() || bLayerWithOneValue)
			ImGui::EndDisabled();
	}

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

bool UIManager::GetOutputSelectionToFile()
{
	return bOutputSelectionToFile;
}

void UIManager::SetOutputSelectionToFile(const bool NewValue)
{
	bOutputSelectionToFile = NewValue;
}

void UIManager::ApplyStandardWindowsSizeAndPosition()
{
	ImGuiWindow* Window = ImGui::FindWindowByName("Histogram");
	if (Window != nullptr)
	{
		Window->SizeFull.x = APPLICATION.GetMainWindow()->GetWidth() * 0.5f;
		Window->Pos.x = APPLICATION.GetMainWindow()->GetWidth() / 2.0f - Window->SizeFull.x / 2.0f;

		Window->SizeFull.y = APPLICATION.GetMainWindow()->GetHeight() * 0.35f;
		Window->Pos.y = APPLICATION.GetMainWindow()->GetHeight() - 10.0f - Window->SizeFull.y;
	}

	Window = ImGui::FindWindowByName("Settings");
	if (Window != nullptr)
	{
		Window->SizeFull.x = APPLICATION.GetMainWindow()->GetWidth() * 0.3f;
		Window->SizeFull.y = APPLICATION.GetMainWindow()->GetHeight() * 0.7f;
	}
}

void UIManager::ShowAboutDialog()
{
	bShouldOpenAboutWindow = true;
}

void UIManager::RenderAboutWindow()
{
	if (bShouldOpenAboutWindow)
	{
		ImGui::OpenPopup("About");
		bShouldOpenAboutWindow = false;
	}

	float PopupW = 400.0f;
	float PopupH = 135.0f;
	ImGui::SetNextWindowSize(ImVec2(PopupW, PopupH));
	if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		int WindowW = 0;
		int WindowH = 0;
		APPLICATION.GetMainWindow()->GetSize(&WindowW, &WindowH);

		ImGui::SetWindowPos(ImVec2(WindowW / 2.0f - ImGui::GetWindowWidth() / 2.0f, WindowH / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		
		std::string Text = "Version: " + std::to_string(APP_VERSION) + "     date: 09\\12\\2025";
		ImVec2 TextSize = ImGui::CalcTextSize(Text.c_str());
		ImGui::SetCursorPosX(PopupW / 2.0f - TextSize.x / 2.0f);
		ImGui::Text(Text.c_str());

		ImGui::Separator();

		Text = "To submit a bug report or provide feedback, ";
		TextSize = ImGui::CalcTextSize(Text.c_str());
		ImGui::SetCursorPosX(PopupW / 2.0f - TextSize.x / 2.0f);
		ImGui::Text(Text.c_str());

		Text = "please email me at kberegovyi@ccom.unh.edu.";
		TextSize = ImGui::CalcTextSize(Text.c_str());
		ImGui::SetCursorPosX(PopupW / 2.0f - TextSize.x / 2.0f);
		ImGui::Text(Text.c_str());

		ImGui::Separator();

		Text = "UNH CCOM";
		TextSize = ImGui::CalcTextSize(Text.c_str());
		ImGui::SetCursorPosX(PopupW / 2.0f - TextSize.x / 2.0f);
		ImGui::Text(Text.c_str());

		ImGui::SetCursorPosX(PopupW / 2.0f - 210.0f / 2.0f);
		ImGui::SetNextItemWidth(210);
		if (ImGui::Button("Close", ImVec2(210.0f, 20.0f)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

glm::dvec2 UIManager::CalculateAreaDistributionAtValue(DataLayer* Layer, float Value)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return glm::dvec2(0.0);

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return glm::dvec2(0.0);

	if (Layer == nullptr || Layer->ElementsToData.empty() || CurrentMeshAnalysisData->TrianglesArea.empty())
		return glm::dvec2(0.0);

	float FirstBin = 0.0;
	float SecondBin = 0.0;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return glm::dvec2(0.0);

	for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		if (Layer->ElementsToData[i] <= Value)
		{
			FirstBin += float(CurrentMeshAnalysisData->TrianglesArea[i]);
		}
		else
		{
			SecondBin += float(CurrentMeshAnalysisData->TrianglesArea[i]);
		}
	}

	return glm::dvec2(FirstBin, SecondBin);
}

void UIManager::OnLayerChange()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	LAYER_RASTERIZATION_MANAGER.ClearAllData();

	UI.bHistogramSelectRegionMode = false;
	UI.Histogram.Clear();
	UI.HeatMapColorRange.Clear();

	DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
	if (ActiveLayer == nullptr)
		return;

	LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMin(0.0f);
	LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMax(0.0f);

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (ActiveLayer->GetMin() != ActiveLayer->GetMax())
	{
		UI.UpdateHistogramData(ActiveLayer, UI.Histogram.GetCurrentBinCount());

		if (CurrentMeshAnalysisData != nullptr)
			CurrentMeshAnalysisData->SetHeatMapType(5);
		UI.HeatMapColorRange.SetColorRangeFunction(GetTurboColorMap);
		UI.HeatMapColorRange.bRenderSlider = true;
	
		float MiddleOfRange = ActiveLayer->GetMin() + (ActiveLayer->GetMax() - ActiveLayer->GetMin()) / 2.0f;
		UI.HeatMapColorRange.SetSliderValue(MiddleOfRange / ActiveLayer->GetMax());

		if (ActiveLayer->GetType() == LAYER_TYPE::COMPARE)
		{
			UI.HeatMapColorRange.SetColorRangeFunction(CompareColormapValue);
			if (CurrentMeshAnalysisData != nullptr)
				CurrentMeshAnalysisData->SetHeatMapType(6);

			UI.HeatMapColorRange.bRenderSlider = false;
			UI.HeatMapColorRange.SetSliderValue(1.0f);
		}

		float NormalizedPosition = 0.0f;
		const int CaptionsCount = 8;
		const float PositionStep = 1.0f / CaptionsCount;
		for (size_t i = 0; i <= CaptionsCount; i++)
		{
			UI.Histogram.SetLegendCaption(i == 0 ? NormalizedPosition + 0.0075f : NormalizedPosition,
				TruncateAfterDot(std::to_string(ActiveLayer->GetMin() + (ActiveLayer->GetMax() - ActiveLayer->GetMin()) * NormalizedPosition)));

			NormalizedPosition += PositionStep;
		}
	}
	else
	{
		if (CurrentMeshAnalysisData != nullptr)
			CurrentMeshAnalysisData->SetHeatMapType(-1);
	}

	if (UI.GetDebugGrid() != nullptr)
	{
		UI.InitDebugGrid(UI.CurrentJitterStepIndexVisualize);
		UI.UpdateRenderingMode(UI.GetDebugGrid(), UI.GetDebugGrid()->RenderingMode);
	}	
}

std::vector<GridInitData_Jitter> ReadJitterSettingsFromDebugInfo(DataLayerDebugInfo* DebugInfo)
{
	std::vector<GridInitData_Jitter> Result;

	if (DebugInfo == nullptr)
		return Result;

	std::istringstream iss(DebugInfo->ToString());
	std::string Line;
	GridInitData_Jitter CurrentData;
	bool bNewData = true;

	while (std::getline(iss, Line))
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

void UIManager::InitDebugGrid(size_t JitterIndex)
{
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

	GridInitData_Jitter* CurrentSettings = &UsedSettings[JitterIndex];
	FEAABB FinalAABB = JITTER_MANAGER.GetAABBForJitteredGrid(CurrentSettings, CurrentLayerResolutionInM);

	DebugGrid->Init(0, FinalAABB, CurrentLayerResolutionInM);
	DebugGrid->FillCellsWithTriangleInfo();
	DebugGrid->bFullyLoaded = true;
}

void UIManager::RenderLayerSettingsTab()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();

	std::string NoInfoText;
	if (ActiveObject == nullptr)
		NoInfoText = "No object loaded.";

	if (ActiveObject != nullptr && ActiveObject->GetLayerCount() == 0)
		NoInfoText = "Object have no layers.";

	if (ActiveObject != nullptr && !(ActiveObject->GetLayerCount() == 0) && LAYER_MANAGER.GetActiveLayerIndex() == -1)
		NoInfoText = "Layer is not selected.";

	if (!NoInfoText.empty())
	{
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize(NoInfoText.c_str()).x / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize(NoInfoText.c_str()).y / 2.0f);

		ImGui::Text(NoInfoText.c_str());
	}

	if (NoInfoText.empty())
	{
		DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();

		if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
			if (ActiveMesh == nullptr)
				return;

			ImGui::Text("Triangle count: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(ActiveMesh->GetVertexCount() / 3).c_str());
		}
		else if (ActiveObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
		{
			FEPointCloud* ActivePointCloud = static_cast<FEPointCloud*>(ActiveObject->GetEngineResource());
			if (ActivePointCloud == nullptr)
				return;

			ImGui::Text("Point count: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(ActivePointCloud->GetPointCount()).c_str());
		}

		ImGui::Text((std::string("ID: ") + ActiveLayer->GetID()).c_str());
		static char CurrentLayerCaption[1024];
		strcpy_s(CurrentLayerCaption, ActiveLayer->GetCaption().c_str());
		ImGui::Text("Caption: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(160);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
		if (ImGui::InputText("##LayerCaptionEdit", CurrentLayerCaption, IM_ARRAYSIZE(CurrentLayerCaption), ImGuiInputTextFlags_EnterReturnsTrue) ||
			ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##LayerCaptionEdit"))
		{
			ActiveLayer->SetCaption(CurrentLayerCaption);
		}

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.6f, 0.1f, 0.2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.65f, 0.2f, 0.2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.75f, 0.6f, 0.1f));
		ImGui::SameLine();
		if (ImGui::Button("Delete Layer"))
		{
			ActiveObject->RemoveLayer(ActiveLayer->GetID());
			//int IndexToDelete = LAYER_MANAGER.GetActiveLayerIndex();
			//ActiveObject->ClearActiveLayer();

			//LAYER_MANAGER.Layers.erase(LAYER_MANAGER.Layers.begin() + IndexToDelete,
			//	LAYER_MANAGER.Layers.begin() + IndexToDelete + 1);

			ImGui::PopStyleColor(3);
			return;
		}
		ImGui::PopStyleColor(3);

		ImGui::Text("Mean:");
		ImGui::SameLine();
		std::string MeanText = "No data.";
		if (ActiveLayer->GetMean() != -FLT_MAX)
			MeanText = std::to_string(ActiveLayer->GetMean());
		ImGui::Text(MeanText.c_str());

		ImGui::Text("Median:");
		ImGui::SameLine();
		std::string MedianText = "No data.";
		if (ActiveLayer->GetMedian() != -FLT_MAX)
			MedianText = std::to_string(ActiveLayer->GetMedian());
		ImGui::Text(MedianText.c_str());

		ImGui::Text("Notes:");
		static char CurrentLayerUserNotes[10000];
		strcpy_s(CurrentLayerUserNotes, ActiveLayer->GetNote().c_str());
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
		if (ImGui::InputTextMultiline("##Notes", CurrentLayerUserNotes, IM_ARRAYSIZE(CurrentLayerUserNotes)))
			ActiveLayer->SetNote(CurrentLayerUserNotes);
		
		ImGui::Text("Debug Info:");
		static char CurrentLayerDebugInfo[10000];
		std::string DebugInfo;
		if (ActiveLayer->DebugInfo != nullptr)
			DebugInfo = ActiveLayer->DebugInfo->ToString();
		strcpy_s(CurrentLayerDebugInfo, DebugInfo.c_str());
		ImGui::BeginDisabled();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
		ImGui::InputTextMultiline("##DebugInfo", CurrentLayerDebugInfo, IM_ARRAYSIZE(CurrentLayerDebugInfo));
		ImGui::EndDisabled();

		ImGui::Separator();
		ImGui::Text("Distribution : ");
		static char CurrentDistributionEdit[1024];
		static glm::vec2 CurrentDistribution = glm::vec2();
		static float LastDistributionValue = 0.0f;

		ImGui::SetNextItemWidth(62);
		if (ImGui::InputText("##DistributionEdit", CurrentDistributionEdit, IM_ARRAYSIZE(CurrentDistributionEdit), ImGuiInputTextFlags_EnterReturnsTrue) ||
			ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##DistributionEdit"))
		{

		}

		ImGui::SameLine();
		if (ImGui::Button("Calculate Distribution", ImVec2(167, 19)))
		{
			float NewValue = float(atof(CurrentDistributionEdit));
			LastDistributionValue = NewValue;
			CurrentDistribution = CalculateAreaDistributionAtValue(ActiveLayer, NewValue);
		}

		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (ActiveObject == nullptr)
			return;

		MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		if (CurrentDistribution != glm::vec2())
		{
			ImGui::Text(("Area below and at " + TruncateAfterDot(std::to_string(LastDistributionValue)) + " value : " + std::to_string(CurrentDistribution.x / CurrentMeshAnalysisData->GetTotalArea() * 100.0f) + " %%").c_str());
			ImGui::Text(("Area with higher than " + TruncateAfterDot(std::to_string(LastDistributionValue)) + " value : " + std::to_string(CurrentDistribution.y / CurrentMeshAnalysisData->GetTotalArea() * 100.0f) + " %%").c_str());
		}
	}
}

void UIManager::RenderGeneralSettingsTab()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();

	if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		ImGui::Checkbox("Wireframe", &bWireframeMode);

		ImGui::Text("Ambiant light intensity:");
		ImGui::SetNextItemWidth(150);
		ImGui::DragFloat("##AmbiantLightScale", &AmbientLightFactor, 0.025f);
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			AmbientLightFactor = 2.2f;
		}
	}

	bool TempBool = bModelCamera;
	if (ImGui::Checkbox("Model camera", &TempBool))
	{
		SetIsModelCamera(TempBool);
	}

	if (bModelCamera && bChooseCameraFocusPointMode && ImGui::IsMouseReleased(0) && ActiveEntity != nullptr)
	{
		glm::dvec3 IntersectionPoint = ANALYSIS_OBJECT_MANAGER.IntersectTriangle(MAIN_SCENE_MANAGER.GetMouseRayDirection());

		IntersectionPoint = glm::dvec3(ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(IntersectionPoint, 1.0));
		if (IntersectionPoint != glm::dvec3(0.0))
		{
			SetIsModelCamera(true, IntersectionPoint);
		}
	}

	if (bModelCamera)
	{
		if (ImGui::Button("Set point on model as a focus point"))
			bChooseCameraFocusPointMode = true;

		FENativeScriptComponent& NativeScriptComponent = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FENativeScriptComponent>();
		float ModelCameraMouseWheelSensitivity = 0.0f;
		NativeScriptComponent.GetVariableValue<float>("MouseWheelSensitivity", ModelCameraMouseWheelSensitivity);
		ImGui::DragFloat("Mouse wheel sensitivity", &ModelCameraMouseWheelSensitivity, 0.0001f, 0.000001f, 100.0f);
		NativeScriptComponent.SetVariableValue("MouseWheelSensitivity", ModelCameraMouseWheelSensitivity);
	}

	ShowCameraTransform();

	ImGui::Separator();
	TempBool = IsInDeveloperMode();
	if (ImGui::Checkbox("Developer mode", &TempBool))
		SetDeveloperMode(TempBool);


	if (!IsInDeveloperMode())
	{
		if (DebugGrid != nullptr && DebugGrid->RenderingMode != 0)
			UpdateRenderingMode(DebugGrid, 0);
	}
	
	/*if (IsInDeveloperMode())
	{
		int TempValue = JITTER_MANAGER.GetDebugJitterToDoCount();
		ImGui::Text("Jitter count for next calculation(set -1 to run all jitters): ");
		ImGui::SetNextItemWidth(120);
		ImGui::DragInt("##Jitter count", &TempValue);
		JITTER_MANAGER.SetDebugJitterToDoCount(TempValue);
	}*/

	if (IsInDeveloperMode() && LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		if (DebugGrid == nullptr)
			UI.InitDebugGrid(JITTER_MANAGER.GetJitterToDoCount() - 1);

		if (DebugGrid != nullptr)
		{
			std::vector<GridInitData_Jitter> UsedSettings;
			UsedSettings = ReadJitterSettingsFromDebugInfo(LAYER_MANAGER.GetActiveLayer()->DebugInfo);

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
						bool is_selected = (CurrentJitterStepIndexVisualize == i);
						if (ImGui::Selectable(std::to_string(i).c_str(), is_selected))
						{
							CurrentJitterStepIndexVisualize = static_cast<int>(i);
							int LastGridRendetingMode = DebugGrid->RenderingMode;

							InitDebugGrid(CurrentJitterStepIndexVisualize);

							DebugGrid->RenderingMode = LastGridRendetingMode;
							if (DebugGrid->RenderingMode == 1)
							{
								UpdateRenderingMode(DebugGrid, 1);
							}
						}

						if (is_selected)
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

			if (ImGui::RadioButton("Do not draw", &DebugGrid->RenderingMode, 0))
			{
				UpdateRenderingMode(DebugGrid, 0);
			}

			if (ImGui::RadioButton("Show cells with triangles", &DebugGrid->RenderingMode, 1))
			{
#ifdef NEW_LINES
				InitDebugGrid(CurrentJitterStepIndexVisualize);
#endif
				UpdateRenderingMode(DebugGrid, 1);
			}

			if (ImGui::RadioButton("Show all cells", &DebugGrid->RenderingMode, 2))
			{
				UpdateRenderingMode(DebugGrid, 2);
			}

#ifdef NEW_LINES
			if (DebugGrid->RenderingMode == 1)
			{
				DebugGrid->AddLinesOfGrid();
			}
#endif

			if (DebugGrid->RenderingMode == 1)
			{
				DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
				if (CurrentLayer == nullptr)
					return;

				switch (CurrentLayer->GetType())
				{
				case LAYER_TYPE::RUGOSITY:
				{
					//RUGOSITY_MANAGER.RenderDebugInfoForSelectedNode(DebugGrid);
					break;
				}

				case LAYER_TYPE::VECTOR_DISPERSION:
				{
					//VECTOR_DISPERSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(DebugGrid);
					break;
				}

				case LAYER_TYPE::FRACTAL_DIMENSION:
				{
					FRACTAL_DIMENSION_LAYER_PRODUCER.RenderDebugInfoWindow(DebugGrid);
					break;
				}

				default:
					break;
				}
			}

			ImGui::Separator();
		}
	}
}

void UIManager::RasterizationSettingsUI()
{
	if (LAYER_MANAGER.GetActiveLayer() == nullptr)
		ImGui::BeginDisabled();

	bool bNeedUpdate = false;

	const char* RasterizationModes[] = { "Min", "Max", "Mean", "Cumulative" };
	int TempInt = LAYER_RASTERIZATION_MANAGER.GetGridRasterizationMode();
	ImGui::Text("Mode: ");
	ImGui::SetNextItemWidth(128);
	if (ImGui::Combo("##Mode", &TempInt, RasterizationModes, IM_ARRAYSIZE(RasterizationModes)))
	{
		if (TempInt != LayerRasterizationManager::GridRasterizationMode::Cumulative)
			LAYER_RASTERIZATION_MANAGER.ActivateAutomaticOutliersSuppression();
		
		bNeedUpdate = true;
		LAYER_RASTERIZATION_MANAGER.SetGridRasterizationMode(static_cast<LayerRasterizationManager::GridRasterizationMode>(TempInt));
	}

	float TempFloat = LAYER_RASTERIZATION_MANAGER.GetResolutionInMeters();
	glm::vec2 MinMax = LAYER_RASTERIZATION_MANAGER.GetMinMaxResolutionInMeters();
	
	ImGui::Text("Choose resolution in meters: ");
	ImGui::Text(("Min value : "
				+ std::to_string(MinMax.y)
				+ " m \nMax value : "
				+ std::to_string(MinMax.x) + " m").c_str());
	static bool bSliderResolutionValueChanged = false;
	static float SliderResolutionNewValue = 0.0f;
	if (ImGui::SliderFloat("##Resolution in meters", &TempFloat, MinMax.y, MinMax.x))
	{
		bSliderResolutionValueChanged = true;
		SliderResolutionNewValue = TempFloat;
	}

	static char CustomResolutionInM[512];
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	ImGui::Text("Input exact value: ");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	ImGui::SetNextItemWidth(128);
	ImGui::InputText("##ResolutionInM", CustomResolutionInM, IM_ARRAYSIZE(CustomResolutionInM));
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::Button("Apply"))
	{
		TempFloat = static_cast<float>(atof(CustomResolutionInM));
		bNeedUpdate = true;
		LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(TempFloat);

		float NewResolution = LAYER_RASTERIZATION_MANAGER.GetResolutionInMeters();
		strcpy_s(CustomResolutionInM, std::to_string(NewResolution).c_str());
	}

	if (bSliderResolutionValueChanged && ImGui::IsMouseReleased(0))
	{
		bNeedUpdate = true;
		bSliderResolutionValueChanged = false;
		LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(SliderResolutionNewValue);
	}

	ImGui::Text(std::string("Output resolution: " + std::to_string(LAYER_RASTERIZATION_MANAGER.GetResolutionInPixelsThatWouldGiveSuchResolutionInMeters(TempFloat))).c_str());

	glm::vec3 ForceProjectionVector = LAYER_RASTERIZATION_MANAGER.GetProjectionVector();
	int SelectedAxis = ForceProjectionVector == glm::vec3(1.0f, 0.0f, 0.0f) ? 0 : ForceProjectionVector == glm::vec3(0.0f, 1.0f, 0.0f) ? 1 : 2;
	ImGui::Text("Select the axis along which the layer should be projected: ");
	if (ImGui::RadioButton("X", &SelectedAxis, 0))
	{
		bNeedUpdate = true;
		ForceProjectionVector = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	ImGui::SameLine();
	if (ImGui::RadioButton("Y", &SelectedAxis, 1))
	{
		bNeedUpdate = true;
		ForceProjectionVector = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	ImGui::SameLine();
	if (ImGui::RadioButton("Z", &SelectedAxis, 2))
	{
		bNeedUpdate = true;
		ForceProjectionVector = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	if (LAYER_RASTERIZATION_MANAGER.GetGridRasterizationMode() == LayerRasterizationManager::GridRasterizationMode::Cumulative)
	{
		static bool bAutomaticPercentOfAreaThatWouldBeRed = true;
		if (ImGui::Checkbox("Automatic outliers suppression", &bAutomaticPercentOfAreaThatWouldBeRed))
		{
			if (bAutomaticPercentOfAreaThatWouldBeRed)
			{
				bNeedUpdate = true;
				LAYER_RASTERIZATION_MANAGER.ActivateAutomaticOutliersSuppression();
			}
		}

		if (bAutomaticPercentOfAreaThatWouldBeRed)
			ImGui::BeginDisabled();

		TempFloat = LAYER_RASTERIZATION_MANAGER.GetCumulativeModePercentOfAreaThatWouldBeRed();
		static bool bSliderThresholdValueChanged = false;
		static float SliderThresholdNewValue = 0.0f;
		ImGui::Text("Choose percent of area that would be above color scale threshold(red): ");
		if (ImGui::SliderFloat("##Percent of area that would be above color scale threshold(red)", &TempFloat, 0.0f, 99.9f))
		{
			bSliderThresholdValueChanged = true;
			SliderThresholdNewValue = TempFloat;
		}

		if (bSliderThresholdValueChanged && ImGui::IsMouseReleased(0))
		{
			bNeedUpdate = true;
			bSliderThresholdValueChanged = false;
			LAYER_RASTERIZATION_MANAGER.SetCumulativeModePercentOfAreaThatWouldBeRed(SliderThresholdNewValue);
		}

		if (bAutomaticPercentOfAreaThatWouldBeRed)
			ImGui::EndDisabled();
	}

	std::string TextForButton = "Activate preview";
	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() == -1)
	{
		if (ImGui::Button(TextForButton.c_str()))
		{
			LAYER_RASTERIZATION_MANAGER.PrepareLayerForExport(LAYER_MANAGER.GetActiveLayer());
		}
	}

	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() != -1)
	{
		if (ImGui::Button("Save to file..."))
		{
			LAYER_RASTERIZATION_MANAGER.PromptUserForSaveLocation();
		}
	}

	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() != -1)
	{
		float CurrentWindowWidth = ImGui::GetWindowWidth() * 0.95f;
		ImGui::Text("Preview:");
		ImGui::Image((void*)(intptr_t)LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID(), ImVec2(CurrentWindowWidth, CurrentWindowWidth));
	}

	if (LAYER_MANAGER.GetActiveLayer() == nullptr)
		ImGui::EndDisabled();

	if (bNeedUpdate)
	{
		LAYER_RASTERIZATION_MANAGER.PrepareLayerForExport(LAYER_MANAGER.GetActiveLayer(), ForceProjectionVector);
	}
}

void UIManager::RenderExportTab()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	

	if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;
		
		ImGui::Text("Selection mode:");
		if (ImGui::RadioButton("None", &LayerSelectionMode, 0))
		{
			CurrentMeshAnalysisData->TriangleSelected.clear();
			LINE_RENDERER.ClearAll();
			LINE_RENDERER.SyncWithGPU();
		}

		if (ImGui::RadioButton("Triangles", &LayerSelectionMode, 1))
		{
			CurrentMeshAnalysisData->TriangleSelected.clear();
			LINE_RENDERER.ClearAll();
			LINE_RENDERER.SyncWithGPU();
		}

		if (ImGui::RadioButton("Area", &LayerSelectionMode, 2))
		{
			CurrentMeshAnalysisData->TriangleSelected.clear();
			LINE_RENDERER.ClearAll();
			LINE_RENDERER.SyncWithGPU();
		}

		if (LayerSelectionMode == 2)
		{
			ImGui::Text("Radius of area to measure: ");
			ImGui::SetNextItemWidth(128);
			ImGui::DragFloat("##RadiusOfAreaToMeasure", &RadiusOfAreaToMeasure, 0.01f);
			if (RadiusOfAreaToMeasure < 0.1f)
				RadiusOfAreaToMeasure = 0.1f;

			ImGui::Checkbox("Output selection data to file", &bOutputSelectionToFile);
		}

		DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
		if (CurrentMeshAnalysisData->TriangleSelected.size() == 1 && ActiveLayer != nullptr)
		{
			ImGui::Separator();
			ImGui::Text("Selected triangle information :");

			std::string Text = "Triangle value : ";
			Text += std::to_string(ActiveLayer->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[0]]);
			ImGui::Text(Text.c_str());

			int HeightLayerIndex = -1;
			for (size_t i = 0; i < ActiveObject->Layers.size(); i++)
			{
				if (ActiveObject->Layers[i]->GetCaption() == "Height")
					HeightLayerIndex = static_cast<int>(i);
			}

			Text = "Triangle height : ";
			double AverageHeight = 0.0;
			if (HeightLayerIndex != -1)
			{
				AverageHeight = ActiveObject->Layers[HeightLayerIndex]->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[0]];
				AverageHeight -= ActiveObject->Layers[HeightLayerIndex]->GetMin();
			}

			Text += std::to_string(AverageHeight);
			ImGui::Text(Text.c_str());
		}
		else if (CurrentMeshAnalysisData->TriangleSelected.size() > 1 && ActiveLayer != nullptr)
		{
			std::string Text = "Area average value : ";
			float TotalRugosity = 0.0f;
			for (size_t i = 0; i < CurrentMeshAnalysisData->TriangleSelected.size(); i++)
			{
				TotalRugosity += ActiveLayer->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[i]];
			}

			TotalRugosity /= CurrentMeshAnalysisData->TriangleSelected.size();
			Text += std::to_string(TotalRugosity);

			ImGui::Text(Text.c_str());

			Text = "Area average height : ";
			double AverageHeight = 0.0;

			int HeightLayerIndex = -1;
			for (size_t i = 0; i < ActiveObject->Layers.size(); i++)
			{
				if (ActiveObject->Layers[i]->GetCaption() == "Height")
					HeightLayerIndex = static_cast<int>(i);
			}

			if (HeightLayerIndex != -1)
			{
				DataLayer* HeightLayer = ActiveObject->Layers[HeightLayerIndex];
				for (size_t i = 0; i < CurrentMeshAnalysisData->TriangleSelected.size(); i++)
				{
					double CurrentHeight = HeightLayer->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[i]];
					AverageHeight += CurrentHeight;
				}

				AverageHeight /= CurrentMeshAnalysisData->TriangleSelected.size();
				AverageHeight -= HeightLayer->GetMin();
			}

			Text += std::to_string(AverageHeight);

			ImGui::Text(Text.c_str());
		}
	}

	ImGui::Separator();

	ImGui::Text("Screenshoot:");
	ImGui::Checkbox("Transparent background", &bUseTransparentBackground);

	if (ImGui::Button("Take screenshoot"))
	{
		bNextFrameForScreenshot = true;
	}

	ImGui::Separator();
	ImGui::Text("Export model with selected layer(as vertex color):");
	if (ImGui::Button("Export to file..."))
	{
		std::string FilePath;
		FILE_SYSTEM.ShowFileSaveDialog(FilePath, MODEL_EXPORT_FILE_FILTER, 1);

		ExportOBJ(FilePath, LAYER_MANAGER.GetActiveLayerIndex());
	}

	ImGui::Separator();
	ImGui::Text("Export layer as image:");
	RasterizationSettingsUI();
}

bool UIManager::ExportOBJ(std::string FilePath, int LayerIndex)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return false;

	if (FilePath.empty())
		return false;

	if (FilePath.find(".obj") == std::string::npos)
		FilePath += ".obj";

	// If LayerIndex is -1, export just model without any layers.
	if (LayerIndex < 0)
	{
		return RESOURCE_MANAGER.ExportFEMeshToOBJ(ActiveMesh, FilePath.c_str());
	}
	else // Export model with layer.
	{
		if (LayerIndex >= ActiveObject->Layers.size())
			return false;

		// To export model with layer, we need to create a new mesh with vertex colors as layer data.
		DataLayer* LayerToExport = ActiveObject->Layers[LayerIndex];

		std::vector<float> ColorData;
		ColorData.resize(CurrentMeshAnalysisData->Vertices.size());
		for (size_t i = 0; i < LayerToExport->ElementsToData.size(); i++)
		{
			float CompexityValueForTriangle = LayerToExport->ElementsToData[i];
			float NormalizedValue = (CompexityValueForTriangle - LayerToExport->GetMin()) / (LayerToExport->MaxVisible - LayerToExport->GetMin());
			if (NormalizedValue > 1.0f)
				NormalizedValue = 1.0f;
			else if (NormalizedValue < 0.0f)
				NormalizedValue = 0.0f;

			glm::vec3 Color = GetTurboColorMap(NormalizedValue);

			int FirstVertexIndex = CurrentMeshAnalysisData->Indices[i * 3] * 3;
			ColorData[FirstVertexIndex] = Color.x;
			ColorData[FirstVertexIndex + 1] = Color.y;
			ColorData[FirstVertexIndex + 2] = Color.z;

			int SecondVertexIndex = CurrentMeshAnalysisData->Indices[i * 3 + 1] * 3;
			ColorData[SecondVertexIndex] = Color.x;
			ColorData[SecondVertexIndex + 1] = Color.y;
			ColorData[SecondVertexIndex + 2] = Color.z;

			int ThirdVertexIndex = CurrentMeshAnalysisData->Indices[i * 3 + 2] * 3;
			ColorData[ThirdVertexIndex] = Color.x;
			ColorData[ThirdVertexIndex + 1] = Color.y;
			ColorData[ThirdVertexIndex + 2] = Color.z;
		}

		std::vector<float> TemporaryVertices; TemporaryVertices.resize(CurrentMeshAnalysisData->Vertices.size());
		for (size_t i = 0; i < CurrentMeshAnalysisData->Vertices.size(); i++)
			TemporaryVertices[i] = static_cast<float>(CurrentMeshAnalysisData->Vertices[i]);
		
		FEMesh* NewMesh = RESOURCE_MANAGER.RawDataToMesh(TemporaryVertices.data(), static_cast<int>(TemporaryVertices.size()),
														 CurrentMeshAnalysisData->UVs.data(), static_cast<int>(CurrentMeshAnalysisData->UVs.size()),
														 CurrentMeshAnalysisData->Normals.data(), static_cast<int>(CurrentMeshAnalysisData->Normals.size()),
														 CurrentMeshAnalysisData->Tangents.data(), static_cast<int>(CurrentMeshAnalysisData->Tangents.size()),
														 CurrentMeshAnalysisData->Indices.data(), static_cast<int>(CurrentMeshAnalysisData->Indices.size()),
														 ColorData.data(), static_cast<int>(ColorData.size()),
														 nullptr, 0, 0,
														 "Exported model with layer");
			
		bool bResult = RESOURCE_MANAGER.ExportFEMeshToOBJ(NewMesh, FilePath.c_str());
		RESOURCE_MANAGER.DeleteFEMesh(NewMesh);

		return bResult;
	}

	return false;
}

void UIManager::RenderSettingsWindow()
{
	if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove))
	{
		ImGuiWindow* ImGuiWindow = ImGui::FindWindowByName("Settings");
		auto AppW = APPLICATION.GetMainWindow()->GetWidth();
		if (ImGuiWindow->Size.x >= APPLICATION.GetMainWindow()->GetWidth() * 0.9)
		{
			ImGuiWindow->Size.x = APPLICATION.GetMainWindow()->GetWidth() * 0.3f;
			ImGuiWindow->SizeFull.x = APPLICATION.GetMainWindow()->GetWidth() * 0.3f;
		}

		ImGuiWindow->Pos.x = APPLICATION.GetMainWindow()->GetWidth() - (ImGuiWindow->SizeFull.x + 1);
		ImGuiWindow->Pos.y = 20;
		

		if (ImGui::BeginTabBar("##Settings", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Layer"))
			{
				RenderLayerSettingsTab();
				ImGui::EndTabItem();
			}

			AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
			FEMesh* ActiveMesh = nullptr;
			if (ActiveObject != nullptr)
				ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());

			if (ActiveMesh == nullptr)
				ImGui::BeginDisabled();

			if (ImGui::BeginTabItem("General"))
			{
				RenderGeneralSettingsTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Export"))
			{
				RenderExportTab();
				ImGui::EndTabItem();
			}

			if (ActiveMesh == nullptr)
				ImGui::EndDisabled();

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

float UIManager::GetAmbientLightFactor()
{
	return AmbientLightFactor;
}

void UIManager::SetAmbientLightFactor(float NewValue)
{
	AmbientLightFactor = NewValue;
}

MeasurementGrid* UIManager::GetDebugGrid()
{
	return DebugGrid;
}

void UIManager::UpdateRenderingMode(MeasurementGrid* Grid, int NewRenderingMode)
{
	if (NewRenderingMode < 0)
		return;

	Grid->RenderingMode = NewRenderingMode;
	Grid->UpdateRenderedLines();

	if (NewRenderingMode == 0)
		return;

	DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
	if (CurrentLayer == nullptr)
		return;

	if (CurrentLayer->GetType() == LAYER_TYPE::UNKNOWN)
		return;

	switch (CurrentLayer->GetType())
	{
		case LAYER_TYPE::RUGOSITY:
		{
			RUGOSITY_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(Grid);
			break;
		}

		case LAYER_TYPE::VECTOR_DISPERSION:
		{
			VECTOR_DISPERSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(Grid);
			break;
		}
		
		case LAYER_TYPE::FRACTAL_DIMENSION:
		{
			FRACTAL_DIMENSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(Grid);
			break;
		}

		default:
			break;
	}
}

void UIManager::SetShouldTakeScreenshot(bool NewValue)
{
	bNextFrameForScreenshot = NewValue;
}

bool UIManager::ShouldTakeScreenshot()
{
	return bNextFrameForScreenshot;
}

void UIManager::SetUseTransparentBackground(bool NewValue)
{
	bUseTransparentBackground = NewValue;
}

bool UIManager::ShouldUseTransparentBackground()
{
	return bUseTransparentBackground;
}

bool UIManager::IsActiveObjectAndLayerValid()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return false;

	if (ActiveObject->GetActiveLayer() == nullptr)
		return false;

	return true;
}

void UIManager::UpdateProgressModalPopupCurrentValue()
{
	if (bJitterCalculationsInProgress)
	{
		ProgressModalPopupCurrentValue = JITTER_MANAGER.GetProgress();
	}
	else if (bLayerRasterizationCalculationsInProgress)
	{
		ProgressModalPopupCurrentValue = LAYER_RASTERIZATION_MANAGER.GetProgress();
	}
}

void UIManager::OnLayerRasterizationCalculationsStart()
{
	UI.bLayerRasterizationCalculationsInProgress = true;
	UI.bShouldCloseProgressPopup = false;
	UI.bShouldOpenProgressPopup = true;
}

void UIManager::OnLayerRasterizationCalculationsEnd()
{
	UI.bShouldCloseProgressPopup = true;
	UI.bLayerRasterizationCalculationsInProgress = false;
}

bool UIManager::IsProgressModalPopupOpen()
{
	return !bShouldCloseProgressPopup;
}