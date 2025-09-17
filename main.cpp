#include "SubSystems/ConsoleJobs/ConsoleJobManager.h"
#include "SubSystems/VRManager/VRManager.h"
using namespace FocalEngine;

glm::vec4 ClearColor = glm::vec4(0.33f, 0.39f, 0.49f, 1.0f);

double MouseX;
double MouseY;

void MouseMoveCallback(double XPos, double YPos)
{
	MouseX = XPos;
	MouseY = YPos;
}

void LoadResource(std::string FileName);

static void DropCallback(int Count, const char** Paths);
void DropCallback(int Count, const char** Paths)
{
	if (UI.IsProgressModalPopupOpen())
		return;

	for (size_t i = 0; i < size_t(Count); i++)
	{
		LoadResource(Paths[i]);
	}
}

void AfterNewResourceLoads(AnalysisObject* NewObject)
{
	if (NewObject == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return;

	ResourceAnalysisData* AnalysisData = NewObject->GetAnalysisData();

	if (!APPLICATION.HasConsoleWindow())
	{
		ActiveEntity->GetComponent<FETransformComponent>().SetPosition(-AnalysisData->GetAABB().GetCenter());
		AnalysisData->Position->SetPosition(-AnalysisData->GetAABB().GetCenter());
	}

	if (!APPLICATION.HasConsoleWindow())
	{
		if (ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() == 1)
			UI.SetIsModelCamera(true);

		if (NewObject->GetType() == DATA_SOURCE_TYPE::MESH)
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("lightDirection", glm::normalize(ANALYSIS_OBJECT_MANAGER.GetAllMeshObjectsAverageNormal()));
	}

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject != nullptr &&
		ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH &&
		ActiveObject->Layers.empty())
	{
		DataLayer* NewLayer = HEIGHT_LAYER_PRODUCER.Calculate();
		if (NewLayer != nullptr)
			ActiveObject->AddLayer(NewLayer);
	}
}

void LoadResource(std::string FileName)
{
	AnalysisObject* LoadedObject = ANALYSIS_OBJECT_MANAGER.LoadResource(FileName);
	if (LoadedObject == nullptr)
		LOG.Add("Failed to load object with path: " + FileName);
}

