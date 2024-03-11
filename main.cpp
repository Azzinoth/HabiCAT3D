#include "SubSystems/ConsoleJobs/ConsoleJobManager.h"
using namespace FocalEngine;

glm::vec4 ClearColor = glm::vec4(0.33f, 0.39f, 0.49f, 1.0f);
std::vector<std::vector<LayerRasterizationManager::GridCell>> Grid;
glm::vec2 SelectedCell = glm::vec2(-1.0);
void MouseClickGrid();
void RenderGrid();
double CurrentFinalResult = 0.0;
MeshLayer* CurrentLayerToExport = nullptr;

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
		if (Grid.size() != 0)
		{
			MouseClickGrid();
			//UI.GetDebugGrid()->MouseClick(MouseX, MouseY);
			return;
		}

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


//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Polygon_2.h>
//#include <CGAL/Polygon_2_algorithms.h>


//#include <CGAL/Partition_traits_2.h>
//#include <CGAL/partition_2.h>
//
//#include <CGAL/Bbox_3.h>
//#include <CGAL/Triangle_3.h>
//#include <CGAL/intersections.h>
//
//typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
////typedef K::Point_2 Point_2;
////typedef CGAL::Polygon_2<K> Polygon_2;
//
//typedef CGAL::Partition_traits_2<K> Traits;
//typedef std::list<Polygon_2> Polygon_list;
//
//typedef Kernel::Point_3 Point_3;
//typedef Kernel::Triangle_3 Triangle_3;
//typedef CGAL::Bbox_3 Bbox_3;




//// Function to calculate the area of a triangle given its vertices
//double TriangleArea_(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c) {
//	glm::dvec3 crossProduct = glm::cross(b - a, c - a);
//	return 0.5 * glm::length(crossProduct);
//}
//
//// Function to calculate the area of a quadrilateral given its vertices
//double QuadrilateralArea_(const std::vector<glm::dvec3>& points) {
//	// Ensure there are exactly four vertices
//	if (points.size() != 4) return 0.0;
//
//	// Split the quadrilateral into two triangles and calculate each area
//	double area1 = TriangleArea_(points[0], points[1], points[2]);
//	double area2 = TriangleArea_(points[2], points[3], points[0]);
//
//	// Sum the areas of the two triangles
//	return area1 + area2;
//}
//
//glm::dvec3 CalculateCentroid_(const std::vector<glm::dvec3>& points) {
//	glm::dvec3 centroid(0.0, 0.0, 0.0);
//	for (const auto& point : points) {
//		centroid += point;
//	}
//	centroid /= static_cast<double>(points.size());
//	return centroid;
//}
//
//bool CompareAngles_(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& centroid) {
//	double angleA = atan2(a.z - centroid.z, a.x - centroid.x);
//	double angleB = atan2(b.z - centroid.z, b.x - centroid.x);
//	return angleA < angleB;
//}
//
//void SortPointsByAngle_(std::vector<glm::dvec3>& points) {
//	glm::dvec3 centroid = CalculateCentroid_(points);
//	std::sort(points.begin(), points.end(), [&centroid](const glm::dvec3& a, const glm::dvec3& b) {
//		return CompareAngles_(a, b, centroid);
//		});
//}
//
////void CorrectPolygonIfNecessary(Polygon_2& polygon, Polygon_list& partitionedPolygons) {
////	if (!polygon.is_simple()) {
////		CGAL::partition_polygon_2(polygon.vertices_begin(), polygon.vertices_end(),
////			std::back_inserter(partitionedPolygons), Traits());
////		// At this point, partitionedPolygons contains simple polygons.
////		// You might need to decide how to handle multiple polygons.
////	}
////	else {
////		partitionedPolygons.push_back(polygon);
////	}
////}
//
//double CalculatePolygonArea_(std::vector<glm::dvec2>& glmPoints) {
//	Polygon_2 polygon;
//	for (const auto& p : glmPoints) {
//		polygon.push_back(Point_2(p.x, p.y));
//	}
//
//	// Check if the polygon is simple (does not self-intersect)
//	if (!polygon.is_simple()) {
//		std::cerr << "Polygon is not simple." << std::endl;
//		return 0.0;
//	}
//
//	// Calculate the area
//	double area = CGAL::to_double(polygon.area());
//
//	return area;
//}
//
//double GetArea_(std::vector<glm::dvec3>& points)
//{
//	////if (points.size() == 3)
//	////{
//	////	return TriangleArea_(points[0], points[1], points[2]);
//	////}
//	/////*else if (points.size() == 4)
//	////{
//	////	return QuadrilateralArea_(points);
//	////}*/
//	////else if (points.size() > 3)
//	////{
//	//	SortPointsByAngle_(points);
//
//	//	double Area = 0.0;
//	//	for (size_t p = 0; p < points.size(); p++)
//	//	{
//	//		const glm::dvec3& v1 = points[p];
//	//		const glm::dvec3& v2 = points[(p + 1) % points.size()];
//	//		Area += v1.x * v2.z - v2.x * v1.z;
//	//	}
//	//	Area = std::abs(Area) * 0.5;
//	//	return Area;
//	///*}
//	//else
//	//{
//	//	return 0.0;
//	//}*/
//
//
//	SortPointsByAngle_(points);
//
//	// FIX ME
//	std::vector<glm::dvec2> glmPoints;
//	for (size_t i = 0; i < points.size(); i++)
//	{
//		glmPoints.push_back(glm::dvec2(points[i].x, points[i].z));
//	}
//
//	return abs(CalculatePolygonArea_(glmPoints));
//
//}

