#include "SubSystems/ConsoleJobs/ConsoleJobManager.h"
using namespace FocalEngine;

glm::vec4 ClearColor = glm::vec4(0.33f, 0.39f, 0.49f, 1.0f);

void SwapCamera(bool bModelCamera)
{
	if (bModelCamera)
	{
		FEModelViewCamera* NewCamera = new FEModelViewCamera("New ModelViewCamera");
		NewCamera->SetAspectRatio(static_cast<float>(ENGINE.GetRenderTargetWidth()) / static_cast<float>(ENGINE.GetRenderTargetHeight()));

		NewCamera->SetTrackingObjectPosition(glm::vec3(0.0f));
		ENGINE.SetCamera(NewCamera);
	}
	else
	{
		FEFreeCamera* NewCamera = new FEFreeCamera("mainCamera");
		NewCamera->SetAspectRatio(static_cast<float>(ENGINE.GetRenderTargetWidth()) / static_cast<float>(ENGINE.GetRenderTargetHeight()));

		ENGINE.SetCamera(NewCamera);
	}

	ENGINE.GetCamera()->SetIsInputActive(false);
}

double MouseX;
double MouseY;

void MouseMoveCallback(double XPos, double YPos)
{
	MouseX = XPos;
	MouseY = YPos;
}

void LoadMesh(std::string FileName);

static void DropCallback(int Count, const char** Paths);
void DropCallback(int Count, const char** Paths)
{
	for (size_t i = 0; i < size_t(Count); i++)
	{
		LoadMesh(Paths[i]);
	}
}

void ScrollCall(double Xoffset, double Yoffset)
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	FEBasicCamera* CurrentCamera = ENGINE.GetCamera();
	if (CurrentCamera->GetCameraType() == 2)
	{
		FEModelViewCamera* ModelViewCamera = reinterpret_cast<FEModelViewCamera*>(CurrentCamera);
		if (!ImGui::GetIO().WantCaptureMouse)
			ModelViewCamera->SetDistanceToModel(ModelViewCamera->GetDistanceToModel() + Yoffset * MESH_MANAGER.ActiveMesh->GetAABB().GetLongestAxisLength() * 0.05f);
	}
}

void AfterMeshLoads()
{
	if (MESH_MANAGER.ActiveEntity != nullptr)
	{
		MESH_MANAGER.ClearBuffers();

		SCENE.DeleteEntity(MESH_MANAGER.ActiveEntity->GetObjectID());
		MESH_MANAGER.ActiveEntity = nullptr;
	}

	FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(MESH_MANAGER.ActiveMesh, MESH_MANAGER.CustomMaterial);
	FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab(NewGameModel);

	MESH_MANAGER.ActiveEntity = SCENE.AddEntity(NewPrefab);

	if (!APPLICATION.HasConsoleWindow())
	{
		MESH_MANAGER.ActiveEntity->Transform.SetPosition(-MESH_MANAGER.ActiveMesh->GetAABB().GetCenter());
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->SetPosition(-MESH_MANAGER.ActiveMesh->GetAABB().GetCenter());
	}

	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->UpdateAverageNormal();

	if (!APPLICATION.HasConsoleWindow())
	{
		UI.SetIsModelCamera(true);
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("lightDirection", glm::normalize(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->GetAverageNormal()));
	}
	
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.empty())
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(HEIGHT_LAYER_PRODUCER.Calculate());
}

void LoadMesh(std::string FileName)
{
	const FEMesh* TempMesh = MESH_MANAGER.LoadMesh(FileName);
	if (TempMesh == nullptr)
	{
		LOG.Add("Failed to load mesh with path: " + FileName);
		return;
	}
}

