#include "UIInspector.h"
using namespace FocalEngine;
#include <shellapi.h>

UIInspector::UIInspector()
{
	LAYER_MANAGER.AddActiveLayerChangedCallback(OnLayerChange);
	COLMAP_DATA_MANAGER.AddOnSelectedImageChangedCallback(OnSelectedImageChangedCallback);
}

UIInspector::~UIInspector() {}

void UIInspector::Render(bool bScreenshotMode)
{
	// Center the window on the screen on first use.
	ImVec2 Center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(Center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Inspector"))
	{
		if (ImGui::BeginTabBar("##InspectorTabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Selected Object"))
			{
				RenderSelectedObjectTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Layer"))
			{
				RenderLayerTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}

void UIInspector::OnSelectedImageChangedCallback(COLMAPProject* Project, int ImageID)
{
	FEEntity* ImageEntity = Project->GetImagesInstancedEntity();
	if (ImageEntity == nullptr)
		return;

	FEScene* Scene = MAIN_SCENE_MANAGER.GetMainScene();
	FENaiveSceneGraphNode* ImageInstancedSceneNode = Scene->SceneGraph.GetNodeByEntityID(ImageEntity->GetObjectID());

	OBJECT_VIEWER_WINDOW.SceneGraphUI->SetNodeSelected(ImageInstancedSceneNode, true);
}

void UIInspector::RenderSelectedObjectTab()
{
	int TreeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;

	FEEntity* SelectedEntity = OBJECT_VIEWER_WINDOW.GetSelectedEntity();
	if (SelectedEntity == nullptr)
	{
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize("No object selected.").x / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize("No object selected.").y / 2.0f);

		ImGui::Text("No object selected.");

		return;
	}

	AnalysisObject* SelectedAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(SelectedEntity->GetObjectID());
	COLMAPProject* CurrentCOLMAPProject = nullptr;
	if (SelectedAnalysisObject != nullptr)
	{
		CurrentCOLMAPProject = COLMAP_DATA_MANAGER.GetProjectByAnalysisObjectID(SelectedAnalysisObject->GetID());
	}
	else
	{
		CurrentCOLMAPProject = COLMAP_DATA_MANAGER.GetProjectByEntityID(SelectedEntity->GetObjectID());
	}

	std::string SelectedObjectType = "Unknown";
	if (SelectedAnalysisObject != nullptr)
	{
		if (SelectedAnalysisObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			SelectedObjectType = "3D Model";
		}
		else if (SelectedAnalysisObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
		{
			SelectedObjectType = "Point Cloud";
		}
	}
	else
	{
		SelectedObjectType = "Photogrammetry";

		COLMAPImage* SelectedImage = CurrentCOLMAPProject->GetSelectedImage();
		if (SelectedImage != nullptr)
		{
			SelectedObjectType += "(Image)";
		}
	}

	ImGui::Text(("Type: " + SelectedObjectType).c_str());

	if (SelectedAnalysisObject != nullptr)
	{
		if (SelectedAnalysisObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			FEMesh* ActiveMesh = static_cast<FEMesh*>(SelectedAnalysisObject->GetEngineResource());
			if (ActiveMesh == nullptr)
				return;

			ImGui::Text("Triangle count: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(ActiveMesh->GetVertexCount() / 3).c_str());
		}
		else if (SelectedAnalysisObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
		{
			FEPointCloud* ActivePointCloud = static_cast<FEPointCloud*>(SelectedAnalysisObject->GetEngineResource());
			if (ActivePointCloud == nullptr)
				return;

			ImGui::Text("Point count: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(ActivePointCloud->GetPointCount()).c_str());
		}

		if (CurrentCOLMAPProject == nullptr)
		{
			if (ImGui::Button("Load Photogrammetry..."))
			{
				std::string LocalPhotogrammetryFolder;
				FILE_SYSTEM.ShowFolderOpenDialog(LocalPhotogrammetryFolder);
				LOAD_PHOTOGRAMMETRY_WINDOW.Show(LocalPhotogrammetryFolder, COLMAP_DATA_MANAGER.FindCOLMAPDataInFolder(LocalPhotogrammetryFolder));
			}
		}
	}
	else
	{
		if (CurrentCOLMAPProject != nullptr && CurrentCOLMAPProject->GetImageCount() > 0)
		{
			AnalysisObject* COLMAPProjectAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(CurrentCOLMAPProject->GetParentAnalysisObjectID());
			if (COLMAPProjectAnalysisObject != nullptr)
			{
				MeshAnalysisData* CurrentMeshAnalysisData = COLMAPProjectAnalysisObject->GetMeshAnalysisData();
				if (CurrentMeshAnalysisData != nullptr)
				{
					ImGui::Separator();
					if (ImGui::Button("Select image that should contain the selected triangle(s)"))
					{
						FEAABB AABBToCheck;
						if (CurrentMeshAnalysisData->TriangleSelected.size() == 1)
						{
							AABBToCheck = FEAABB(CurrentMeshAnalysisData->Triangles[CurrentMeshAnalysisData->TriangleSelected[0]]);
						}
						else if (CurrentMeshAnalysisData->TriangleSelected.size() > 1)
						{
							std::vector<glm::vec3> PointsToInclude;
							for (size_t i = 0; i < CurrentMeshAnalysisData->TriangleSelected.size(); i++)
							{
								std::vector<glm::dvec3> CurrentTriangle = CurrentMeshAnalysisData->Triangles[CurrentMeshAnalysisData->TriangleSelected[i]];
								PointsToInclude.insert(PointsToInclude.end(), CurrentTriangle.begin(), CurrentTriangle.end());
							}

							AABBToCheck = FEAABB(PointsToInclude);
						}

						// AABB is in model space, so we need to transform it to world space
						AABBToCheck = AABBToCheck.Transform(COLMAPProjectAnalysisObject->GetEntity()->GetComponent<FETransformComponent>().GetWorldMatrix());
						CurrentCOLMAPProject->HighlightImagesThatSeeAABB(AABBToCheck);
					}

					ImGui::Separator();
				}
			}

			ImGui::Text("Number of images read: %d", static_cast<int>(CurrentCOLMAPProject->GetImageCount()));
			ImGui::Text("Choose image to see info:");

			static int ImageIDToSelect = 0;
			ImGui::InputInt("Image ID", &ImageIDToSelect);
			if (ImGui::Button("Select image by ID"))
				CurrentCOLMAPProject->SelectImageByID(ImageIDToSelect);

			COLMAPImage* SelectedImage = CurrentCOLMAPProject->GetSelectedImage();
			if (SelectedImage != nullptr)
			{
				if (ImGui::TreeNodeEx("Selected Photogrammetry Image", TreeFlags))
				{
					ImGui::Text("Image name: %s", SelectedImage->GetName().c_str());
					ImGui::Text("Image ID: %d", SelectedImage->GetID());
					ImGui::Text("Camera ID: %d", SelectedImage->GetCameraID());
					glm::dquat Rotation = SelectedImage->GetOriginalRotation();
					ImGui::Text("Camera Original Rotation (quat): W: %.6f X: %.6f Y: %.6f Z: %.6f", Rotation.w, Rotation.x, Rotation.y, Rotation.z);
					glm::dvec3 Translation = SelectedImage->GetOriginalTranslation();
					ImGui::Text("Camera Original Translation: X: %.3f Y: %.3f Z: %.3f", Translation.x, Translation.y, Translation.z);

					glm::vec3 ImageCenter = SelectedImage->GetPosition();
					ImGui::Text("Camera Center: X: %.3f Y: %.3f Z: %.3f", ImageCenter.x, ImageCenter.y, ImageCenter.z);

					COLMAPCamera* ImageCamera = CurrentCOLMAPProject->GetCameraForImage(SelectedImage->GetID());
					COLMAPPhysicalCamera* PhysicalCamera = ImageCamera->GetPhysicalCamera();
					FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
					if (ImageCamera != nullptr && CameraEntity != nullptr)
					{
						if (ImGui::Button("Render view from a current image camera"))
							CurrentCOLMAPProject->RenderViewFromImage(SelectedImage->GetID());

						if (ImGui::Button("Render view from a current image camera(Depth 8-bit)"))
							CurrentCOLMAPProject->RenderViewFromImage(SelectedImage->GetID(), true);

						if (ImGui::Button("Render view from a current image camera(Depth 16-bit)"))
							CurrentCOLMAPProject->RenderViewFromImage(SelectedImage->GetID(), true, FE_DEPTH_EXPORT_16BIT_PNG);

						std::string PhotoPath = CurrentCOLMAPProject->GetPathToPhotoByImageID(SelectedImage->GetID());
						if (!FILE_SYSTEM.DoesFileExist(PhotoPath))
							ImGui::BeginDisabled();

						ImGui::Text("Full photo path: ");
						ImGui::Text(PhotoPath.c_str());

						if (ImGui::Button("Show original Photo"))
						{
							ShellExecute(NULL, "open", PhotoPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
						}

						if (!FILE_SYSTEM.DoesFileExist(PhotoPath))
							ImGui::EndDisabled();
					}

					ImGui::TreePop();
				}
			}
			else
			{
				ImGui::Text("No image selected.");
			}
		}
	}

	if (DEVELOPER_MODE.IsOn())
	{
		FETransformComponent& TransformComponent = SelectedEntity->GetComponent<FETransformComponent>();
		if (ImGui::TreeNodeEx("Selected Object Transform", TreeFlags))
		{
			UI_CORE.ShowTransformConfiguration(SelectedEntity->GetObjectID(), &TransformComponent);

			ImGui::TreePop();
		}
	}
}

void UIInspector::OnLayerChange()
{
	UI_INSPECTOR.CurrentDistribution = glm::vec2(0.0f);
	strcpy_s(UI_INSPECTOR.CurrentDistributionEdit, "");
}

glm::dvec2 UIInspector::CalculateWeightDistributionAtValue(DataLayer* Layer, float Value)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return glm::dvec2(0.0);

	if (Layer == nullptr || Layer->ElementsToData.empty())
		return glm::dvec2(0.0);

	float WeightBelowOrEqual = 0.0;
	float WeightAbove = 0.0;
	switch (ActiveObject->GetType())
	{
		case DATA_SOURCE_TYPE::MESH:
		{
			MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData == nullptr || CurrentMeshAnalysisData->TrianglesArea.empty())
				return glm::dvec2(0.0);

			for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
			{
				if (Layer->ElementsToData[i] <= Value)
				{
					WeightBelowOrEqual += float(CurrentMeshAnalysisData->TrianglesArea[i]);
				}
				else
				{
					WeightAbove += float(CurrentMeshAnalysisData->TrianglesArea[i]);
				}
			}
			break;
		}
		case DATA_SOURCE_TYPE::POINT_CLOUD:
		{
			PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
			if (CurrentPointCloudAnalysisData == nullptr)
				return glm::dvec2(0.0);

			// For point clouds, each point has weight equal to 1.0.
			for (int i = 0; i < CurrentPointCloudAnalysisData->RawPointCloudData.size(); i++)
			{
				if (Layer->ElementsToData[i] <= Value)
				{
					WeightBelowOrEqual += 1.0;
				}
				else
				{
					WeightAbove += 1.0;
				}
			}
			break;
		}

		default:
			return glm::dvec2(0.0);
	}

	return glm::dvec2(WeightBelowOrEqual, WeightAbove);
}

void UIInspector::RenderLayerTab()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();

	std::string NoInfoText;
	if (ActiveObject == nullptr)
		NoInfoText = "No object loaded.";

	if (ActiveObject != nullptr && ActiveObject->GetLayerCount() == 0)
		NoInfoText = "Object have no layers.";

	if (ActiveObject != nullptr && !(ActiveObject->GetLayerCount() == 0) && LAYER_MANAGER.GetActiveLayerIndex() == -1)
		NoInfoText = "Layer is not selected.";

	if (!NoInfoText.empty())
	{
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize(NoInfoText.c_str()).x / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize(NoInfoText.c_str()).y / 2.0f);

		ImGui::Text(NoInfoText.c_str());
	}

	if (NoInfoText.empty())
	{
		int TreeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;
		DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
		if (ImGui::TreeNodeEx("General Info", TreeFlags))
		{
			ImGui::Text((std::string("ID: ") + ActiveLayer->GetID()).c_str());
			static char CurrentLayerCaption[1024];
			strcpy_s(CurrentLayerCaption, ActiveLayer->GetCaption().c_str());
			ImGui::Text("Caption: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(160);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
			if (ImGui::InputText("##LayerCaptionEdit", CurrentLayerCaption, IM_ARRAYSIZE(CurrentLayerCaption), ImGuiInputTextFlags_EnterReturnsTrue) ||
				ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##LayerCaptionEdit"))
			{
				ActiveLayer->SetCaption(CurrentLayerCaption);
			}

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.6f, 0.1f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.65f, 0.2f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.75f, 0.6f, 0.1f));
			ImGui::SameLine();
			if (ImGui::Button("Delete Layer"))
			{
				ActiveObject->RemoveLayer(ActiveLayer->GetID());

				ImGui::PopStyleColor(3);
				return;
			}
			ImGui::PopStyleColor(3);

			ImGui::Text("Mean:");
			ImGui::SameLine();
			std::string MeanText = "No data.";
			if (ActiveLayer->GetMean() != -FLT_MAX)
				MeanText = std::to_string(ActiveLayer->GetMean());
			ImGui::Text(MeanText.c_str());

			ImGui::Text("Median:");
			ImGui::SameLine();
			std::string MedianText = "No data.";
			if (ActiveLayer->GetMedian() != -FLT_MAX)
				MedianText = std::to_string(ActiveLayer->GetMedian());
			ImGui::Text(MedianText.c_str());

			ImGui::Text("Notes:");
			static char CurrentLayerUserNotes[10000];
			strcpy_s(CurrentLayerUserNotes, ActiveLayer->GetNote().c_str());
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
			if (ImGui::InputTextMultiline("##Notes", CurrentLayerUserNotes, IM_ARRAYSIZE(CurrentLayerUserNotes)))
				ActiveLayer->SetNote(CurrentLayerUserNotes);

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Debug Info", TreeFlags))
		{
			ImGui::Text("Debug Info:");
			static char CurrentLayerDebugInfo[10000];
			std::string DebugInfo;
			if (ActiveLayer->DebugInfo != nullptr)
				DebugInfo = ActiveLayer->DebugInfo->ToString();
			strcpy_s(CurrentLayerDebugInfo, DebugInfo.c_str());
			ImGui::BeginDisabled();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
			ImGui::InputTextMultiline("##DebugInfo", CurrentLayerDebugInfo, IM_ARRAYSIZE(CurrentLayerDebugInfo));
			ImGui::EndDisabled();

			ImGui::TreePop();
		}

		LayerInterpolationData* InterpolationData = ActiveLayer->GetInterpolationData();
		if (ActiveLayer->GetType() == LAYER_TYPE::INTERPOLATION && InterpolationData != nullptr)
		{
			if (ImGui::TreeNodeEx("Layers interpolation", TreeFlags))
			{
				ImGui::Text("Layers blend factor:");
				float GlobalFactor = InterpolationData->GetInterpolationFactor();
				ImGui::DragFloat("##InterpolationFactor", &GlobalFactor, 0.001f, 0.0f, 1.0f, "%.3f");
				InterpolationData->SetInterpolationFactor(GlobalFactor);

				bool bUseMinMaxInterpolation = InterpolationData->IsMinMaxInterpolationEnabled();
				ImGui::Checkbox("Use Min/Max interpolation", &bUseMinMaxInterpolation);
				InterpolationData->SetMinMaxInterpolationEnabled(bUseMinMaxInterpolation);

				ImGui::TreePop();
			}
		}

		if (ImGui::TreeNodeEx("Distribution calculation", TreeFlags))
		{
			static float LastDistributionValue = 0.0f;

			ImGui::SetNextItemWidth(62);
			if (ImGui::InputText("##DistributionEdit", CurrentDistributionEdit, IM_ARRAYSIZE(CurrentDistributionEdit), ImGuiInputTextFlags_EnterReturnsTrue) ||
				ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##DistributionEdit"))
			{

			}

			ImGui::SameLine();
			if (ImGui::Button("Calculate Distribution", ImVec2(167, 19)))
			{
				float NewValue = float(atof(CurrentDistributionEdit));
				LastDistributionValue = NewValue;
				CurrentDistribution = CalculateWeightDistributionAtValue(ActiveLayer, NewValue);
			}

			AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
			if (ActiveObject == nullptr)
				return;

			double TotalWeight = 0.0;
			std::string WeightUnit;
			if (CurrentDistribution != glm::vec2())
			{
				switch (ActiveObject->GetType())
				{
				case DATA_SOURCE_TYPE::MESH:
				{
					MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
					if (CurrentMeshAnalysisData == nullptr)
						return;

					TotalWeight = CurrentMeshAnalysisData->GetTotalArea();
					WeightUnit = "area";
					break;
				}

				case DATA_SOURCE_TYPE::POINT_CLOUD:
				{
					PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
					if (CurrentPointCloudAnalysisData == nullptr)
						return;

					TotalWeight = static_cast<double>(CurrentPointCloudAnalysisData->RawPointCloudData.size());
					WeightUnit = "points";
					break;
				}

				default:
					return;
				}

				if (TotalWeight > 0.0)
				{
					double PercentageBelowOrEqual = (CurrentDistribution.x / TotalWeight) * 100.0;
					double PercentageAbove = (CurrentDistribution.y / TotalWeight) * 100.0;

					ImGui::Text((WeightUnit + " below and at " + UI_CORE.TruncateAfterDot(std::to_string(LastDistributionValue)) + " value : " + std::to_string(PercentageBelowOrEqual) + " %%").c_str());
					ImGui::Text((WeightUnit + " with higher than " + UI_CORE.TruncateAfterDot(std::to_string(LastDistributionValue)) + " value : " + std::to_string(PercentageAbove) + " %%").c_str());
				}
			}

			ImGui::TreePop();
		}

		if (DEVELOPER_MODE.IsOn() && LAYER_MANAGER.GetActiveLayerIndex() != -1)
		{
			if (ImGui::TreeNodeEx("Layer Debug Info", TreeFlags))
			{
				DEVELOPER_MODE.ShowLayerDebugUI();
				ImGui::TreePop();
			}
		}
	}
}