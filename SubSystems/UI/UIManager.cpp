#include "UIManager.h"
using namespace FocalEngine;
UIManager* UIManager::Instance = nullptr;
void(*UIManager::SwapCameraImpl)(bool) = nullptr;

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

	RUGOSITY_MANAGER.SetOnRugosityCalculationsEndCallback(OnRugosityCalculationsEnd);
	RUGOSITY_MANAGER.SetOnRugosityCalculationsStartCallback(OnRugosityCalculationsStart);

	MESH_MANAGER.AddLoadCallback(UIManager::OnMeshUpdate);
	LAYER_MANAGER.AddActiveLayerChangedCallback(UIManager::AfterLayerChange);

	AddNewLayerIcon = FETexture::LoadPNGTexture("Resources/AddNewLayer.png");
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

void UIManager::ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform)
{
	// ********************* POSITION *********************
	glm::vec3 position = Transform->GetPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + Name).c_str(), &position[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + Name).c_str(), &position[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + Name).c_str(), &position[2], 0.1f);
	Transform->SetPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = Transform->GetRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + Name).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + Name).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + Name).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	Transform->SetRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &Transform->uniformScaling);
	glm::vec3 scale = Transform->GetScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + Name).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + Name).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + Name).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);

	glm::vec3 oldScale = Transform->GetScale();
	Transform->ChangeXScaleBy(scale[0] - oldScale[0]);
	Transform->ChangeYScaleBy(scale[1] - oldScale[1]);
	Transform->ChangeZScaleBy(scale[2] - oldScale[2]);
}