void UpdateMeshSelectedTrianglesRendering(FEMesh* Mesh)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	if (CurrentMeshAnalysisData->TriangleSelected.size() == 1)
	{
		LINE_RENDERER.ClearAll();

		std::vector<glm::dvec3> TranformedTrianglePoints = CurrentMeshAnalysisData->Triangles[CurrentMeshAnalysisData->TriangleSelected[0]];
		for (size_t i = 0; i < TranformedTrianglePoints.size(); i++)
		{
			TranformedTrianglePoints[i] = CurrentMeshAnalysisData->Position->GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[i], 1.0f);
		}

		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));

		if (!CurrentMeshAnalysisData->TrianglesNormals.empty())
		{
			glm::vec3 Point = TranformedTrianglePoints[0];
			glm::vec3 Normal = CurrentMeshAnalysisData->TrianglesNormals[CurrentMeshAnalysisData->TriangleSelected[0]][0];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[1];
			Normal = CurrentMeshAnalysisData->TrianglesNormals[CurrentMeshAnalysisData->TriangleSelected[0]][1];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[2];
			Normal = CurrentMeshAnalysisData->TrianglesNormals[CurrentMeshAnalysisData->TriangleSelected[0]][2];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
		}

		LINE_RENDERER.SyncWithGPU();
	}
	else if (CurrentMeshAnalysisData->TriangleSelected.size() > 1)
	{
		FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
		if (ActiveEntity == nullptr)
			return;

		LINE_RENDERER.ClearAll();
		
		for (size_t i = 0; i < CurrentMeshAnalysisData->TriangleSelected.size(); i++)
		{
			std::vector<glm::dvec3> TranformedTrianglePoints = CurrentMeshAnalysisData->Triangles[CurrentMeshAnalysisData->TriangleSelected[i]];
			for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
			{
				TranformedTrianglePoints[j] = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
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
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	if (CurrentMeshAnalysisData->TriangleSelected.size() < 2)
		return;

	bool bCurrentSettings = LOG.IsFileOutputActive();
	if (!bCurrentSettings)
		LOG.SetFileOutput(true);

	std::string Text = "Area radius : " + std::to_string(UI.GetRadiusOfAreaToMeasure());
	LOG.Add(Text, FILE_SYSTEM.GetFileName(ActiveObject->GetFilePath()));

	Text = "Area approximate center : X - ";
	const glm::vec3 Center = CurrentMeshAnalysisData->TrianglesCentroids[CurrentMeshAnalysisData->TriangleSelected[0]];
	Text += std::to_string(Center.x);
	Text += " Y - ";
	Text += std::to_string(Center.y);
	Text += " Z - ";
	Text += std::to_string(Center.z);
	LOG.Add(Text, FILE_SYSTEM.GetFileName(ActiveObject->GetFilePath()));

	for (size_t i = 0; i < ActiveObject->Layers.size(); i++)
	{
		DataLayer* CurrentLayer = ActiveObject->Layers[i];

		Text = "Layer \"" + CurrentLayer->GetCaption() + "\" : \n";
		Text += "Area average value : ";
		float Total = 0.0f;
		for (size_t j = 0; j < CurrentMeshAnalysisData->TriangleSelected.size(); j++)
		{
			Total += CurrentLayer->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[i]];
		}

		Total /= CurrentMeshAnalysisData->TriangleSelected.size();
		Text += std::to_string(Total);
		LOG.Add(Text, FILE_SYSTEM.GetFileName(ActiveObject->GetFilePath()));
	}

	if (!bCurrentSettings)
		LOG.SetFileOutput(false);
}

void MouseButtonCallback(int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(true);
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		//LAYER_RASTERIZATION_MANAGER.DebugMouseClick();

		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		FEMesh* ActiveMesh = nullptr;
		if (ActiveObject != nullptr && ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
			ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());

		if (ActiveMesh != nullptr)
		{
			if (UI.GetLayerSelectionMode() == 1)
			{
				ANALYSIS_OBJECT_MANAGER.SelectTriangle(MAIN_SCENE_MANAGER.GetMouseRayDirection());
			}
			else if (UI.GetLayerSelectionMode() == 2)
			{
				if (ANALYSIS_OBJECT_MANAGER.SelectTrianglesInRadius(MAIN_SCENE_MANAGER.GetMouseRayDirection(), UI.GetRadiusOfAreaToMeasure()) && UI.GetOutputSelectionToFile())
				{
					OutputSelectedAreaInfoToFile();
				}
			}

			UpdateMeshSelectedTrianglesRendering(ActiveMesh);
		}

		if (ActiveMesh != nullptr && UI.GetDebugGrid() != nullptr)
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
	APPLICATION.GetMainWindow()->GetSize(&W, &H);

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
	JITTER_MANAGER.GetInstance();

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

	if (UI.ShouldTakeScreenshot())
	{
		ClearColor.w = UI.ShouldUseTransparentBackground() ? 0.0f : 1.0f;
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);

		UI.SetShouldTakeScreenshot(false);
		SCREENSHOT_MANAGER.TakeScreenshot();
		return;
	}

	/*bool bVRMode = ENGINE.IsVREnabled();
	if (ImGui::Checkbox("Enter VR mode", &bVRMode))
	{
		if (bVRMode)
		{
			if (ENGINE.EnableVR(FERenderingPipeline::Forward_Simplified))
			{
				VR_MANAGER.Initialize();
			}
		}
		else
		{
			ENGINE.DisableVR();
		}
	}

	if (bVRMode)
	{
		VR_MANAGER.Update();

		FEEntity* VRRigEntity = OpenXR_MANAGER.GetVRRigEntity();
		if (VRRigEntity != nullptr)
		{
			FETransformComponent& VRRigTransform = VRRigEntity->GetComponent<FETransformComponent>();
			glm::vec3 VRRigPosition = VRRigTransform.GetPosition();

			ImGui::Text("VRRig Position : ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##X Left Controller", &VRRigPosition[0], 0.01f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##Y Left Controller", &VRRigPosition[1], 0.01f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##Z Left Controller", &VRRigPosition[2], 0.01f);

			VRRigTransform.SetPosition(VRRigPosition);
		}
	}*/

	LINE_RENDERER.Render();

	UI.Render();

	if (FirstFrame)
	{
		FirstFrame = false;
		UI.ApplyStandardWindowsSizeAndPosition();
	}
}

