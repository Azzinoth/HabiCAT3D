#include "UIManager.h"
using namespace FocalEngine;
UIManager* UIManager::Instance = nullptr;
UIManager::UIManager()
{
	ScrollerTest.SetOrientation(false);
}

UIManager::~UIManager() {}

void UIManager::showTransformConfiguration(std::string name, FETransformComponent* transform)
{
	// ********************* POSITION *********************
	glm::vec3 position = transform->getPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + name).c_str(), &position[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + name).c_str(), &position[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + name).c_str(), &position[2], 0.1f);
	transform->setPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = transform->getRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + name).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + name).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + name).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	transform->setRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &transform->uniformScaling);
	glm::vec3 scale = transform->getScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + name).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + name).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + name).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);

	glm::vec3 oldScale = transform->getScale();
	transform->changeXScaleBy(scale[0] - oldScale[0]);
	transform->changeYScaleBy(scale[1] - oldScale[1]);
	transform->changeZScaleBy(scale[2] - oldScale[2]);
}

void UIManager::showCameraTransform()
{
	/*glm::vec3 ViewDirection = currentCamera->getForward();
	std::string Text = "ViewDirection : X - ";
	Text += std::to_string(ViewDirection.x);
	Text += " Y - ";
	Text += std::to_string(ViewDirection.y);
	Text += " Z - ";
	Text += std::to_string(ViewDirection.z);
	ImGui::Text(Text.c_str());

	if (currentMesh != nullptr)
	{
		glm::vec3 ToMeshDirection = glm::normalize(currentMesh->AABB.getCenter() - currentCamera->getPosition());
		Text = "ToMeshDirection : X - ";
		Text += std::to_string(ToMeshDirection.x);
		Text += " Y - ";
		Text += std::to_string(ToMeshDirection.y);
		Text += " Z - ";
		Text += std::to_string(ToMeshDirection.z);
		ImGui::Text(Text.c_str());
	}*/

	// ********* POSITION *********
	glm::vec3 cameraPosition = currentCamera->getPosition();

	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##X pos", &cameraPosition[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Y pos", &cameraPosition[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Z pos", &cameraPosition[2], 0.1f);

	currentCamera->setPosition(cameraPosition);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	//ImGui::push
	if (ImGui::Button("Copy##Position"))
	{
		APPLICATION.SetClipboardText(CameraPositionToStr());
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	if (ImGui::Button("Paste##Position"))
	{
		StrToCameraPosition(APPLICATION.GetClipboardText());
	}

	// ********* ROTATION *********
	glm::vec3 cameraRotation = glm::vec3(currentCamera->getYaw(), currentCamera->getPitch(), currentCamera->getRoll());

	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##X rot", &cameraRotation[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Y rot", &cameraRotation[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Z rot", &cameraRotation[2], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	if (ImGui::Button("Copy##Rotation"))
	{
		APPLICATION.SetClipboardText(CameraRotationToStr());
	}

	currentCamera->setYaw(cameraRotation[0]);
	currentCamera->setPitch(cameraRotation[1]);
	currentCamera->setRoll(cameraRotation[2]);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	if (ImGui::Button("Paste##Rotation"))
	{
		StrToCameraRotation(APPLICATION.GetClipboardText());
	}

	float cameraSpeed = currentCamera->getMovementSpeed();
	ImGui::Text("Camera speed: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Camera_speed", &cameraSpeed, 0.01f, 0.01f, 100.0f);
	currentCamera->setMovementSpeed(cameraSpeed);

	ImGui::SameLine();
	ImGui::Text(("Thread count: " + std::to_string(THREAD_POOL.GetThreadCount())).c_str());
}

void UIManager::SetCamera(FEFreeCamera* newCamera)
{
	currentCamera = newCamera;
}

void UIManager::SetMeshShader(FEShader* newShader)
{
	meshShader = newShader;
}

bool UIManager::GetWireFrameMode()
{
	return wireframeMode;
}

void UIManager::SetWireFrameMode(bool NewValue)
{
	wireframeMode = NewValue;
}

bool UIManager::GetDeveloperMode()
{
	return DeveloperMode;
}

void UIManager::SetDeveloperMode(bool NewValue)
{
	DeveloperMode = NewValue;
}

void UIManager::RenderMainWindow(FEMesh* currentMesh)
{
	ImGui::Checkbox("Developer UI", &DeveloperMode);

	showCameraTransform();

	bool TempBool = RUGOSITY_MANAGER.GetUseFindSmallestRugosity();
	if (ImGui::Checkbox("New algorithm", &TempBool))
	{
		RUGOSITY_MANAGER.SetUseFindSmallestRugosity(TempBool);
	}

	if (currentMesh != nullptr)
	{
		if (DeveloperMode)
		{
			// ********************* LIGHT DIRECTION *********************
			glm::vec3 position = *reinterpret_cast<glm::vec3*>(meshShader->getParameter("lightDirection")->data);
			ImGui::Text("Light direction : ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			ImGui::DragFloat((std::string("##X pos : ")).c_str(), &position[0], 0.1f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			ImGui::DragFloat((std::string("##Y pos : ")).c_str(), &position[1], 0.1f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			ImGui::DragFloat((std::string("##Z pos : ")).c_str(), &position[2], 0.1f);

			meshShader->getParameter("lightDirection")->updateData(position);

			ImGui::Checkbox("Wireframe", &wireframeMode);

			//if (currentMesh->minRugorsity != DBL_MAX)
			//	ImGui::Text(("minRugorsity: " + std::to_string(currentMesh->minRugorsity)).c_str());

			//if (currentMesh->maxRugorsity != -DBL_MAX)
			//	ImGui::Text(("maxRugorsity: " + std::to_string(currentMesh->maxRugorsity)).c_str());

			if (currentMesh->TriangleSelected.size() == 1)
			{
				ImGui::Separator();
				ImGui::Text("Selected triangle information :");

				std::string Text = "Index : " + std::to_string(currentMesh->TriangleSelected[0]);
				ImGui::Text(Text.c_str());

				Text = "First vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][0].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][0].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][0].z);
				ImGui::Text(Text.c_str());

				Text = "Second vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][1].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][1].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][1].z);
				ImGui::Text(Text.c_str());

				Text = "Third vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][2].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][2].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected[0]][2].z);
				ImGui::Text(Text.c_str());

				Text = "Segment ID : ";
				if (!currentMesh->originalTrianglesToSegments.empty())
				{
					Text += std::to_string(currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected[0]]);
				}
				else
				{
					Text += "No information.";
				}
				ImGui::Text(Text.c_str());

				Text = "Segment normal : ";
				if (!currentMesh->originalTrianglesToSegments.empty() && !currentMesh->segmentsNormals.empty())
				{
					glm::vec3 normal = currentMesh->segmentsNormals[currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected[0]]];

					Text += "x: " + std::to_string(normal.x);
					Text += " y: " + std::to_string(normal.y);
					Text += " z: " + std::to_string(normal.z);
				}
				else
				{
					Text += "No information.";
				}
				ImGui::Text(Text.c_str());
			}

			if (ImGui::Button("Generate second rugosity layer"))
			{
				RUGOSITY_MANAGER.calculateRugorsityWithJitterAsyn(currentMesh, 1);
			}

			ImGui::Separator();
			if (/*RUGOSITY_MANAGER.currentSDF == nullptr*/1)
			{
				if (ImGui::Button("Generate SDF"))
				{
					RUGOSITY_MANAGER.JitterCounter = 0;
					currentMesh->TrianglesRugosity.clear();
					currentMesh->rugosityData.clear();

					RUGOSITY_MANAGER.bLastJitter = true;
					RUGOSITY_MANAGER.RunCreationOfSDFAsync(currentMesh);

					//calculateSDF(currentMesh, SDFDimention);
					//MoveRugosityInfoToMesh(currentSDF, true);
				}

				ImGui::Text(("For current mesh \n Min value : "
					+ std::to_string(RUGOSITY_MANAGER.LowestResolution)
					+ " m \n Max value : "
					+ std::to_string(RUGOSITY_MANAGER.HigestResolution) + " m").c_str());

				ImGui::SetNextItemWidth(128);
				ImGui::DragFloat("##ResolutonInM", &RUGOSITY_MANAGER.ResolutonInM, 0.01f);

				if (RUGOSITY_MANAGER.ResolutonInM < RUGOSITY_MANAGER.LowestResolution)
					RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.LowestResolution;

				if (RUGOSITY_MANAGER.ResolutonInM > RUGOSITY_MANAGER.HigestResolution)
					RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.HigestResolution;

				ImGui::Checkbox("Weighted normals", &RUGOSITY_MANAGER.bWeightedNormals);
				ImGui::Checkbox("Normalized normals", &RUGOSITY_MANAGER.bNormalizedNormals);

				if (ImGui::Button("Generate SDF with Jitter"))
				{
					TIME.BeginTimeStamp("TimeTookToJitter");

					RUGOSITY_MANAGER.calculateRugorsityWithJitterAsyn(currentMesh);

					TimeTookToJitter = TIME.EndTimeStamp("TimeTookToJitter");
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::DragInt("Smoothing factor", &RUGOSITY_MANAGER.SmoothingFactor);
				if (RUGOSITY_MANAGER.SmoothingFactor < 2)
					RUGOSITY_MANAGER.SmoothingFactor = 2;
			}
			else if (RUGOSITY_MANAGER.currentSDF != nullptr && RUGOSITY_MANAGER.currentSDF->bFullyLoaded)
			{
				if (ImGui::Button("Delete SDF"))
				{
					//currentSDF->mesh clear all rugosity info

					LINE_RENDERER.clearAll();
					LINE_RENDERER.SyncWithGPU();

					delete RUGOSITY_MANAGER.currentSDF;
					RUGOSITY_MANAGER.currentSDF = nullptr;
				}
			}

			if (RUGOSITY_MANAGER.newSDFSeen > 0 && RUGOSITY_MANAGER.newSDFSeen < RUGOSITY_MANAGER.SmoothingFactor)
			{
				std::string Text = "Progress: " + std::to_string(RUGOSITY_MANAGER.newSDFSeen) + " out of " + std::to_string(RUGOSITY_MANAGER.SmoothingFactor);
				ImGui::Text(Text.c_str());
			}

			if (RUGOSITY_MANAGER.currentSDF != nullptr && RUGOSITY_MANAGER.currentSDF->bFullyLoaded)
			{
				if (ImGui::Checkbox("Show Rugosity", &currentMesh->showRugosity))
				{
					if (currentMesh->showRugosity)
					{
						currentMesh->colorMode = 5;
					}
					else
					{
						currentMesh->colorMode = 0;
					}
				}

				ImGui::Text("Color scheme: ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(128);
				if (ImGui::BeginCombo("##ChooseColorScheme", (RUGOSITY_MANAGER.colorSchemeIndexToString(currentMesh->colorMode)).c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < RUGOSITY_MANAGER.colorSchemesList.size(); i++)
					{
						bool is_selected = ((RUGOSITY_MANAGER.colorSchemeIndexToString(currentMesh->colorMode)).c_str() == RUGOSITY_MANAGER.colorSchemesList[i]);
						if (ImGui::Selectable(RUGOSITY_MANAGER.colorSchemesList[i].c_str(), is_selected))
						{
							currentMesh->colorMode = RUGOSITY_MANAGER.colorSchemeIndexFromString(RUGOSITY_MANAGER.colorSchemesList[i]);
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				float maxRugorsity = currentMesh->maxRugorsity;
				ImGui::Text("Max rugorsity for color scaling: ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(128);
				ImGui::DragFloat("##MaxRugorsity", &maxRugorsity, 0.01f);
				if (maxRugorsity < currentMesh->minRugorsity)
					maxRugorsity = currentMesh->minRugorsity + 0.1f;
				currentMesh->maxRugorsity = maxRugorsity;

				ImGui::Separator();

				float TotalTime = 0.0f;

				std::string debugTimers = "Building of SDF grid : " + std::to_string(RUGOSITY_MANAGER.currentSDF->TimeTookToGenerateInMS) + " ms";
				debugTimers += "\n";
				TotalTime += RUGOSITY_MANAGER.currentSDF->TimeTookToGenerateInMS;

				debugTimers += "Fill cells with triangle info : " + std::to_string(RUGOSITY_MANAGER.currentSDF->TimeTookFillCellsWithTriangleInfo) + " ms";
				debugTimers += "\n";
				TotalTime += RUGOSITY_MANAGER.currentSDF->TimeTookFillCellsWithTriangleInfo;

				debugTimers += "Calculate rugosity : " + std::to_string(RUGOSITY_MANAGER.currentSDF->TimeTookCalculateRugosity) + " ms";
				debugTimers += "\n";
				TotalTime += RUGOSITY_MANAGER.currentSDF->TimeTookCalculateRugosity;

				debugTimers += "Assign colors to mesh : " + std::to_string(RUGOSITY_MANAGER.currentSDF->TimeTookFillMeshWithRugosityData) + " ms";
				debugTimers += "\n";
				TotalTime += RUGOSITY_MANAGER.currentSDF->TimeTookFillMeshWithRugosityData;

				debugTimers += "Total time : " + std::to_string(TotalTime) + " ms";
				debugTimers += "\n";

				debugTimers += "TimeTookToJitter : " + std::to_string(TimeTookToJitter) + " ms";
				debugTimers += "\n";

				ImGui::Text((debugTimers).c_str());

				ImGui::Text(("debugTotalTrianglesInCells: " + std::to_string(RUGOSITY_MANAGER.currentSDF->debugTotalTrianglesInCells)).c_str());

				ImGui::Separator();
				ImGui::Text("Visualization of SDF:");

				if (ImGui::RadioButton("Do not draw", &RUGOSITY_MANAGER.currentSDF->RenderingMode, 0))
				{
					RUGOSITY_MANAGER.currentSDF->RenderingMode = 0;

					RUGOSITY_MANAGER.currentSDF->UpdateRenderLines();
					//LINE_RENDERER.clearAll();
					//LINE_RENDERER.SyncWithGPU();
				}

				if (ImGui::RadioButton("Show cells with triangles", &RUGOSITY_MANAGER.currentSDF->RenderingMode, 1))
				{
					RUGOSITY_MANAGER.currentSDF->RenderingMode = 1;

					RUGOSITY_MANAGER.currentSDF->UpdateRenderLines();
					//LINE_RENDERER.clearAll();
					//addLinesOFSDF(RUGOSITY_MANAGER.currentSDF);
					//LINE_RENDERER.SyncWithGPU();
				}

				if (ImGui::RadioButton("Show all cells", &RUGOSITY_MANAGER.currentSDF->RenderingMode, 2))
				{
					RUGOSITY_MANAGER.currentSDF->RenderingMode = 2;

					RUGOSITY_MANAGER.currentSDF->UpdateRenderLines();
					//LINE_RENDERER.clearAll();
					//addLinesOFSDF(RUGOSITY_MANAGER.currentSDF);
					//LINE_RENDERER.SyncWithGPU();
				}

				ImGui::Separator();
			}
			else if (RUGOSITY_MANAGER.currentSDF != nullptr && !RUGOSITY_MANAGER.currentSDF->bFullyLoaded)
			{
				ImGui::Text("Creating SDF...");
			}

			ImGui::Separator();
			if (RUGOSITY_MANAGER.currentSDF != nullptr && RUGOSITY_MANAGER.currentSDF->selectedCell != glm::vec3(0.0))
			{
				SDFNode currentlySelectedCell = RUGOSITY_MANAGER.currentSDF->data[RUGOSITY_MANAGER.currentSDF->selectedCell.x][RUGOSITY_MANAGER.currentSDF->selectedCell.y][RUGOSITY_MANAGER.currentSDF->selectedCell.z];

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text("Selected cell info: ");

				std::string indexes = "i: " + std::to_string(RUGOSITY_MANAGER.currentSDF->selectedCell.x);
				indexes += " j: " + std::to_string(RUGOSITY_MANAGER.currentSDF->selectedCell.y);
				indexes += " k: " + std::to_string(RUGOSITY_MANAGER.currentSDF->selectedCell.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((indexes).c_str());

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text(("value: " + std::to_string(currentlySelectedCell.value)).c_str());

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text(("distanceToTrianglePlane: " + std::to_string(currentlySelectedCell.distanceToTrianglePlane)).c_str());

				ImGui::Separator();

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text("Rugosity info: ");

				std::string text = "averageCellNormal x: " + std::to_string(currentlySelectedCell.averageCellNormal.x);
				text += " y: " + std::to_string(currentlySelectedCell.averageCellNormal.y);
				text += " z: " + std::to_string(currentlySelectedCell.averageCellNormal.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((text).c_str());


				text = "CellTrianglesCentroid x: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.x);
				text += " y: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.y);
				text += " z: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((text).c_str());

				if (currentlySelectedCell.approximateProjectionPlane != nullptr)
				{
					//glm::vec3 NormalStart = currentlySelectedCell.approximateProjectionPlane->PointOnPlane + choosenEntity->transform.getPosition();
					//glm::vec3 NormalEnd = NormalStart + currentlySelectedCell.approximateProjectionPlane->Normal;
					//RENDERER.drawLine(NormalStart, NormalEnd, glm::vec3(0.1f, 0.1f, 0.9f), 0.25f);
				}

				ImGui::Text(("Rugosity : " + std::to_string(currentlySelectedCell.rugosity)).c_str());

				static std::string debugText;

				if (ImGui::Button("Show debug info"))
				{
					RUGOSITY_MANAGER.currentSDF->calculateCellRugosity(&RUGOSITY_MANAGER.currentSDF->
						data[RUGOSITY_MANAGER.currentSDF->selectedCell.x][RUGOSITY_MANAGER.currentSDF->selectedCell.y][RUGOSITY_MANAGER.currentSDF->selectedCell.z], &debugText);
				}

				ImGui::Text(debugText.c_str());
			}
		}
		else
		{
			ImGui::Separator();

			static bool CalculationTrigered = false;
			if (ImGui::Button("Calculate rugosity"))
			{
				RUGOSITY_MANAGER.SmoothingFactor = 64;
				RUGOSITY_MANAGER.calculateRugorsityWithJitterAsyn(currentMesh);
				currentMesh->colorMode = 5;
				currentMesh->showRugosity = true;

				CalculationTrigered = true;
				ImGui::OpenPopup("Calculating...");
			}

			ImGui::Text("Grid size:");
			static int SmallScaleFeatures = 0;
			if (ImGui::RadioButton(("Small (Grid size - " + std::to_string(RUGOSITY_MANAGER.LowestResolution) + " m)").c_str(), &SmallScaleFeatures, 0))
			{

			}

			if (ImGui::RadioButton(("Large (Grid size - " + std::to_string(RUGOSITY_MANAGER.HigestResolution) + " m)").c_str(), &SmallScaleFeatures, 1))
			{

			}

			if (ImGui::RadioButton("Custom", &SmallScaleFeatures, 3))
			{

			}

			if (SmallScaleFeatures == 0)
			{
				RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.LowestResolution;
			}
			else if (SmallScaleFeatures == 1)
			{
				RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.HigestResolution;
			}
			else
			{
				ImGui::Text(("For current mesh \n Min value : " 
					+ std::to_string(RUGOSITY_MANAGER.LowestResolution) 
					+ " m \n Max value : " 
					+ std::to_string(RUGOSITY_MANAGER.HigestResolution) + " m").c_str());

				ImGui::SetNextItemWidth(128);
				ImGui::DragFloat("##ResolutonInM", &RUGOSITY_MANAGER.ResolutonInM, 0.01f);

				if (RUGOSITY_MANAGER.ResolutonInM < RUGOSITY_MANAGER.LowestResolution)
					RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.LowestResolution;

				if (RUGOSITY_MANAGER.ResolutonInM > RUGOSITY_MANAGER.HigestResolution)
					RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.HigestResolution;
			}

			if (RUGOSITY_MANAGER.currentSDF != nullptr && RUGOSITY_MANAGER.currentSDF->bFullyLoaded)
			{
				float maxRugorsity = currentMesh->maxRugorsity;
				ImGui::Text("Heat map sensitivity: ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(128);
				ImGui::DragFloat("##MaxRugorsity", &maxRugorsity, 0.01f);
				if (maxRugorsity < currentMesh->minRugorsity)
					maxRugorsity = currentMesh->minRugorsity + 0.1f;
				currentMesh->maxRugorsity = maxRugorsity;

				if (ImGui::Checkbox("Show Rugosity", &currentMesh->showRugosity))
				{
					if (currentMesh->showRugosity)
					{
						currentMesh->colorMode = 5;
					}
					else
					{
						currentMesh->colorMode = 0;
					}
				}

				ImGui::Text("Selection mode:");
				if (ImGui::RadioButton("None", &RugositySelectionMode, 0))
				{
					currentMesh->TriangleSelected.clear();
					LINE_RENDERER.clearAll();
					LINE_RENDERER.SyncWithGPU();
				}

				if (ImGui::RadioButton("Triangles", &RugositySelectionMode, 1))
				{
					currentMesh->TriangleSelected.clear();
					LINE_RENDERER.clearAll();
					LINE_RENDERER.SyncWithGPU();
				}

				if (ImGui::RadioButton("Area", &RugositySelectionMode, 2))
				{
					currentMesh->TriangleSelected.clear();
					LINE_RENDERER.clearAll();
					LINE_RENDERER.SyncWithGPU();
				}

				if (RugositySelectionMode == 2)
				{
					ImGui::Text("Radius of area to measure: ");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(128);
					ImGui::DragFloat("##AreaToMeasureRugosity", &AreaToMeasureRugosity, 0.01f);
					if (AreaToMeasureRugosity < 0.1f)
						AreaToMeasureRugosity = 0.1f;
				}

				if (currentMesh->TriangleSelected.size() == 1)
				{
					ImGui::Separator();
					ImGui::Text("Selected triangle information :");

					std::string Text = "Triangle rugosity : ";
					if (!currentMesh->TrianglesRugosity.empty())
					{
						Text += std::to_string(currentMesh->TrianglesRugosity[currentMesh->TriangleSelected[0]]);
					}
					else
					{
						Text += "No information.";
					}
					ImGui::Text(Text.c_str());
				}
				else if (currentMesh->TriangleSelected.size() > 1)
				{
					std::string Text = "Area rugosity : ";

					if (!currentMesh->TrianglesRugosity.empty())
					{
						float TotalRugosity = 0.0f;
						for (size_t i = 0; i < currentMesh->TriangleSelected.size(); i++)
						{
							TotalRugosity += currentMesh->TrianglesRugosity[currentMesh->TriangleSelected[i]];
						}

						TotalRugosity /= currentMesh->TriangleSelected.size();
						Text += std::to_string(TotalRugosity);
					}
					else
					{
						Text += "No information.";
					}

					ImGui::Text(Text.c_str());
				}
			}

			ImGui::SetNextWindowSize(ImVec2(300, 50));
			if (ImGui::BeginPopupModal("Calculating...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				float persentFinished = float(RUGOSITY_MANAGER.newSDFSeen) / float(RUGOSITY_MANAGER.SmoothingFactor);
				std::string ProgressText = "Progress: " + std::to_string(persentFinished * 100.0f);
				ProgressText += " %";
				ImGui::SetCursorPosX(90);
				ImGui::Text(ProgressText.c_str());

				if (!THREAD_POOL.IsAnyThreadHaveActiveJob())
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
	}

	RenderLegend();
}

std::string UIManager::CameraPositionToStr()
{
	glm::vec3 CameraPosition = currentCamera->getPosition();
	return "( X:" + std::to_string(CameraPosition.x) + " Y:" + std::to_string(CameraPosition.y) + " Z:" + std::to_string(CameraPosition.z) + " )";
}

void UIManager::StrToCameraPosition(std::string text)
{
	size_t StartPosition = text.find("( X:");
	if (StartPosition == std::string::npos)
		return;

	size_t EndPosition = text.find(" Y:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("( X:") < 0 ||
		StartPosition + strlen("( X:") + EndPosition - (StartPosition + strlen("( X:")) >= text.size())
		return;

	std::string temp = text.substr(StartPosition + strlen("( X:"), EndPosition - (StartPosition + strlen("( X:")));

	if (temp.empty())
		return;

	float X = atof(temp.c_str());

	StartPosition = text.find("Y:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = text.find(" Z:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Y:") < 0 ||
		StartPosition + strlen("Y:") + EndPosition - (StartPosition + strlen("Y:")) >= text.size())
		return;

	temp = text.substr(StartPosition + strlen("Y:"), EndPosition - (StartPosition + strlen("Y:")));

	if (temp.empty())
		return;

	float Y = atof(temp.c_str());

	StartPosition = text.find("Z:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = text.find(" )");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Z:") < 0 ||
		StartPosition + strlen("Z:") + EndPosition - (StartPosition + strlen("Z:")) >= text.size())
		return;

	temp = text.substr(StartPosition + strlen("Z:"), EndPosition - (StartPosition + strlen("Z:")));

	if (temp.empty())
		return;

	float Z = atof(temp.c_str());

	currentCamera->setPosition(glm::vec3(X, Y, Z));
}

std::string UIManager::CameraRotationToStr()
{
	glm::vec3 CameraRotation = glm::vec3(currentCamera->getYaw(), currentCamera->getPitch(), currentCamera->getRoll());
	return "( X:" + std::to_string(CameraRotation.x) + " Y:" + std::to_string(CameraRotation.y) + " Z:" + std::to_string(CameraRotation.z) + " )";
}

void UIManager::StrToCameraRotation(std::string text)
{
	size_t StartPosition = text.find("( X:");
	if (StartPosition == std::string::npos)
		return;

	size_t EndPosition = text.find(" Y:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("( X:") < 0 ||
		StartPosition + strlen("( X:") + EndPosition - (StartPosition + strlen("( X:")) >= text.size())
		return;

	std::string temp = text.substr(StartPosition + strlen("( X:"), EndPosition - (StartPosition + strlen("( X:")));

	if (temp.empty())
		return;

	float X = atof(temp.c_str());

	StartPosition = text.find("Y:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = text.find(" Z:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Y:") < 0 ||
		StartPosition + strlen("Y:") + EndPosition - (StartPosition + strlen("Y:")) >= text.size())
		return;

	temp = text.substr(StartPosition + strlen("Y:"), EndPosition - (StartPosition + strlen("Y:")));

	if (temp.empty())
		return;

	float Y = atof(temp.c_str());

	StartPosition = text.find("Z:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = text.find(" )");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Z:") < 0 ||
		StartPosition + strlen("Z:") + EndPosition - (StartPosition + strlen("Z:")) >= text.size())
		return;

	temp = text.substr(StartPosition + strlen("Z:"), EndPosition - (StartPosition + strlen("Z:")));

	if (temp.empty())
		return;

	float Z = atof(temp.c_str());

	currentCamera->setYaw(X);
	currentCamera->setPitch(Y);
	currentCamera->setRoll(Z);
}

void UIManager::updateCurrentMesh(FEMesh* NewMesh)
{
	currentMesh = NewMesh;

	delete RUGOSITY_MANAGER.currentSDF;
	RUGOSITY_MANAGER.currentSDF = nullptr;

	LINE_RENDERER.clearAll();
	LINE_RENDERER.SyncWithGPU();
}

float UIManager::GetAreaToMeasureRugosity()
{
	return AreaToMeasureRugosity;
}

void UIManager::SetAreaToMeasureRugosity(float NewValue)
{
	AreaToMeasureRugosity = NewValue;
}

int UIManager::GetRugositySelectionMode()
{
	return RugositySelectionMode;
}

void UIManager::SetRugositySelectionMode(int NewValue)
{
	RugositySelectionMode = NewValue;
}

void UIManager::RenderLegend()
{
	static float turbo_srgb_floats[256][3] = { {0.18995,0.07176,0.23217},{0.19483,0.08339,0.26149},{0.19956,0.09498,0.29024},{0.20415,0.10652,0.31844},{0.20860,0.11802,0.34607},{0.21291,0.12947,0.37314},{0.21708,0.14087,0.39964},{0.22111,0.15223,0.42558},{0.22500,0.16354,0.45096},{0.22875,0.17481,0.47578},{0.23236,0.18603,0.50004},{0.23582,0.19720,0.52373},{0.23915,0.20833,0.54686},{0.24234,0.21941,0.56942},{0.24539,0.23044,0.59142},{0.24830,0.24143,0.61286},{0.25107,0.25237,0.63374},{0.25369,0.26327,0.65406},{0.25618,0.27412,0.67381},{0.25853,0.28492,0.69300},{0.26074,0.29568,0.71162},{0.26280,0.30639,0.72968},{0.26473,0.31706,0.74718},{0.26652,0.32768,0.76412},{0.26816,0.33825,0.78050},{0.26967,0.34878,0.79631},{0.27103,0.35926,0.81156},{0.27226,0.36970,0.82624},{0.27334,0.38008,0.84037},{0.27429,0.39043,0.85393},{0.27509,0.40072,0.86692},{0.27576,0.41097,0.87936},{0.27628,0.42118,0.89123},{0.27667,0.43134,0.90254},{0.27691,0.44145,0.91328},{0.27701,0.45152,0.92347},{0.27698,0.46153,0.93309},{0.27680,0.47151,0.94214},{0.27648,0.48144,0.95064},{0.27603,0.49132,0.95857},{0.27543,0.50115,0.96594},{0.27469,0.51094,0.97275},{0.27381,0.52069,0.97899},{0.27273,0.53040,0.98461},{0.27106,0.54015,0.98930},{0.26878,0.54995,0.99303},{0.26592,0.55979,0.99583},{0.26252,0.56967,0.99773},{0.25862,0.57958,0.99876},{0.25425,0.58950,0.99896},{0.24946,0.59943,0.99835},{0.24427,0.60937,0.99697},{0.23874,0.61931,0.99485},{0.23288,0.62923,0.99202},{0.22676,0.63913,0.98851},{0.22039,0.64901,0.98436},{0.21382,0.65886,0.97959},{0.20708,0.66866,0.97423},{0.20021,0.67842,0.96833},{0.19326,0.68812,0.96190},{0.18625,0.69775,0.95498},{0.17923,0.70732,0.94761},{0.17223,0.71680,0.93981},{0.16529,0.72620,0.93161},{0.15844,0.73551,0.92305},{0.15173,0.74472,0.91416},{0.14519,0.75381,0.90496},{0.13886,0.76279,0.89550},{0.13278,0.77165,0.88580},{0.12698,0.78037,0.87590},{0.12151,0.78896,0.86581},{0.11639,0.79740,0.85559},{0.11167,0.80569,0.84525},{0.10738,0.81381,0.83484},{0.10357,0.82177,0.82437},{0.10026,0.82955,0.81389},{0.09750,0.83714,0.80342},{0.09532,0.84455,0.79299},{0.09377,0.85175,0.78264},{0.09287,0.85875,0.77240},{0.09267,0.86554,0.76230},{0.09320,0.87211,0.75237},{0.09451,0.87844,0.74265},{0.09662,0.88454,0.73316},{0.09958,0.89040,0.72393},{0.10342,0.89600,0.71500},{0.10815,0.90142,0.70599},{0.11374,0.90673,0.69651},{0.12014,0.91193,0.68660},{0.12733,0.91701,0.67627},{0.13526,0.92197,0.66556},{0.14391,0.92680,0.65448},{0.15323,0.93151,0.64308},{0.16319,0.93609,0.63137},{0.17377,0.94053,0.61938},{0.18491,0.94484,0.60713},{0.19659,0.94901,0.59466},{0.20877,0.95304,0.58199},{0.22142,0.95692,0.56914},{0.23449,0.96065,0.55614},{0.24797,0.96423,0.54303},{0.26180,0.96765,0.52981},{0.27597,0.97092,0.51653},{0.29042,0.97403,0.50321},{0.30513,0.97697,0.48987},{0.32006,0.97974,0.47654},{0.33517,0.98234,0.46325},{0.35043,0.98477,0.45002},{0.36581,0.98702,0.43688},{0.38127,0.98909,0.42386},{0.39678,0.99098,0.41098},{0.41229,0.99268,0.39826},{0.42778,0.99419,0.38575},{0.44321,0.99551,0.37345},{0.45854,0.99663,0.36140},{0.47375,0.99755,0.34963},{0.48879,0.99828,0.33816},{0.50362,0.99879,0.32701},{0.51822,0.99910,0.31622},{0.53255,0.99919,0.30581},{0.54658,0.99907,0.29581},{0.56026,0.99873,0.28623},{0.57357,0.99817,0.27712},{0.58646,0.99739,0.26849},{0.59891,0.99638,0.26038},{0.61088,0.99514,0.25280},{0.62233,0.99366,0.24579},{0.63323,0.99195,0.23937},{0.64362,0.98999,0.23356},{0.65394,0.98775,0.22835},{0.66428,0.98524,0.22370},{0.67462,0.98246,0.21960},{0.68494,0.97941,0.21602},{0.69525,0.97610,0.21294},{0.70553,0.97255,0.21032},{0.71577,0.96875,0.20815},{0.72596,0.96470,0.20640},{0.73610,0.96043,0.20504},{0.74617,0.95593,0.20406},{0.75617,0.95121,0.20343},{0.76608,0.94627,0.20311},{0.77591,0.94113,0.20310},{0.78563,0.93579,0.20336},{0.79524,0.93025,0.20386},{0.80473,0.92452,0.20459},{0.81410,0.91861,0.20552},{0.82333,0.91253,0.20663},{0.83241,0.90627,0.20788},{0.84133,0.89986,0.20926},{0.85010,0.89328,0.21074},{0.85868,0.88655,0.21230},{0.86709,0.87968,0.21391},{0.87530,0.87267,0.21555},{0.88331,0.86553,0.21719},{0.89112,0.85826,0.21880},{0.89870,0.85087,0.22038},{0.90605,0.84337,0.22188},{0.91317,0.83576,0.22328},{0.92004,0.82806,0.22456},{0.92666,0.82025,0.22570},{0.93301,0.81236,0.22667},{0.93909,0.80439,0.22744},{0.94489,0.79634,0.22800},{0.95039,0.78823,0.22831},{0.95560,0.78005,0.22836},{0.96049,0.77181,0.22811},{0.96507,0.76352,0.22754},{0.96931,0.75519,0.22663},{0.97323,0.74682,0.22536},{0.97679,0.73842,0.22369},{0.98000,0.73000,0.22161},{0.98289,0.72140,0.21918},{0.98549,0.71250,0.21650},{0.98781,0.70330,0.21358},{0.98986,0.69382,0.21043},{0.99163,0.68408,0.20706},{0.99314,0.67408,0.20348},{0.99438,0.66386,0.19971},{0.99535,0.65341,0.19577},{0.99607,0.64277,0.19165},{0.99654,0.63193,0.18738},{0.99675,0.62093,0.18297},{0.99672,0.60977,0.17842},{0.99644,0.59846,0.17376},{0.99593,0.58703,0.16899},{0.99517,0.57549,0.16412},{0.99419,0.56386,0.15918},{0.99297,0.55214,0.15417},{0.99153,0.54036,0.14910},{0.98987,0.52854,0.14398},{0.98799,0.51667,0.13883},{0.98590,0.50479,0.13367},{0.98360,0.49291,0.12849},{0.98108,0.48104,0.12332},{0.97837,0.46920,0.11817},{0.97545,0.45740,0.11305},{0.97234,0.44565,0.10797},{0.96904,0.43399,0.10294},{0.96555,0.42241,0.09798},{0.96187,0.41093,0.09310},{0.95801,0.39958,0.08831},{0.95398,0.38836,0.08362},{0.94977,0.37729,0.07905},{0.94538,0.36638,0.07461},{0.94084,0.35566,0.07031},{0.93612,0.34513,0.06616},{0.93125,0.33482,0.06218},{0.92623,0.32473,0.05837},{0.92105,0.31489,0.05475},{0.91572,0.30530,0.05134},{0.91024,0.29599,0.04814},{0.90463,0.28696,0.04516},{0.89888,0.27824,0.04243},{0.89298,0.26981,0.03993},{0.88691,0.26152,0.03753},{0.88066,0.25334,0.03521},{0.87422,0.24526,0.03297},{0.86760,0.23730,0.03082},{0.86079,0.22945,0.02875},{0.85380,0.22170,0.02677},{0.84662,0.21407,0.02487},{0.83926,0.20654,0.02305},{0.83172,0.19912,0.02131},{0.82399,0.19182,0.01966},{0.81608,0.18462,0.01809},{0.80799,0.17753,0.01660},{0.79971,0.17055,0.01520},{0.79125,0.16368,0.01387},{0.78260,0.15693,0.01264},{0.77377,0.15028,0.01148},{0.76476,0.14374,0.01041},{0.75556,0.13731,0.00942},{0.74617,0.13098,0.00851},{0.73661,0.12477,0.00769},{0.72686,0.11867,0.00695},{0.71692,0.11268,0.00629},{0.70680,0.10680,0.00571},{0.69650,0.10102,0.00522},{0.68602,0.09536,0.00481},{0.67535,0.08980,0.00449},{0.66449,0.08436,0.00424},{0.65345,0.07902,0.00408},{0.64223,0.07380,0.00401},{0.63082,0.06868,0.00401},{0.61923,0.06367,0.00410},{0.60746,0.05878,0.00427},{0.59550,0.05399,0.00453},{0.58336,0.04931,0.00486},{0.57103,0.04474,0.00529},{0.55852,0.04028,0.00579},{0.54583,0.03593,0.00638},{0.53295,0.03169,0.00705},{0.51989,0.02756,0.00780},{0.50664,0.02354,0.00863},{0.49321,0.01963,0.00955},{0.47960,0.01583,0.01055} };

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(100, 400));
	ImGui::Begin("##RugosityLegend", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

	int WindowX = ImGui::GetCurrentWindow()->Pos.x;
	int WindowY = ImGui::GetCurrentWindow()->Pos.y;

	ImColor SomeColor = ImColor(255, 255, 255, 255);

	ImVec2 LegendSize = ImVec2(20, 300);
	ImVec2 LegendPosition = ImVec2(15, 15);

	for (size_t i = 0; i < LegendSize.y; i++)
	{
		float factor = i / float(LegendSize.y);
		int index = int(255 * factor);

		ImColor SomeColor = ImColor(int(turbo_srgb_floats[index][0] * 255),
									int(turbo_srgb_floats[index][1] * 255),
									int(turbo_srgb_floats[index][2] * 255), 255);

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(WindowX, WindowY + LegendSize.y - i + 1) + LegendPosition,
												  ImVec2(WindowX + LegendSize.x, WindowY + LegendSize.y - i) + LegendPosition,
												  SomeColor);
	}

	ScrollerTest.SetColor(ImColor(255, 155, 155, 255));
	ScrollerTest.SetPosition(ImVec2(10.0f, 10.0f));
	ScrollerTest.Render();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

FEArrowScroller::FEArrowScroller(const bool Horizontal)
{
	bHorizontal = Horizontal;

	bSelected = false;
	bMouseHover = false;

	bWindowFlagWasAdded = false;
	OriginalWindowFlags = 0;

	LastFrameDelta = 0;
	Size = 20.0f;

	Color = ImColor(10, 10, 40, 255);
	SelectedColor = ImColor(115, 115, 255, 255);

	AvailableRange = ImVec2(-FLT_MAX, FLT_MAX);
}

ImVec2 FEArrowScroller::GetPosition() const
{
	return Position;
}

void FEArrowScroller::SetPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;

	if (bHorizontal)
	{
		Area.left = static_cast<LONG>(Position.x - Size / 2.0f);
		Area.right = static_cast<LONG>(Position.x + Size / 2.0f);
		Area.top = static_cast<LONG>(Position.y - Size);
		Area.bottom = static_cast<LONG>(Position.y);
	}
	else
	{
		Area.left = static_cast<LONG>(Position.x - Size);
		Area.right = static_cast<LONG>(Position.x);
		Area.top = static_cast<LONG>(Position.y - Size / 2.0f);
		Area.bottom = static_cast<LONG>(Position.y + Size / 2.0f);
	}
}

bool FEArrowScroller::IsSelected() const
{
	return bSelected;
}

void FEArrowScroller::SetSelected(const bool NewValue)
{
	bSelected = NewValue;
}

void FEArrowScroller::Render()
{
	const float MouseXWindows = ImGui::GetIO().MousePos.x - ImGui::GetCurrentWindow()->Pos.x;
	const float MouseYWindows = ImGui::GetIO().MousePos.y - ImGui::GetCurrentWindow()->Pos.y;

	bMouseHover = false;
	if (MouseXWindows >= Area.left && MouseXWindows < Area.right &&
		MouseYWindows >= Area.top && MouseYWindows < Area.bottom)
	{
		bMouseHover = true;
	}

	if (!bMouseHover && bWindowFlagWasAdded)
	{
		bWindowFlagWasAdded = false;
		ImGui::GetCurrentWindow()->Flags = OriginalWindowFlags;
	}

	if (!(ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_NoMove) && bMouseHover)
	{
		bWindowFlagWasAdded = true;
		OriginalWindowFlags = ImGui::GetCurrentWindow()->Flags;
		ImGui::GetCurrentWindow()->Flags |= ImGuiWindowFlags_NoMove;
	}

	if (ImGui::GetIO().MouseClicked[0])
	{
		bMouseHover ? SetSelected(true) : SetSelected(false);
	}

	if (ImGui::GetIO().MouseReleased[0])
		SetSelected(false);

	LastFrameDelta = 0;
	if (IsSelected())
	{
		LastFrameDelta = bHorizontal ? MouseXWindows - LastFrameMouseX : MouseYWindows - LastFrameMouseY;

		if (bHorizontal)
		{
			if (GetPosition().x + LastFrameDelta <= AvailableRange.y && GetPosition().x + LastFrameDelta >= AvailableRange.x)
			{
				SetPosition(ImVec2(GetPosition().x + LastFrameDelta, GetPosition().y));
			}
		}
		else
		{
			if (GetPosition().y + LastFrameDelta <= AvailableRange.y && GetPosition().y + LastFrameDelta >= AvailableRange.x)
			{
				SetPosition(ImVec2(GetPosition().x, GetPosition().y + LastFrameDelta));
			}
		}
	}

	LastFrameMouseX = MouseXWindows;
	LastFrameMouseY = MouseYWindows;

	ImVec2 P1;
	ImVec2 P2;
	ImVec2 P3;

	if (bHorizontal)
	{
		P1 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.left,
			ImGui::GetCurrentWindow()->Pos.y + Area.top);
		P2 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.right,
			ImGui::GetCurrentWindow()->Pos.y + Area.top);
		P3 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.left + (Area.right - Area.left) / 2.0f,
			ImGui::GetCurrentWindow()->Pos.y + Area.bottom);
	}
	else
	{
		P1 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.left,
			ImGui::GetCurrentWindow()->Pos.y + Area.top);
		P2 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.left,
			ImGui::GetCurrentWindow()->Pos.y + Area.bottom);
		P3 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.right,
			ImGui::GetCurrentWindow()->Pos.y + Area.top + (Area.right - Area.left) / 2.0f);
	}

	if (IsSelected())
	{
		ImGui::GetWindowDrawList()->AddTriangleFilled(P1, P2, P3, SelectedColor);
	}
	else
	{
		ImGui::GetWindowDrawList()->AddTriangleFilled(P1, P2, P3, Color);
	}
}

float FEArrowScroller::GetLastFrameDelta() const
{
	return LastFrameDelta;
}

float FEArrowScroller::GetSize() const
{
	return Size;
}

void FEArrowScroller::SetSize(const float NewValue)
{
	if (NewValue > 1.0f)
		Size = NewValue;
}

ImColor FEArrowScroller::GetColor() const
{
	return Color;
}

void FEArrowScroller::SetColor(const ImColor NewValue)
{
	Color = NewValue;
}

ImColor FEArrowScroller::GetSelectedColor() const
{
	return SelectedColor;
}

void FEArrowScroller::SetSelectedColor(const ImColor NewValue)
{
	SelectedColor = NewValue;
}

void FEArrowScroller::SetAvailableRange(const ImVec2 NewValue)
{
	AvailableRange = NewValue;
}

void FEArrowScroller::LiftRangeRestrictions()
{
	AvailableRange = ImVec2(-FLT_MAX, FLT_MAX);
}

void FEArrowScroller::SetOrientation(const bool IsHorisontal)
{
	bHorizontal = IsHorisontal;
}