void UIManager::ShowCameraTransform()
{
	if (!bModelCamera)
	{
		// ********* POSITION *********
		glm::vec3 cameraPosition = CurrentCamera->GetPosition();

		ImGui::Text("Position : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##X pos", &cameraPosition[0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Y pos", &cameraPosition[1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Z pos", &cameraPosition[2], 0.1f);

		CurrentCamera->SetPosition(cameraPosition);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Copy##Position"))
		{
			APPLICATION.SetClipboardText(CameraPositionToStr());
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Paste##Position"))
		{
			StrToCameraPosition(APPLICATION.GetClipboardText());
		}

		// ********* ROTATION *********
		glm::vec3 CameraRotation = glm::vec3(CurrentCamera->GetYaw(), CurrentCamera->GetPitch(), CurrentCamera->GetRoll());

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
		{
			APPLICATION.SetClipboardText(CameraRotationToStr());
		}

		CurrentCamera->SetYaw(CameraRotation[0]);
		CurrentCamera->SetPitch(CameraRotation[1]);
		CurrentCamera->SetRoll(CameraRotation[2]);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Paste##Rotation"))
		{
			StrToCameraRotation(APPLICATION.GetClipboardText());
		}

		float CameraSpeed = CurrentCamera->GetMovementSpeed();
		ImGui::Text("Camera speed: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Camera_speed", &CameraSpeed, 0.01f, 0.01f, 100.0f);
		CurrentCamera->SetMovementSpeed(CameraSpeed);

		CurrentCamera->UpdateViewMatrix();

		if (bDeveloperMode)
		{
			ImGui::SameLine();
			ImGui::Text(("Thread count: " + std::to_string(THREAD_POOL.GetThreadCount())).c_str());
		}
	}
	else
	{
		/*float tempFloat = currentCamera->CurrentPolarAngle;
		ImGui::Text("CurrentPolarAngle : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##CurrentPolarAngle", &tempFloat, 0.1f);
		currentCamera->CurrentPolarAngle = tempFloat;

		tempFloat = currentCamera->CurrentAzimutAngle;
		ImGui::Text("CurrentAzimutAngle : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##CurrentAzimutAngle", &tempFloat, 0.1f);
		currentCamera->CurrentAzimutAngle = tempFloat;*/

		if (bDeveloperMode)
		{
			ImGui::Text(("Thread count: " + std::to_string(THREAD_POOL.GetThreadCount())).c_str());
		}
	}
}

void UIManager::SetCamera(FEBasicCamera* NewCamera)
{
	CurrentCamera = NewCamera;
	SDF::CurrentCamera = CurrentCamera;
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
				{
					MESH_MANAGER.LoadMesh(FilePath);
				}
			}

			if (MESH_MANAGER.ActiveMesh == nullptr)
				ImGui::BeginDisabled();

			if (ImGui::MenuItem("Save..."))
			{
				MESH_MANAGER.SaveRUGMesh(MESH_MANAGER.ActiveMesh);
			}

			if (MESH_MANAGER.ActiveMesh == nullptr)
				ImGui::EndDisabled();

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
			{
				APPLICATION.Terminate();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Info"))
		{
			if (ImGui::MenuItem("About..."))
			{
				OpenAboutWindow();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	RenderSettingsWindow();
	RenderLegend();
	RenderLayerChooseWindow();
	RenderHistogramWindow();
	RenderAboutWindow();

	NEW_LAYER_WINDOW.Render();

	if (UI.bShouldOpenProgressPopup)
	{
		UI.bShouldOpenProgressPopup = false;
		ImGui::OpenPopup("Calculating...");
	}

	ImGui::SetNextWindowSize(ImVec2(300, 50));
	if (ImGui::BeginPopupModal("Calculating...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		int WindowW = 0;
		int WindowH = 0;
		APPLICATION.GetWindowSize(&WindowW, &WindowH);

		ImGui::SetWindowPos(ImVec2(WindowW / 2.0f - ImGui::GetWindowWidth() / 2.0f, WindowH / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		float Progress = float(JITTER_MANAGER.GetJitterDoneCount()) / float(JITTER_MANAGER.GetJitterToDoCount());
		std::string ProgressText = "Progress: " + std::to_string(Progress * 100.0f);
		ProgressText += " %";
		ImGui::SetCursorPosX(90);
		ImGui::Text(ProgressText.c_str());

		if (bShouldCloseProgressPopup)
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

std::string UIManager::CameraPositionToStr()
{
	const glm::vec3 CameraPosition = CurrentCamera->GetPosition();
	return "( X:" + std::to_string(CameraPosition.x) + " Y:" + std::to_string(CameraPosition.y) + " Z:" + std::to_string(CameraPosition.z) + " )";
}

void UIManager::StrToCameraPosition(std::string Text)
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

	CurrentCamera->SetPosition(glm::vec3(X, Y, Z));
}

std::string UIManager::CameraRotationToStr()
{
	const glm::vec3 CameraRotation = glm::vec3(CurrentCamera->GetYaw(), CurrentCamera->GetPitch(), CurrentCamera->GetRoll());
	return "( X:" + std::to_string(CameraRotation.x) + " Y:" + std::to_string(CameraRotation.y) + " Z:" + std::to_string(CameraRotation.z) + " )";
}

void UIManager::StrToCameraRotation(std::string Text)
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

	CurrentCamera->SetYaw(X);
	CurrentCamera->SetPitch(Y);
	CurrentCamera->SetRoll(Z);
}

void UIManager::OnMeshUpdate()
{
	LINE_RENDERER.clearAll();
	LINE_RENDERER.SyncWithGPU();

	UI.Histogram.Clear();
	UI.HeatMapColorRange.Clear();
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

void UIManager::OnRugosityCalculationsStart()
{
	UI.bShouldCloseProgressPopup = false;
	UI.bShouldOpenProgressPopup = true;
}

void UIManager::OnJitterCalculationsStart()
{
	UI.bShouldCloseProgressPopup = false;
	UI.bShouldOpenProgressPopup = true;
}

void WriteJitterSettingsToDebugInfo(MeshLayerDebugInfo* DebugInfo, std::vector<SDFInitData_Jitter>& JitterSettings)
{
	if (DebugInfo == nullptr)
		return;

	for (size_t i = 0; i < JitterSettings.size(); i++)
	{
		DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftX", JitterSettings[i].ShiftX);
		DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftY", JitterSettings[i].ShiftY);
		DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftZ", JitterSettings[i].ShiftZ);
		DebugInfo->AddEntry("Jitter " + std::to_string(i) + " GridScale", JitterSettings[i].GridScale);
	}
}

void UIManager::OnRugosityCalculationsEnd(MeshLayer NewLayer)
{
	MESH_MANAGER.ActiveMesh->AddLayer(NewLayer);
	MESH_MANAGER.ActiveMesh->Layers.back().SetType(LAYER_TYPE::RUGOSITY);
	
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Rugosity"));

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	std::vector<float> TrianglesToStandardDeviation;
	auto PerJitterResult = JITTER_MANAGER.GetPerJitterResult();
	for (int i = 0; i < MESH_MANAGER.ActiveMesh->Triangles.size(); i++)
	{
		std::vector<float> CurrentTriangleResults;
		for (int j = 0; j < JITTER_MANAGER.GetJitterToDoCount(); j++)
		{
			CurrentTriangleResults.push_back(PerJitterResult[j][i]);
		}

		TrianglesToStandardDeviation.push_back(UI.FindStandardDeviation(CurrentTriangleResults));
	}
	MESH_MANAGER.ActiveMesh->AddLayer(TrianglesToStandardDeviation);
	MESH_MANAGER.ActiveMesh->Layers.back().SetType(LAYER_TYPE::STANDARD_DEVIATION);
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));

	MESH_MANAGER.ActiveMesh->Layers.back().DebugInfo = new MeshLayerDebugInfo();
	MeshLayerDebugInfo* DebugInfo = MESH_MANAGER.ActiveMesh->Layers.back().DebugInfo;
	DebugInfo->Type = "RugosityStandardDeviationLayerDebugInfo";
	DebugInfo->AddEntry("Start time", StarTime);
	DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
	DebugInfo->AddEntry("Source layer caption", MESH_MANAGER.ActiveMesh->Layers[MESH_MANAGER.ActiveMesh->Layers.size() - 2].GetCaption());

	// Add per jitter debug info to the standard deviation layer.
	WriteJitterSettingsToDebugInfo(DebugInfo, JITTER_MANAGER.GetLastUsedJitterSettings());

	LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 2);
}

void UIManager::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	UI.bShouldCloseProgressPopup = true;
	UI.CurrentJitterStepIndexVisualize = JITTER_MANAGER.GetLastUsedJitterSettings().size() - 1;

	// Add per jitter debug info to the layer.
	WriteJitterSettingsToDebugInfo(NewLayer.DebugInfo, JITTER_MANAGER.GetLastUsedJitterSettings());
	/*std::vector<SDFInitData_Jitter> UsedSettings;
	UsedSettings = JITTER_MANAGER.GetLastUsedJitterSettings();

	for (size_t i = 0; i < UsedSettings.size(); i++)
	{
		NewLayer.DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftX", UsedSettings[i].ShiftX);
		NewLayer.DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftY", UsedSettings[i].ShiftY);
		NewLayer.DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftZ", UsedSettings[i].ShiftZ);
		NewLayer.DebugInfo->AddEntry("Jitter " + std::to_string(i) + " GridScale", UsedSettings[i].GridScale);
	}*/
}

void UIManager::OnVectorDispersionCalculationsEnd(MeshLayer NewLayer)
{
	MESH_MANAGER.ActiveMesh->AddLayer(NewLayer);
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Vector Dispersion"));
	LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 1);

	/*uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	std::vector<float> TrianglesToStandardDeviation;
	for (int i = 0; i < MESH_MANAGER.ActiveMesh->Triangles.size(); i++)
	{
		std::vector<float> CurrentTriangleResults;
		for (int j = 0; j < JITTER_MANAGER.JitterToDoCount; j++)
		{
			CurrentTriangleResults.push_back(JITTER_MANAGER.PerJitterResult[j][i]);
		}

		TrianglesToStandardDeviation.push_back(UI.FindStandardDeviation(CurrentTriangleResults));
	}
	MESH_MANAGER.ActiveMesh->AddLayer(TrianglesToStandardDeviation);
	MESH_MANAGER.ActiveMesh->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));*/

	//MESH_MANAGER.ActiveMesh->Layers.back().DebugInfo = new MeshLayerDebugInfo();
	//MeshLayerDebugInfo* DebugInfo = MESH_MANAGER.ActiveMesh->Layers.back().DebugInfo;
	//DebugInfo->Type = "RugosityStandardDeviationLayerDebugInfo";
	//DebugInfo->AddEntry("Start time", StarTime);
	//DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
	//DebugInfo->AddEntry("Source layer caption", MESH_MANAGER.ActiveMesh->Layers[MESH_MANAGER.ActiveMesh->Layers.size() - 2].GetCaption());

	//LAYER_MANAGER.SetActiveLayerIndex(MESH_MANAGER.ActiveMesh->Layers.size() - 2);

	UI.bShouldCloseProgressPopup = true;
}

static auto TurboColorMapValue = [](float Value) {
	static double turbo_srgb_floats[256][3] = { {0.18995,0.07176,0.23217},{0.19483,0.08339,0.26149},{0.19956,0.09498,0.29024},{0.20415,0.10652,0.31844},{0.20860,0.11802,0.34607},{0.21291,0.12947,0.37314},{0.21708,0.14087,0.39964},{0.22111,0.15223,0.42558},{0.22500,0.16354,0.45096},{0.22875,0.17481,0.47578},{0.23236,0.18603,0.50004},{0.23582,0.19720,0.52373},{0.23915,0.20833,0.54686},{0.24234,0.21941,0.56942},{0.24539,0.23044,0.59142},{0.24830,0.24143,0.61286},{0.25107,0.25237,0.63374},{0.25369,0.26327,0.65406},{0.25618,0.27412,0.67381},{0.25853,0.28492,0.69300},{0.26074,0.29568,0.71162},{0.26280,0.30639,0.72968},{0.26473,0.31706,0.74718},{0.26652,0.32768,0.76412},{0.26816,0.33825,0.78050},{0.26967,0.34878,0.79631},{0.27103,0.35926,0.81156},{0.27226,0.36970,0.82624},{0.27334,0.38008,0.84037},{0.27429,0.39043,0.85393},{0.27509,0.40072,0.86692},{0.27576,0.41097,0.87936},{0.27628,0.42118,0.89123},{0.27667,0.43134,0.90254},{0.27691,0.44145,0.91328},{0.27701,0.45152,0.92347},{0.27698,0.46153,0.93309},{0.27680,0.47151,0.94214},{0.27648,0.48144,0.95064},{0.27603,0.49132,0.95857},{0.27543,0.50115,0.96594},{0.27469,0.51094,0.97275},{0.27381,0.52069,0.97899},{0.27273,0.53040,0.98461},{0.27106,0.54015,0.98930},{0.26878,0.54995,0.99303},{0.26592,0.55979,0.99583},{0.26252,0.56967,0.99773},{0.25862,0.57958,0.99876},{0.25425,0.58950,0.99896},{0.24946,0.59943,0.99835},{0.24427,0.60937,0.99697},{0.23874,0.61931,0.99485},{0.23288,0.62923,0.99202},{0.22676,0.63913,0.98851},{0.22039,0.64901,0.98436},{0.21382,0.65886,0.97959},{0.20708,0.66866,0.97423},{0.20021,0.67842,0.96833},{0.19326,0.68812,0.96190},{0.18625,0.69775,0.95498},{0.17923,0.70732,0.94761},{0.17223,0.71680,0.93981},{0.16529,0.72620,0.93161},{0.15844,0.73551,0.92305},{0.15173,0.74472,0.91416},{0.14519,0.75381,0.90496},{0.13886,0.76279,0.89550},{0.13278,0.77165,0.88580},{0.12698,0.78037,0.87590},{0.12151,0.78896,0.86581},{0.11639,0.79740,0.85559},{0.11167,0.80569,0.84525},{0.10738,0.81381,0.83484},{0.10357,0.82177,0.82437},{0.10026,0.82955,0.81389},{0.09750,0.83714,0.80342},{0.09532,0.84455,0.79299},{0.09377,0.85175,0.78264},{0.09287,0.85875,0.77240},{0.09267,0.86554,0.76230},{0.09320,0.87211,0.75237},{0.09451,0.87844,0.74265},{0.09662,0.88454,0.73316},{0.09958,0.89040,0.72393},{0.10342,0.89600,0.71500},{0.10815,0.90142,0.70599},{0.11374,0.90673,0.69651},{0.12014,0.91193,0.68660},{0.12733,0.91701,0.67627},{0.13526,0.92197,0.66556},{0.14391,0.92680,0.65448},{0.15323,0.93151,0.64308},{0.16319,0.93609,0.63137},{0.17377,0.94053,0.61938},{0.18491,0.94484,0.60713},{0.19659,0.94901,0.59466},{0.20877,0.95304,0.58199},{0.22142,0.95692,0.56914},{0.23449,0.96065,0.55614},{0.24797,0.96423,0.54303},{0.26180,0.96765,0.52981},{0.27597,0.97092,0.51653},{0.29042,0.97403,0.50321},{0.30513,0.97697,0.48987},{0.32006,0.97974,0.47654},{0.33517,0.98234,0.46325},{0.35043,0.98477,0.45002},{0.36581,0.98702,0.43688},{0.38127,0.98909,0.42386},{0.39678,0.99098,0.41098},{0.41229,0.99268,0.39826},{0.42778,0.99419,0.38575},{0.44321,0.99551,0.37345},{0.45854,0.99663,0.36140},{0.47375,0.99755,0.34963},{0.48879,0.99828,0.33816},{0.50362,0.99879,0.32701},{0.51822,0.99910,0.31622},{0.53255,0.99919,0.30581},{0.54658,0.99907,0.29581},{0.56026,0.99873,0.28623},{0.57357,0.99817,0.27712},{0.58646,0.99739,0.26849},{0.59891,0.99638,0.26038},{0.61088,0.99514,0.25280},{0.62233,0.99366,0.24579},{0.63323,0.99195,0.23937},{0.64362,0.98999,0.23356},{0.65394,0.98775,0.22835},{0.66428,0.98524,0.22370},{0.67462,0.98246,0.21960},{0.68494,0.97941,0.21602},{0.69525,0.97610,0.21294},{0.70553,0.97255,0.21032},{0.71577,0.96875,0.20815},{0.72596,0.96470,0.20640},{0.73610,0.96043,0.20504},{0.74617,0.95593,0.20406},{0.75617,0.95121,0.20343},{0.76608,0.94627,0.20311},{0.77591,0.94113,0.20310},{0.78563,0.93579,0.20336},{0.79524,0.93025,0.20386},{0.80473,0.92452,0.20459},{0.81410,0.91861,0.20552},{0.82333,0.91253,0.20663},{0.83241,0.90627,0.20788},{0.84133,0.89986,0.20926},{0.85010,0.89328,0.21074},{0.85868,0.88655,0.21230},{0.86709,0.87968,0.21391},{0.87530,0.87267,0.21555},{0.88331,0.86553,0.21719},{0.89112,0.85826,0.21880},{0.89870,0.85087,0.22038},{0.90605,0.84337,0.22188},{0.91317,0.83576,0.22328},{0.92004,0.82806,0.22456},{0.92666,0.82025,0.22570},{0.93301,0.81236,0.22667},{0.93909,0.80439,0.22744},{0.94489,0.79634,0.22800},{0.95039,0.78823,0.22831},{0.95560,0.78005,0.22836},{0.96049,0.77181,0.22811},{0.96507,0.76352,0.22754},{0.96931,0.75519,0.22663},{0.97323,0.74682,0.22536},{0.97679,0.73842,0.22369},{0.98000,0.73000,0.22161},{0.98289,0.72140,0.21918},{0.98549,0.71250,0.21650},{0.98781,0.70330,0.21358},{0.98986,0.69382,0.21043},{0.99163,0.68408,0.20706},{0.99314,0.67408,0.20348},{0.99438,0.66386,0.19971},{0.99535,0.65341,0.19577},{0.99607,0.64277,0.19165},{0.99654,0.63193,0.18738},{0.99675,0.62093,0.18297},{0.99672,0.60977,0.17842},{0.99644,0.59846,0.17376},{0.99593,0.58703,0.16899},{0.99517,0.57549,0.16412},{0.99419,0.56386,0.15918},{0.99297,0.55214,0.15417},{0.99153,0.54036,0.14910},{0.98987,0.52854,0.14398},{0.98799,0.51667,0.13883},{0.98590,0.50479,0.13367},{0.98360,0.49291,0.12849},{0.98108,0.48104,0.12332},{0.97837,0.46920,0.11817},{0.97545,0.45740,0.11305},{0.97234,0.44565,0.10797},{0.96904,0.43399,0.10294},{0.96555,0.42241,0.09798},{0.96187,0.41093,0.09310},{0.95801,0.39958,0.08831},{0.95398,0.38836,0.08362},{0.94977,0.37729,0.07905},{0.94538,0.36638,0.07461},{0.94084,0.35566,0.07031},{0.93612,0.34513,0.06616},{0.93125,0.33482,0.06218},{0.92623,0.32473,0.05837},{0.92105,0.31489,0.05475},{0.91572,0.30530,0.05134},{0.91024,0.29599,0.04814},{0.90463,0.28696,0.04516},{0.89888,0.27824,0.04243},{0.89298,0.26981,0.03993},{0.88691,0.26152,0.03753},{0.88066,0.25334,0.03521},{0.87422,0.24526,0.03297},{0.86760,0.23730,0.03082},{0.86079,0.22945,0.02875},{0.85380,0.22170,0.02677},{0.84662,0.21407,0.02487},{0.83926,0.20654,0.02305},{0.83172,0.19912,0.02131},{0.82399,0.19182,0.01966},{0.81608,0.18462,0.01809},{0.80799,0.17753,0.01660},{0.79971,0.17055,0.01520},{0.79125,0.16368,0.01387},{0.78260,0.15693,0.01264},{0.77377,0.15028,0.01148},{0.76476,0.14374,0.01041},{0.75556,0.13731,0.00942},{0.74617,0.13098,0.00851},{0.73661,0.12477,0.00769},{0.72686,0.11867,0.00695},{0.71692,0.11268,0.00629},{0.70680,0.10680,0.00571},{0.69650,0.10102,0.00522},{0.68602,0.09536,0.00481},{0.67535,0.08980,0.00449},{0.66449,0.08436,0.00424},{0.65345,0.07902,0.00408},{0.64223,0.07380,0.00401},{0.63082,0.06868,0.00401},{0.61923,0.06367,0.00410},{0.60746,0.05878,0.00427},{0.59550,0.05399,0.00453},{0.58336,0.04931,0.00486},{0.57103,0.04474,0.00529},{0.55852,0.04028,0.00579},{0.54583,0.03593,0.00638},{0.53295,0.03169,0.00705},{0.51989,0.02756,0.00780},{0.50664,0.02354,0.00863},{0.49321,0.01963,0.00955},{0.47960,0.01583,0.01055} };

	const int index = int(255 * Value);

	return ImColor(int(turbo_srgb_floats[index][0] * 255),
				   int(turbo_srgb_floats[index][1] * 255),
				   int(turbo_srgb_floats[index][2] * 255), 255);
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
		HeatMapColorRange.SetColorRangeFunction(TurboColorMapValue);

	if (bScreenshotMode && MESH_MANAGER.ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()];

		HeatMapColorRange.Legend.Clear();
		HeatMapColorRange.Legend.SetCaption(1.0f, "max: " + TruncateAfterDot(std::to_string(CurrentLayer->Min + (CurrentLayer->Max - CurrentLayer->Min) * HeatMapColorRange.GetCeilingValue())));
		const float MiddleOfUsedRange = (HeatMapColorRange.GetCeilingValue() + CurrentLayer->MinVisible / CurrentLayer->Max) / 2.0f;
		HeatMapColorRange.Legend.SetCaption(0.0f, "min: " + TruncateAfterDot(std::to_string(CurrentLayer->MinVisible)));
	}

	HeatMapColorRange.Render(bScreenshotMode);

	if (bScreenshotMode)
	{
		ImGui::End();
		ImGui::PopStyleVar(2);
		return;
	}

	static char CurrentRugosityMax[1024];
	static float LastValue = HeatMapColorRange.GetCeilingValue();
	if (MESH_MANAGER.ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()];
		if (abs(CurrentLayer->Max) < 100000 && LastValue != HeatMapColorRange.GetCeilingValue())
		{
			LastValue = HeatMapColorRange.GetCeilingValue();
			strcpy(CurrentRugosityMax, TruncateAfterDot(std::to_string(CurrentLayer->Min + (CurrentLayer->Max - CurrentLayer->Min) * HeatMapColorRange.GetCeilingValue())).c_str());
		}

		HeatMapColorRange.Legend.Clear();
		HeatMapColorRange.Legend.SetCaption(1.0f, "max: " + TruncateAfterDot(std::to_string(CurrentLayer->Max)));

		HeatMapColorRange.Legend.SetCaption(HeatMapColorRange.GetCeilingValue(), "current: " + TruncateAfterDot(std::to_string(CurrentLayer->Min + (CurrentLayer->Max - CurrentLayer->Min) * HeatMapColorRange.GetCeilingValue())));

		const float MiddleOfUsedRange = (HeatMapColorRange.GetCeilingValue() + CurrentLayer->MinVisible / CurrentLayer->Max) / 2.0f;
		HeatMapColorRange.Legend.SetCaption(0.0f, "min: " + TruncateAfterDot(std::to_string(CurrentLayer->MinVisible)));

		CurrentLayer->MaxVisible = CurrentLayer->Min + (CurrentLayer->Max - CurrentLayer->Min) * HeatMapColorRange.GetCeilingValue();
	}

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
		if (MESH_MANAGER.ActiveMesh != nullptr)
		{
			MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()];

			float NewValue = float(atof(CurrentRugosityMax));
			if (NewValue < CurrentLayer->Min)
				NewValue = CurrentLayer->Min;

			HeatMapColorRange.SetCeilingValue((NewValue - CurrentLayer->Min) / float(CurrentLayer->Max - CurrentLayer->Min));
		}
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

void UIManager::GetUsableSpaceForLayerList(ImVec2& UsableSpaceStart, ImVec2& UsableSpaceEnd)
{
	ImGuiWindow* SettingsWindow = ImGui::FindWindowByName("Settings");
	ImGuiWindow* LegendWindow = ImGui::FindWindowByName("Heat map legend");

	UsableSpaceStart = ImVec2(0.0f, 0.0f);
	UsableSpaceEnd = ImVec2(APPLICATION.GetWindowWidth(), APPLICATION.GetWindowHeight());
	if (SettingsWindow != nullptr && LegendWindow != nullptr)
	{
		UsableSpaceStart.x = LegendWindow->Pos.x + LegendWindow->SizeFull.x;
		UsableSpaceStart.y = 20;

		UsableSpaceEnd.x = SettingsWindow->Pos.x;
		UsableSpaceEnd.y = APPLICATION.GetWindowHeight() - 20;
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

	ButtonUsed = std::min(size_t(ButtonUsed), MESH_MANAGER.ActiveMesh->Layers.size());

	if (ButtonUsed == 0)
		return Result;

	Result += GetLayerListButtonSize("No Layer").x + 16;
	for (int i = 0; i < ButtonUsed; i++)
	{
		Result += GetLayerListButtonSize(MESH_MANAGER.ActiveMesh->Layers[i].GetCaption()).x + ButtonSpacing * 2.0f + 4;
	}

	Result += FirstLastButtonPadding * 2.0f;
	
	return Result;
}

void UIManager::RenderLayerChooseWindow()
{
	static int RowCount = 1;

	const int ButtonSpacing = 6;
	const int FirstLastButtonPadding = 4;
	const int RowHeight = 26;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImVec2 UsableSpaceStart, UsableSpaceEnd;
	GetUsableSpaceForLayerList(UsableSpaceStart, UsableSpaceEnd);

	int UsableSpaceCenter = UsableSpaceStart.x + (UsableSpaceEnd.x - UsableSpaceStart.x) / 2.0f;
	int UsableSpaceWidth = UsableSpaceEnd.x - UsableSpaceStart.x;

	int TotalWidthNeeded = 0;

	if (MESH_MANAGER.ActiveMesh != nullptr)
		TotalWidthNeeded = TotalWidthNeededForLayerList(MESH_MANAGER.ActiveMesh->Layers.size());
	if (TotalWidthNeeded == 0)
	{
		if (MESH_MANAGER.ActiveMesh == nullptr)
		{
			TotalWidthNeeded = ImGui::CalcTextSize("No Data.").x + 18;
		}
		else
		{
			TotalWidthNeeded = GetLayerListButtonSize("No Layer").x + 16;
			TotalWidthNeeded += FirstLastButtonPadding * 2.0f;
		}
	}
	else if (TotalWidthNeeded > UsableSpaceWidth - 10.0f)
	{
		TotalWidthNeeded = UsableSpaceWidth - 10.0f;
	}

	const float CurrentWindowW = TotalWidthNeeded;
	const float CurrentWindowH = 6 + RowHeight * RowCount;

	ImVec2 LayerListWindowPosition = ImVec2(UsableSpaceCenter - CurrentWindowW / 2.0f, 21);
	ImGui::SetNextWindowPos(LayerListWindowPosition);
	ImGui::SetNextWindowSize(ImVec2(CurrentWindowW, CurrentWindowH));
	ImGui::Begin("Layers", nullptr, ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoCollapse |
									ImGuiWindowFlags_NoScrollbar |
									ImGuiWindowFlags_NoTitleBar);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
	if (MESH_MANAGER.ActiveMesh == nullptr)
	{
		std::string NoDataText = "No Data.";
		ImVec2 TextSize = ImGui::CalcTextSize(NoDataText.c_str());

		ImGui::SetCursorPos(ImVec2(CurrentWindowW / 2.0f - TextSize.x / 2.0f, CurrentWindowH / 2.0f - TextSize.y / 2.0f));
		ImGui::Text(NoDataText.c_str());

		ImGui::PopStyleVar(3);
		ImGui::End();

		return;
	}

	int CurrentRow = 0;
	int PreviousButtonWidth = 0;
	int YPosition = ImGui::GetCursorPosY() + 7;
	for (int i = -1; i < int(MESH_MANAGER.ActiveMesh->Layers.size()); i++)
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
			ImGui::SetCursorPosY(YPosition);
			if (ImGui::Button("No Layer"))
			{
				LAYER_MANAGER.SetActiveLayerIndex(i);
			}
		}
		else
		{
			if (ImGui::GetCursorPosX() + GetLayerListButtonSize(MESH_MANAGER.ActiveMesh->Layers[i].GetCaption()).x + ButtonSpacing * 2.0f + 4 > (UsableSpaceWidth - 10))
			{
				ImGui::SetCursorPosX(FirstLastButtonPadding * 2.0f);
				CurrentRow++;
			}

			ImGui::SetCursorPosY(YPosition + CurrentRow * RowHeight);
			if (ImGui::Button((MESH_MANAGER.ActiveMesh->Layers[i].GetCaption() + "##" + std::to_string(i)).c_str()))
			{
				LAYER_MANAGER.SetActiveLayerIndex(i);
			}
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
		if (ImGui::ImageButton((void*)(intptr_t)AddNewLayerIcon->GetTextureID(), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 0))
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

bool UIManager::GetIsModelCamera()
{
	return bModelCamera;
}

void UIManager::SetIsModelCamera(const bool NewValue)
{
	SwapCameraImpl(NewValue);

	CurrentCamera->Reset();
	CurrentCamera->SetFarPlane(MESH_MANAGER.ActiveMesh->AABB.getSize() * 5.0f);

	int MainWindowW = 0;
	int MainWindowH = 0;
	APPLICATION.GetWindowSize(&MainWindowW, &MainWindowH);
	CurrentCamera->SetAspectRatio(float(MainWindowW) / float(MainWindowH));

	if (NewValue)
	{
		FEModelViewCamera* ModelCamera = reinterpret_cast<FEModelViewCamera*>(CurrentCamera);
		ModelCamera->SetDistanceToModel(MESH_MANAGER.ActiveMesh->AABB.getSize() * 1.5f);
	}
	else
	{
		CurrentCamera->SetPosition(glm::vec3(0.0f, 0.0f, MESH_MANAGER.ActiveMesh->AABB.getSize() * 1.5f));
		CurrentCamera->SetYaw(0.0f);
		CurrentCamera->SetPitch(0.0f);
		CurrentCamera->SetRoll(0.0f);

		CurrentCamera->SetMovementSpeed(MESH_MANAGER.ActiveMesh->AABB.getSize() / 5.0f);
	}

	bModelCamera = NewValue;
}

void UIManager::UpdateHistogramData(MeshLayer* FromLayer, int NewBinCount)
{
	std::vector<double> Values;
	std::vector<double> Weights;

	for (const auto& tuple : FromLayer->ValueTriangleAreaAndIndex)
	{
		Values.push_back(std::get<0>(tuple));
		Weights.push_back(std::get<1>(tuple));
	}

	Histogram.FillDataBins(Values, Weights, NewBinCount);
}

void UIManager::RenderHistogramWindow()
{
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

			if (MESH_MANAGER.ActiveMesh != nullptr &&
				bHistogramPixelBins &&
				LAYER_MANAGER.GetActiveLayerIndex() != -1)
			{
				UpdateHistogramData(&MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()], Histogram.GetCurrentBinCount());
			}

			LastWindowW = HistogramWindow->SizeFull.x;
			LastWindowH = HistogramWindow->SizeFull.y;
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Histogram", nullptr))
	{
		if (MESH_MANAGER.ActiveMesh == nullptr || LAYER_MANAGER.GetActiveLayerIndex() == -1)
			ImGui::BeginDisabled();
		
		ImGui::SetCursorPos(ImVec2(12.0f, 30.0f));
		if (ImGui::Checkbox("Select region mode", &bHistogramSelectRegionMode))
		{
			if (MESH_MANAGER.ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
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

		if (MESH_MANAGER.ActiveMesh == nullptr || LAYER_MANAGER.GetActiveLayerIndex() == -1)
			ImGui::EndDisabled();

		Histogram.Render();

		if (bHistogramSelectRegionMode && MESH_MANAGER.ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
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

			// Render text that corresponds to the min value
			ImGui::SetCursorPos(Histogram.GetPosition() + HistogramSelectRegionMin.GetPixelPosition() - ImVec2(HistogramSelectRegionMin.GetSize() * 0.90f, HistogramSelectRegionMin.GetSize() * 1.65f));
			std::string MinValue = TruncateAfterDot(std::to_string(LAYER_MANAGER.GetActiveLayer()->Min + LAYER_MANAGER.GetActiveLayer()->Max * HistogramSelectRegionMin.GetRangePosition()), 3);
			ImGui::Text(MinValue.c_str());

			// Line that corresponds to the min value
			ImVec2 ArrowPosition = HistogramWindow->Pos + Histogram.GetPosition() + HistogramSelectRegionMin.GetPixelPosition();
			ImGui::GetWindowDrawList()->AddRectFilled(ArrowPosition - ImVec2(1.0f, 0.0f),
													  ArrowPosition + ImVec2(1.0f, Histogram.GetSize().y - 1.0f),
													  ImColor(56.0f / 255.0f, 205.0f / 255.0f, 137.0f / 255.0f, 165.0f / 255.0f));

			// Render text that corresponds to the min value
			ImGui::SetCursorPos(Histogram.GetPosition() + HistogramSelectRegionMax.GetPixelPosition() - ImVec2(HistogramSelectRegionMax.GetSize() * 0.90f, HistogramSelectRegionMax.GetSize() * 1.65f));
			std::string MaxValue = TruncateAfterDot(std::to_string(LAYER_MANAGER.GetActiveLayer()->Min + LAYER_MANAGER.GetActiveLayer()->Max * HistogramSelectRegionMax.GetRangePosition()), 3);
			ImGui::Text(MaxValue.c_str());

			// Line that corresponds to the max value
			ArrowPosition = HistogramWindow->Pos + Histogram.GetPosition() + HistogramSelectRegionMax.GetPixelPosition();
			ImGui::GetWindowDrawList()->AddRectFilled(ArrowPosition - ImVec2(1.0f, 0.0f),
													  ArrowPosition + ImVec2(1.0f, Histogram.GetSize().y - 1.0f),
													  ImColor(156.0f / 255.0f, 105.0f / 255.0f, 137.0f / 255.0f, 165.0f / 255.0f));
			

			MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()];
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
				NewBinCount = HistogramWindow->SizeFull.x - 20;

			UpdateHistogramData(&MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()], NewBinCount);
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
						TempInt = HistogramWindow->SizeFull.x - 20;
				}

				if (Histogram.GetCurrentBinCount() != TempInt)
				{
					if (MESH_MANAGER.ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)	
						UpdateHistogramData(&MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()], TempInt);
				}
			}
		}
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
	ImGuiWindow* window = ImGui::FindWindowByName("Histogram");
	if (window != nullptr)
	{
		window->SizeFull.x = APPLICATION.GetWindowWidth() * 0.5;
		window->Pos.x = APPLICATION.GetWindowWidth() / 2 - window->SizeFull.x / 2;

		window->SizeFull.y = APPLICATION.GetWindowHeight() * 0.35;
		window->Pos.y = APPLICATION.GetWindowHeight() - 10 - window->SizeFull.y;
	}

	window = ImGui::FindWindowByName("Settings");
	if (window != nullptr)
	{
		window->SizeFull.x = APPLICATION.GetWindowWidth() * 0.30;
		window->SizeFull.y = APPLICATION.GetWindowHeight() * 0.7;
	}
}