GLFWimage ConvertIconToGLFWImage(HICON Icon)
{
	ICONINFO IconInfo;
	GetIconInfo(Icon, &IconInfo);
	BITMAP BMP;
	GetObject(IconInfo.hbmColor, sizeof(BITMAP), &BMP);

	GLFWimage Result;
	Result.width = BMP.bmWidth;
	Result.height = BMP.bmHeight;

	int BytesPerPixel = BMP.bmBitsPixel / 8;
	int Size = Result.width * Result.height * 4;
	Result.pixels = new unsigned char[Size];

	// Get the bits from the bitmap and store them in the GLFWimage
	GetBitmapBits(IconInfo.hbmColor, Size, Result.pixels);

	// Convert BGR to RGB
	for (int i = 0; i < Size; i += 4)
	{
		std::swap(Result.pixels[i], Result.pixels[i + 2]); // Swap B and R
	}

	// Clean up
	DeleteObject(IconInfo.hbmColor);
	DeleteObject(IconInfo.hbmMask);

	return Result;
}

FEEntity* TestEntity = nullptr;
void CreateTestEntity()
{
	FEMesh* SphereMesh = RESOURCE_MANAGER.GetMesh("7F251E3E0D08013E3579315F");

	FEMaterial* GreenMaterial = RESOURCE_MANAGER.CreateMaterial();
	GreenMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	GreenMaterial->SetBaseColor(glm::vec3(0.0f, 1.0f, 0.0f));
	FEGameModel* GreenSphereGameModel = RESOURCE_MANAGER.CreateGameModel(SphereMesh, GreenMaterial);

	TestEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Test entity");
	TestEntity->AddComponent<FEGameModelComponent>(GreenSphereGameModel);
	TestEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
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
		Console->SetTitle("HabiCAT3D console");

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
		ENGINE.InitWindow(1280, 720, "HabiCAT3D");
		// If I will directly assign result of APPLICATION.AddWindow to UI.MainWindow, then in Release build with full optimization app will crash, because of execution order.
		FEWindow* MainWindow = APPLICATION.GetMainWindow();

		GLFWimage Icon = ConvertIconToGLFWImage(LoadIcon(hInstance, MAKEINTRESOURCE(101)));
		glfwSetWindowIcon(MainWindow->GetGlfwWindow(), 1, &Icon);

		APPLICATION.GetMainWindow()->SetRenderFunction(MainWindowRender);
		APPLICATION.GetMainWindow()->AddOnDropCallback(DropCallback);
		APPLICATION.GetMainWindow()->AddOnMouseMoveCallback(MouseMoveCallback);
		APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(MouseButtonCallback);
		APPLICATION.GetMainWindow()->AddOnResizeCallback(WindowResizeCallback);

		FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
		CAMERA_SYSTEM.SetCameraRenderingPipeline(CameraEntity, FERenderingPipeline::Forward_Simplified);
		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		CameraComponent.SetClearColor(glm::vec4(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w));
		CameraComponent.SetNearPlane(0.1f);
		CameraComponent.SetActive(false);

		ANALYSIS_OBJECT_MANAGER.AddOnLoadCallback(AfterNewResourceLoads);

		SCREENSHOT_MANAGER.Init();

		//CreateTestEntity();

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