
#include "SubSystems/UIManager.h"
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
	currentCamera->setIsInputActive(false);

	UI.SetCamera(currentCamera);
	RUGOSITY_MANAGER.currentCamera = currentCamera;
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
	glm::dvec4 eyeCoords = glm::inverse(currentCamera->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(currentCamera->getViewMatrix()) * eyeCoords;
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

		FreeCamera->setRenderTargetCenterX(centerX);
		FreeCamera->setRenderTargetCenterY(centerY);

		FreeCamera->setRenderTargetShiftX(shiftX);
		FreeCamera->setRenderTargetShiftY(shiftY);
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
	if (UI.GetIsModelCamera())
		reinterpret_cast<FEModelViewCamera*>(currentCamera)->SetDistanceToModel(reinterpret_cast<FEModelViewCamera*>(currentCamera)->GetDistanceToModel() + Yoffset * 2.0);
}

void AfterMeshLoads()
{
	MESH_MANAGER.ActiveMesh->fillTrianglesData();

	MESH_MANAGER.ActiveMesh->Position->setPosition(-MESH_MANAGER.ActiveMesh->AABB.getCenter());
	MESH_MANAGER.ActiveMesh->UpdateAverageNormal();

	UI.SetIsModelCamera(true);

	glm::vec3 AverageNormal = MESH_MANAGER.ActiveMesh->GetAverageNormal();
	glm::vec3 TransformedCenter = MESH_MANAGER.ActiveMesh->Position->getTransformMatrix() * glm::vec4(MESH_MANAGER.ActiveMesh->AABB.getCenter(), 1.0f);

	MESH_MANAGER.MeshShader->getParameter("lightDirection")->updateData(glm::normalize(MESH_MANAGER.ActiveMesh->GetAverageNormal()));

	MESH_MANAGER.ActiveMesh->colorMode = 0;
	if (MESH_MANAGER.ActiveMesh->getColorCount() != 0)
		MESH_MANAGER.ActiveMesh->colorMode = 1;

	// When loading from binary format we would have that data in file.
	if (!MESH_MANAGER.ActiveMesh->rugosityData.empty() && !MESH_MANAGER.ActiveMesh->TrianglesRugosity.empty())
	{
		MESH_MANAGER.ActiveMesh->fillRugosityDataToGPU();
		RUGOSITY_MANAGER.ForceOnRugosityCalculationsEnd();
	}
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
		currentCamera->mouseMoveInput(xpos, ypos);

	mouseX = xpos;
	mouseY = ypos;
}

void keyButtonCallback(int key, int scancode, int action, int mods)
{
	currentCamera->keyboardInput(key, scancode, action, mods);
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

	std::string Text = "Area radius : " + std::to_string(UI.GetAreaToMeasureRugosity());
	LOG.Add(Text, "Selections");

	Text = "Area approximate center : X - ";
	const glm::vec3 Center = MESH_MANAGER.ActiveMesh->TrianglesCentroids[MESH_MANAGER.ActiveMesh->TriangleSelected[0]];
	Text += std::to_string(Center.x);
	Text += " Y - ";
	Text += std::to_string(Center.y);
	Text += " Z - ";
	Text += std::to_string(Center.z);
	LOG.Add(Text, "Selections");

	Text = "Area rugosity : ";
	float TotalRugosity = 0.0f;
	for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->TriangleSelected.size(); i++)
	{
		TotalRugosity += MESH_MANAGER.ActiveMesh->TrianglesRugosity[MESH_MANAGER.ActiveMesh->TriangleSelected[i]];
	}

	TotalRugosity /= MESH_MANAGER.ActiveMesh->TriangleSelected.size();
	Text += std::to_string(TotalRugosity);
	LOG.Add(Text, "Selections");

	Text = "Area average height : ";
	double AverageHeight = 0.0;
	for (size_t i = 0; i < MESH_MANAGER.ActiveMesh->TriangleSelected.size(); i++)
	{
		double CurrentHeight = 0.0;
		for (size_t j = 0; j < 3; j++)
		{
			glm::vec3 CurrentPoint = MESH_MANAGER.ActiveMesh->Triangles[MESH_MANAGER.ActiveMesh->TriangleSelected[i]][j];
			CurrentHeight += glm::dot(glm::vec3(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix() * glm::vec4(CurrentPoint, 1.0)), MESH_MANAGER.ActiveMesh->AverageNormal);
		}

		CurrentHeight /= 3.0;
		AverageHeight += CurrentHeight;
	}

	AverageHeight /= MESH_MANAGER.ActiveMesh->TriangleSelected.size();
	AverageHeight -= MESH_MANAGER.ActiveMesh->MinHeight;

	Text += std::to_string(AverageHeight);

	LOG.Add(Text, "Selections");
	LOG.Add("\n", "Selections");
}

