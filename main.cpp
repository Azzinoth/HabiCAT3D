#include "SubSystems/ConsoleJobs/ConsoleJobManager.h"
using namespace FocalEngine;

#include <windows.h>
#include <psapi.h>

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

double mouseX;
double mouseY;

glm::dvec3 mouseRay(double mouseX, double mouseY)
{
	int W, H;
	UI.MainWindow->GetSize(&W, &H);

	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / W - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / H;

	const glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(ENGINE.GetCamera()->GetProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(ENGINE.GetCamera()->GetViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

void LoadMesh(std::string FileName);

static void dropCallback(int count, const char** paths);
void dropCallback(int count, const char** paths)
{
	for (size_t i = 0; i < size_t(count); i++)
	{
		LoadMesh(paths[i]);
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
			ModelViewCamera->SetDistanceToModel(ModelViewCamera->GetDistanceToModel() + Yoffset * MESH_MANAGER.ActiveMesh->GetAABB().GetSize() * 0.05f);
	}
}

void AfterMeshLoads()
{

	//FEMaterial* NewMaterial = RESOURCE_MANAGER.CreateMaterial();
	//NewMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	//NewMaterial->SetBaseColor(glm::vec3(0.0f, 1.0f, 0.0f));


	FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(MESH_MANAGER.ActiveMesh, MESH_MANAGER.CustomMaterial/*NewMaterial*/);
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
	// FIX ME
	//LINE_RENDERER.clearAll();

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() == 1)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]];
		for (size_t i = 0; i < TranformedTrianglePoints.size(); i++)
		{
			TranformedTrianglePoints[i] = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[i], 1.0f);
		}

		// FIX ME
		/*LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));*/

		if (!COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals.empty())
		{
			glm::vec3 Point = TranformedTrianglePoints[0];
			glm::vec3 Normal = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]][0];
			// FIX ME
			//LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[1];
			Normal = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]][1];
			// FIX ME
			//LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[2];
			Normal = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]][2];
			// FIX ME
			//LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
		}

		// FIX ME
		//LINE_RENDERER.SyncWithGPU();
	}
	else if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() > 1)
	{
		// FIX ME
		/*for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size(); i++)
		{
			std::vector<glm::vec3> TranformedTrianglePoints = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[i]];
			for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
			{
				TranformedTrianglePoints[j] = Mesh->Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
			}

			LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
			LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
			LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		}

		LINE_RENDERER.SyncWithGPU();*/
	}
}