//void ClipAgainstPlane(std::vector<glm::vec3>& InVertices, const glm::vec3& PlaneNormal, float PlaneConstant, std::vector<glm::vec3>& OutVertices)
//{
//	if (InVertices.empty())
//		return;
//
//	const glm::vec3& v1 = InVertices.back();
//	float d1 = glm::dot(v1, PlaneNormal) - PlaneConstant;
//
//	for (const auto& v2 : InVertices)
//	{
//		float d2 = glm::dot(v2, PlaneNormal) - PlaneConstant;
//
//		if (d1 * d2 < 0.0f)
//		{
//			float t = d1 / (d1 - d2);
//			OutVertices.push_back(v1 + t * (v2 - v1));
//		}
//
//		if (d2 >= 0.0f)
//			OutVertices.push_back(v2);
//
//		d1 = d2;
//	}
//}
//
//float CalculateTriangleAreaInsideAABB(FEAABB& AABB, std::vector<glm::vec3>& TriangleVertices)
//{
//	// Ensure the triangle is defined by exactly three vertices
//	if (TriangleVertices.size() != 3)
//		return 0.0f;
//
//	// Check if the AABB and triangle intersect
//	if (!GEOMETRY.IsAABBIntersectTriangle(AABB, TriangleVertices))
//		return 0.0f;
//
//	// Clip the triangle vertices against the AABB planes
//	std::vector<glm::vec3> ClippedVertices;
//	ClippedVertices.reserve(7); // At most 7 vertices after clipping
//
//	// Clip against each AABB plane
//	ClipAgainstPlane(TriangleVertices, glm::vec3(1, 0, 0), AABB.GetMin().x, ClippedVertices);
//	TriangleVertices = ClippedVertices;
//	ClippedVertices.clear();
//
//	ClipAgainstPlane(TriangleVertices, glm::vec3(-1, 0, 0), -AABB.GetMax().x, ClippedVertices);
//	TriangleVertices = ClippedVertices;
//	ClippedVertices.clear();
//
//	ClipAgainstPlane(TriangleVertices, glm::vec3(0, 1, 0), AABB.GetMin().y, ClippedVertices);
//	TriangleVertices = ClippedVertices;
//	ClippedVertices.clear();
//
//	ClipAgainstPlane(TriangleVertices, glm::vec3(0, -1, 0), -AABB.GetMax().y, ClippedVertices);
//	TriangleVertices = ClippedVertices;
//	ClippedVertices.clear();
//
//	ClipAgainstPlane(TriangleVertices, glm::vec3(0, 0, 1), AABB.GetMin().z, ClippedVertices);
//	TriangleVertices = ClippedVertices;
//	ClippedVertices.clear();
//
//	ClipAgainstPlane(TriangleVertices, glm::vec3(0, 0, -1), -AABB.GetMax().z, ClippedVertices);
//	TriangleVertices = ClippedVertices;
//
//	// Calculate the area of the clipped polygon
//	float Area = 0.0f;
//	for (size_t i = 0; i < TriangleVertices.size(); ++i)
//	{
//		const glm::vec3& v1 = TriangleVertices[i];
//		const glm::vec3& v2 = TriangleVertices[(i + 1) % TriangleVertices.size()];
//		Area += v1.x * v2.y - v2.x * v1.y;
//	}
//	Area = std::abs(Area) * 0.5f;
//
//	return Area;
//}




