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

// START OF THE CODE FROM LayerRasterizationManager.h

struct GridCell
{
	FEAABB AABB;
	//float AABBVolume = 0.0f;
	std::vector<int> TrianglesInCell;
	float Value = 0.0f;
	//std::vector<float> TriangleAABBOverlaps;
};

std::vector<std::vector<GridCell>> GenerateGridProjection(FEAABB& OriginalAABB, const glm::vec3& Axis, int Resolution)
{
	std::vector<std::vector<GridCell>> Grid;

	if (Axis.x + Axis.y + Axis.z != 1.0f)
		return Grid;

	Grid.resize(Resolution);
	for (int i = 0; i < Resolution; i++)
	{
		Grid[i].resize(Resolution);
	}

	// Get the original AABB min and max vectors
	glm::vec3 Min = OriginalAABB.GetMin();
	glm::vec3 Max = OriginalAABB.GetMax();

	// Determine the number of divisions along each axis
	glm::vec3 Size = Max - Min;
	glm::vec3 DivisionSize = Size / static_cast<float>(Resolution);

	// Fix the division size for the specified axis to cover the full length of the original AABB
	if (Axis.x > 0.0) DivisionSize.x = Size.x;
	if (Axis.y > 0.0) DivisionSize.y = Size.y;
	if (Axis.z > 0.0) DivisionSize.z = Size.z;

	// Loop through each division to create the grid
	for (int i = 0; i < Resolution; i++)
	{
		for (int j = 0; j < Resolution; j++)
		{
			// Calculate min and max for the current cell
			glm::vec3 CellMin = Min;
			glm::vec3 CellMax = Min + DivisionSize;

			if (Axis.x > 0.0f)
			{
				CellMin.y += DivisionSize.y * i;
				CellMax.y += DivisionSize.y * i;

				CellMin.z += DivisionSize.z * j;
				CellMax.z += DivisionSize.z * j;
			}
			else if (Axis.y > 0.0f)
			{
				CellMin.x += DivisionSize.x * i;
				CellMax.x += DivisionSize.x * i;

				CellMin.z += DivisionSize.z * j;
				CellMax.z += DivisionSize.z * j;
			}
			else if (Axis.z > 0.0f)
			{
				CellMin.x += DivisionSize.x * i;
				CellMax.x += DivisionSize.x * i;

				CellMin.y += DivisionSize.y * j;
				CellMax.y += DivisionSize.y * j;
			}

			// Ensure we don't exceed original bounds due to floating point arithmetic
			CellMax = glm::min(CellMax, Max);

			// Create a new AABB for the grid cell
			GridCell NewCell;
			NewCell.AABB = FEAABB(CellMin, CellMax);
			//NewCell.AABBVolume = NewCell.AABB.GetVolume();
			Grid[i][j] = NewCell;
		}
	}

	return Grid;
}