void OutputSeletedAreaInfoToFile()
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
				// FIX ME
				//MESH_MANAGER.ActiveMesh->SelectTriangle(mouseRay(mouseX, mouseY), CurrentCamera);
			}
			else if (UI.GetLayerSelectionMode() == 2)
			{
				// FIX ME
				/*if (MESH_MANAGER.ActiveMesh->SelectTrianglesInRadius(mouseRay(mouseX, mouseY), CurrentCamera, UI.GetRadiusOfAreaToMeasure()) && UI.GetOutputSelectionToFile())
				{
					OutputSeletedAreaInfoToFile();
				}*/
			}

			UpdateMeshSelectedTrianglesRendering(MESH_MANAGER.ActiveMesh);
		}

		if (MESH_MANAGER.ActiveMesh != nullptr && UI.GetDebugSDF() != nullptr)
		{
			if (UI.GetDebugSDF()->RenderingMode == 0)
			{
				/*LINE_RENDERER.clearAll();

				if (CurrentMesh->TriangleSelected != -1)
				{
					LINE_RENDERER.AddLineToBuffer(FELine(CurrentMesh->Triangles[CurrentMesh->TriangleSelected][0], CurrentMesh->Triangles[CurrentMesh->TriangleSelected][1], glm::vec3(1.0f, 1.0f, 0.0f)));
					LINE_RENDERER.AddLineToBuffer(FELine(CurrentMesh->Triangles[CurrentMesh->TriangleSelected][0], CurrentMesh->Triangles[CurrentMesh->TriangleSelected][2], glm::vec3(1.0f, 1.0f, 0.0f)));
					LINE_RENDERER.AddLineToBuffer(FELine(CurrentMesh->Triangles[CurrentMesh->TriangleSelected][1], CurrentMesh->Triangles[CurrentMesh->TriangleSelected][2], glm::vec3(1.0f, 1.0f, 0.0f)));

					if (!CurrentMesh->TrianglesNormals.empty())
					{
						glm::vec3 Point = CurrentMesh->Triangles[CurrentMesh->TriangleSelected][0];
						glm::vec3 Normal = CurrentMesh->TrianglesNormals[CurrentMesh->TriangleSelected][0];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

						Point = CurrentMesh->Triangles[CurrentMesh->TriangleSelected][1];
						Normal = CurrentMesh->TrianglesNormals[CurrentMesh->TriangleSelected][1];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

						Point = CurrentMesh->Triangles[CurrentMesh->TriangleSelected][2];
						Normal = CurrentMesh->TrianglesNormals[CurrentMesh->TriangleSelected][2];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
					}

					if (!CurrentMesh->originalTrianglesToSegments.empty() && !CurrentMesh->segmentsNormals.empty())
					{
						glm::vec3 Centroid = (CurrentMesh->Triangles[CurrentMesh->TriangleSelected][0] +
							CurrentMesh->Triangles[CurrentMesh->TriangleSelected][1] +
							CurrentMesh->Triangles[CurrentMesh->TriangleSelected][2]) / 3.0f;

						glm::vec3 Normal = CurrentMesh->segmentsNormals[CurrentMesh->originalTrianglesToSegments[CurrentMesh->TriangleSelected]];
						LINE_RENDERER.AddLineToBuffer(FELine(Centroid, Centroid + Normal, glm::vec3(1.0f, 0.0f, 0.0f)));
					}

					LINE_RENDERER.SyncWithGPU();
				}*/
			}
			else
			{
				UI.GetDebugSDF()->MouseClick(mouseX, mouseY);
				UI.UpdateRenderingMode(UI.GetDebugSDF(), UI.GetDebugSDF()->RenderingMode);
				//UI.GetDebugSDF()->UpdateRenderedLines();
			}
		}
	}
}

void windowResizeCallback(int width, int height)
{
	int W, H;
	UI.MainWindow->GetSize(&W, &H);

	UI.ApplyStandardWindowsSizeAndPosition();
	SCREENSHOT_MANAGER.RenderTargetWasResized();
}