//glm::vec3 UpAxis;
//int Resolution;

void CreateGrid(MeshLayer* LayerToExport)
{
	CurrentLayerToExport = LayerToExport;
	if (CurrentLayerToExport == nullptr)
		return;

	LAYER_RASTERIZATION_MANAGER.CurrentUpAxis = LAYER_RASTERIZATION_MANAGER.ConvertToClosestAxis(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->GetAverageNormal());

	Grid = LAYER_RASTERIZATION_MANAGER.GenerateGridProjection(MESH_MANAGER.ActiveMesh->GetAABB(), LAYER_RASTERIZATION_MANAGER.CurrentUpAxis, LAYER_RASTERIZATION_MANAGER.CurrentResolution);

	//GridRasterizationThreadData* Input = reinterpret_cast<GridRasterizationThreadData*>(InputData);
	//std::vector<GridUpdateTask>* Output = reinterpret_cast<std::vector<GridUpdateTask>*>(OutputData);

	//std::vector<std::vector<LayerRasterizationManager::GridCell>>& Grid = *Input->Grid;
	const glm::vec3 UpAxis = LAYER_RASTERIZATION_MANAGER.CurrentUpAxis;
	const int Resolution = LAYER_RASTERIZATION_MANAGER.CurrentResolution;
	//const int FirstIndexInTriangleArray = Input->FirstIndexInTriangleArray;
	//const int LastIndexInTriangleArray = Input->LastIndexInTriangleArray;

	glm::vec3 CellSize = Grid[0][0].AABB.GetSize();
	const glm::vec3 GridMin = Grid[0][0].AABB.GetMin();
	const glm::vec3 GridMax = Grid[Resolution - 1][Resolution - 1].AABB.GetMax();

	int TriangleCount = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size();

	//double TotalTime = 0.0;
	//double TimeTakenFillCellsWithTriangleInfo = 0.0;

	//TIME.BeginTimeStamp("Total time");

	if (UpAxis.x > 0.0)
	{
		for (int l = 0; l < TriangleCount; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			int FirstAxisEndIndex = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
			int FirstAxisStartIndex = static_cast<int>(Distance / CellSize.y) - 1;
			if (FirstAxisStartIndex < 0)
				FirstAxisStartIndex = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
			FirstAxisEndIndex -= static_cast<int>(Distance / CellSize.y);
			FirstAxisEndIndex++;
			if (FirstAxisEndIndex >= Resolution)
				FirstAxisEndIndex = Resolution;

			for (size_t i = FirstAxisStartIndex; i < FirstAxisEndIndex; i++)
			{
				int SecondAxisEndIndex = Resolution;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
				int SecondAxisStartIndex = static_cast<int>(Distance / CellSize.z) - 1;
				if (SecondAxisStartIndex < 0)
					SecondAxisStartIndex = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
				SecondAxisEndIndex -= static_cast<int>(Distance / CellSize.z);
				SecondAxisEndIndex++;
				if (SecondAxisEndIndex >= Resolution)
					SecondAxisEndIndex = Resolution;

				for (size_t j = SecondAxisStartIndex; j < SecondAxisEndIndex; j++)
				{
					auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l];
					if (CurrentTriangle.size() != 3)
						continue;

					std::vector<glm::dvec3> CurrentTriangleDouble;
					CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z));
					CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z));
					CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z));

					// FIX ME, USE THIS POINTS AND ALSO UNIFY THE CODE
					//if (GEOMETRY.PointCountOfIntersection(Grid[i][j].AABB, CurrentTriangleDouble).size() > 0)
					//	Grid[i][j].TrianglesInCell.push_back(l);

					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Grid[i][j].TrianglesInCell.push_back(l);
					//if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
					//	Grid[i][j].TrianglesInCell.push_back(l);
				}
			}
		}
	}
	if (UpAxis.y > 0.0)
	{
		for (int l = 0; l < TriangleCount; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			int FirstAxisEndIndex = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			int FirstAxisStartIndex = static_cast<int>(Distance / CellSize.x) - 1;
			if (FirstAxisStartIndex < 0)
				FirstAxisStartIndex = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			FirstAxisEndIndex -= static_cast<int>(Distance / CellSize.x);
			FirstAxisEndIndex++;
			if (FirstAxisEndIndex >= Resolution)
				FirstAxisEndIndex = Resolution;

			for (size_t i = FirstAxisStartIndex; i < FirstAxisEndIndex; i++)
			{
				int SecondAxisEndIndex = Resolution;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
				int SecondAxisStartIndex = static_cast<int>(Distance / CellSize.z) - 1;
				if (SecondAxisStartIndex < 0)
					SecondAxisStartIndex = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
				SecondAxisEndIndex -= static_cast<int>(Distance / CellSize.z);
				SecondAxisEndIndex++;
				if (SecondAxisEndIndex >= Resolution)
					SecondAxisEndIndex = Resolution;

				for (size_t j = SecondAxisStartIndex; j < SecondAxisEndIndex; j++)
				{
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Grid[i][j].TrianglesInCell.push_back(l);
				}
			}
		}
	}
	else if (UpAxis.z > 0.0)
	{
		for (int l = 0; l < TriangleCount; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			int FirstAxisEndIndex = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			int FirstAxisStartIndex = static_cast<int>(Distance / CellSize.x) - 1;
			if (FirstAxisStartIndex < 0)
				FirstAxisStartIndex = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			FirstAxisEndIndex -= static_cast<int>(Distance / CellSize.x);
			FirstAxisEndIndex++;
			if (FirstAxisEndIndex >= Resolution)
				FirstAxisEndIndex = Resolution;

			for (size_t i = FirstAxisStartIndex; i < FirstAxisEndIndex; i++)
			{
				int SecondAxisEndIndex = static_cast<int>(Resolution);

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
				int SecondAxisStartIndex = static_cast<int>(Distance / CellSize.y) - 1;
				if (SecondAxisStartIndex < 0)
					SecondAxisStartIndex = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
				SecondAxisEndIndex -= static_cast<int>(Distance / CellSize.y);
				SecondAxisEndIndex++;
				if (SecondAxisEndIndex >= Resolution)
					SecondAxisEndIndex = Resolution;

				for (size_t j = SecondAxisStartIndex; j < SecondAxisEndIndex; j++)
				{
					auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l];
					if (CurrentTriangle.size() != 3)
						continue;

					std::vector<glm::dvec3> CurrentTriangleDouble;
					CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z));
					CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z));
					CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z));

					// FIX ME, USE THIS POINTS AND ALSO UNIFY THE CODE
					auto result = GEOMETRY.GetIntersectionPoints(Grid[i][j].AABB, CurrentTriangleDouble);
					if (result.size() > 0)
						Grid[i][j].TrianglesInCell.push_back(l);

					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
					{
						if (result.size() <= 0)
						{
							std::string Output = "glm::vec3 FirstPoint = glm::vec3(" + std::to_string(CurrentTriangle[0].x) + ", " + std::to_string(CurrentTriangle[0].y) + ", " + std::to_string(CurrentTriangle[0].z) + ") * Multiplicator;\n";
							Output += "glm::vec3 SecondPoint = glm::vec3(" + std::to_string(CurrentTriangle[1].x) + ", " + std::to_string(CurrentTriangle[1].y) + ", " + std::to_string(CurrentTriangle[1].z) + ") * Multiplicator;\n";
							Output += "glm::vec3 ThirdPoint = glm::vec3(" + std::to_string(CurrentTriangle[2].x) + ", " + std::to_string(CurrentTriangle[2].y) + ", " + std::to_string(CurrentTriangle[2].z) + ") * Multiplicator;\n";

							Output += "glm::vec3 Min = glm::vec3(" + std::to_string(Grid[i][j].AABB.GetMin().x) + ", " + std::to_string(Grid[i][j].AABB.GetMin().y) + ", " + std::to_string(Grid[i][j].AABB.GetMin().z) + ") * Multiplicator;\n";
							Output += "glm::vec3 Max = glm::vec3(" + std::to_string(Grid[i][j].AABB.GetMax().x) + ", " + std::to_string(Grid[i][j].AABB.GetMax().y) + ", " + std::to_string(Grid[i][j].AABB.GetMax().z) + ") * Multiplicator;\n";

							int y = 0;
							y++;
						}

						Grid[i][j].TrianglesInCell.push_back(l);
					}
					else
					{
						if (result.size() > 0)
						{


							GEOMETRY.GetIntersectionPoints(Grid[i][j].AABB, CurrentTriangleDouble);
							GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);
							int y = 0;
							y++;
						}


						
					}
						


					/*if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Grid[i][j].TrianglesInCell.push_back(l);*/
				}
			}
		}
	}
}