glm::vec3 ConvertToClosestAxis(const glm::vec3& Vector)
{
	// Calculate the absolute values of the vector components
	float absX = glm::abs(Vector.x);
	float absY = glm::abs(Vector.y);
	float absZ = glm::abs(Vector.z);

	// Determine the largest component
	if (absX > absY && absX > absZ)
	{
		// X component is largest, so vector is closest to the X axis
		return glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else if (absY > absX && absY > absZ)
	{
		// Y component is largest, so vector is closest to the Y axis
		return glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else
	{
		// Z component is largest (or it's a tie, in which case we default to Z), so vector is closest to the Z axis
		return glm::vec3(0.0f, 0.0f, 1.0f);
	}
}

struct GridUpdateTask
{
	int FirstIndex = -1;
	int SecondIndex = -1;
	int TriangleIndexToAdd = -1;

	GridUpdateTask::GridUpdateTask(int FirstIndex, int SecondIndex, int TriangleIndexToAdd)
		: FirstIndex(FirstIndex), SecondIndex(SecondIndex), TriangleIndexToAdd(TriangleIndexToAdd)
	{
	}
};

struct GridRasterizationThreadData
{
	std::vector<std::vector<GridCell>>* Grid;
	glm::vec3 UpAxis;
	int Resolution;
	int FirstIndexInTriangleArray;
	int LastIndexInTriangleArray;
};

void GridRasterizationThread(void* InputData, void* OutputData)
{
	GridRasterizationThreadData* Input = reinterpret_cast<GridRasterizationThreadData*>(InputData);
	std::vector<GridUpdateTask>* Output = reinterpret_cast<std::vector<GridUpdateTask>*>(OutputData);

	std::vector<std::vector<GridCell>>& Grid = *Input->Grid;
	const glm::vec3 UpAxis = Input->UpAxis;
	const int Resolution = Input->Resolution;
	const int FirstIndexInTriangleArray = Input->FirstIndexInTriangleArray;
	const int LastIndexInTriangleArray = Input->LastIndexInTriangleArray;

	glm::vec3 CellSize = Grid[0][0].AABB.GetSize();
	const glm::vec3 GridMin = Grid[0][0].AABB.GetMin();
	const glm::vec3 GridMax = Grid[Resolution - 1][Resolution - 1].AABB.GetMax();

	//double TotalTime = 0.0;
	//double TimeTakenFillCellsWithTriangleInfo = 0.0;

	//TIME.BeginTimeStamp("Total time");


	if (UpAxis.y > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l < LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			int XEnd = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			int XBegin = static_cast<int>(Distance / CellSize.x) - 1;
			if (XBegin < 0)
				XBegin = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			XEnd -= static_cast<int>(Distance / CellSize.x);
			XEnd++;
			if (XEnd >= Resolution)
				XEnd = Resolution;

			for (size_t i = XBegin; i < XEnd; i++)
			{
				int ZEnd = Resolution;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
				int ZBegin = static_cast<int>(Distance / CellSize.z) - 1;
				if (ZBegin < 0)
					ZBegin = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
				ZEnd -= static_cast<int>(Distance / CellSize.z);
				ZEnd++;
				if (ZEnd >= Resolution)
					ZEnd = Resolution;

				for (size_t j = ZBegin; j < ZEnd; j++)
				{
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Output->push_back(GridUpdateTask( i, j, l )); //Grid[i][j].TrianglesInCell.push_back(l);
				}
			}
		}

	}
	else if (UpAxis.z > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l < LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			int XEnd = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			int XBegin = static_cast<int>(Distance / CellSize.x) - 1;
			if (XBegin < 0)
				XBegin = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			XEnd -= static_cast<int>(Distance / CellSize.x);
			XEnd++;
			if (XEnd >= Resolution)
				XEnd = Resolution;

			for (size_t i = XBegin; i < XEnd; i++)
			{
				int YEnd = static_cast<int>(Resolution);

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
				int YBegin = static_cast<int>(Distance / CellSize.y) - 1;
				if (YBegin < 0)
					YBegin = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
				YEnd -= static_cast<int>(Distance / CellSize.y);
				YEnd++;
				if (YEnd >= Resolution)
					YEnd = Resolution;

				for (size_t j = YBegin; j < YEnd; j++)
				{
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Output->push_back(GridUpdateTask(i, j, l)); //Grid[i][j].TrianglesInCell.push_back(l);
				}
			}
		}
	}
}

std::vector<GridUpdateTask> MainThreadGridUpdateTasks;
std::vector<std::vector<GridCell>> Grid;
int GatherGridRasterizationThreadCount = 0;
MeshLayer* CurrentLayer = nullptr;
int CurrentResolution = 256;
int GridRasterizationMode = 2;
glm::vec3 CurrentUpAxis = glm::vec3(0.0f);
const int GridRasterizationModeMin = 0;
const int GridRasterizationModeMax = 1;
const int GridRasterizationModeMean = 2;
const int GridRasterizationModeCumulative = 3;
int CumulativeOutliers = 1;