void UIManager::OpenAboutWindow()
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

	int PopupW = 400;
	int PopupH = 135;
	ImGui::SetNextWindowSize(ImVec2(PopupW, PopupH));
	if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		int WindowW = 0;
		int WindowH = 0;
		APPLICATION.GetWindowSize(&WindowW, &WindowH);

		ImGui::SetWindowPos(ImVec2(WindowW / 2.0f - ImGui::GetWindowWidth() / 2.0f, WindowH / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		
		std::string Text = "Version: " + std::to_string(APP_VERSION) + "     date: 06\\09\\2023";
		ImVec2 TextSize = ImGui::CalcTextSize(Text.c_str());
		ImGui::SetCursorPosX(PopupW / 2 - TextSize.x / 2);
		ImGui::Text(Text.c_str());

		ImGui::Separator();

		Text = "To submit a bug report or provide feedback, ";
		TextSize = ImGui::CalcTextSize(Text.c_str());
		ImGui::SetCursorPosX(PopupW / 2 - TextSize.x / 2);
		ImGui::Text(Text.c_str());

		Text = "please email me at kberegovyi@ccom.unh.edu.";
		TextSize = ImGui::CalcTextSize(Text.c_str());
		ImGui::SetCursorPosX(PopupW / 2 - TextSize.x / 2);
		ImGui::Text(Text.c_str());

		ImGui::Separator();

		Text = "UNH CCOM";
		TextSize = ImGui::CalcTextSize(Text.c_str());
		ImGui::SetCursorPosX(PopupW / 2 - TextSize.x / 2);
		ImGui::Text(Text.c_str());

		ImGui::SetCursorPosX(PopupW / 2 - 210 / 2);
		ImGui::SetNextItemWidth(210);
		if (ImGui::Button("Close", ImVec2(210, 20)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

glm::dvec2 UIManager::LayerValuesAreaDistribution(MeshLayer* Layer, float Value)
{
	if (Layer == nullptr || Layer->GetParentMesh() == nullptr || Layer->TrianglesToData.empty() || Layer->GetParentMesh()->TrianglesArea.empty())
		return glm::dvec2(0.0);

	float FirstBin = 0.0;
	float SecondBin = 0.0;

	FEMesh* Mesh = Layer->GetParentMesh();

	for (int i = 0; i < Mesh->Triangles.size(); i++)
	{
		if (Layer->TrianglesToData[i] <= Value)
		{
			FirstBin += float(Mesh->TrianglesArea[i]);
		}
		else
		{
			SecondBin += float(Mesh->TrianglesArea[i]);
		}
	}

	return glm::dvec2(FirstBin, SecondBin);
}

void UIManager::AfterLayerChange()
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	UI.bHistogramSelectRegionMode = false;
	UI.Histogram.Clear();
	UI.HeatMapColorRange.Clear();

	if (LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMin(0.0f);
		LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMax(0.0f);

		if (MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()].Min != MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()].Max)
		{
			UI.UpdateHistogramData(&MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()], UI.Histogram.GetCurrentBinCount());

			MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()];

			float NormalizedPosition = 0.0f;
			const int CaptionsCount = 8;
			const float PositionStep = 1.0f / CaptionsCount;
			for (size_t i = 0; i <= CaptionsCount; i++)
			{
				UI.Histogram.SetLegendCaption(i == 0 ? NormalizedPosition + 0.0075f : NormalizedPosition,
					TruncateAfterDot(std::to_string(CurrentLayer->Min + (CurrentLayer->Max - CurrentLayer->Min) * NormalizedPosition)));

				NormalizedPosition += PositionStep;
			}

			if (CurrentLayer->MaxVisible == CurrentLayer->Max)
			{
				float MiddleOfRange = CurrentLayer->Min + (CurrentLayer->Max - CurrentLayer->Min) / 2.0f;
				UI.HeatMapColorRange.SetCeilingValue(MiddleOfRange / CurrentLayer->Max);
			}
			else
			{
				UI.HeatMapColorRange.SetCeilingValue(CurrentLayer->MaxVisible / CurrentLayer->Max);
			}

			UI.HeatMapColorRange.SetRangeBottomLimit(CurrentLayer->Min / CurrentLayer->Max);
		}

		if (UI.GetDebugSDF() != nullptr)
		{
			UI.InitDebugSDF(UI.CurrentJitterStepIndexVisualize);
			UI.UpdateRenderingMode(UI.GetDebugSDF(), UI.GetDebugSDF()->RenderingMode);
		}
	}
}