void SelectCell(int X, int Y)
{
	if (Grid.size() == 0)
		return;

	if (X < 0 || X >= Grid.size() || Y < 0 || Y >= Grid[0].size())
		return;

	LINE_RENDERER.ClearAll();

	CurrentFinalResult = 0.0;
	SelectedCell = glm::vec2(X, Y);

	double AreaInCell = 0.0;

	for (size_t i = 0; i < Grid[X][Y].TrianglesInCell.size(); i++)
	{
		auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Grid[X][Y].TrianglesInCell[i]];
		if (CurrentTriangle.size() != 3)
			continue;

		std::vector<glm::dvec3> CurrentTriangleDouble;
		CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z));
		CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z));
		CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z));

		std::vector<glm::dvec3> DebugResult = GEOMETRY.GetIntersectionPoints(Grid[X][Y].AABB, CurrentTriangleDouble);


		std::string Output = "glm::vec3 FirstPoint = glm::vec3(" + std::to_string(CurrentTriangle[0].x) + ", " + std::to_string(CurrentTriangle[0].y) + ", " + std::to_string(CurrentTriangle[0].z) + ") * Multiplicator;\n";
		Output += "glm::vec3 SecondPoint = glm::vec3(" + std::to_string(CurrentTriangle[1].x) + ", " + std::to_string(CurrentTriangle[1].y) + ", " + std::to_string(CurrentTriangle[1].z) + ") * Multiplicator;\n";
		Output += "glm::vec3 ThirdPoint = glm::vec3(" + std::to_string(CurrentTriangle[2].x) + ", " + std::to_string(CurrentTriangle[2].y) + ", " + std::to_string(CurrentTriangle[2].z) + ") * Multiplicator;\n";

		Output += "glm::vec3 Min = glm::vec3(" + std::to_string(Grid[X][Y].AABB.GetMin().x) + ", " + std::to_string(Grid[X][Y].AABB.GetMin().y) + ", " + std::to_string(Grid[X][Y].AABB.GetMin().z) + ") * Multiplicator;\n";
		Output += "glm::vec3 Max = glm::vec3(" + std::to_string(Grid[X][Y].AABB.GetMax().x) + ", " + std::to_string(Grid[X][Y].AABB.GetMax().y) + ", " + std::to_string(Grid[X][Y].AABB.GetMax().z) + ") * Multiplicator;\n";

		int y = 0;
		y++;

		for (size_t l = 0; l < CurrentTriangleDouble.size(); l++)
		{
			if (Grid[X][Y].AABB.ContainsPoint(CurrentTriangleDouble[l]))
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

		for (size_t l = 0; l < DebugResult.size(); l++)
		{
			glm::dvec3 TransformedPoint = MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(DebugResult[l], 1.0f);
			LINE_RENDERER.AddLineToBuffer(FECustomLine(TransformedPoint, TransformedPoint + glm::dvec3(0.0, 1.0, 0.0), glm::vec3(1.0f, 0.0f, 0.0f)));
		}

		double CurrentTrianlgeArea = 0.0;
		CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetArea(DebugResult);







		//// Define a triangle
		//Point_3 p1(CurrentTriangleDouble[0].x, CurrentTriangleDouble[0].y, CurrentTriangleDouble[0].z);
		//Point_3 p2(CurrentTriangleDouble[1].x, CurrentTriangleDouble[1].y, CurrentTriangleDouble[1].z);
		//Point_3 p3(CurrentTriangleDouble[2].x, CurrentTriangleDouble[2].y, CurrentTriangleDouble[2].z);
		//Triangle_3 triangle(p1, p2, p3);

		//// Define an AABB
		//Bbox_3 aabb(Grid[X][Y].AABB.GetMin().x, Grid[X][Y].AABB.GetMin().y, Grid[X][Y].AABB.GetMin().z, Grid[X][Y].AABB.GetMax().x, Grid[X][Y].AABB.GetMax().y, Grid[X][Y].AABB.GetMax().z);


		//// Compute the intersection between the triangle and the AABB
		//auto result = CGAL::intersection(triangle, aabb);

		//double CGALArea = 0.0;
		//// Check the type of the intersection result and calculate the area
		//if (result) {
		//	if (const Triangle_3* intersect_triangle = boost::get<Triangle_3>(&*result)) {
		//		//double area = CGAL::to_double(CGAL::area(*intersect_triangle));
		//		//std::cout << "Intersection is a triangle with area: " << area << std::endl;
		//		int y = 0;
		//		y++;
		//	}
		//	else if (const Kernel::Segment_3* intersect_segment = boost::get<Kernel::Segment_3>(&*result)) {
		//		std::cout << "Intersection is a segment. Area is zero." << std::endl;
		//	}
		//	else if (const Point_3* intersect_point = boost::get<Point_3>(&*result)) {
		//		std::cout << "Intersection is a point. Area is zero." << std::endl;
		//	}
		//	else if (const std::vector<Point_3>* intersect_points = boost::get<std::vector<Point_3>>(&*result)) {
		//		// Create a polygon from the intersection points
		//		Polygon_2 polygon;
		//		for (const auto& point : *intersect_points) {
		//			polygon.push_back(Point_2(point.x(), point.z()));
		//		}
		//		double CGALArea = CGAL::to_double(polygon.area());
		//		std::cout << "Intersection is a polygon with area: " << area << std::endl;
		//	}
		//}
		//else {
		//	std::cout << "No intersection. Area is zero." << std::endl;
		//}












		/*if (CurrentTrianlgeArea == 0.0)
		{
			std::string Output = "glm::vec3 FirstPoint = glm::vec3(" + std::to_string(CurrentTriangle[0].x) + ", " + std::to_string(CurrentTriangle[0].y) + ", " + std::to_string(CurrentTriangle[0].z) + ") * Multiplicator;\n";
			Output += "glm::vec3 SecondPoint = glm::vec3(" + std::to_string(CurrentTriangle[1].x) + ", " + std::to_string(CurrentTriangle[1].y) + ", " + std::to_string(CurrentTriangle[1].z) + ") * Multiplicator;\n";
			Output += "glm::vec3 ThirdPoint = glm::vec3(" + std::to_string(CurrentTriangle[2].x) + ", " + std::to_string(CurrentTriangle[2].y) + ", " + std::to_string(CurrentTriangle[2].z) + ") * Multiplicator;\n";

			Output += "glm::vec3 Min = glm::vec3(" + std::to_string(Grid[X][Y].AABB.GetMin().x) + ", " + std::to_string(Grid[X][Y].AABB.GetMin().y) + ", " + std::to_string(Grid[X][Y].AABB.GetMin().z) + ") * Multiplicator;\n";
			Output += "glm::vec3 Max = glm::vec3(" + std::to_string(Grid[X][Y].AABB.GetMax().x) + ", " + std::to_string(Grid[X][Y].AABB.GetMax().y) + ", " + std::to_string(Grid[X][Y].AABB.GetMax().z) + ") * Multiplicator;\n";

			int y = 0;
			y++;

			if (DebugResult.size() > 1)
			{
				int y = 0;
				y++;
			}
		}*/

		if (CurrentTrianlgeArea != 0.0 && !isnan(CurrentTrianlgeArea))
		{
			//double CurrentTriangleCoef = CurrentTrianlgeArea / COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[X][Y].TrianglesInCell[i]];
			double CurrentTriangleValue = CurrentLayerToExport->TrianglesToData[Grid[X][Y].TrianglesInCell[i]];
			CurrentFinalResult += CurrentTriangleValue * /*CGALArea*/CurrentTrianlgeArea/*CurrentTriangleCoef*/;
			AreaInCell += CurrentTrianlgeArea;
		}
	}
	
	AreaInCell;

	RenderGrid();
}

