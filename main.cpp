#include <windows.h>
#include <psapi.h>
//#include <ntddk.h>

//#include <pdh.h>
//#include <pdhmsg.h>
//#pragma comment(lib, "pdh.lib")

#include "SubSystems/UI/UIManager.h"
using namespace FocalEngine;

FEBasicCamera* currentCamera = nullptr;

void SwapCamera(bool bModelCamera)
{
	delete currentCamera;

	if (bModelCamera)
	{
		currentCamera = new FEModelViewCamera("mainCamera");
	}
	else
	{
		currentCamera = new FEFreeCamera("mainCamera");
	}
	currentCamera->SetIsInputActive(false);

	UI.SetCamera(currentCamera);
}

double mouseX;
double mouseY;

glm::dvec3 mouseRay(double mouseX, double mouseY)
{
	int W, H;
	APPLICATION.GetWindowSize(&W, &H);

	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / W - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / H;

	const glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(currentCamera->GetProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(currentCamera->GetViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

void renderTargetCenterForCamera()
{
	int centerX, centerY = 0;
	int shiftX, shiftY = 0;

	int xpos, ypos;
	APPLICATION.GetWindowPosition(&xpos, &ypos);
	
	int windowW, windowH = 0;
	APPLICATION.GetWindowSize(&windowW, &windowH);
	centerX = xpos + (windowW / 2);
	centerY = ypos + (windowH / 2);

	shiftX = xpos;
	shiftY = ypos;

	if (!UI.GetIsModelCamera())
	{
		FEFreeCamera* FreeCamera = reinterpret_cast<FEFreeCamera*>(currentCamera);

		FreeCamera->SetRenderTargetCenterX(centerX);
		FreeCamera->SetRenderTargetCenterY(centerY);

		FreeCamera->SetRenderTargetShiftX(shiftX);
		FreeCamera->SetRenderTargetShiftY(shiftY);
	}
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

	if (UI.GetIsModelCamera())
		reinterpret_cast<FEModelViewCamera*>(currentCamera)->SetDistanceToModel(reinterpret_cast<FEModelViewCamera*>(currentCamera)->GetDistanceToModel() + Yoffset * MESH_MANAGER.ActiveMesh->AABB.getSize() * 0.05f);
}

static std::string FileNameForSelectionOutput = "Selections";

void AfterMeshLoads()
{
	MESH_MANAGER.ActiveMesh->Position->setPosition(-MESH_MANAGER.ActiveMesh->AABB.getCenter());
	MESH_MANAGER.ActiveMesh->UpdateAverageNormal();

	UI.SetIsModelCamera(true);

	MESH_MANAGER.MeshShader->getParameter("lightDirection")->updateData(glm::normalize(MESH_MANAGER.ActiveMesh->GetAverageNormal()));

	if (MESH_MANAGER.ActiveMesh->Layers.empty())
		MESH_MANAGER.ActiveMesh->AddLayer(HEIGHT_LAYER_PRODUCER.Calculate(MESH_MANAGER.ActiveMesh));

	FileNameForSelectionOutput = MESH_MANAGER.ActiveMesh->FileName;
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

void mouseMoveCallback(double xpos, double ypos)
{
	if (currentCamera != nullptr)
		currentCamera->MouseMoveInput(xpos, ypos);

	mouseX = xpos;
	mouseY = ypos;
}

void keyButtonCallback(int key, int scancode, int action, int mods)
{
	currentCamera->KeyboardInput(key, scancode, action, mods);
}

void UpdateMeshSelectedTrianglesRendering(FEMesh* Mesh)
{
	LINE_RENDERER.clearAll();

	if (Mesh->TriangleSelected.size() == 1)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = Mesh->Triangles[Mesh->TriangleSelected[0]];
		for (size_t i = 0; i < TranformedTrianglePoints.size(); i++)
		{
			TranformedTrianglePoints[i] = Mesh->Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[i], 1.0f);
		}

		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));

		if (!Mesh->TrianglesNormals.empty())
		{
			glm::vec3 Point = TranformedTrianglePoints[0];
			glm::vec3 Normal = Mesh->TrianglesNormals[Mesh->TriangleSelected[0]][0];
			LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[1];
			Normal = Mesh->TrianglesNormals[Mesh->TriangleSelected[0]][1];
			LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

			Point = TranformedTrianglePoints[2];
			Normal = Mesh->TrianglesNormals[Mesh->TriangleSelected[0]][2];
			LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
		}

		/*if (!mesh->originalTrianglesToSegments.empty() && !mesh->segmentsNormals.empty())
		{
			glm::vec3 Centroid = (TranformedTrianglePoints[0] +
				TranformedTrianglePoints[1] +
				TranformedTrianglePoints[2]) / 3.0f;

			glm::vec3 Normal = mesh->segmentsNormals[mesh->originalTrianglesToSegments[mesh->TriangleSelected[0]]];
			LINE_RENDERER.AddLineToBuffer(FELine(Centroid, Centroid + Normal, glm::vec3(1.0f, 0.0f, 0.0f)));
		}*/

		LINE_RENDERER.SyncWithGPU();
	}
	else if (Mesh->TriangleSelected.size() > 1)
	{
		for (size_t i = 0; i < Mesh->TriangleSelected.size(); i++)
		{
			std::vector<glm::vec3> TranformedTrianglePoints = Mesh->Triangles[Mesh->TriangleSelected[i]];
			for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
			{
				TranformedTrianglePoints[j] = Mesh->Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
			}

			LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
			LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
			LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		}

		LINE_RENDERER.SyncWithGPU();
	}
}