void UpdateMeshSelectedTrianglesRendering(FEMesh* Mesh)
{
	LINE_RENDERER.ClearAll();

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() == 1)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]];
		for (size_t i = 0; i < TranformedTrianglePoints.size(); i++)
		{
			TranformedTrianglePoints[i] = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[i], 1.0f);
		}

		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));

		if (!COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals.empty())
		{
			glm::vec3 Point = TranformedTrianglePoints[0];
			glm::vec3 Normal = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]][0];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[1];
			Normal = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]][1];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[2];
			Normal = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]][2];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
		}

		LINE_RENDERER.SyncWithGPU();
	}
	else if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() > 1)
	{
		for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size(); i++)
		{
			std::vector<glm::vec3> TranformedTrianglePoints = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[i]];
			for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
			{
				TranformedTrianglePoints[j] = MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
			}

			LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
			LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
			LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		}

		LINE_RENDERER.SyncWithGPU();
	}
}

void OutputSelectedAreaInfoToFile()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() < 2)
		return;

	std::string Text = "Area radius : " + std::to_string(UI.GetRadiusOfAreaToMeasure());
	LOG.Add(Text, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->FileName);

	Text = "Area approximate center : X - ";
	const glm::vec3 Center = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesCentroids[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]];
	Text += std::to_string(Center.x);
	Text += " Y - ";
	Text += std::to_string(Center.y);
	Text += " Z - ";
	Text += std::to_string(Center.z);
	LOG.Add(Text, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->FileName);

	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size(); i++)
	{
		MeshLayer* CurrentLayer = &COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i];

		Text = "Layer \"" + CurrentLayer->GetCaption() + "\" : \n";
		Text += "Area average value : ";
		float Total = 0.0f;
		for (size_t j = 0; j < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size(); j++)
		{
			Total += CurrentLayer->TrianglesToData[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[i]];
		}

		Total /= COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size();
		Text += std::to_string(Total);
		LOG.Add(Text, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->FileName);
	}
}

void mouseButtonCallback(int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ENGINE.GetCamera()->SetIsInputActive(false);
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		ENGINE.GetCamera()->SetIsInputActive(true);
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		ENGINE.GetCamera()->SetIsInputActive(false);
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		if (MESH_MANAGER.ActiveMesh != nullptr)
		{
			if (UI.GetLayerSelectionMode() == 1)
			{
				MESH_MANAGER.SelectTriangle(ENGINE.ConstructMouseRay());
			}
			else if (UI.GetLayerSelectionMode() == 2)
			{
				if (MESH_MANAGER.SelectTrianglesInRadius(ENGINE.ConstructMouseRay(), UI.GetRadiusOfAreaToMeasure()) && UI.GetOutputSelectionToFile())
				{
					OutputSelectedAreaInfoToFile();
				}
			}

			UpdateMeshSelectedTrianglesRendering(MESH_MANAGER.ActiveMesh);
		}

		if (MESH_MANAGER.ActiveMesh != nullptr && UI.GetDebugGrid() != nullptr)
		{
			if (UI.GetDebugGrid()->RenderingMode != 0)
			{
				UI.GetDebugGrid()->MouseClick(MouseX, MouseY);
				UI.UpdateRenderingMode(UI.GetDebugGrid(), UI.GetDebugGrid()->RenderingMode);
			}
		}
	}
}

void WindowResizeCallback(int Width, int Height)
{
	int W, H;
	UI.MainWindow->GetSize(&W, &H);

	UI.ApplyStandardWindowsSizeAndPosition();
	SCREENSHOT_MANAGER.RenderTargetWasResized();
}

void AddFontOnSecondFrame()
{
	static bool bFirstTime = true;
	static bool bFontCreated = false;

	if (bFirstTime)
	{
		bFirstTime = false;
	}
	else
	{
		if (!bFontCreated)
		{
			glfwMakeContextCurrent(APPLICATION.GetMainWindow()->GetGlfwWindow());
			ImGui::SetCurrentContext(APPLICATION.GetMainWindow()->GetImGuiContext());
		
			bFontCreated = true;
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/Cousine-Regular.ttf", 32);
			ImGui::GetIO().Fonts->Build();
			ImGui_ImplOpenGL3_CreateFontsTexture();
		}
	}
}

void ConsoleMainFunction()
{
	// Wait until the console window is created
	bool Success = APPLICATION.HasConsoleWindow();
	while (!Success)
	{
		Success = APPLICATION.HasConsoleWindow();
	}

	// To ensure initialisation of JITTER_MANAGER
	JITTER_MANAGER.getInstance();

	while (true)
	{
		CONSOLE_JOB_MANAGER.Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (!APPLICATION.IsNotTerminated())
			break;
	}
}