void MouseClickGrid()
{
	SelectedCell = glm::vec2(-1.0);

	float DistanceToCell = 999999.0f;
	float LastDistanceToCell = 999999.0f;
	for (size_t i = 0; i < Grid.size(); i++)
	{
		for (size_t j = 0; j < Grid[i].size(); j++)
		{
			FEAABB FinalAABB = Grid[i][j].AABB.Transform(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->GetTransformMatrix());
			if (FinalAABB.RayIntersect(ENGINE.GetCamera()->GetPosition(), ENGINE.ConstructMouseRay(), DistanceToCell))
			{
				if (LastDistanceToCell > DistanceToCell)
				{
					LastDistanceToCell = DistanceToCell;
					SelectCell(i, j);
					return;
				}
			}
		}
	}
}

void RenderGrid()
{
	

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			LayerRasterizationManager::GridCell& Cell = Grid[i][j];

			glm::vec3 Color = glm::vec3(0.0f, 1.0f, 0.0f);

			if (SelectedCell.x == i && SelectedCell.y == j)
			{
				Color = glm::vec3(1.0f, 1.0f, 0.0f);
				LINE_RENDERER.RenderAABB(Cell.AABB.Transform(MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix()), Color);


				for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
				{
					const auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Grid[i][j].TrianglesInCell[k]];

					std::vector<glm::vec3> TranformedTrianglePoints = CurrentTriangle;
					for (size_t l = 0; l < TranformedTrianglePoints.size(); l++)
					{
						TranformedTrianglePoints[l] = MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[l], 1.0f);
					}

					LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 0.0f, 1.0f)));
					LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 0.0f, 1.0f)));
					LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 0.0f, 1.0f)));
				}


			}
			
			//if (SelectedCell == glm::vec2(-1.0f))
			//	LINE_RENDERER.RenderAABB(Cell.AABB.Transform(MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix()), Color);
		}
	}

	LINE_RENDERER.SyncWithGPU();
}