void OutputSeletedAreaInfoToFile()
{
	if (MESH_MANAGER.ActiveMesh->TriangleSelected.size() < 2)
		return;

	std::string Text = "Area radius : " + std::to_string(UI.GetRadiusOfAreaToMeasure());
	LOG.Add(Text, FileNameForSelectionOutput);

	Text = "Area approximate center : X - ";
	const glm::vec3 Center = MESH_MANAGER.ActiveMesh->TrianglesCentroids[MESH_MANAGER.ActiveMesh->TriangleSelected[0]];
	Text += std::to_string(Center.x);
	Text += " Y - ";
	Text += std::to_string(Center.y);
	Text += " Z - ";
	Text += std::to_string(Center.z);
	LOG.Add(Text, FileNameForSelectionOutput);

	for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->Layers.size(); i++)
	{
		MeshLayer* CurrentLayer = &MESH_MANAGER.ActiveMesh->Layers[i];

		Text = "Layer \"" + CurrentLayer->GetCaption() + "\" : \n";
		Text += "Area average value : ";
		float Total = 0.0f;
		for (size_t j = 0; j < MESH_MANAGER.ActiveMesh->TriangleSelected.size(); j++)
		{
			Total += CurrentLayer->TrianglesToData[MESH_MANAGER.ActiveMesh->TriangleSelected[i]];
		}

		Total /= MESH_MANAGER.ActiveMesh->TriangleSelected.size();
		Text += std::to_string(Total);
		LOG.Add(Text, FileNameForSelectionOutput);
	}
}

void mouseButtonCallback(int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		currentCamera->SetIsInputActive(false);
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		currentCamera->SetIsInputActive(true);
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		currentCamera->SetIsInputActive(false);
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		if (MESH_MANAGER.ActiveMesh != nullptr)
		{
			if (UI.GetLayerSelectionMode() == 1)
			{
				MESH_MANAGER.ActiveMesh->SelectTriangle(mouseRay(mouseX, mouseY), currentCamera);
			}
			else if (UI.GetLayerSelectionMode() == 2)
			{
				if (MESH_MANAGER.ActiveMesh->SelectTrianglesInRadius(mouseRay(mouseX, mouseY), currentCamera, UI.GetRadiusOfAreaToMeasure()) &&
					UI.GetOutputSelectionToFile())
				{
					OutputSeletedAreaInfoToFile();
				}
			}

			UpdateMeshSelectedTrianglesRendering(MESH_MANAGER.ActiveMesh);
		}

		if (MESH_MANAGER.ActiveMesh != nullptr && RUGOSITY_MANAGER.currentSDF != nullptr)
		{
			if (RUGOSITY_MANAGER.currentSDF->RenderingMode == 0)
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
				RUGOSITY_MANAGER.currentSDF->MouseClick(mouseX, mouseY);
				RUGOSITY_MANAGER.currentSDF->UpdateRenderLines();
			}
		}
	}
}