void ConsoleThreadCode(void* InputData)
{
	// To keep console window open
	while (APPLICATION.IsNotTerminated())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void MainWindowRender()
{
	static bool FirstFrame = true;

	const char* rasterizationModes[] = { "Min", "Max", "Mean", "Cumulative"};
	int TempInt = LAYER_RASTERIZATION_MANAGER.GetGridRasterizationMode();
	ImGui::Combo("Rasterization Mode", &TempInt, rasterizationModes, IM_ARRAYSIZE(rasterizationModes));
	LAYER_RASTERIZATION_MANAGER.SetGridRasterizationMode(static_cast<LayerRasterizationManager::GridRasterizationMode>(TempInt));

	TempInt = LAYER_RASTERIZATION_MANAGER.GetGridResolution();
	ImGui::SliderInt("Resolution", &TempInt, 2, 4096);
	//LAYER_RASTERIZATION_MANAGER.SetGridResolution(TempInt);

	float TempFloat = LAYER_RASTERIZATION_MANAGER.GetCumulativeModeLowerOutlierPercentile();
	ImGui::SliderFloat("Lower outlier percentile", &TempFloat, 0.0f, 99.9f);
	LAYER_RASTERIZATION_MANAGER.SetCumulativeModeLowerOutlierPercentile(TempFloat);

	TempFloat = LAYER_RASTERIZATION_MANAGER.GetCumulativeModeUpperOutlierPercentile();
	ImGui::SliderFloat("Upper outlier percentile", &TempFloat, 0.1f, 100.0f);
	LAYER_RASTERIZATION_MANAGER.SetCumulativeModeUpperOutlierPercentile(TempFloat);

	glm::vec3 ForceProjectionVector = LAYER_RASTERIZATION_MANAGER.GetProjectionVector();
	int SelectedAxis = ForceProjectionVector == glm::vec3(1.0f, 0.0f, 0.0f) ? 0 : ForceProjectionVector == glm::vec3(0.0f, 1.0f, 0.0f) ? 1 : 2;
	ImGui::Text("Select the axis along which the layer should be projected: ");
	if (ImGui::RadioButton("X", &SelectedAxis, 0))
	{
		ForceProjectionVector = glm::vec3(1.0f, 0.0f, 0.0f);
		LAYER_RASTERIZATION_MANAGER.PrepareCurrentLayerForExport(LAYER_MANAGER.GetActiveLayer(), ForceProjectionVector);
	}

	ImGui::SameLine();
	if (ImGui::RadioButton("Y", &SelectedAxis, 1))
	{
		ForceProjectionVector = glm::vec3(0.0f, 1.0f, 0.0f);
		LAYER_RASTERIZATION_MANAGER.PrepareCurrentLayerForExport(LAYER_MANAGER.GetActiveLayer(), ForceProjectionVector);
	}

	ImGui::SameLine();
	if (ImGui::RadioButton("Z", &SelectedAxis, 2))
	{
		ForceProjectionVector = glm::vec3(0.0f, 0.0f, 1.0f);
		LAYER_RASTERIZATION_MANAGER.PrepareCurrentLayerForExport(LAYER_MANAGER.GetActiveLayer(), ForceProjectionVector);
	}

	std::string TextForButton = "Prepare data for export";
	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() != -1)
		TextForButton = "Refresh data for export";

	if (ImGui::Button(TextForButton.c_str()))
	{
		LAYER_RASTERIZATION_MANAGER.PrepareCurrentLayerForExport(LAYER_MANAGER.GetActiveLayer());
	}

	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() != -1)
	{
		ImGui::Text("Result preview:");
		ImGui::Image((void*)(intptr_t)LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID(), ImVec2(512, 512));
	}

	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() == -1)
		ImGui::BeginDisabled();

	if (ImGui::Button("Save to file"))
	{
		LAYER_RASTERIZATION_MANAGER.PromptUserForSaveLocation();
	}

	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() == -1)
		ImGui::EndDisabled();

	if (UI.ShouldTakeScreenshot())
	{
		ClearColor.w = UI.ShouldUseTransparentBackground() ? 0.0f : 1.0f;
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);

		UI.SetShouldTakeScreenshot(false);
		SCREENSHOT_MANAGER.TakeScreenshot();
		return;
	}

	if (ENGINE.GetCamera()->GetCameraType() == 1)
		ENGINE.RenderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.GetCamera()));

	if (MESH_MANAGER.ActiveEntity != nullptr)
	{
		if (UI.GetWireFrameMode())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		MESH_RENDERER.RenderFEMesh(MESH_MANAGER.ActiveMesh);
	}

	LINE_RENDERER.Render();

	UI.Render();

	if (FirstFrame)
	{
		FirstFrame = false;
		UI.ApplyStandardWindowsSizeAndPosition();
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//LOG.SetFileOutput(true);

	const auto ProcessorCount = THREAD_POOL.GetLogicalCoreCount();
	const unsigned int HowManyToUse = ProcessorCount > 4 ? ProcessorCount - 2 : 1;

	THREAD_POOL.SetConcurrentThreadCount(HowManyToUse);

	bool bIsConsoleModeRequested = false;
	std::vector<CommandLineAction> ParsedCommandActions;

	ParsedCommandActions = APPLICATION.ParseCommandLine(lpCmdLine);
	if (!ParsedCommandActions.empty())
		std::transform(ParsedCommandActions[0].Action.begin(), ParsedCommandActions[0].Action.end(), ParsedCommandActions[0].Action.begin(), [](unsigned char c) { return std::tolower(c); });

	if (!ParsedCommandActions.empty() && ParsedCommandActions[0].Action == "console")
	{
		bIsConsoleModeRequested = true;
		ParsedCommandActions.erase(ParsedCommandActions.begin());
	}

	if (bIsConsoleModeRequested)
	{
		FEConsoleWindow* Console = APPLICATION.CreateConsoleWindow(ConsoleThreadCode);
		Console->WaitForCreation();
		Console->SetTitle("Rugosity Calculator console");

		std::vector<ConsoleJob*> ParsedJobs = CONSOLE_JOB_MANAGER.ConvertCommandAction(ParsedCommandActions);
		for (size_t i = 0; i < ParsedJobs.size(); i++)
		{
			CONSOLE_JOB_MANAGER.AddJob(ParsedJobs[i]);
		}

		while (APPLICATION.IsNotTerminated())
		{
			APPLICATION.BeginFrame();

			ConsoleMainFunction();
			APPLICATION.RenderWindows();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			APPLICATION.EndFrame();
		}
	}
	else
	{
		ENGINE.InitWindow(1280, 720, "Rugosity Calculator");
		ENGINE.ActivateSimplifiedRenderingMode();
		// If I will directly assign result of APPLICATION.AddWindow to UI.MainWindow, then in Release build with full optimization app will crash, because of execution order.
		FEWindow* MainWinodw = APPLICATION.GetMainWindow();

		UI.MainWindow = MainWinodw;
		UI.MainWindow->SetRenderFunction(MainWindowRender);

		UI.MainWindow->AddOnDropCallback(DropCallback);
		UI.MainWindow->AddOnMouseMoveCallback(MouseMoveCallback);
		UI.MainWindow->AddOnMouseButtonCallback(mouseButtonCallback);
		UI.MainWindow->AddOnResizeCallback(WindowResizeCallback);
		UI.MainWindow->AddOnScrollCallback(ScrollCall);

		ENGINE.SetClearColor(glm::vec4(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w));
		RENDERER.SetSkyEnabled(false);

		UI.SwapCameraImpl = SwapCamera;

		MESH_MANAGER.AddLoadCallback(AfterMeshLoads);

		SCREENSHOT_MANAGER.Init();

		ENGINE.GetCamera()->SetIsInputActive(false);

		while (ENGINE.IsNotTerminated())
		{
			AddFontOnSecondFrame();

			ENGINE.BeginFrame();

			ENGINE.Render();

			ENGINE.EndFrame();
		}
	}

	return 0;
}