float RAMUsed()
{
	//PDH_STATUS status;
	//PDH_HQUERY query;
	//PDH_HCOUNTER counter;
	//PDH_FMT_COUNTERVALUE value;

	//// Open a query object
	//status = PdhOpenQuery(NULL, NULL, &query);
	//if (status != ERROR_SUCCESS) {
	//	std::cerr << "Error opening query: " << status << std::endl;
	//	return 1;
	//}

	//// Get the current process ID
	//DWORD process_id = GetCurrentProcessId();

	//// Add the "Private Bytes" performance counter for the current process
	//char counter_path[MAX_PATH];
	//sprintf_s(counter_path, sizeof(counter_path), "\\Process(*)\\Private Bytes");
	//status = PdhAddEnglishCounter(query, counter_path, process_id, &counter);
	//if (status != ERROR_SUCCESS) {
	//	std::cerr << "Error adding counter: " << status << std::endl;
	//	return 1;
	//}

	//// Collect the performance data
	//status = PdhCollectQueryData(query);
	//if (status != ERROR_SUCCESS) {
	//	std::cerr << "Error collecting data: " << status << std::endl;
	//	return 1;
	//}

	//// Get the counter value
	//status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);
	//if (status != ERROR_SUCCESS) {
	//	std::cerr << "Error getting counter value: " << status << std::endl;
	//	return 1;
	//}


	//// Print the memory usage
	//std::cout << "Private Bytes: " << value.doubleValue << " bytes" << std::endl;

	//MEMORYSTATUSEX memInfo;
	//memInfo.dwLength = sizeof(MEMORYSTATUSEX);

	//if (GlobalMemoryStatusEx(&memInfo)) {
	//	DWORDLONG totalPhysicalMemory = memInfo.ullTotalPhys;
	//	DWORDLONG freePhysicalMemory = memInfo.ullAvailPhys;
	//	DWORDLONG usedPhysicalMemory = totalPhysicalMemory - freePhysicalMemory;

	//	double totalPhysicalMemoryInMB = totalPhysicalMemory / 1024.0 / 1024.0;
	//	double usedPhysicalMemoryInMB = usedPhysicalMemory / 1024.0 / 1024.0;
	//	double freePhysicalMemoryInMB = freePhysicalMemory / 1024.0 / 1024.0;

	//	double freeRAMPercent = freePhysicalMemoryInMB / totalPhysicalMemoryInMB * 100.0;

	//	std::cout << "Total Physical Memory: " << totalPhysicalMemory << " bytes" << std::endl;
	//	std::cout << "Free Physical Memory: " << freePhysicalMemory << " bytes" << std::endl;
	//	std::cout << "Used Physical Memory: " << usedPhysicalMemory << " bytes" << std::endl;
	//}
	//else {
	//	std::cerr << "Error getting memory information" << std::endl;
	//	return 1;
	//}

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T MemCommited = pmc.PrivateUsage;
	SIZE_T MemWorkingSet = pmc.WorkingSetSize;

	float MemCommitedInKB = float(MemCommited) / 1024.0f;
	float MemWorkingSetInKB = float(MemWorkingSet) / 1024.0f;

	float MemCommitedInMB = MemCommitedInKB/ 1024.0f;
	float MemWorkingSetInMB = MemWorkingSetInKB / 1024.0f;

	return MemCommitedInMB;
}

// ************ PART OF DEBUG CODE ************

std::vector<glm::vec3> TrianglePoints;
FETransformComponent TriangleTransform;
FEAABB AABBbox;
bool bAABBTriangleCollision = false;

void ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform)
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

	glm::vec3 OldScale = Transform->GetScale();
	Transform->ChangeXScaleBy(scale[0] - OldScale[0]);
	Transform->ChangeYScaleBy(scale[1] - OldScale[1]);
	Transform->ChangeZScaleBy(scale[2] - OldScale[2]);
}

bool RayTriangleIntersection(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& triangleVertices, float& distance, glm::vec3* HitPoint)
{
	if (triangleVertices.size() != 3)
		return false;

	const float a = RayDirection[0];
	const float b = triangleVertices[0][0] - triangleVertices[1][0];
	const float c = triangleVertices[0][0] - triangleVertices[2][0];

	const float d = RayDirection[1];
	const float e = triangleVertices[0][1] - triangleVertices[1][1];
	const float f = triangleVertices[0][1] - triangleVertices[2][1];

	const float g = RayDirection[2];
	const float h = triangleVertices[0][2] - triangleVertices[1][2];
	const float j = triangleVertices[0][2] - triangleVertices[2][2];

	const float k = triangleVertices[0][0] - RayOrigin[0];
	const float l = triangleVertices[0][1] - RayOrigin[1];
	const float m = triangleVertices[0][2] - RayOrigin[2];

	const glm::mat3 temp0 = glm::mat3(a, b, c,
		d, e, f,
		g, h, j);

	const float determinant0 = glm::determinant(temp0);

	const glm::mat3 temp1 = glm::mat3(k, b, c,
		l, e, f,
		m, h, j);

	const float determinant1 = glm::determinant(temp1);

	const float t = determinant1 / determinant0;


	const glm::mat3 temp2 = glm::mat3(a, k, c,
		d, l, f,
		g, m, j);

	const float determinant2 = glm::determinant(temp2);
	const float u = determinant2 / determinant0;

	const float determinant3 = glm::determinant(glm::mat3(a, b, k,
		d, e, l,
		g, h, m));

	const float v = determinant3 / determinant0;

	if (t >= 0.00001 &&
		u >= 0.00001 && v >= 0.00001 &&
		u <= 1 && v <= 1 &&
		u + v >= 0.00001 &&
		u + v <= 1 && t > 0.00001)
	{
		if (HitPoint != nullptr)
			*HitPoint = triangleVertices[0] + u * (triangleVertices[1] - triangleVertices[0]) + v * (triangleVertices[2] - triangleVertices[0]);

		distance = t;
		return true;
	}

	return false;
}