//// Function to clip a line segment against an AABB
//std::vector<glm::vec3> clipLineSegment(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& min, const glm::vec3& max) {
//	std::vector<glm::vec3> clippedPoints;
//
//	float tEnter = 0.0f;
//	float tLeave = 1.0f;
//
//	for (int i = 0; i < 3; i++) {
//		if (p1[i] < min[i] && p2[i] < min[i]) return clippedPoints;
//		if (p1[i] > max[i] && p2[i] > max[i]) return clippedPoints;
//
//		float tEnterPlane = (min[i] - p1[i]) / (p2[i] - p1[i]);
//		float tLeavePlane = (max[i] - p1[i]) / (p2[i] - p1[i]);
//
//		if (tEnterPlane > tEnter) tEnter = tEnterPlane;
//		if (tLeavePlane < tLeave) tLeave = tLeavePlane;
//	}
//
//	if (tEnter <= tLeave) {
//		clippedPoints.push_back(p1 + tEnter * (p2 - p1));
//		clippedPoints.push_back(p1 + tLeave * (p2 - p1));
//	}
//
//	return clippedPoints;
//}
//
//// Function to clip a triangle against an AABB
//std::vector<glm::vec3> clipTriangle(const std::vector<glm::vec3>& triangle, const glm::vec3& min, const glm::vec3& max) {
//	std::vector<glm::vec3> clippedPolygon;
//
//	clippedPolygon = clipLineSegment(triangle[0], triangle[1], min, max);
//	std::vector<glm::vec3> tempPolygon = clipLineSegment(triangle[1], triangle[2], min, max);
//	clippedPolygon.insert(clippedPolygon.end(), tempPolygon.begin(), tempPolygon.end());
//	tempPolygon = clipLineSegment(triangle[2], triangle[0], min, max);
//	clippedPolygon.insert(clippedPolygon.end(), tempPolygon.begin(), tempPolygon.end());
//
//	return clippedPolygon;
//}
//
//// Function to calculate the area of a polygon
//float calculatePolygonArea(const std::vector<glm::vec3>& polygon) {
//	float area = 0.0f;
//
//	for (size_t i = 0; i < polygon.size(); i++) {
//		const glm::vec3& p1 = polygon[i];
//		const glm::vec3& p2 = polygon[(i + 1) % polygon.size()];
//		area += p1.x * p2.y - p2.x * p1.y;
//	}
//
//	return std::abs(area) * 0.5f;
//}
//
//// Function to calculate the area of a triangle inside an AABB
//float calculateTriangleAreaInsideAABB(const std::vector<glm::vec3>& triangle, const glm::vec3& min, const glm::vec3& max) {
//	std::vector<glm::vec3> clippedPolygon = clipTriangle(triangle, min, max);
//	return calculatePolygonArea(clippedPolygon);
//}

