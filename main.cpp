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
			SETTINGS_WINDOW.SetIsModelCamera(true);

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
	ANALYSIS_OBJECT_MANAGER.LoadResource(FileName);
}

void MouseButtonCallback(int Button, int Action, int Mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
		return;
	}

	if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_PRESS)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(true);
	}
	else if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_RELEASE)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
	}
}

void WindowResizeCallback(int Width, int Height)
{
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

	if (UI_INSPECTOR.ShouldTakeScreenshot())
	{
		ClearColor.w = UI_INSPECTOR.ShouldUseTransparentBackground() ? 0.0f : 1.0f;
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);

		UI_INSPECTOR.SetShouldTakeScreenshot(false);
		SCREENSHOT_MANAGER.TakeScreenshot();
		return;
	}

	bool bGraphDebugWindow = true;
	if (bGraphDebugWindow)
	{
		static float XValue = 0.0f;
		ImGui::InputFloat("x value of data point", &XValue, 0.1f, 1.0f, "%.3f");

		static float YValue = 0.0f;
		ImGui::InputFloat("y value of data point", &YValue, 0.1f, 1.0f, "%.3f");

		static int StackID = 0;
		ImGui::InputInt("stack ID of data point", &StackID);

		if (ImGui::Button("Add data point to a graph"))
		{
			FEGraphDataPoint NewDataPoint;
			NewDataPoint.XValue = XValue;
			NewDataPoint.YValue = YValue;
			NewDataPoint.StackID = StackID;
			UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints({ NewDataPoint });
		}

		ImGui::Separator();

		static std::string FirstChoosenActiveObjectID;
		static int FirstChoosenLayerIndex = -1;
		static std::string SecondChoosenActiveObjectID;
		static int SecondChoosenLayerIndex = -1;

		bool bHaveAnyObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() > 0;
		if (bHaveAnyObject)
		{
			AnalysisObject* FirstObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(FirstChoosenActiveObjectID);
			std::string FirstActiveObjectString = "Choose active object";
			if (!FirstChoosenActiveObjectID.empty() && FirstObject != nullptr)
				FirstActiveObjectString = FirstObject->GetName();

			// First layer object combo box
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("First active object: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			if (ImGui::BeginCombo("##ChooseFirstActiveObject", FirstActiveObjectString.c_str(), ImGuiWindowFlags_None))
			{
				std::vector<std::string> AllObjectIDs = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectsIDList();
				for (size_t i = 0; i < AllObjectIDs.size(); i++)
				{
					AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AllObjectIDs[i]);
					if (CurrentObject == nullptr)
						continue;

					bool bIsSelected = (AllObjectIDs[i] == FirstChoosenActiveObjectID);
					if (ImGui::Selectable(CurrentObject->GetName().c_str(), bIsSelected))
					{
						FirstChoosenActiveObjectID = AllObjectIDs[i];
						FirstChoosenLayerIndex = -1;
					}
					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			std::string FirstLayerString = "Choose layer";
			if (FirstChoosenLayerIndex != -1 && FirstObject != nullptr)
				FirstLayerString = FirstObject->Layers[FirstChoosenLayerIndex]->GetCaption();

			// First layer combo box
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("First layer: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			if (FirstObject != nullptr)
			{
				if (ImGui::BeginCombo("##ChooseFirstLayer", FirstLayerString.c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < FirstObject->GetLayerCount(); i++)
					{
						bool bIsSelected = (i == FirstChoosenLayerIndex);
						if (ImGui::Selectable(FirstObject->Layers[i]->GetCaption().c_str(), bIsSelected))
						{
							FirstChoosenLayerIndex = static_cast<int>(i);
						}

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			AnalysisObject* SecondObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(SecondChoosenActiveObjectID);
			std::string SecondActiveObjectString = "Choose active object";
			if (!SecondActiveObjectString.empty() && SecondObject != nullptr)
				SecondActiveObjectString = SecondObject->GetName();

			// Second layer object combo box
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("Second active object: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			if (ImGui::BeginCombo("##ChooseSecondActiveObject", SecondActiveObjectString.c_str(), ImGuiWindowFlags_None))
			{
				std::vector<std::string> AllObjectIDs = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectsIDList();
				for (size_t i = 0; i < AllObjectIDs.size(); i++)
				{
					AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AllObjectIDs[i]);
					if (CurrentObject == nullptr)
						continue;
					bool bIsSelected = (AllObjectIDs[i] == SecondChoosenActiveObjectID);
					if (ImGui::Selectable(CurrentObject->GetName().c_str(), bIsSelected))
					{
						SecondChoosenActiveObjectID = AllObjectIDs[i];
						SecondChoosenLayerIndex = -1;
					}
					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			std::string SecondString = "Choose layer";
			if (SecondChoosenLayerIndex != -1 && SecondObject != nullptr)
				SecondString = SecondObject->Layers[SecondChoosenLayerIndex]->GetCaption();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("Second layer: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			if (SecondObject != nullptr)
			{
				if (ImGui::BeginCombo("##ChooseSecondLayer", SecondString.c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < SecondObject->Layers.size(); i++)
					{
						bool bIsSelected = (i == SecondChoosenLayerIndex);
						if (ImGui::Selectable(SecondObject->Layers[i]->GetCaption().c_str(), bIsSelected))
						{
							SecondChoosenLayerIndex = static_cast<int>(i);
						}

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			if (ImGui::Button("Update histogram data"))
			{
				if (FirstChoosenLayerIndex != -1 && SecondChoosenLayerIndex != -1 &&
					FirstObject != nullptr && SecondObject != nullptr)
				{
					UI.GetHistogramPointer()->Clear();

					DataLayer* FirstLayer = FirstObject->Layers[FirstChoosenLayerIndex];
					DataLayer* SecondLayer = SecondObject->Layers[SecondChoosenLayerIndex];

					if (FirstLayer->ValueWeightAndIndex.empty() || SecondLayer->ValueWeightAndIndex.empty())
						return;

					std::vector<double> Values;
					std::vector<double> Weights;
					int BinsCount = 128;

					for (const auto& Tuple : FirstLayer->ValueWeightAndIndex)
					{
						Values.push_back(std::get<0>(Tuple));
						Weights.push_back(std::get<1>(Tuple));
					}

					std::vector<FEGraphDataPoint> FirstGraphDataPoints = UI.GetHistogramPointer()->ConvertToDataPoints(Values, Weights, BinsCount);
					Values.clear();
					Weights.clear();

					for (const auto& Tuple : SecondLayer->ValueWeightAndIndex)
					{
						Values.push_back(std::get<0>(Tuple));
						Weights.push_back(std::get<1>(Tuple));
					}

					std::vector<FEGraphDataPoint> SecondGraphDataPoints = UI.GetHistogramPointer()->ConvertToDataPoints(Values, Weights, BinsCount);
					for (size_t i = 0; i < SecondGraphDataPoints.size(); i++)
					{
						SecondGraphDataPoints[i].StackID = 1;
					}

					UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(FirstGraphDataPoints);
					UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(SecondGraphDataPoints);
				}
			}

			if (ImGui::Button("Add Annotations to graph"))
			{
				UI.GetHistogramPointer()->Clear();

				AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
				if (ActiveObject == nullptr)
					return;

				DataLayer* ActiveLayerData = ActiveObject->GetActiveLayer();
				AnnotationData* AnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(ActiveObject->GetID());
				if (AnnotationData != nullptr && ActiveLayerData != nullptr)
				{
					MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
					if (CurrentMeshAnalysisData == nullptr)
						return;

					FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
					if (ActiveMesh == nullptr)
						return;

					std::vector<std::tuple<double, double, int>> NoAnnotationHistogramData;
					for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
					{
						if (AnnotationData->PerTriangleID[i] == -1)
						{
							double CurrentLayerTriangleValue = ActiveLayerData->ElementsToData[i];
							NoAnnotationHistogramData.push_back(std::make_tuple(CurrentLayerTriangleValue, CurrentMeshAnalysisData->TrianglesArea[i], i));
						}
					}

					std::vector<double> Values;
					std::vector<double> Weights;
					int BinsCount = 128;

					for (const auto& Tuple : NoAnnotationHistogramData)
					{
						Values.push_back(std::get<0>(Tuple));
						Weights.push_back(std::get<1>(Tuple));
					}

					std::vector<FEGraphDataPoint> NoAnnotationGraphDataPoints = UI.GetHistogramPointer()->ConvertToDataPoints(Values, Weights, BinsCount);
					UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(NoAnnotationGraphDataPoints);

					Values.clear();
					Weights.clear();

					std::vector<AnnotationInfo> AllAnnotationInfo = AnnotationData->GetAllAnnotationInfos();
					std::vector<std::vector<FEGraphDataPoint>> AnnotationGraphDataPoints;
					std::unordered_map<int, int> AnnotationIDToGraphStackIndex;
					int GraphStackIndex = 1;
					for (size_t i = 0; i < AllAnnotationInfo.size(); i++)
					{
						if (AllAnnotationInfo[i].HistogramData.empty())
							continue;

						for (const auto& Tuple : AllAnnotationInfo[i].HistogramData)
						{
							Values.push_back(std::get<0>(Tuple));
							Weights.push_back(std::get<1>(Tuple));
						}

						std::vector<FEGraphDataPoint> CurrentAnnotationGraphDataPoints = UI.GetHistogramPointer()->ConvertToDataPoints(Values, Weights, BinsCount);
						for (size_t j = 0; j < CurrentAnnotationGraphDataPoints.size(); j++)
							CurrentAnnotationGraphDataPoints[j].StackID = GraphStackIndex;

						if (!CurrentAnnotationGraphDataPoints.empty())
						{
							AnnotationIDToGraphStackIndex[AllAnnotationInfo[i].ID] = GraphStackIndex;
							AllAnnotationInfo[i].StackGraphIndex = GraphStackIndex;
							GraphStackIndex++;
						}
						else
						{
							AllAnnotationInfo[i].StackGraphIndex = -1;
						}

						AnnotationGraphDataPoints.push_back(CurrentAnnotationGraphDataPoints);
					}

					for (size_t i = 0; i < AnnotationGraphDataPoints.size(); i++)
					{
						UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(AnnotationGraphDataPoints[i]);
					}


					/*for (size_t i = 0; i < AnnotationGraphDataPoints.size(); i++)
					{
						int CurrentAnnotationID = AllAnnotationInfo[i].ID;
						if (AnnotationIDToGraphStackIndex.find(CurrentAnnotationID) != AnnotationIDToGraphStackIndex.end())
						{
							int CurrentGraphStackIndex = AnnotationIDToGraphStackIndex[CurrentAnnotationID];
							FEGraphStackInfo* CurrentStackInfo = UI.GetHistogramPointer()->GetGraphPointer()->GetStackInfoByID(CurrentGraphStackIndex);
							if (CurrentStackInfo != nullptr)
							{
								glm::vec4 Color = AllAnnotationInfo[i].GetColor();
								CurrentStackInfo->StartGradientColor = ImColor(Color.x, Color.y, Color.z);
								CurrentStackInfo->EndGradientColor = ImColor(Color.x, Color.y, Color.z);
							}
						}
					}*/

					for (size_t i = 0; i < AllAnnotationInfo.size(); i++)
					{
						int StackGraphIndex = AllAnnotationInfo[i].StackGraphIndex;
						if (StackGraphIndex != -1)
						{
							FEGraphStackInfo* CurrentStackInfo = UI.GetHistogramPointer()->GetGraphPointer()->GetStackInfoByID(StackGraphIndex);
							if (CurrentStackInfo != nullptr)
							{
								glm::vec4 Color = AllAnnotationInfo[i].GetColor();
								CurrentStackInfo->StartGradientColor = ImColor(Color.x, Color.y, Color.z);
								CurrentStackInfo->EndGradientColor = ImColor(Color.x, Color.y, Color.z);
							}
						}


						/*int CurrentAnnotationID = AllAnnotationInfo[i].ID;
						if (AnnotationIDToGraphStackIndex.find(CurrentAnnotationID) != AnnotationIDToGraphStackIndex.end())
						{
							int CurrentGraphStackIndex = AnnotationIDToGraphStackIndex[CurrentAnnotationID];
							FEGraphStackInfo* CurrentStackInfo = UI.GetHistogramPointer()->GetGraphPointer()->GetStackInfoByID(CurrentGraphStackIndex);
							if (CurrentStackInfo != nullptr)
							{
								glm::vec4 Color = AllAnnotationInfo[i].GetColor();
								CurrentStackInfo->StartGradientColor = ImColor(Color.x, Color.y, Color.z);
								CurrentStackInfo->EndGradientColor = ImColor(Color.x, Color.y, Color.z);
							}
						}*/
					}
					
					



				}

			}

			// Here I should have input for reordering stacks
			// It should accept string like "2,0,1" where each number is stack index
			static char OrderInputBuffer[1024] = "";
			ImGui::InputText("Stack Indices (e.g. 2,0,1)", OrderInputBuffer, sizeof(OrderInputBuffer));

			if (ImGui::Button("Reorder Stacks"))
			{
				std::vector<int> NewOrder;

				std::string StringRepresentation(OrderInputBuffer);
				std::stringstream StringStream(StringRepresentation);
				std::string Token;

				while (std::getline(StringStream, Token, ','))
				{
					try {
						// std::stoi converts string to int and handles whitespace automatically
						NewOrder.push_back(std::stoi(Token));
					}
					catch (...) {
						// Catch invalid inputs (like letters or empty strings) so the app doesn't crash
					}
				}

				UI.GetHistogramPointer()->GetGraphPointer()->ChangeStackOrder(NewOrder);
			}
		}

		ImGui::Separator();
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
	}

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

void TestReadShapeFile()
{
	std::vector<std::vector<glm::vec2>> polygons;
	std::vector<glm::vec2> polylines;

	//auto* ds = (GDALDataset*)GDALOpenEx("ne_110m_admin_0_countries/ne_110m_admin_0_countries.shp", GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	auto* ds = (GDALDataset*)GDALOpenEx("D:/CloudBathymetry_Client_Master/Resources/NCCoast50m/NCCoast50m.shp", GDAL_OF_VECTOR, nullptr, nullptr, nullptr);


	auto* layer = ds->GetLayer(0);

	//double x = 0.0; // longitude
	//double y = 0.0; // latitude

	//OGRPoint testPoint(x, y);

	//for (auto& feature : layer) {
	//	OGRGeometry* geom = feature->GetGeometryRef();
	//	if (geom->Contains(&testPoint)) {
	//		// inside this polygon
	//	}
	//}

	for (auto& feature : layer) {
		OGRGeometry* geom = feature->GetGeometryRef();

		if (auto* poly = dynamic_cast<OGRPolygon*>(geom)) {
			// single polygon
			OGRLinearRing* ring = poly->getExteriorRing();
			int n = ring->getNumPoints();
			std::vector<glm::vec2> points(n);
			for (int i = 0; i < n; i++)
				points[i] = glm::vec2(ring->getX(i), ring->getY(i));
			polygons.push_back(std::move(points));
		}
		else if (auto* multi = dynamic_cast<OGRMultiPolygon*>(geom)) {
			for (int p = 0; p < multi->getNumGeometries(); p++) {
				auto* poly = dynamic_cast<OGRPolygon*>(multi->getGeometryRef(p));
				if (!poly) continue;
				OGRLinearRing* ring = poly->getExteriorRing();
				int n = ring->getNumPoints();
				std::vector<glm::vec2> points(n);
				for (int i = 0; i < n; i++)
					points[i] = glm::vec2(ring->getX(i), ring->getY(i));
				polygons.push_back(std::move(points));
			}
		}
		else if (auto* line = dynamic_cast<OGRLineString*>(geom)) {
			int n = line->getNumPoints();
			//std::vector<glm::vec2> points(n);
			for (int i = 0; i < n; i++)
				polylines.push_back(glm::vec2(line->getX(i), line->getY(i)));
			//points[i] = glm::vec2(line->getX(i), line->getY(i));

		//polylines = std::move(points);
		//polylines.push_back(std::move(points));
		}
	}

	int layerCount = ds->GetLayerCount();
	std::string LayerInfo = "Number of layers: " + std::to_string(layerCount) + "\n";

	for (int i = 0; i < layerCount; i++) {
		OGRLayer* layer = ds->GetLayer(i);
		const char* name = layer->GetName();
		int featureCount = static_cast<int>(layer->GetFeatureCount());

		OGRwkbGeometryType type = layer->GetGeomType();
		const char* typeName = OGRGeometryTypeToName(type);

		LayerInfo += "Layer " + std::to_string(i) + ": '" + name + "' | Features: " + std::to_string(featureCount) + " | Geometry: " + typeName + "\n";

		//printf("Layer %d: '%s' | Features: %d | Geometry: %s\n", i, name, featureCount, typeName);
	}

	GDALClose(ds);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//LOG.SetFileOutput(true);
	GDALAllRegister();

	const auto ProcessorCount = THREAD_POOL.GetLogicalCoreCount();
	const unsigned int HowManyToUse = ProcessorCount > 4 ? ProcessorCount - 2 : 1;

	THREAD_POOL.SetConcurrentThreadCount(HowManyToUse);

	bool bIsConsoleModeRequested = false;
	std::vector<CommandLineAction> ParsedCommandActions;

	ParsedCommandActions = APPLICATION.ParseCommandLine(lpCmdLine);
	if (!ParsedCommandActions.empty())
		std::transform(ParsedCommandActions[0].Action.begin(), ParsedCommandActions[0].Action.end(), ParsedCommandActions[0].Action.begin(), [](unsigned char c) { return std::tolower(c); });

	//TestReadShapeFile();

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

		ANALYSIS_OBJECT_MANAGER.AddOnObjectLoadCallback(AfterNewResourceLoads);

		SCREENSHOT_MANAGER.Init();
		DEVELOPER_MODE.Initialize();
		ANNOTATION_MANAGER.Initialize();

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