bool AABBSideTriangleIntersection(FEAABB& box, std::vector<glm::vec3>& triangleVertices)
{
	// Define the 8 corners of the AABB
	std::vector<glm::vec3> corners;
	corners.push_back(box.GetMin());
	corners.push_back(glm::vec3(box.GetMin().x, box.GetMin().y, box.GetMax().z));
	corners.push_back(glm::vec3(box.GetMin().x, box.GetMax().y, box.GetMin().z));
	corners.push_back(glm::vec3(box.GetMin().x, box.GetMax().y, box.GetMax().z));
	corners.push_back(glm::vec3(box.GetMax().x, box.GetMin().y, box.GetMin().z));
	corners.push_back(glm::vec3(box.GetMax().x, box.GetMin().y, box.GetMax().z));
	corners.push_back(glm::vec3(box.GetMax().x, box.GetMax().y, box.GetMin().z));
	corners.push_back(box.GetMax());

	std::vector<std::pair<glm::vec3, glm::vec3>> edgesRays = {
		{corners[0], corners[1] - corners[0]},
		{corners[1], corners[3] - corners[1]},
		{corners[3], corners[2] - corners[3]},
		{corners[2], corners[0] - corners[2]},
		{corners[4], corners[5] - corners[4]},
		{corners[5], corners[7] - corners[5]},
		{corners[7], corners[6] - corners[7]},
		{corners[6], corners[4] - corners[6]},
		{corners[0], corners[4] - corners[0]},
		{corners[1], corners[5] - corners[1]},
		{corners[3], corners[7] - corners[3]},
		{corners[2], corners[6] - corners[2]}
	};

	// Check each ray for intersection with the triangle
	for (const auto& ray : edgesRays)
	{
		glm::vec3 origin = ray.first;
		glm::vec3 direction = ray.second;

		float distance;
		glm::vec3 hitPoint;

		if (RayTriangleIntersection(origin, direction, triangleVertices, distance, &hitPoint))
		{
			// If the intersection point is within the length of the AABB side,
			// then the AABB and triangle intersect
			if (glm::length(hitPoint - origin) <= glm::length(direction))
			{
				// FIX ME
				//LINE_RENDERER.AddLineToBuffer(FELine(hitPoint, hitPoint + direction * glm::vec3(0.1f), glm::vec3(1.0f, 0.0f, 0.0f)));
				return true;
			}
		}
	}

	return false;
}