void mouseButtonCallback(int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		currentCamera->setIsInputActive(false);
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		currentCamera->setIsInputActive(true);
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		currentCamera->setIsInputActive(false);
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		if (MESH_MANAGER.ActiveMesh != nullptr)
		{
			if (UI.GetRugositySelectionMode() == 1)
			{
				MESH_MANAGER.ActiveMesh->SelectTriangle(mouseRay(mouseX, mouseY), currentCamera);
			}
			else if (UI.GetRugositySelectionMode() == 2)
			{
				if (MESH_MANAGER.ActiveMesh->SelectTrianglesInRadius(mouseRay(mouseX, mouseY), currentCamera, UI.GetAreaToMeasureRugosity()) &&
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
				/*LINE_RENDERER.clearAll();
				addLinesOFSDF(RUGOSITY_MANAGER.currentSDF);
				LINE_RENDERER.SyncWithGPU();*/
			}
		}
	}
}

void RenderFEMesh(FEMesh* Mesh)
{
	MESH_MANAGER.MeshShader->getParameter("colorMode")->updateData(Mesh->colorMode);

	MESH_MANAGER.MeshShader->getParameter("minRugorsity")->updateData(float(Mesh->minVisibleRugorsity));
	MESH_MANAGER.MeshShader->getParameter("maxRugorsity")->updateData(float(Mesh->maxVisibleRugorsity));

	// Height heat map.
	MESH_MANAGER.MeshShader->getParameter("AverageNormal")->updateData(Mesh->AverageNormal);
	MESH_MANAGER.MeshShader->getParameter("minHeight")->updateData(float(Mesh->MinHeight));
	MESH_MANAGER.MeshShader->getParameter("maxHeight")->updateData(float(Mesh->MaxHeight));
	
	if (Mesh->TriangleSelected.size() > 1 && UI.GetRugositySelectionMode() == 2)
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

	if ((Mesh->vertexAttributes & FE_SEGMENTS_COLORS) == FE_SEGMENTS_COLORS) FE_GL_ERROR(glEnableVertexAttribArray(7));
	if ((Mesh->vertexAttributes & FE_RUGOSITY_FIRST) == FE_RUGOSITY_FIRST) FE_GL_ERROR(glEnableVertexAttribArray(8));
	if ((Mesh->vertexAttributes & FE_RUGOSITY_SECOND) == FE_RUGOSITY_SECOND) FE_GL_ERROR(glEnableVertexAttribArray(9));

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
	currentCamera->setAspectRatio(float(W) / float(H));

	UI.ApplyStandardWindowsSizeAndPosition();
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
	currentCamera->setIsInputActive(false);
	currentCamera->setAspectRatio(1280.0f / 720.0f);

	UI.SetCamera(currentCamera);
	RUGOSITY_MANAGER.currentCamera = currentCamera;
	UI.SwapCameraImpl = SwapCamera;

	MESH_MANAGER.AddLoadCallback(AfterMeshLoads);

	static bool FirstFrame = true;
	while (APPLICATION.IsWindowOpened())
	{
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		APPLICATION.BeginFrame();

		renderTargetCenterForCamera();
		currentCamera->move(10);

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
					iterator->second.updateData(currentCamera->getViewMatrix());

				if (iterator->second.nameHash == FEProjectionMatrix_hash)
					iterator->second.updateData(currentCamera->getProjectionMatrix());

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
	}

	return 0;
}