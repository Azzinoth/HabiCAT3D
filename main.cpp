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

void AfterMeshLoads()
{
	if (MESH_MANAGER.ActiveEntity != nullptr)
	{
		MESH_MANAGER.ClearBuffers();

		MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(MESH_MANAGER.ActiveEntity->GetObjectID());
		MESH_MANAGER.ActiveEntity = nullptr;
	}

	FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(MESH_MANAGER.ActiveMesh, MESH_MANAGER.CustomMaterial);
	MESH_MANAGER.ActiveEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Main entity");
	MESH_MANAGER.ActiveEntity->AddComponent<FEGameModelComponent>(NewGameModel);

	MeshGeometryData* CurrentMeshData = ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData;
	if (CurrentMeshData == nullptr)
		return;

	if (!APPLICATION.HasConsoleWindow())
	{
		MESH_MANAGER.ActiveEntity->GetComponent<FETransformComponent>().SetPosition(-MESH_MANAGER.ActiveMesh->GetAABB().GetCenter());
		CurrentMeshData->Position->SetPosition(-MESH_MANAGER.ActiveMesh->GetAABB().GetCenter());
	}

	CurrentMeshData->UpdateAverageNormal();

	if (!APPLICATION.HasConsoleWindow())
	{
		UI.SetIsModelCamera(true);
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("lightDirection", glm::normalize(CurrentMeshData->GetAverageNormal()));
	}

	if (LAYER_MANAGER.GetLayerCount() == 0)
		LAYER_MANAGER.AddLayer(HEIGHT_LAYER_PRODUCER.Calculate());
}

void LoadResource(std::string FileName)
{
	const FEMesh* TempMesh = MESH_MANAGER.LoadResource(FileName);
	if (TempMesh == nullptr)
	{
		LOG.Add("Failed to load mesh with path: " + FileName);
		return;
	}
}