void AddTriangleLines()
{
	std::vector<glm::vec3> TransformedTrianglePoints = TrianglePoints;
	TransformedTrianglePoints[0] = TriangleTransform.GetTransformMatrix() * glm::vec4(TransformedTrianglePoints[0], 1.0f);
	TransformedTrianglePoints[1] = TriangleTransform.GetTransformMatrix() * glm::vec4(TransformedTrianglePoints[1], 1.0f);
	TransformedTrianglePoints[2] = TriangleTransform.GetTransformMatrix() * glm::vec4(TransformedTrianglePoints[2], 1.0f);

	// FIX ME
	/*LINE_RENDERER.AddLineToBuffer(FELine(TransformedTrianglePoints[0], TransformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
	LINE_RENDERER.AddLineToBuffer(FELine(TransformedTrianglePoints[0], TransformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
	LINE_RENDERER.AddLineToBuffer(FELine(TransformedTrianglePoints[1], TransformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));*/

	//bAABBTriangleCollision = AABBbox.AABBIntersect(FEAABB(TransformedTrianglePoints));
	//bAABBTriangleCollision = AABBbox.IntersectsTriangle(TransformedTrianglePoints[0], TransformedTrianglePoints[1], TransformedTrianglePoints[2]);

	/*bAABBTriangleCollision = false;
	if (AABBbox.containsPoint(TransformedTrianglePoints[0]) || AABBbox.containsPoint(TransformedTrianglePoints[1]) || AABBbox.containsPoint(TransformedTrianglePoints[2]))
		bAABBTriangleCollision = true;

	if (!bAABBTriangleCollision)
	{
		bAABBTriangleCollision = AABBSideTriangleIntersection(AABBbox, TransformedTrianglePoints);
	}*/

	// FIX ME
	//bAABBTriangleCollision = AABBbox.IntersectsTriangle(TransformedTrianglePoints);
}

void ReRenderAll()
{
	// FIX ME
	/*LINE_RENDERER.clearAll();
	LINE_RENDERER.RenderAABB(AABBbox, bAABBTriangleCollision ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f));
	AddTriangleLines();
	LINE_RENDERER.SyncWithGPU();*/
}