void AfterAllGridRasterizationThreadFinished()
{
	//MessageBoxA(NULL, "GatherGridRasterizationThreadWork!", "Mouse Position", MB_OK);

	for (int i = 0; i < MainThreadGridUpdateTasks.size(); i++)
	{
		Grid[MainThreadGridUpdateTasks[i].FirstIndex][MainThreadGridUpdateTasks[i].SecondIndex].TrianglesInCell.push_back(MainThreadGridUpdateTasks[i].TriangleIndexToAdd);
	}

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			if (!Grid[i][j].TrianglesInCell.empty())
			{
				float CurrentCellTotalArea = 0.0f;
				for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
				{
					CurrentCellTotalArea += static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]]);
				}

				if (CurrentCellTotalArea == 0.0f)
				{
					Grid[i][j].Value = 0.0f;
					continue;
				}

				float FinalResult = 0.0f;

				if (GridRasterizationMode == GridRasterizationModeMin)
				{
					float MinValue = FLT_MAX;

					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						float CurrentValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
						if (CurrentValue < MinValue)
							MinValue = CurrentValue;
					}

					FinalResult = MinValue;
				}
				else if (GridRasterizationMode == GridRasterizationModeMax)
				{
					float MaxValue = -FLT_MAX;

					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						float CurrentValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
						if (CurrentValue > MaxValue)
							MaxValue = CurrentValue;
					}

					FinalResult = MaxValue;
					
				}
				else if (GridRasterizationMode == GridRasterizationModeMean)
				{
					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						float CurrentTriangleCoef = static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]] / CurrentCellTotalArea);
						//float CurrentTriangleCoef = 1.0f;
						//float CurrentTriangleCoef = Grid[i][j].TriangleAABBOverlaps[k];
						FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]] * CurrentTriangleCoef;
					}

					//FinalResult /= static_cast<float>(Grid[i][j].TrianglesInCell.size());
					
				}
				else if (GridRasterizationMode == GridRasterizationModeCumulative)
				{
					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						/*float CurrentTrianlgeArea = calculateTriangleAreaInsideAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Grid[i][j].TrianglesInCell[k]], Grid[i][j].AABB.GetMin(), Grid[i][j].AABB.GetMax());
						if (CurrentTrianlgeArea == 0.0f)
							continue;

						float CurrentTriangleCoef = CurrentTrianlgeArea / CurrentCellTotalArea;
						if (CurrentTrianlgeArea == 0.0f || isnan(CurrentTriangleCoef))
							continue;*/

						FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]] /** CurrentTriangleCoef*/;
					}
				}

				Grid[i][j].Value = FinalResult;
				if (isnan(Grid[i][j].Value))
					Grid[i][j].Value = 0.0f;
			}
		}
	}

	float MinForColorMap = CurrentLayer->MinVisible;
	float MaxForColorMap = CurrentLayer->MaxVisible;
	if (GridRasterizationMode == GridRasterizationModeCumulative)
	{
		std::vector<float> FlattenGrid;
		FlattenGrid.reserve(Grid.size() * Grid[0].size());

		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				FlattenGrid.push_back(Grid[i][j].Value);
			}
		}

		JITTER_MANAGER.AdjustOutliers(FlattenGrid, 0.0f, (100 - CumulativeOutliers) / 100.0f);

		// Updated the grid values
		int Index = 0;
		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				Grid[i][j].Value = FlattenGrid[Index];
				Index++;
			}
		}

		float MaxValue = -FLT_MAX;
		float MinValue = FLT_MAX;

		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				if (Grid[i][j].Value > MaxValue)
					MaxValue = Grid[i][j].Value;
				if (Grid[i][j].Value < MinValue)
					MinValue = Grid[i][j].Value;
			}
		}

		MinForColorMap = MinValue;
		MaxForColorMap = MaxValue;
	}

	std::vector<unsigned char> ImageRawData;
	ImageRawData.reserve(CurrentResolution * CurrentResolution * 4);
	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			// Using Grid[j][i] instead of Grid[i][j] to rotate the image by 90 degrees
			if (Grid[j][i].TrianglesInCell.empty() || Grid[j][i].Value == 0.0f)
			{
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));
			}
			else
			{
				// Normalize the value to the range [0, 1] (0 = min, 1 = max)
				float NormalizedValue = (Grid[j][i].Value - MinForColorMap) / (MaxForColorMap - MinForColorMap);

				// It could be more than 1 because I am using user set max value.
				if (NormalizedValue > 1.0f)
					NormalizedValue = 1.0f;

				glm::vec3 Color = GetTurboColorMap(NormalizedValue);

				unsigned char R = static_cast<unsigned char>(Color.x * 255.0f);
				ImageRawData.push_back(R);
				unsigned char G = static_cast<unsigned char>(Color.y * 255.0f);
				ImageRawData.push_back(G);
				unsigned char B = static_cast<unsigned char>(Color.z * 255.0f);
				ImageRawData.push_back(B);
				ImageRawData.push_back(static_cast<unsigned char>(255));
			}
		}
	}

	if (CurrentUpAxis.y > 0.0)
	{
		lodepng::encode("test.png", ImageRawData, CurrentResolution, CurrentResolution);
	}
	if (CurrentUpAxis.z > 0.0)
	{
		// Flip the image vertically
		std::vector<unsigned char> FlippedImageRawData;
		FlippedImageRawData.reserve(CurrentResolution* CurrentResolution * 4);

		for (int i = CurrentResolution - 1; i >= 0; i--)
		{
			for (int j = 0; j < CurrentResolution; j++)
			{
				int index = (i * CurrentResolution + j) * 4;
				FlippedImageRawData.push_back(ImageRawData[index + 0]);
				FlippedImageRawData.push_back(ImageRawData[index + 1]);
				FlippedImageRawData.push_back(ImageRawData[index + 2]);
				FlippedImageRawData.push_back(ImageRawData[index + 3]);
			}
		}

		lodepng::encode("test.png", FlippedImageRawData, CurrentResolution, CurrentResolution);
	}

	Grid.clear();
	MainThreadGridUpdateTasks.clear();
	GatherGridRasterizationThreadCount = 0;
	CurrentLayer = nullptr;
	CurrentResolution = 0;
	CurrentUpAxis = glm::vec3(0.0f);
}