void UpdateMeshSelectedTrianglesRendering(FEMesh* Mesh)
{
	MeshGeometryData* CurrentMeshData = ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData;
	if (CurrentMeshData == nullptr)
		return;

	if (CurrentMeshData->TriangleSelected.size() == 1)
	{
		LINE_RENDERER.ClearAll();

		std::vector<glm::dvec3> TranformedTrianglePoints = CurrentMeshData->Triangles[CurrentMeshData->TriangleSelected[0]];
		for (size_t i = 0; i < TranformedTrianglePoints.size(); i++)
		{
			TranformedTrianglePoints[i] = CurrentMeshData->Position->GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[i], 1.0f);
		}

		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));

		if (!CurrentMeshData->TrianglesNormals.empty())
		{
			glm::vec3 Point = TranformedTrianglePoints[0];
			glm::vec3 Normal = CurrentMeshData->TrianglesNormals[CurrentMeshData->TriangleSelected[0]][0];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[1];
			Normal = CurrentMeshData->TrianglesNormals[CurrentMeshData->TriangleSelected[0]][1];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[2];
			Normal = CurrentMeshData->TrianglesNormals[CurrentMeshData->TriangleSelected[0]][2];
			LINE_RENDERER.AddLineToBuffer(FECustomLine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
		}

		LINE_RENDERER.SyncWithGPU();
	}
	else if (CurrentMeshData->TriangleSelected.size() > 1)
	{
		LINE_RENDERER.ClearAll();

		for (size_t i = 0; i < CurrentMeshData->TriangleSelected.size(); i++)
		{
			std::vector<glm::dvec3> TranformedTrianglePoints = CurrentMeshData->Triangles[CurrentMeshData->TriangleSelected[i]];
			for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
			{
				TranformedTrianglePoints[j] = MESH_MANAGER.ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
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
	if (!ANALYSIS_OBJECT_MANAGER.HaveMeshData())
		return;

	MeshGeometryData* CurrentMeshData = ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData;
	if (CurrentMeshData == nullptr)
		return;

	if (CurrentMeshData->TriangleSelected.size() < 2)
		return;

	bool bCurrentSettings = LOG.IsFileOutputActive();
	if (!bCurrentSettings)
		LOG.SetFileOutput(true);

	std::string Text = "Area radius : " + std::to_string(UI.GetRadiusOfAreaToMeasure());
	LOG.Add(Text, CurrentMeshData->FileName);

	Text = "Area approximate center : X - ";
	const glm::vec3 Center = CurrentMeshData->TrianglesCentroids[CurrentMeshData->TriangleSelected[0]];
	Text += std::to_string(Center.x);
	Text += " Y - ";
	Text += std::to_string(Center.y);
	Text += " Z - ";
	Text += std::to_string(Center.z);
	LOG.Add(Text, CurrentMeshData->FileName);

	for (size_t i = 0; i < LAYER_MANAGER.Layers.size(); i++)
	{
		DataLayer* CurrentLayer = &LAYER_MANAGER.Layers[i];

		Text = "Layer \"" + CurrentLayer->GetCaption() + "\" : \n";
		Text += "Area average value : ";
		float Total = 0.0f;
		for (size_t j = 0; j < CurrentMeshData->TriangleSelected.size(); j++)
		{
			Total += CurrentLayer->ElementsToData[CurrentMeshData->TriangleSelected[i]];
		}

		Total /= CurrentMeshData->TriangleSelected.size();
		Text += std::to_string(Total);
		LOG.Add(Text, CurrentMeshData->FileName);
	}

	if (!bCurrentSettings)
		LOG.SetFileOutput(false);
}

void mouseButtonCallback(int button, int action, int mods)
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

		if (MESH_MANAGER.ActiveMesh != nullptr)
		{
			if (UI.GetLayerSelectionMode() == 1)
			{
				MESH_MANAGER.SelectTriangle(MAIN_SCENE_MANAGER.GetMouseRayDirection());
			}
			else if (UI.GetLayerSelectionMode() == 2)
			{
				if (MESH_MANAGER.SelectTrianglesInRadius(MAIN_SCENE_MANAGER.GetMouseRayDirection(), UI.GetRadiusOfAreaToMeasure()) && UI.GetOutputSelectionToFile())
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

void CalculateOneNodePointCount(GridNode* CurrentNode)
{
	if (CurrentNode->PointsInCell.empty())
		return;
	
	CurrentNode->UserData = CurrentNode->PointsInCell.size();
}

void OnJitterCalculationsEnd(DataLayer NewLayer)
{
	NewLayer.ElementsToData;

	float Min = FLT_MAX;
	float Max = -FLT_MAX;
	for (size_t i = 0; i < NewLayer.ElementsToData.size(); i++)
	{
		if (NewLayer.ElementsToData[i] < Min)
			Min = NewLayer.ElementsToData[i];
		if (NewLayer.ElementsToData[i] > Max)
			Max = NewLayer.ElementsToData[i];
	}

	PointCloudGeometryData* CurrentPointCloudData = ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData;

	// Update color based on min/max
	for (size_t i = 0; i < CurrentPointCloudData->RawPointCloudData.size(); i++)
	{
		float NormalizedValue = (NewLayer.ElementsToData[i] - Min) / (Max - Min);
		glm::vec3 NewColor = GetTurboColorMap(NormalizedValue);
		CurrentPointCloudData->RawPointCloudData[i].R = static_cast<unsigned char>(NewColor.x * 255.0f);
		CurrentPointCloudData->RawPointCloudData[i].G = static_cast<unsigned char>(NewColor.y * 255.0f);
		CurrentPointCloudData->RawPointCloudData[i].B = static_cast<unsigned char>(NewColor.z * 255.0f);
	}

	FEPointCloud* PointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(CurrentPointCloudData->RawPointCloudData);

	MESH_MANAGER.CurrentPointCloudEntity->RemoveComponent<FEPointCloudComponent>();
	RESOURCE_MANAGER.DeleteFEPointCloud(MESH_MANAGER.CurrentPointCloud);
	MESH_MANAGER.CurrentPointCloud = PointCloud;
	MESH_MANAGER.CurrentPointCloudEntity->AddComponent<FEPointCloudComponent>(MESH_MANAGER.CurrentPointCloud);


	//if (!RUGOSITY_LAYER_PRODUCER.bWaitForJitterResult)
	//	return;

	NewLayer.SetDataSourceType(DATA_SOURCE_TYPE::POINT_CLOUD);
	//NewLayer.SetType(RUGOSITY);
	
	//RUGOSITY_LAYER_PRODUCER.bWaitForJitterResult = false;
	//NewLayer.DebugInfo->Type = "RugosityDataLayerDebugInfo";

	//std::string AlgorithmUsed = RUGOSITY_LAYER_PRODUCER.RugosityAlgorithmList[0];
	//if (RUGOSITY_LAYER_PRODUCER.bUseFindSmallestRugosity)
	//	AlgorithmUsed = RUGOSITY_LAYER_PRODUCER.RugosityAlgorithmList[1];

	//if (RUGOSITY_LAYER_PRODUCER.bUseCGALVariant)
	//	AlgorithmUsed = RUGOSITY_LAYER_PRODUCER.RugosityAlgorithmList[2];

	//NewLayer.DebugInfo->AddEntry("Algorithm used", AlgorithmUsed);

	//if (AlgorithmUsed == "Min Rugosity(default)")
	//	NewLayer.DebugInfo->AddEntry("Orientation set name", RUGOSITY_LAYER_PRODUCER.GetOrientationSetForMinRugosityName());

	//std::string DeleteOutliers = "No";
	//// Remove outliers.
	//if (RUGOSITY_LAYER_PRODUCER.bDeleteOutliers || (RUGOSITY_LAYER_PRODUCER.bUseFindSmallestRugosity && RUGOSITY_LAYER_PRODUCER.OrientationSetForMinRugosity == "1"))
	//{
	//	DeleteOutliers = "Yes";
	//	JITTER_MANAGER.AdjustOutliers(NewLayer.ElementsToData, 0.00f, 0.99f);
	//}
	//NewLayer.DebugInfo->AddEntry("Delete outliers", DeleteOutliers);

	//std::string OverlapAware = "No";
	//if (RUGOSITY_LAYER_PRODUCER.bUniqueProjectedArea)
	//	OverlapAware = "Yes";
	//NewLayer.DebugInfo->AddEntry("Unique projected area (very slow)", OverlapAware);

	//std::string OverlapAwareApproximation = "No";
	//if (RUGOSITY_LAYER_PRODUCER.bUniqueProjectedAreaApproximation)
	//	OverlapAwareApproximation = "Yes";
	//NewLayer.DebugInfo->AddEntry("Approximation of unique projected area", OverlapAwareApproximation);

	//LastTimeTookForCalculation = float(TIME.EndTimeStamp("CalculateRugorsityTotal"));

	//LAYER_MANAGER.AddLayer(NewLayer);
	//LAYER_MANAGER.Layers.back().SetType(LAYER_TYPE::RUGOSITY);
	//LAYER_MANAGER.Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Rugosity"));

	//LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(LAYER_MANAGER.Layers.size() - 1));

	//if (RUGOSITY_LAYER_PRODUCER.bCalculateStandardDeviation)
	//{
	//	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	//	std::vector<float> TrianglesToStandardDeviation = JITTER_MANAGER.ProduceStandardDeviationData();
	//	LAYER_MANAGER.AddLayer(TrianglesToStandardDeviation);
	//	LAYER_MANAGER.Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));

	//	LAYER_MANAGER.Layers.back().DebugInfo = new DataLayerDebugInfo();
	//	DataLayerDebugInfo* DebugInfo = LAYER_MANAGER.Layers.back().DebugInfo;
	//	DebugInfo->Type = "RugosityStandardDeviationLayerDebugInfo";
	//	DebugInfo->AddEntry("Start time", StartTime);
	//	DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
	//	DebugInfo->AddEntry("Source layer ID", LAYER_MANAGER.Layers[LAYER_MANAGER.Layers.size() - 2].GetID());
	//	DebugInfo->AddEntry("Source layer caption", LAYER_MANAGER.Layers[LAYER_MANAGER.Layers.size() - 2].GetCaption());
	//}

	//if (OnRugosityCalculationsEndCallbackImpl != nullptr)
	//	OnRugosityCalculationsEndCallbackImpl(NewLayer);
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

	bool bVRMode = ENGINE.IsVREnabled();
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

	if (MESH_MANAGER.CurrentPointCloudEntity != nullptr)
	{
		if (ImGui::Button("Calculate on point cloud"))
		{
			JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
			JITTER_MANAGER.CalculateWithGridJitterAsync(CalculateOneNodePointCount);
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

		/*glm::vec3 ControllerPosition = FEOpenXR_INPUT.GetLeftControllerPosition();
		ImGui::Text("Left Controller Position : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##X Left Controller", &ControllerPosition[0], 0.01f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Y Left Controller", &ControllerPosition[1], 0.01f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Z Left Controller", &ControllerPosition[2], 0.01f);


		ControllerPosition = FEOpenXR_INPUT.GetRightControllerPosition();

		ImGui::Text("Right Controller Position : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##X Right Controller", &ControllerPosition[0], 0.01f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Y Right Controller", &ControllerPosition[1], 0.01f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Z Right Controller", &ControllerPosition[2], 0.01f);

		if (ImGui::Button("Haptic"))
		{
			FEOpenXR_INPUT.TriggerHapticFeedback(0.5f, 0.5f, 0.5f, false);
		}*/
	}

	if (MESH_MANAGER.ActiveEntity != nullptr)
	{
		if (UI.GetWireFrameMode())
		{
			MESH_MANAGER.ActiveEntity->GetComponent<FEGameModelComponent>().SetWireframeMode(true);
		}
		else
		{
			MESH_MANAGER.ActiveEntity->GetComponent<FEGameModelComponent>().SetWireframeMode(false);
		}

		// RenderFEMesh
		MESH_MANAGER.UpdateUniforms();

		// Unnecessary part
		MESH_MANAGER.ActiveEntity->SetComponentVisible(ComponentVisibilityType::ALL, true);

		// This part should be done by Engine.
		FE_GL_ERROR(glBindVertexArray(MESH_MANAGER.ActiveMesh->GetVaoID()));

		if (MESH_MANAGER.ActiveMesh->GetColorCount() > 0) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if (MESH_MANAGER.GetFirstLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(7));
		if (MESH_MANAGER.GetSecondLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(8));
		// This part should be done by Engine END.

		// That should happen in Engine. In RenderingPipeline.
		//RENDERER.RenderGameModelComponentForward(MESH_MANAGER.ActiveEntity, MAIN_SCENE_MANAGER.GetMainCamera(), false);


		// Unnecessary part
		//MESH_MANAGER.ActiveEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

		//MESH_RENDERER.RenderFEMesh(MESH_MANAGER.ActiveMesh);

		// RenderFEMesh END

		
	}

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
		APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(mouseButtonCallback);
		APPLICATION.GetMainWindow()->AddOnResizeCallback(WindowResizeCallback);

		FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
		CAMERA_SYSTEM.SetCameraRenderingPipeline(CameraEntity, FERenderingPipeline::Forward_Simplified);
		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		CameraComponent.SetClearColor(glm::vec4(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w));
		CameraComponent.SetNearPlane(0.1f);
		CameraComponent.SetActive(false);

		MESH_MANAGER.AddLoadCallback(AfterMeshLoads);

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