void RenderFEMesh(FEMesh* Mesh)
{
	MESH_MANAGER.MeshShader->getParameter("AmbientFactor")->updateData(UI.GetAmbientLightFactor());
	MESH_MANAGER.MeshShader->getParameter("HaveColor")->updateData(Mesh->getColorCount() != 0);
	MESH_MANAGER.MeshShader->getParameter("HeatMapType")->updateData(Mesh->HeatMapType);
	MESH_MANAGER.MeshShader->getParameter("LayerIndex")->updateData(LAYER_MANAGER.GetActiveLayerIndex());
	
	if (LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		MESH_MANAGER.MeshShader->getParameter("LayerMin")->updateData(float(Mesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()].MinVisible));
		MESH_MANAGER.MeshShader->getParameter("LayerMax")->updateData(float(Mesh->Layers[LAYER_MANAGER.GetActiveLayerIndex()].MaxVisible));
	}
	
	if (Mesh->TriangleSelected.size() > 1 && UI.GetLayerSelectionMode() == 2)
	{
		MESH_MANAGER.MeshShader->getParameter("MeasuredRugosityAreaRadius")->updateData(Mesh->LastMeasuredRugosityAreaRadius);
		MESH_MANAGER.MeshShader->getParameter("MeasuredRugosityAreaCenter")->updateData(Mesh->LastMeasuredRugosityAreaCenter);
	}
	else
	{
		MESH_MANAGER.MeshShader->getParameter("MeasuredRugosityAreaRadius")->updateData(-1.0f);
	}

	FE_GL_ERROR(glBindVertexArray(Mesh->GetVaoID()));
	if ((Mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((Mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((Mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((Mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((Mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));

	if ((Mesh->vertexAttributes & FE_RUGOSITY_FIRST) == FE_RUGOSITY_FIRST) FE_GL_ERROR(glEnableVertexAttribArray(7));
	if ((Mesh->vertexAttributes & FE_RUGOSITY_SECOND) == FE_RUGOSITY_SECOND) FE_GL_ERROR(glEnableVertexAttribArray(8));

	if ((Mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, Mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	if ((Mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, Mesh->getVertexCount()));

	glBindVertexArray(0);
}

void windowResizeCallback(int width, int height)
{
	int W, H;
	APPLICATION.GetWindowSize(&W, &H);
	currentCamera->SetAspectRatio(float(W) / float(H));

	UI.ApplyStandardWindowsSizeAndPosition();
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


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LOG.SetFileOutput(true);

	APPLICATION.InitWindow(1280, 720, "Rugosity Calculator");
	APPLICATION.SetDropCallback(dropCallback);
	APPLICATION.SetKeyCallback(keyButtonCallback);
	APPLICATION.SetMouseMoveCallback(mouseMoveCallback);
	APPLICATION.SetMouseButtonCallback(mouseButtonCallback);
	APPLICATION.SetWindowResizeCallback(windowResizeCallback);
	APPLICATION.SetScrollCallback(ScrollCall);

	const auto processor_count = THREAD_POOL.GetLogicalCoreCount();
	const unsigned int HowManyToUse = processor_count > 4 ? processor_count - 2 : 1;

	THREAD_POOL.SetConcurrentThreadCount(HowManyToUse);

	glClearColor(153.0f / 255.0f, 217.0f / 255.0f, 234.0f / 255.0f, 1.0f);
	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));

	static int FEWorldMatrix_hash = int(std::hash<std::string>{}("FEWorldMatrix"));
	static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));

	currentCamera = new FEModelViewCamera("mainCamera");
	currentCamera->SetIsInputActive(false);
	currentCamera->SetAspectRatio(1280.0f / 720.0f);

	UI.SetCamera(currentCamera);
	UI.SwapCameraImpl = SwapCamera;

	MESH_MANAGER.AddLoadCallback(AfterMeshLoads);

	static bool FirstFrame = true;
	while (APPLICATION.IsWindowOpened())
	{
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		APPLICATION.BeginFrame();

		renderTargetCenterForCamera();
		currentCamera->Move(10);

		//ImGui::ShowDemoWindow();

		if (MESH_MANAGER.ActiveMesh != nullptr)
		{
			MESH_MANAGER.MeshShader->start();

			auto iterator = MESH_MANAGER.MeshShader->parameters.begin();
			while (iterator != MESH_MANAGER.MeshShader->parameters.end())
			{
				if (iterator->second.nameHash == FEWorldMatrix_hash)
					iterator->second.updateData(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix());

				if (iterator->second.nameHash == FEViewMatrix_hash)
					iterator->second.updateData(currentCamera->GetViewMatrix());

				if (iterator->second.nameHash == FEProjectionMatrix_hash)
					iterator->second.updateData(currentCamera->GetProjectionMatrix());

				iterator++;
			}

			MESH_MANAGER.MeshShader->loadDataToGPU();

			if (UI.GetWireFrameMode())
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			RenderFEMesh(MESH_MANAGER.ActiveMesh);

			//if (UI.TestMesh)
			//	renderFEMesh(UI.TestMesh);

			/*FETransformComponent* newPosition = new FETransformComponent();
			newPosition->setPosition(glm::vec3(0.0, 0.0, 5.0));
			testShader->getParameter("FEWorldMatrix")->updateData(newPosition->getTransformMatrix());
			testShader->loadDataToGPU();

			renderFEMesh(compareToMesh);*/
			
			MESH_MANAGER.MeshShader->stop();
		}

		LINE_RENDERER.Render(currentCamera);
			
		//UI.RenderMainWindow(CurrentMesh);
		UI.Render();

		if (FirstFrame)
		{
			FirstFrame = false;
			UI.ApplyStandardWindowsSizeAndPosition();
		}
		
		APPLICATION.EndFrame();

		/*auto ID = GetCurrentProcess();
		auto test = RAMUsed();
		int y = 0;*/
	}

	return 0;
}