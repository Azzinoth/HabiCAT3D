#include "UIManager.h"
using namespace FocalEngine;
UIManager* UIManager::Instance = nullptr;
UIManager::UIManager() {}
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

	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	if (ImGui::Button("Paste##Rotation"))
	{
		StrToCameraRotation(APPLICATION.GetClipboardText());
	}

	currentCamera->setYaw(cameraRotation[0]);
	currentCamera->setPitch(cameraRotation[1]);
	currentCamera->setRoll(cameraRotation[2]);

	float cameraSpeed = currentCamera->getMovementSpeed();
	ImGui::Text("Camera speed: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Camera_speed", &cameraSpeed, 0.01f, 0.01f, 100.0f);
	currentCamera->setMovementSpeed(cameraSpeed);
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
	showCameraTransform();

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
			if (ImGui::RadioButton(("Small (Grid size - " + std::to_string(RUGOSITY_MANAGER.LowestResolution)/*RUGOSITY_MANAGER.ResolutionsAvailableToCurrentMeshList[0]*/ + ")").c_str(), &SmallScaleFeatures, 0))
			{

			}

			if (ImGui::RadioButton(("Large (Grid size - " + std::to_string(RUGOSITY_MANAGER.HigestResolution)/*RUGOSITY_MANAGER.ResolutionsAvailableToCurrentMeshList.back()*/ + ")").c_str(), &SmallScaleFeatures, 1))
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