void TestTriangleAndAABBboxIntersections()
{
	if (TrianglePoints.size() == 0)
	{
		TrianglePoints.push_back(glm::vec3(-0.1f));
		TrianglePoints.push_back(glm::vec3(1.0f));
		TrianglePoints.push_back(glm::vec3(-0.5f, 1.0f, 0.0f));

		AABBbox = FEAABB(glm::vec3(0.0f), glm::vec3(1.0f));
	}

	if (ImGui::Begin("Test Intersections", nullptr, 0))
	{
		ImGui::Text("First triangle point : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##A's X pos : ", &TrianglePoints[0][0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##A's Y pos : ", &TrianglePoints[0][1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##A's Z pos : ", &TrianglePoints[0][2], 0.1f);


		ImGui::Text("Second triangle point : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##B's X pos : ", &TrianglePoints[1][0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##B's Y pos : ", &TrianglePoints[1][1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##B's Z pos : ", &TrianglePoints[1][2], 0.1f);


		ImGui::Text("Third triangle point : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##C's X pos : ", &TrianglePoints[2][0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##C's Y pos : ", &TrianglePoints[2][1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##C's Z pos : ", &TrianglePoints[2][2], 0.1f);

		ShowTransformConfiguration("Triangle transformations", &TriangleTransform);

		glm::vec3 AABBCenter = AABBbox.GetCenter();
		ImGui::Text("AABB box center: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##Center X pos : ", &AABBCenter[0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##Center Y pos : ", &AABBCenter[1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##Center Z pos : ", &AABBCenter[2], 0.1f);


		float AABBSize = AABBbox.GetSize();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat("##AABBSize : ", &AABBSize, 0.1f);

		AABBbox = FEAABB(AABBCenter, AABBSize);
	}

	ImGui::End();
	ReRenderAll();
}

// ************ PART OF DEBUG CODE END ************

void AddFontOnSecondFrame()
{
	static bool bFirstTime = true;
	if (bFirstTime)
	{
		bFirstTime = false;
	}
	else
	{
		glfwMakeContextCurrent(APPLICATION.GetMainWindow()->GetGlfwWindow());
		ImGui::SetCurrentContext(APPLICATION.GetMainWindow()->GetImGuiContext());
		
		if (ImGui::GetIO().Fonts->Fonts.Size == 1)
		{
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
	static int FEWorldMatrix_hash = int(std::hash<std::string>{}("FEWorldMatrix"));
	static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));
	static bool FirstFrame = true;

	// FIX ME ?
	//FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

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

	//ImGui::ShowDemoWindow();
	//TestTriangleAndAABBboxIntersections();

	// FIX ME
	if (MESH_MANAGER.ActiveEntity != nullptr)
	{
	//	MESH_MANAGER.MeshShader->Start();

	//	auto iterator = MESH_MANAGER.MeshShader->parameters.begin();
	//	while (iterator != MESH_MANAGER.MeshShader->parameters.end())
	//	{
	//		if (iterator->second.nameHash == FEWorldMatrix_hash)
	//			iterator->second.updateData(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix());

	//		if (iterator->second.nameHash == FEViewMatrix_hash)
	//			iterator->second.updateData(CurrentCamera->GetViewMatrix());

	//		if (iterator->second.nameHash == FEProjectionMatrix_hash)
	//			iterator->second.updateData(CurrentCamera->GetProjectionMatrix());

	//		iterator++;
	//	}

	//	MESH_MANAGER.MeshShader->LoadDataToGPU();

		if (UI.GetWireFrameMode())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		MESH_RENDERER.RenderFEMesh(MESH_MANAGER.ActiveMesh);

	//	MESH_MANAGER.MeshShader->Stop();
	}

	//LINE_RENDERER.Render(CurrentCamera);

	UI.Render();

	if (FirstFrame)
	{
		FirstFrame = false;
		UI.ApplyStandardWindowsSizeAndPosition();
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LOG.SetFileOutput(true);

	const auto processor_count = THREAD_POOL.GetLogicalCoreCount();
	const unsigned int HowManyToUse = processor_count > 4 ? processor_count - 2 : 1;

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
		//FEWindow* MainWinodw = APPLICATION.AddWindow(1280, 720, "Rugosity Calculator");
		FEWindow* MainWinodw = APPLICATION.GetMainWindow();

		UI.MainWindow = MainWinodw;
		UI.MainWindow->SetRenderFunction(MainWindowRender);

		UI.MainWindow->AddOnDropCallback(dropCallback);
		UI.MainWindow->AddOnMouseButtonCallback(mouseButtonCallback);
		UI.MainWindow->AddOnResizeCallback(windowResizeCallback);
		UI.MainWindow->AddOnScrollCallback(ScrollCall);

		ENGINE.SetClearColor(glm::vec4(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w));
		RENDERER.SetSkyEnabled(false);
		//glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
		//FE_GL_ERROR(glEnable(GL_DEPTH_TEST));


		UI.SwapCameraImpl = SwapCamera;

		MESH_MANAGER.AddLoadCallback(AfterMeshLoads);

		SCREENSHOT_MANAGER.Init();

		ENGINE.GetCamera()->SetIsInputActive(false);


		

		while (ENGINE.IsNotTerminated())
		{
			// FIX ME
			//if (ImGui::GetCurrentContext() != nullptr)
			//	AddFontOnSecondFrame();

			ENGINE.BeginFrame();

			ENGINE.Render();

			ENGINE.EndFrame();

			/*if (MESH_MANAGER.CustomMeshShader == nullptr)
				MESH_MANAGER.CustomMeshShader = new FEShader("MainMeshShader", sTestVS, sTestFS);*/
		}

		//while (APPLICATION.IsNotTerminated())
		//{
		//	if (ImGui::GetCurrentContext() != nullptr)
		//		AddFontOnSecondFrame();

		//	APPLICATION.BeginFrame();

		//	APPLICATION.RenderWindows();

		//	APPLICATION.EndFrame();

		//	/*auto ID = GetCurrentProcess();
		//	auto test = RAMUsed();
		//	int y = 0;*/
		//}
	}

	return 0;
}