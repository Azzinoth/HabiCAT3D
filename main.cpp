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
	LAYER_RASTERIZATION_MANAGER.SetGridRasterizationMode(TempInt);

	TempInt = LAYER_RASTERIZATION_MANAGER.GetGridResolution();
	ImGui::SliderInt("Resolution", &TempInt, 2, 4096);
	LAYER_RASTERIZATION_MANAGER.SetGridResolution(TempInt);

	//TempInt = LAYER_RASTERIZATION_MANAGER.GetCumulativeOutliers();
	//ImGui::SliderInt("Cumulative_Outliers_Upper", &TempInt, 0, 99);
	//LAYER_RASTERIZATION_MANAGER.SetCumulativeOutliers(TempInt);

	ImGui::SliderFloat("Cumulative_Outliers_Upper", &LAYER_RASTERIZATION_MANAGER.CumulativeOutliersUpper, 0.1f, 100.0f);
	ImGui::SliderFloat("Cumulative_Outliers_Lower", &LAYER_RASTERIZATION_MANAGER.CumulativeOutliersLower, 0.0f, 99.9f);

	ImGui::Checkbox("Use CGAL", &LAYER_RASTERIZATION_MANAGER.bUsingCGAL);

	if (ImGui::Button("Prepare data for export"))
	{
		LAYER_RASTERIZATION_MANAGER.PrepareCurrentLayerForExport(LAYER_MANAGER.GetActiveLayer());
	}

	if (LAYER_RASTERIZATION_MANAGER.ResultPreview != nullptr)
	{
		ImGui::Text("Result preview:");
		ImGui::Image((void*)(intptr_t)LAYER_RASTERIZATION_MANAGER.ResultPreview->GetTextureID(), ImVec2(512, 512));
	}

	ImGui::Checkbox("32 bit GeoTiff", &LAYER_RASTERIZATION_MANAGER.b32BitsExport);

	if (ImGui::Button("Save to file"))
	{
		LAYER_RASTERIZATION_MANAGER.SaveToFile("test.png");
	}

	float CameraSpeed = ENGINE.GetCamera()->GetMovementSpeed();
	ImGui::Text("Camera speed: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Camera_speed", &CameraSpeed, 0.01f, 0.01f, 100.0f);
	ENGINE.GetCamera()->SetMovementSpeed(CameraSpeed);

	// Output dubug info from LAYER_RASTERIZATION_MANAGER, like min, max, mean, standard deviation, etc.
	ImGui::Text("Total area used: %f", LAYER_RASTERIZATION_MANAGER.Debug_TotalAreaUsed);
	ImGui::Text("Rasterization min: %f", LAYER_RASTERIZATION_MANAGER.Debug_ResultRawMin);
	ImGui::Text("Rasterization max: %f", LAYER_RASTERIZATION_MANAGER.Debug_ResultRawMax);
	ImGui::Text("Rasterization mean: %f", LAYER_RASTERIZATION_MANAGER.Debug_ResultRawMean);
	ImGui::Text("Rasterization standard deviation: %f", LAYER_RASTERIZATION_MANAGER.Debug_ResultRawStandardDeviation);
	ImGui::Text("Rasterization distribution skewness: %f", LAYER_RASTERIZATION_MANAGER.Debug_ResultRawSkewness);
	ImGui::Text("Rasterization distribution kurtosis: %f", LAYER_RASTERIZATION_MANAGER.Debug_ResultRawKurtosis);


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

		static bool bNeedToRenderAABB = true;
		if (bNeedToRenderAABB)
		{
			bNeedToRenderAABB = false;
			LINE_RENDERER.RenderAABB(MESH_MANAGER.ActiveMesh->GetAABB().Transform(MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix()), glm::vec3(0.0f, 0.0f, 1.0f));

			//ExportCurrentLayerAsMap();

			LINE_RENDERER.SyncWithGPU();
		}
	}

	static bool bNeedToRenderAABB = true;
	if (bNeedToRenderAABB)
	{
		bNeedToRenderAABB = false;

		glm::vec3 Multiplicator = glm::vec3(100.0f, 1.0f, 100.0f);
		//Multiplicator = glm::vec3(1.0f);

		/*glm::vec3 FirstPoint = glm::vec3(0.040420, 0.531200, 2.167910) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(0.016880, 0.552290, 2.118570) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(0.002030, 0.547620, 2.223800) * Multiplicator;
		glm::vec3 Min = glm::vec3(0.000000, 0.000000, 2.120456) * Multiplicator;
		glm::vec3 Max = glm::vec3(0.014455, 2.464900, 2.135822) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(0.000000, 0.344400, 2.605680) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(0.004200, 0.356840, 2.147420) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(0.497080, 0.328360, 2.253990) * Multiplicator;
		glm::vec3 Min = glm::vec3(0.014455, 0.000000, 2.381672) * Multiplicator;
		glm::vec3 Max = glm::vec3(0.028910, 2.464900, 2.397038) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(0.020720, 0.725140, 0.320140) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(0.118330, 0.696310, 0.286380) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(0.006580, 0.760780, 0.276570) * Multiplicator;
		glm::vec3 Min = glm::vec3(0.000000, 0.000000, 0.291947) * Multiplicator;
		glm::vec3 Max = glm::vec3(0.014455, 2.464900, 0.307312) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(0.118330, 0.696310, 0.286380) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(0.064130, 0.724510, 0.218980) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(0.006580, 0.760780, 0.276570) * Multiplicator;
		glm::vec3 Min = glm::vec3(0.014455, 0.000000, 0.261216) * Multiplicator;
		glm::vec3 Max = glm::vec3(0.028910, 2.464900, 0.276581) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(0.038800, 2.239700, 0.398700) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(0.000000, 2.227500, 0.427600) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(0.107900, 2.196200, 0.423900) * Multiplicator;
		glm::vec3 Min = glm::vec3(0.000000, 0.000000, 0.401198) * Multiplicator;
		glm::vec3 Max = glm::vec3(0.036145, 3.415800, 0.434632) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(1.855600, 1.664400, 8.358300) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(1.864200, 1.670400, 8.270300) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(1.799500, 1.645200, 8.370400) * Multiplicator;
		glm::vec3 Min = glm::vec3(1.843391, 0.000000, 8.324867) * Multiplicator;
		glm::vec3 Max = glm::vec3(1.879536, 3.415800, 8.358300) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(1.855600, 1.664400, 8.358300) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(1.799500, 1.645200, 8.370400) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(1.860500, 1.697100, 8.432200) * Multiplicator;
		glm::vec3 Min = glm::vec3(1.843391, 0.000000, 8.324867) * Multiplicator;
		glm::vec3 Max = glm::vec3(1.879536, 3.415800, 8.358300) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(7.089000, 2.791000, 8.296500) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(7.092500, 2.792900, 8.258000) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(7.069100, 2.724800, 8.282000) * Multiplicator;
		glm::vec3 Min = glm::vec3(7.084405, 0.000000, 8.224567) * Multiplicator;
		glm::vec3 Max = glm::vec3(7.120550, 3.415800, 8.258000) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(2.007777, 0.856372, 0.505804) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(2.032649, 0.816146, 0.647117) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(2.159220, 0.826750, 0.591183) * Multiplicator;
		glm::vec3 Min = glm::vec3(2.156437, 0.000000, 0.579817) * Multiplicator;
		glm::vec3 Max = glm::vec3(2.168156, 1.000000, 0.589814) * Multiplicator;*/

		/*glm::vec3 FirstPoint = glm::vec3(2.007777, 0.856372, 0.505804) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(2.159220, 0.826750, 0.591183) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(2.318697, 0.860085, 0.560189) * Multiplicator;
		glm::vec3 Min = glm::vec3(2.156437, 0.000000, 0.579817) * Multiplicator;
		glm::vec3 Max = glm::vec3(2.168156, 1.000000, 0.589814) * Multiplicator;*/

		//glm::vec3 FirstPoint = glm::vec3(1.315981, 0.518629, 0.995136) * Multiplicator;
		//glm::vec3 SecondPoint = glm::vec3(1.331583, 0.601360, 0.932809) * Multiplicator;
		//glm::vec3 ThirdPoint = glm::vec3(1.190084, 0.657671, 0.872898) * Multiplicator;
		//glm::vec3 Min = glm::vec3(1.195416, 0.000000, 0.879723) * Multiplicator;
		//glm::vec3 Max = glm::vec3(1.207136, 1.000000, 0.889719) * Multiplicator;


		glm::vec3 FirstPoint = glm::vec3(19.337408, 6.413627, 1.789177) * Multiplicator;
		glm::vec3 SecondPoint = glm::vec3(19.342543, 6.330275, 1.850492) * Multiplicator;
		glm::vec3 ThirdPoint = glm::vec3(19.374857, 6.348990, 1.842657) * Multiplicator;
		glm::vec3 Min = glm::vec3(19.332293, 6.258456, 0.000000) * Multiplicator;
		glm::vec3 Max = glm::vec3(19.492064, 6.427604, 3.346664) * Multiplicator;



		FEAABB TempAABB = FEAABB(Min, Max);
		/*std::vector<glm::vec3> Points;
		TempAABB.RayIntersect(ThirdPoint, FirstPoint - ThirdPoint, Points);*/


		
			


		std::vector<glm::vec3> CurrentTriangle;
		CurrentTriangle.push_back(FirstPoint);
		CurrentTriangle.push_back(SecondPoint);
		CurrentTriangle.push_back(ThirdPoint);


		bool res = GEOMETRY.IsAABBIntersectTriangle(TempAABB, CurrentTriangle);

		std::vector<glm::dvec3> CurrentTriangleDouble;
		CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z));
		CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z));
		CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z));



		std::vector<glm::dvec3> DebugResult;
		DebugResult = GEOMETRY.GetIntersectionPoints(TempAABB, CurrentTriangleDouble);

		for (size_t i = 0; i < DebugResult.size(); i++)
		{
			LINE_RENDERER.AddLineToBuffer(FECustomLine(DebugResult[i], DebugResult[i] + glm::dvec3(0.0, 1.0, 0.0), glm::vec3(1.0f, 0.0f, 0.0f)));
		}

		for (size_t l = 0; l < CurrentTriangleDouble.size(); l++)
		{
			if (TempAABB.ContainsPoint(CurrentTriangleDouble[l]))
			{
				bool bAlreadyExists = false;
				int PointsThatAreNotSame = 0;
				for (size_t q = 0; q < DebugResult.size(); q++)
				{
					if (abs(DebugResult[q] - CurrentTriangleDouble[l]).x > glm::dvec3(DBL_EPSILON).x ||
						abs(DebugResult[q] - CurrentTriangleDouble[l]).y > glm::dvec3(DBL_EPSILON).y ||
						abs(DebugResult[q] - CurrentTriangleDouble[l]).z > glm::dvec3(DBL_EPSILON).z)
					{
						PointsThatAreNotSame++;
					}
				}

				if (PointsThatAreNotSame != DebugResult.size())
					bAlreadyExists = true;
				
				if (!bAlreadyExists)
					DebugResult.push_back(CurrentTriangle[l]);
			}
		}

		/*if (TempAABB.ContainsPoint(CurrentTriangleDouble[0]))
		{
			DebugResult.push_back(CurrentTriangleDouble[0]);
		}

		if (TempAABB.ContainsPoint(CurrentTriangleDouble[1]))
		{
			DebugResult.push_back(CurrentTriangleDouble[1]);
		}

		if (TempAABB.ContainsPoint(CurrentTriangleDouble[2]))
		{
			DebugResult.push_back(CurrentTriangleDouble[2]);
		}*/

		double CurrentTrianlgeArea = 0.0;

		if (DebugResult.size() > 2)
		{
			//DebugCount++;

			// Calculate the area of the clipped polygon
			double Area = 0.0;
			for (size_t p = 0; p < DebugResult.size(); p++)
			{
				const glm::dvec3& v1 = DebugResult[p];
				const glm::dvec3& v2 = DebugResult[(p + 1) % DebugResult.size()];
				Area += v1.x * v2.y - v2.x * v1.y;
			}
			CurrentTrianlgeArea = std::abs(Area) * 0.5;


			//double Area_ = QuadrilateralArea_(DebugResult);

			CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetArea(DebugResult);

			if (CurrentTrianlgeArea == 0.0)
			{
				double Area = 0.0;
				for (size_t p = 0; p < DebugResult.size(); p++)
				{
					DebugResult[p] *= 100.0;
					const glm::dvec3& v1 = DebugResult[p];
					const glm::dvec3& v2 = DebugResult[(p + 1) % DebugResult.size()];
					Area += v1.x * v2.y - v2.x * v1.y;
				}
				CurrentTrianlgeArea = std::abs(Area) * 0.5;
			}
		}

		CurrentTrianlgeArea /= 100.0;

		//float alternative = CalculateTriangleAreaInsideAABB(TempAABB, CurrentTriangle);	





		LINE_RENDERER.RenderAABB(FEAABB(Min, Max), glm::vec3(0.0f, 0.0f, 1.0f));


		LINE_RENDERER.AddLineToBuffer(FECustomLine(FirstPoint, SecondPoint, glm::vec3(1.0f, 0.0f, 1.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(SecondPoint, ThirdPoint, glm::vec3(0.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(ThirdPoint, FirstPoint, glm::vec3(0.0f, 0.0f, 1.0f)));

		//ExportCurrentLayerAsMap();

		LINE_RENDERER.SyncWithGPU();
	}
	




	LINE_RENDERER.Render();

	UI.Render();

	if (FirstFrame)
	{
		FirstFrame = false;
		UI.ApplyStandardWindowsSizeAndPosition();
	}
}

void GDALLoadTest()
{
	//GDALAllRegister(); // Initialize GDAL





	//const char* filename = "output.tif";
	//int xSize = 1024; // Width of the dataset
	//int ySize = 1024; // Height of the dataset
	//int nBands = 1; // Number of bands
	//GDALDataType type = GDT_Float32; // Data type of the bands

	//// Get the GeoTIFF driver
	//GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	//if (driver == nullptr) {
	//	std::cerr << "GTiff driver not available." << std::endl;
	//	exit(1);
	//}

	//// Create a new GeoTIFF file
	//GDALDataset* dataset = driver->Create(filename, xSize, ySize, nBands, type, nullptr);
	//if (dataset == nullptr) {
	//	std::cerr << "Creation of output file failed." << std::endl;
	//	exit(1);
	//}

	//// Set geotransform and projection if necessary
	//double geotransform[6] = { 0, 1, 0, 0, 0, -1 }; // Example values
	//dataset->SetGeoTransform(geotransform); // Set the affine transformation coefficients
	//dataset->SetProjection("WGS84"); // Set the projection

	//// Allocate data buffer for a single band
	//float* pData = (float*)CPLMalloc(sizeof(float) * xSize * ySize);

	//// Populate pData with your data

	//float value = 1.0f;
	//for (size_t i = 0; i < xSize; i++)
	//{
	//	for (size_t j = 0; j < ySize; j++)
	//	{
	//		pData[i * xSize + j] = value++;
	//	}
	//}

	//// Write data to the first band
	//GDALRasterBand* band = dataset->GetRasterBand(1);
	//CPLErr err = band->RasterIO(GF_Write, 0, 0, xSize, ySize, pData, xSize, ySize, type, 0, 0);
	//if (err != CE_None) {
	//	std::cerr << "Error writing data to file." << std::endl;
	//	CPLFree(pData);
	//	GDALClose(dataset);
	//	exit(1);
	//}

	//// Cleanup
	//CPLFree(pData);
	//GDALClose(dataset);











	//// Create a new GeoTIFF dataset
	//const char* outputFilename = "output.tif";
	//int width = 1000;
	//int height = 1000;
	//int bandCount = 1;
	//GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	//GDALDataset* outputDataset = driver->Create(outputFilename, width, height, bandCount, GDT_Float32, NULL);

	//// Set geospatial metadata
	//double geoTransform[6] = { 0, 1, 0, 0, 0, -1 }; // Example geotransform
	//outputDataset->SetGeoTransform(geoTransform);
	//const char* projection = "EPSG:4326"; // Example projection (WGS84)
	//outputDataset->SetProjection(projection);

	//// Write float 32-bit data to the dataset
	//float* data = new float[width * height];

	//float value = 1.0f;
	//for (size_t i = 0; i < width; i++)
	//{
	//	for (size_t j = 0; j < height; j++)
	//	{
	//		data[i * width + j] = value++;
	//	}
	//}
	//// Fill the data array with your float 32-bit information
	//// ...

	//GDALRasterBand* band = outputDataset->GetRasterBand(1);
	//band->WriteBlock(0, 0, data);

	//// Close the dataset
	//GDALClose(outputDataset);
	//delete[] data;















	//const char* filename = "cea.tif";
	//GDALDataset* dataset = static_cast<GDALDataset*>(GDALOpen(filename, GA_ReadOnly));
	//if (dataset == nullptr) {
	//	std::cerr << "Error opening file" << std::endl;
	//	//return 1;
	//}

	//// Basic info about the raster
	//std::cout << "Size is " << dataset->GetRasterXSize() << "x" << dataset->GetRasterYSize()
	//	<< ", " << dataset->GetRasterCount() << " bands." << std::endl;

	//// Accessing geospatial metadata
	//double adfGeoTransform[6];
	//if (dataset->GetGeoTransform(adfGeoTransform) == CE_None) {
	//	std::cout << "Origin = (" << adfGeoTransform[0] << ", "
	//		<< adfGeoTransform[3] << ")" << std::endl;
	//	std::cout << "Pixel Size = (" << adfGeoTransform[1] << ", "
	//		<< adfGeoTransform[5] << ")" << std::endl;
	//}

	//const char* proj = dataset->GetProjectionRef();
	//if (proj != nullptr) {
	//	std::cout << "Projection is: " << proj << std::endl;
	//}

	//GDALClose(dataset);
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
		GDALLoadTest();
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