float UIManager::FindStandardDeviation(std::vector<float> DataPoints)
{
	float Mean = 0.0f;
	for (int i = 0; i < DataPoints.size(); i++)
	{
		Mean += DataPoints[i];
	}
	Mean /= DataPoints.size();

	float NewMean = 0.0f;
	for (int i = 0; i < DataPoints.size(); i++)
	{
		DataPoints[i] -= Mean;
		DataPoints[i] = std::pow(DataPoints[i], 2.0);
		NewMean += DataPoints[i];
	}
	NewMean /= DataPoints.size();

	return std::sqrt(NewMean);
}

std::vector<SDFInitData_Jitter> ReadJitterSettingsFromDebugInfo(MeshLayerDebugInfo* DebugInfo)
{
	std::vector<SDFInitData_Jitter> Result;

	if (DebugInfo == nullptr)
		return Result;

	std::istringstream iss(DebugInfo->ToString());
	std::string Line;
	SDFInitData_Jitter CurrentData;
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
			CurrentData = SDFInitData_Jitter();
		}
		else if (bNewData)
		{
			// If it's a new "Jitter" line, skip to the next. If it's other text, it will be ignored.
			continue;
		}
	}

	return Result;
}

void UIManager::InitDebugSDF(size_t JitterIndex)
{
	if (LAYER_MANAGER.GetActiveLayer() == nullptr)
		return;

	std::vector<SDFInitData_Jitter> UsedSettings;
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
	MeshLayer* Layer = LAYER_MANAGER.GetActiveLayer();
	for (size_t i = 0; i < Layer->DebugInfo->Entries.size(); i++)
	{
		if (Layer->DebugInfo->Entries[i].Name == "Resolution used")
		{
			std::string Data = Layer->DebugInfo->Entries[i].RawData;
			Data.erase(Data.begin() + Data.find(" m."), Data.end());
			CurrentLayerResolutionInM = atof(Data.c_str());
			break;
		}
	}

	if (CurrentLayerResolutionInM <= 0.0f)
		return;

	delete DebugSDF;
	DebugSDF = new SDF();

	SDFInitData_Jitter* CurrentSettings = &UsedSettings[JitterIndex];
	FEAABB finalAABB = MESH_MANAGER.ActiveMesh->AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	transformMatrix = glm::scale(transformMatrix, glm::vec3(CurrentSettings->GridScale));
	finalAABB = finalAABB.transform(transformMatrix);

	const glm::vec3 center = MESH_MANAGER.ActiveMesh->AABB.getCenter() + glm::vec3(CurrentSettings->ShiftX, CurrentSettings->ShiftY, CurrentSettings->ShiftZ) * CurrentLayerResolutionInM;
	const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;

	DebugSDF->Init(0, finalAABB, CurrentLayerResolutionInM);
	DebugSDF->FillCellsWithTriangleInfo();
	DebugSDF->bFullyLoaded = true;
}