void GatherGridRasterizationThreadWork(void* OutputData)
{
	std::vector<GridUpdateTask>* Result = reinterpret_cast<std::vector<GridUpdateTask>*>(OutputData);

	MainThreadGridUpdateTasks.insert(MainThreadGridUpdateTasks.end(), Result->begin(), Result->end());
	GatherGridRasterizationThreadCount++;
	delete Result;

	if (GatherGridRasterizationThreadCount == 10)
		AfterAllGridRasterizationThreadFinished();
}

void ExportCurrentLayerAsMap()
{
	CurrentLayer = LAYER_MANAGER.GetActiveLayer();
	if (CurrentLayer == nullptr)
		return;

	CurrentUpAxis = ConvertToClosestAxis(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->GetAverageNormal());
	//CurrentResolution = 16;

	Grid = GenerateGridProjection(MESH_MANAGER.ActiveMesh->GetAABB(), CurrentUpAxis, CurrentResolution);
	//for (auto& Cell : Grid)
	//{
		//LINE_RENDERER.RenderAABB(Cell.AABB.Transform(MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix()), glm::vec3(1.0f, 0.0f, 0.0f));
	//}
	//bool bOneThread = false;

	//if (bOneThread)
	//{
	//	glm::vec3 CellSize = Grid[0][0].AABB.GetSize();
	//	const glm::vec3 GridMin = Grid[0][0].AABB.GetMin();
	//	const glm::vec3 GridMax = Grid[CurrentResolution - 1][CurrentResolution - 1].AABB.GetMax();

	//	double TotalTime = 0.0;
	//	double TimeTakenFillCellsWithTriangleInfo = 0.0;

	//	TIME.BeginTimeStamp("Total time");


	//	if (CurrentUpAxis.y > 0.0)
	//	{
	//		for (int l = 0; l < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); l++)
	//		{
	//			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

	//			int XEnd = CurrentResolution;

	//			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
	//			int XBegin = static_cast<int>(Distance / CellSize.x) - 1;
	//			if (XBegin < 0)
	//				XBegin = 0;

	//			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
	//			XEnd -= static_cast<int>(Distance / CellSize.x);
	//			XEnd++;
	//			if (XEnd >= CurrentResolution)
	//				XEnd = CurrentResolution;

	//			for (size_t i = XBegin; i < XEnd; i++)
	//			{
	//				int ZEnd = CurrentResolution;

	//				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
	//				int ZBegin = static_cast<int>(Distance / CellSize.z) - 1;
	//				if (ZBegin < 0)
	//					ZBegin = 0;

	//				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
	//				ZEnd -= static_cast<int>(Distance / CellSize.z);
	//				ZEnd++;
	//				if (ZEnd >= CurrentResolution)
	//					ZEnd = CurrentResolution;

	//				for (size_t j = ZBegin; j < ZEnd; j++)
	//				{
	//					TIME.BeginTimeStamp("Fill cells with triangle info");

	//					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
	//						Grid[i][j].TrianglesInCell.push_back(l);

	//					/*if (Grid[i][j].AABB.AABBIntersect(TriangleAABB))
	//					{
	//						Grid[i][j].TrianglesInCell.push_back(l);
	//					}*/

	//					TimeTakenFillCellsWithTriangleInfo += TIME.EndTimeStamp("Fill cells with triangle info");
	//				}
	//			}
	//		}

	//	}
	//	else if (CurrentUpAxis.z > 0.0)
	//	{
	//		for (int l = 0; l < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); l++)
	//		{
	//			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

	//			int XEnd = CurrentResolution;

	//			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
	//			int XBegin = static_cast<int>(Distance / CellSize.x) - 1;
	//			if (XBegin < 0)
	//				XBegin = 0;

	//			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
	//			XEnd -= static_cast<int>(Distance / CellSize.x);
	//			XEnd++;
	//			if (XEnd >= CurrentResolution)
	//				XEnd = CurrentResolution;

	//			for (size_t i = XBegin; i < XEnd; i++)
	//			{
	//				int YEnd = static_cast<int>(CurrentResolution);

	//				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
	//				int YBegin = static_cast<int>(Distance / CellSize.y) - 1;
	//				if (YBegin < 0)
	//					YBegin = 0;

	//				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
	//				YEnd -= static_cast<int>(Distance / CellSize.y);
	//				YEnd++;
	//				if (YEnd >= CurrentResolution)
	//					YEnd = CurrentResolution;

	//				for (size_t j = YBegin; j < YEnd; j++)
	//				{
	//					TIME.BeginTimeStamp("Fill cells with triangle info");

	//					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
	//						Grid[i][j].TrianglesInCell.push_back(l);

	//					/*if (Grid[i][j].AABB.AABBIntersect(TriangleAABB))
	//					{
	//						Grid[i][j].TrianglesInCell.push_back(l);
	//					}*/

	//					TimeTakenFillCellsWithTriangleInfo += TIME.EndTimeStamp("Fill cells with triangle info");
	//				}
	//			}
	//		}
	//	}

	//	TotalTime = TIME.EndTimeStamp("Total time");
	//	MessageBoxA(NULL, ("Time for IsAABBIntersectTriangle: " + std::to_string(TimeTakenFillCellsWithTriangleInfo)).c_str(), "Mouse Position", MB_OK);
	//	MessageBoxA(NULL, ("Total Time: " + std::to_string(TotalTime)).c_str(), "Mouse Position", MB_OK);

	//	double PercentageFromTotalTime = (TimeTakenFillCellsWithTriangleInfo / TotalTime) * 100.0;
	//	MessageBoxA(NULL, ("PercentageFromTotalTime: " + std::to_string(PercentageFromTotalTime)).c_str(), "Mouse Position", MB_OK);

	//	for (int i = 0; i < Grid.size(); i++)
	//	{
	//		for (int j = 0; j < Grid[i].size(); j++)
	//		{
	//			if (!Grid[i][j].TrianglesInCell.empty())
	//			{
	//				float CurrentCellTotalArea = 0.0f;
	//				for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
	//				{
	//					CurrentCellTotalArea += static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]]);
	//				}

	//				float FinalResult = 0.0f;
	//				for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
	//				{
	//					float CurrentTriangleCoef = static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]] / CurrentCellTotalArea);
	//					//float CurrentTriangleCoef = 1.0f;
	//					//float CurrentTriangleCoef = Grid[i][j].TriangleAABBOverlaps[k];
	//					FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]] * CurrentTriangleCoef;
	//				}

	//				//FinalResult /= static_cast<float>(Grid[i][j].TrianglesInCell.size());
	//				Grid[i][j].Value = FinalResult;
	//			}
	//		}
	//	}

	//	std::vector<unsigned char> ImageRawData;
	//	ImageRawData.reserve(CurrentResolution * CurrentResolution * 4);
	//	for (int i = 0; i < Grid.size(); i++)
	//	{
	//		for (int j = 0; j < Grid[i].size(); j++)
	//		{
	//			if (Grid[i][j].TrianglesInCell.empty())
	//			{
	//				ImageRawData.push_back(static_cast<unsigned char>(0));
	//				ImageRawData.push_back(static_cast<unsigned char>(0));
	//				ImageRawData.push_back(static_cast<unsigned char>(0));
	//				ImageRawData.push_back(static_cast<unsigned char>(0));
	//			}
	//			else
	//			{
	//				// Normalize the value to the range [0, 1] (0 = min, 1 = max)
	//				float NormalizedValue = (Grid[i][j].Value - CurrentLayer->MinVisible) / (CurrentLayer->MaxVisible - CurrentLayer->MinVisible);

	//				// It could be more than 1 because I am using user set max value.
	//				if (NormalizedValue > 1.0f)
	//					NormalizedValue = 1.0f;

	//				glm::vec3 Color = MESH_RENDERER.GetTurboColorMapValue(NormalizedValue);

	//				unsigned char R = static_cast<unsigned char>(Color.x);
	//				ImageRawData.push_back(R);
	//				unsigned char G = static_cast<unsigned char>(Color.y);
	//				ImageRawData.push_back(G);
	//				unsigned char B = static_cast<unsigned char>(Color.z);
	//				ImageRawData.push_back(B);
	//				ImageRawData.push_back(static_cast<unsigned char>(255));
	//			}
	//		}
	//	}

	//	lodepng::encode("test.png", ImageRawData, CurrentResolution, CurrentResolution);
	//}
	//else
	//{
		int NumberOfTrianglesPerThread = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() / 10);

		std::vector<GridRasterizationThreadData*> ThreadData;
		for (int i = 0; i < 10; i++)
		{
			GridRasterizationThreadData* NewThreadData = new GridRasterizationThreadData();
			NewThreadData->Grid = &Grid;
			NewThreadData->UpAxis = CurrentUpAxis;
			NewThreadData->Resolution = CurrentResolution;
			NewThreadData->FirstIndexInTriangleArray = i * NumberOfTrianglesPerThread;

			if (i == 9)
				NewThreadData->LastIndexInTriangleArray = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() - 1;
			else
				NewThreadData->LastIndexInTriangleArray = (i + 1) * NumberOfTrianglesPerThread;

			std::vector<GridUpdateTask>* OutputTasks = new std::vector<GridUpdateTask>();
			ThreadData.push_back(NewThreadData);

			THREAD_POOL.Execute(GridRasterizationThread, NewThreadData, OutputTasks, GatherGridRasterizationThreadWork);
		}
	/*}*/
}

void MainWindowRender()
{
	//static int FEWorldMatrix_hash = int(std::hash<std::string>{}("FEWorldMatrix"));
	//static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	//static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));
	static bool FirstFrame = true;

	const char* rasterizationModes[] = { "Min", "Max", "Mean", "Cumulative"};
	ImGui::Combo("Rasterization Mode", &GridRasterizationMode, rasterizationModes, IM_ARRAYSIZE(rasterizationModes));

	ImGui::SliderInt("Resolution", &CurrentResolution, 2, 4096);
	ImGui::SliderInt("Cumulative_Outliers", &CumulativeOutliers, 0, 99);

	if (ImGui::Button("ExportCurrentLayerAsMap"))
	{
		ExportCurrentLayerAsMap();
	}

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