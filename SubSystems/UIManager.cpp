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
	// ********* POSITION *********
	glm::vec3 cameraPosition = currentCamera->getPosition();

	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##X pos", &cameraPosition[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Y pos", &cameraPosition[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Z pos", &cameraPosition[2], 0.1f);

	currentCamera->setPosition(cameraPosition);

	// ********* ROTATION *********
	glm::vec3 cameraRotation = glm::vec3(currentCamera->getYaw(), currentCamera->getPitch(), currentCamera->getRoll());

	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##X rot", &cameraRotation[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Y rot", &cameraRotation[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Z rot", &cameraRotation[2], 0.1f);

	currentCamera->setYaw(cameraRotation[0]);
	currentCamera->setPitch(cameraRotation[1]);
	currentCamera->setRoll(cameraRotation[2]);

	float cameraSpeed = currentCamera->getMovementSpeed();
	ImGui::Text("Camera speed in m/s : ");
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

void UIManager::RenderMainWindow(FEMesh* currentMesh)
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

	showCameraTransform();

	if (currentMesh != nullptr)
	{
		ImGui::Checkbox("Wireframe", &wireframeMode);

		//if (currentMesh->minRugorsity != DBL_MAX)
		//	ImGui::Text(("minRugorsity: " + std::to_string(currentMesh->minRugorsity)).c_str());

		//if (currentMesh->maxRugorsity != -DBL_MAX)
		//	ImGui::Text(("maxRugorsity: " + std::to_string(currentMesh->maxRugorsity)).c_str());

		if (currentMesh->TriangleSelected != -1)
		{
			ImGui::Separator();
			ImGui::Text("Selected triangle information :");

			std::string Text = "Index : " + std::to_string(currentMesh->TriangleSelected);
			ImGui::Text(Text.c_str());

			Text = "First vertex : ";
			Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].x);
			Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].y);
			Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].z);
			ImGui::Text(Text.c_str());

			Text = "Second vertex : ";
			Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].x);
			Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].y);
			Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].z);
			ImGui::Text(Text.c_str());

			Text = "Third vertex : ";
			Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].x);
			Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].y);
			Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].z);
			ImGui::Text(Text.c_str());

			Text = "Segment ID : ";
			if (!currentMesh->originalTrianglesToSegments.empty())
			{
				Text += std::to_string(currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected]);
			}
			else
			{
				Text += "No information.";
			}
			ImGui::Text(Text.c_str());

			Text = "Segment normal : ";
			if (!currentMesh->originalTrianglesToSegments.empty() && !currentMesh->segmentsNormals.empty())
			{
				glm::vec3 normal = currentMesh->segmentsNormals[currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected]];

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

			ImGui::SameLine();
			ImGui::SetNextItemWidth(128);
			if (ImGui::BeginCombo("##ChooseSDFDimention", std::to_string(RUGOSITY_MANAGER.SDFDimention).c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < RUGOSITY_MANAGER.dimentionsList.size(); i++)
				{
					bool is_selected = (std::to_string(RUGOSITY_MANAGER.SDFDimention) == RUGOSITY_MANAGER.dimentionsList[i]);
					if (ImGui::Selectable(RUGOSITY_MANAGER.dimentionsList[i].c_str(), is_selected))
					{
						RUGOSITY_MANAGER.SDFDimention = atoi(RUGOSITY_MANAGER.dimentionsList[i].c_str());
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

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
}