void UIManager::RenderSettingsWindow()
{
	if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove))
	{
		ImGuiWindow* window = ImGui::FindWindowByName("Settings");
		auto AppW = APPLICATION.GetWindowWidth();
		if (window->Size.x >= APPLICATION.GetWindowWidth() * 0.9)
		{
			window->Size.x = APPLICATION.GetWindowWidth() * 0.3;
			window->SizeFull.x = APPLICATION.GetWindowWidth() * 0.3;
		}

		window->Pos.x = APPLICATION.GetWindowWidth() - (window->SizeFull.x + 1);
		window->Pos.y = 20;
		

		if (ImGui::BeginTabBar("##Settings", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Layer"))
			{
				std::string NoInfoText;
				if (MESH_MANAGER.ActiveMesh == nullptr)
					NoInfoText = "No model loaded.";

				if (MESH_MANAGER.ActiveMesh != nullptr && MESH_MANAGER.ActiveMesh->Layers.empty())
					NoInfoText = "Model have no layers.";

				if (MESH_MANAGER.ActiveMesh != nullptr && !MESH_MANAGER.ActiveMesh->Layers.empty() && LAYER_MANAGER.GetActiveLayerIndex() == -1)
					NoInfoText = "Layer is not selected.";

				if (!NoInfoText.empty())
				{
					ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize(NoInfoText.c_str()).x / 2.0f);
					ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize(NoInfoText.c_str()).y / 2.0f);

					ImGui::Text(NoInfoText.c_str());
				}

				if (NoInfoText.empty())
				{
					MeshLayer* Layer = &MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()];

					static char CurrentLayerCaption[1024];
					strcpy(CurrentLayerCaption, Layer->GetCaption().c_str());
					ImGui::Text("Caption: ");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(160);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
					if (ImGui::InputText("##LayerCaptionEdit", CurrentLayerCaption, IM_ARRAYSIZE(CurrentLayerCaption), ImGuiInputTextFlags_EnterReturnsTrue) ||
						ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##LayerCaptionEdit"))
					{
						Layer->SetCaption(CurrentLayerCaption);
					}

					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.6f, 0.1f, 0.2f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.65f, 0.2f, 0.2f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.75f, 0.6f, 0.1f));
					ImGui::SameLine();
					if (ImGui::Button("Delete Layer"))
					{
						int IndexToDelete = LAYER_MANAGER.GetActiveLayerIndex();
						LAYER_MANAGER.SetActiveLayerIndex(-1);

						MESH_MANAGER.ActiveMesh->Layers.erase(MESH_MANAGER.ActiveMesh->Layers.begin() + IndexToDelete,
															  MESH_MANAGER.ActiveMesh->Layers.begin() + IndexToDelete + 1);
					
						ImGui::PopStyleColor(3);
						ImGui::EndTabItem();
						ImGui::EndTabBar();
						ImGui::End();

						return;
					}
					ImGui::PopStyleColor(3);

					ImGui::Text("Mean:");
					ImGui::SameLine();
					std::string MeanText = "No data.";
					if (Layer->GetMean() != -FLT_MAX)
						MeanText = std::to_string(Layer->GetMean());
					ImGui::Text(MeanText.c_str());

					ImGui::Text("Median:");
					ImGui::SameLine();
					std::string MedianText = "No data.";
					if (Layer->GetMedian() != -FLT_MAX)
						MedianText = std::to_string(Layer->GetMedian());
					ImGui::Text(MedianText.c_str());

					ImGui::Text("Notes:");
					static char CurrentLayerUserNotes[10000];
					strcpy(CurrentLayerUserNotes, Layer->GetNote().c_str());
					ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
					if (ImGui::InputTextMultiline("##Notes", CurrentLayerUserNotes, IM_ARRAYSIZE(CurrentLayerUserNotes)))
					{
						Layer->SetNote(CurrentLayerUserNotes);
					}

					ImGui::Text("Debug Info:");
					static char CurrentLayerDebugInfo[10000];
					std::string DebugInfo;
					if (Layer->DebugInfo != nullptr)
						DebugInfo = Layer->DebugInfo->ToString();
					strcpy(CurrentLayerDebugInfo, DebugInfo.c_str());
					ImGui::BeginDisabled();
					ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
					ImGui::InputTextMultiline("##DebugInfo", CurrentLayerDebugInfo, IM_ARRAYSIZE(CurrentLayerDebugInfo));
					ImGui::EndDisabled();

					ImGui::Separator();
					ImGui::Text("Selection mode:");
					if (ImGui::RadioButton("None", &LayerSelectionMode, 0))
					{
						MESH_MANAGER.ActiveMesh->TriangleSelected.clear();
						LINE_RENDERER.clearAll();
						LINE_RENDERER.SyncWithGPU();
					}

					if (ImGui::RadioButton("Triangles", &LayerSelectionMode, 1))
					{
						MESH_MANAGER.ActiveMesh->TriangleSelected.clear();
						LINE_RENDERER.clearAll();
						LINE_RENDERER.SyncWithGPU();
					}

					if (ImGui::RadioButton("Area", &LayerSelectionMode, 2))
					{
						MESH_MANAGER.ActiveMesh->TriangleSelected.clear();
						LINE_RENDERER.clearAll();
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

					if (MESH_MANAGER.ActiveMesh->TriangleSelected.size() == 1)
					{
						MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()];

						ImGui::Separator();
						ImGui::Text("Selected triangle information :");

						std::string Text = "Triangle value : ";
						Text += std::to_string(CurrentLayer->TrianglesToData[MESH_MANAGER.ActiveMesh->TriangleSelected[0]]);
						ImGui::Text(Text.c_str());

						int HeightLayerIndex = -1;
						for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->Layers.size(); i++)
						{
							if (MESH_MANAGER.ActiveMesh->Layers[i].GetCaption() == "Height")
								HeightLayerIndex = i;
						}

						Text = "Triangle height : ";
						double AverageHeight = 0.0;
						if (HeightLayerIndex != -1)
						{
							AverageHeight = MESH_MANAGER.ActiveMesh->Layers[HeightLayerIndex].TrianglesToData[MESH_MANAGER.ActiveMesh->TriangleSelected[0]];
							AverageHeight -= MESH_MANAGER.ActiveMesh->Layers[HeightLayerIndex].Min;
						}

						Text += std::to_string(AverageHeight);

						ImGui::Text(Text.c_str());
					}
					else if (MESH_MANAGER.ActiveMesh->TriangleSelected.size() > 1)
					{
						MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()];

						std::string Text = "Area average value : ";
						float TotalRugosity = 0.0f;
						for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->TriangleSelected.size(); i++)
						{
							TotalRugosity += CurrentLayer->TrianglesToData[MESH_MANAGER.ActiveMesh->TriangleSelected[i]];
						}

						TotalRugosity /= MESH_MANAGER.ActiveMesh->TriangleSelected.size();
						Text += std::to_string(TotalRugosity);

						ImGui::Text(Text.c_str());

						Text = "Area average height : ";
						double AverageHeight = 0.0;

						int HeightLayerIndex = -1;
						for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->Layers.size(); i++)
						{
							if (MESH_MANAGER.ActiveMesh->Layers[i].GetCaption() == "Height")
								HeightLayerIndex = i;
						}

						if (HeightLayerIndex != -1)
						{
							MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[HeightLayerIndex];

							for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->TriangleSelected.size(); i++)
							{
								double CurrentHeight = CurrentLayer->TrianglesToData[MESH_MANAGER.ActiveMesh->TriangleSelected[i]];
								AverageHeight += CurrentHeight;
							}

							AverageHeight /= MESH_MANAGER.ActiveMesh->TriangleSelected.size();
							AverageHeight -= CurrentLayer->Min;
						}

						Text += std::to_string(AverageHeight);

						ImGui::Text(Text.c_str());
					}

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
						CurrentDistribution = LayerValuesAreaDistribution(&MESH_MANAGER.ActiveMesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()], NewValue);
					}

					if (CurrentDistribution != glm::vec2())
					{
						ImGui::Text(("Area below and at " + TruncateAfterDot(std::to_string(LastDistributionValue)) + " value : " + std::to_string(CurrentDistribution.x / MESH_MANAGER.ActiveMesh->TotalArea * 100.0f) + " %%").c_str());
						ImGui::Text(("Area with higher than " + TruncateAfterDot(std::to_string(LastDistributionValue)) + " value : " + std::to_string(CurrentDistribution.y / MESH_MANAGER.ActiveMesh->TotalArea * 100.0f) + " %%").c_str());
					}
				}

				ImGui::EndTabItem();
			}

			if (MESH_MANAGER.ActiveMesh != nullptr)
			{
				if (ImGui::BeginTabItem("General"))
				{
					ImGui::Checkbox("Wireframe", &bWireframeMode);

					ImGui::Text("Ambiant light intensity:");
					ImGui::SetNextItemWidth(150);
					ImGui::DragFloat("##AmbiantLightScale", &AmbientLightFactor, 0.025f);
					ImGui::SameLine();
					if (ImGui::Button("Reset"))
					{
						AmbientLightFactor = 2.8f;
					}

					bool TempBool = bModelCamera;
					if (ImGui::Checkbox("Model camera", &TempBool))
					{
						SetIsModelCamera(TempBool);
					}

					ShowCameraTransform();

					ImGui::Separator();
					TempBool = IsInDeveloperMode();
					if (ImGui::Checkbox("Developer mode", &TempBool))
						SetDeveloperMode(TempBool);
					
					if (IsInDeveloperMode())
					{
						int TempValue = JITTER_MANAGER.GetDebugJitterToDoCount();
						ImGui::Text("Jitter count for next calculation(set -1 to run all jitters): ");
						ImGui::SetNextItemWidth(120);
						ImGui::DragInt("##Jitter count", &TempValue);
						JITTER_MANAGER.SetDebugJitterToDoCount(TempValue);
					}

					if (IsInDeveloperMode() && LAYER_MANAGER.GetActiveLayerIndex() != -1)
					{
						if (DebugSDF == nullptr)
							UI.InitDebugSDF(JITTER_MANAGER.GetJitterToDoCount() - 1);

						std::vector<SDFInitData_Jitter> UsedSettings;
						UsedSettings = ReadJitterSettingsFromDebugInfo(LAYER_MANAGER.GetActiveLayer()->DebugInfo);

						if (UsedSettings.size() > 0)
						{
							if (CurrentJitterStepIndexVisualize < 0 || CurrentJitterStepIndexVisualize >= UsedSettings.size())
								CurrentJitterStepIndexVisualize = UsedSettings.size() - 1;

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
										CurrentJitterStepIndexVisualize = i;
										int LastSDFRendetingMode = DebugSDF->RenderingMode;

										InitDebugSDF(CurrentJitterStepIndexVisualize);

										DebugSDF->RenderingMode = LastSDFRendetingMode;
										if (DebugSDF->RenderingMode == 1)
										{
											UpdateRenderingMode(DebugSDF, 1);
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
						
						ImGui::Text("Visualization of SDF:");

						if (ImGui::RadioButton("Do not draw", &DebugSDF->RenderingMode, 0))
						{
							UpdateRenderingMode(DebugSDF, 0);
						}

						if (ImGui::RadioButton("Show cells with triangles", &DebugSDF->RenderingMode, 1))
						{
#ifdef NEW_LINES
							InitDebugSDF(CurrentJitterStepIndexVisualize);
#endif
							UpdateRenderingMode(DebugSDF, 1);
						}

						if (ImGui::RadioButton("Show all cells", &DebugSDF->RenderingMode, 2))
						{
							UpdateRenderingMode(DebugSDF, 2);
						}

#ifdef NEW_LINES
						if (DebugSDF->RenderingMode == 1)
						{
							DebugSDF->AddLinesOfSDF();
						}
#endif

						if (DebugSDF->RenderingMode == 1)
						{
							MeshLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
							if (CurrentLayer == nullptr)
								return;

							switch (CurrentLayer->GetType())
							{
								case LAYER_TYPE::RUGOSITY:
								{
									//RUGOSITY_MANAGER.RenderDebugInfoForSelectedNode(DebugSDF);
									break;
								}

								case LAYER_TYPE::VECTOR_DISPERSION:
								{
									//VECTOR_DISPERSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(DebugSDF);
									break;
								}

								case LAYER_TYPE::FRACTAL_DIMENSION:
								{
									FRACTAL_DIMENSION_LAYER_PRODUCER.RenderDebugInfoWindow(DebugSDF);
									break;
								}

								default:
									break;
							}
						}					

						ImGui::Separator();
					}

					ImGui::EndTabItem();
				}
			}

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

SDF* UIManager::GetDebugSDF()
{
	return DebugSDF;
}

void UIManager::UpdateRenderingMode(SDF* SDF, int NewRenderingMode)
{
	if (NewRenderingMode < 0)
		return;

	SDF->RenderingMode = NewRenderingMode;
	SDF->UpdateRenderedLines();

	if (NewRenderingMode == 0)
		return;

	if (LAYER_TYPE::UNKNOWN)
		return;

	MeshLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
	if (CurrentLayer == nullptr)
		return;

	switch (CurrentLayer->GetType())
	{
		case LAYER_TYPE::RUGOSITY:
		{
			RUGOSITY_MANAGER.RenderDebugInfoForSelectedNode(SDF);
			break;
		}

		case LAYER_TYPE::VECTOR_DISPERSION:
		{
			VECTOR_DISPERSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(SDF);
			break;
		}
		
		case LAYER_TYPE::FRACTAL_DIMENSION:
		{
			FRACTAL_DIMENSION_LAYER_PRODUCER.RenderDebugInfoForSelectedNode(SDF);
			break;
		}

		default:
			break;
	}
}

//void UIManager::OnGraphMouseClick(float NormalizedPosition)
//{
//	MeshLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
//	if (CurrentLayer == nullptr)
//		return;
//
//	static bool bIsFirstClick = true;
//
//	if (bIsFirstClick)
//	{
//		bIsFirstClick = false;
//		CurrentLayer->SetSelectedRangeMin(NormalizedPosition);
//	}
//	else
//	{
//		bIsFirstClick = true;
//		CurrentLayer->SetSelectedRangeMax(NormalizedPosition);
//	}
//}