void MainWindowRender()
{
	//// Define a triangle
	//Point_3 p1(0, 0, 0);
	//Point_3 p2(1, 0, 0);
	//Point_3 p3(0, 1, 0);
	//Triangle_3 triangle(p1, p2, p3);

	//// Define an AABB
	//Bbox_3 aabb(0.25, 0.25, -0.5, 0.75, 0.75, 0.5);

	//// Compute the intersection between the triangle and the AABB
	//auto result = CGAL::intersection(triangle, aabb);

	//// Check the type of the intersection result
	//if (result) {
	//	if (const Triangle_3* intersect_triangle = boost::get<Triangle_3>(&*result)) {
	//		std::cout << "Intersection is a triangle." << std::endl;
	//		// Access the intersecting triangle using intersect_triangle
	//	}
	//	else if (const Kernel::Segment_3* intersect_segment = boost::get<Kernel::Segment_3>(&*result)) {
	//		std::cout << "Intersection is a segment." << std::endl;
	//		// Access the intersecting segment using intersect_segment
	//	}
	//	else if (const Point_3* intersect_point = boost::get<Point_3>(&*result)) {
	//		std::cout << "Intersection is a point." << std::endl;
	//		// Access the intersecting point using intersect_point
	//	}
	//}
	//else {
	//	std::cout << "No intersection." << std::endl;
	//}



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

	if (ImGui::Button("ExportCurrentLayerAsMap"))
	{
		LAYER_RASTERIZATION_MANAGER.ExportCurrentLayerAsMap(LAYER_MANAGER.GetActiveLayer());
	}

	if (ImGui::Button("CreateDebugGrid"))
	{
		CreateGrid(LAYER_MANAGER.GetActiveLayer());
	}

	if (ImGui::Button("RenderGrid"))
	{
		RenderGrid();
	}

	std::string SelectedCellText = "Selected cell - X : " + std::to_string(int(SelectedCell.x)) + " Y : " + std::to_string(int(SelectedCell.y));
	ImGui::Text(SelectedCellText.c_str());

	static int TempCellX = 0;
	static int TempCellY = 0;
	ImGui::InputInt("SelectedCellX", &TempCellX);
	
	TempInt = SelectedCell.y;
	ImGui::InputInt("SelectedCellY", &TempCellY);
	
	if (ImGui::Button("Select cell"))
	{
		SelectCell(TempCellX, TempCellY);
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