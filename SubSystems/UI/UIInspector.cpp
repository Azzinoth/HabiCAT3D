#include "UIInspector.h"
using namespace FocalEngine;
#include <shellapi.h>

UIInspector::UIInspector()
{
	LAYER_MANAGER.AddActiveLayerChangedCallback(OnLayerChange);
	COLMAP_DATA_MANAGER.AddOnSelectedImageChangedCallback(OnSelectedImageChangedCallback);
	ANALYSIS_OBJECT_MANAGER.AddOnActiveObjectChangeCallback(OnActiveObjectChange);
	ANALYSIS_OBJECT_MANAGER.AddOnObjectLoadCallback(OnObjectLoad);

	APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(MouseButtonCallback);
	APPLICATION.GetMainWindow()->AddOnScrollCallback(MouseScrollCallback);
}

void UIInspector::MouseScrollCallback(double XOffset, double YOffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FENativeScriptComponent>())
		return;

	FENativeScriptComponent& NativeScriptComponent = CameraEntity->GetComponent<FENativeScriptComponent>();
	if (!NativeScriptComponent.IsInitialized())
		return;

	float CurrentDistance = 0.0f;
	if (!NativeScriptComponent.GetVariableValue<float>("DistanceToModel", CurrentDistance))
		return;

	float MouseWheelSensitivity = 1.0f;
	NativeScriptComponent.GetVariableValue<float>("MouseWheelSensitivity", MouseWheelSensitivity);

	float NewDistance = CurrentDistance + static_cast<float>(YOffset) * 2.0f * MouseWheelSensitivity;
	if (NewDistance < 0.1f)
		NewDistance = 0.1f;

	NativeScriptComponent.SetVariableValue("DistanceToModel", NewDistance);
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

			if (ImGui::BeginTabItem("Export"))
			{
				RenderExportTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void UIInspector::OnSelectedImageChangedCallback(COLMAPProject* Project, int ImageID)
{
	FEEntity* ImageEntity = Project->GetImagesInstancedEntity();
	if (ImageEntity == nullptr)
		return;

	FEScene* Scene = MAIN_SCENE_MANAGER.GetMainScene();
	FENaiveSceneGraphNode* ImageInstancedSceneNode = Scene->SceneGraph.GetNodeByEntityID(ImageEntity->GetObjectID());

	OBJECT_VIEWER_WINDOW.SetNodeSelected(ImageInstancedSceneNode, true);
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

	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByEntityID(SelectedEntity->GetObjectID());

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
		SelectedObjectType = "Unknown";

		std::string SelectedEntityName = SelectedEntity->GetName();
		if (SelectedEntityName.find("Photogrammetry") != std::string::npos)
		{
			SelectedObjectType = "Photogrammetry";
		}
		else if (SelectedEntityName.find("Annotation") != std::string::npos)
		{
			SelectedObjectType = "Annotation";
		}
		else if (SelectedEntityName.find("Images") != std::string::npos)
		{
			COLMAPImage* SelectedImage = CurrentCOLMAPProject->GetSelectedImage();
			if (SelectedImage != nullptr)
				SelectedObjectType = "Photogrammetry (Image)";
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

			MeshAnalysisData* CurrentMeshAnalysisData = SelectedAnalysisObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData != nullptr)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
				if (ImGui::TreeNodeEx("Geometry selection", TreeFlags))
				{
					ImGui::Text("Selection mode:");
					if (ImGui::RadioButton("None", &MeshSelectionMode, 0))
					{
						CurrentMeshAnalysisData->TriangleSelected.clear();
						CleanUpSelectionLinesComponent();
					}

					if (ImGui::RadioButton("Triangles", &MeshSelectionMode, 1))
					{
						CurrentMeshAnalysisData->TriangleSelected.clear();
						CleanUpSelectionLinesComponent();
					}

					if (ImGui::RadioButton("Area", &MeshSelectionMode, 2))
					{
						CurrentMeshAnalysisData->TriangleSelected.clear();
						CleanUpSelectionLinesComponent();
					}

					if (MeshSelectionMode == 2)
					{
						ImGui::Text("Radius of area to measure: ");
						ImGui::SetNextItemWidth(128);
						ImGui::DragFloat("##RadiusOfAreaToSelect", &RadiusOfAreaToSelect, 0.01f);
						if (RadiusOfAreaToSelect < 0.1f)
							RadiusOfAreaToSelect = 0.1f;

						//ImGui::Checkbox("Output selection data to file", &bOutputSelectionToFile);
					}

					DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
					if (CurrentMeshAnalysisData->TriangleSelected.size() == 1 && ActiveLayer != nullptr)
					{
						ImGui::Separator();
						ImGui::Text("Selected triangle information :");

						std::vector<DataLayer*> Layers = LAYER_MANAGER.GetAllLayersOfActiveObject();
						std::string Text = "Value per layer:\n";
						for (size_t i = 0; i < Layers.size(); i++)
						{
							std::string CurrentCaption = Layers[i]->GetCaption();
							float CurrentValue = 0.0f;
							if (Layers[i]->GetType() == LAYER_TYPE::INTERPOLATION)
							{
								CurrentValue = std::numeric_limits<float>::quiet_NaN();
							}
							else
							{
								CurrentValue = Layers[i]->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[0]];
							}

							Text += CurrentCaption + " : " + std::to_string(CurrentValue) + "\n";
						}

						ImGui::Text(Text.c_str());
					}
					else if (CurrentMeshAnalysisData->TriangleSelected.size() > 1 && ActiveLayer != nullptr)
					{
						ImGui::Text("Selected area information : ");
						std::string Text = "Average values per layer:\n";

						std::vector<DataLayer*> Layers = LAYER_MANAGER.GetAllLayersOfActiveObject();
						for (size_t i = 0; i < Layers.size(); i++)
						{
							std::string CurrentCaption = Layers[i]->GetCaption();
							float TotalValue = 0.0f;
							if (Layers[i]->GetType() == LAYER_TYPE::INTERPOLATION)
							{
								TotalValue = std::numeric_limits<float>::quiet_NaN();
							}
							else
							{
								for (size_t j = 0; j < CurrentMeshAnalysisData->TriangleSelected.size(); j++)
								{
									TotalValue += Layers[i]->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[j]];
								}
							}

							float AverageValue = std::numeric_limits<float>::quiet_NaN();
							if (!isnan(TotalValue))
								AverageValue = TotalValue / CurrentMeshAnalysisData->TriangleSelected.size();

							Text += CurrentCaption + " : " + std::to_string(AverageValue) + "\n";
						}

						ImGui::Text(Text.c_str());
					}

					ImGui::TreePop();
				}
				ImGui::PopStyleVar();
			}
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

		if (CurrentAnnotationData == nullptr)
		{
			if (ImGui::Button("Add annotations"))
			{
				AddAnnotationToCurrentObject();
			}

			if (ImGui::Button("Load annotations from shape file..."))
			{
				FEEntity* SelectedEntity = OBJECT_VIEWER_WINDOW.GetSelectedEntity();
				if (SelectedEntity != nullptr)
				{
					std::string ShapeFilePath;
					FILE_SYSTEM.ShowFileOpenDialog(ShapeFilePath, VECTOR_LOAD_FILE_FILTER, 1);
					
					if (!ShapeFilePath.empty() && FILE_SYSTEM.DoesFileExist(ShapeFilePath))
					{
						ANNOTATION_MANAGER.InitializeReadAnnotationDataFromShapeFile(ShapeFilePath, ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject());

						FENaiveSceneGraphNode* AnnotationSceneNode = MAIN_SCENE_MANAGER.GetMainScene()->SceneGraph.GetNodeByEntityID(SelectedEntity->GetObjectID());
						OBJECT_VIEWER_WINDOW.ExpandToNode(AnnotationSceneNode);
						OBJECT_VIEWER_WINDOW.SetNodeSelected(AnnotationSceneNode, true);
					}
				}
			}
		}
	}
	else
	{
		if (SelectedObjectType == "Photogrammetry" || SelectedObjectType == "Photogrammetry (Image)")
		{
			if (CurrentCOLMAPProject != nullptr && CurrentCOLMAPProject->GetImageCount() > 0)
				RenderPhotogrammetryInformation(CurrentCOLMAPProject);
			
		}
		else if (SelectedObjectType == "Annotation")
		{
			if (CurrentAnnotationData != nullptr)
				RenderAnnotationInformation(CurrentAnnotationData);
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

void UIInspector::AddAnnotationToCurrentObject()
{
	FEEntity* SelectedEntity = OBJECT_VIEWER_WINDOW.GetSelectedEntity();
	if (SelectedEntity == nullptr)
		return;

	AnalysisObject* SelectedAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(SelectedEntity->GetObjectID());
	if (SelectedAnalysisObject == nullptr)
		return;

	ANNOTATION_MANAGER.AddAnnotationToAnalysisObject(SelectedAnalysisObject->GetID());

	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(SelectedAnalysisObject->GetID());
	FEEntity* AnnotationEntity = CurrentAnnotationData->GetEntity();

	FENaiveSceneGraphNode* AnnotationSceneNode = MAIN_SCENE_MANAGER.GetMainScene()->SceneGraph.GetNodeByEntityID(AnnotationEntity->GetObjectID());
	OBJECT_VIEWER_WINDOW.ExpandToNode(AnnotationSceneNode);
	OBJECT_VIEWER_WINDOW.SetNodeSelected(AnnotationSceneNode, true);
}

void UIInspector::RenderPhotogrammetryInformation(COLMAPProject* CurrentCOLMAPProject)
{
	if (CurrentCOLMAPProject == nullptr)
		return;

	int TreeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;

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
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
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
		ImGui::PopStyleVar();
	}
	else
	{
		ImGui::Text("No image selected.");

		if (ImGui::Button("Bulk render view from all image cameras"))
		{
			bool bAutoOpenFiles = CurrentCOLMAPProject->GetCurrentViewRenderSettings()->GetAutoOpenResult();
			CurrentCOLMAPProject->GetCurrentViewRenderSettings()->SetAutoOpenResult(false);

			if (!FILE_SYSTEM.DoesDirectoryExist(FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut/"))
				FILE_SYSTEM.MakeDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut/");

			std::vector<int> ImagesIDList = CurrentCOLMAPProject->GetImagesIDList();
			for (size_t i = 0; i < ImagesIDList.size(); i++)
			{
				std::string OriginalPhotoPath = CurrentCOLMAPProject->GetPathToPhotoByImageID(ImagesIDList[i]);
				std::string OriginalPhotoFileName = FILE_SYSTEM.GetFileName(OriginalPhotoPath, false);
				CurrentCOLMAPProject->RenderViewFromImage(ImagesIDList[i], false, FE_DEPTH_EXPORT_NONE, FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut/" + OriginalPhotoFileName + "_Color" + ".png");
			}

			CurrentCOLMAPProject->GetCurrentViewRenderSettings()->SetAutoOpenResult(bAutoOpenFiles);
		}

		if (ImGui::Button("Bulk render view from all image cameras(Depth 8-bit)"))
		{
			bool bAutoOpenFiles = CurrentCOLMAPProject->GetCurrentViewRenderSettings()->GetAutoOpenResult();
			CurrentCOLMAPProject->GetCurrentViewRenderSettings()->SetAutoOpenResult(false);

			if (!FILE_SYSTEM.DoesDirectoryExist(FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_8bit_Depth/"))
				FILE_SYSTEM.MakeDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_8bit_Depth/");

			std::vector<int> ImagesIDList = CurrentCOLMAPProject->GetImagesIDList();
			for (size_t i = 0; i < ImagesIDList.size(); i++)
			{
				std::string OriginalPhotoPath = CurrentCOLMAPProject->GetPathToPhotoByImageID(ImagesIDList[i]);
				std::string OriginalPhotoFileName = FILE_SYSTEM.GetFileName(OriginalPhotoPath, false);
				CurrentCOLMAPProject->RenderViewFromImage(ImagesIDList[i], true, FE_DEPTH_EXPORT_GRAYSCALE_PNG, FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_8bit_Depth/" + OriginalPhotoFileName + "_Depth8bit" + ".png");
			}

			CurrentCOLMAPProject->GetCurrentViewRenderSettings()->SetAutoOpenResult(bAutoOpenFiles);
		}

		if (ImGui::Button("Bulk render view from all image cameras(Depth 16-bit)"))
		{
			bool bAutoOpenFiles = CurrentCOLMAPProject->GetCurrentViewRenderSettings()->GetAutoOpenResult();
			CurrentCOLMAPProject->GetCurrentViewRenderSettings()->SetAutoOpenResult(false);

			if (!FILE_SYSTEM.DoesDirectoryExist(FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_16bit_Depth/"))
				FILE_SYSTEM.MakeDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_16bit_Depth/");

			std::vector<int> ImagesIDList = CurrentCOLMAPProject->GetImagesIDList();
			for (size_t i = 0; i < ImagesIDList.size(); i++)
			{
				std::string OriginalPhotoPath = CurrentCOLMAPProject->GetPathToPhotoByImageID(ImagesIDList[i]);
				std::string OriginalPhotoFileName = FILE_SYSTEM.GetFileName(OriginalPhotoPath, false);
				CurrentCOLMAPProject->RenderViewFromImage(ImagesIDList[i], true, FE_DEPTH_EXPORT_16BIT_PNG, FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_16bit_Depth/" + OriginalPhotoFileName + "_Depth16bit" + ".png");
			}

			CurrentCOLMAPProject->GetCurrentViewRenderSettings()->SetAutoOpenResult(bAutoOpenFiles);
		}

		if (ImGui::Button("Bulk render view from all image cameras(Depth 32-bit float PFM)"))
		{
			bool bAutoOpenFiles = CurrentCOLMAPProject->GetCurrentViewRenderSettings()->GetAutoOpenResult();
			CurrentCOLMAPProject->GetCurrentViewRenderSettings()->SetAutoOpenResult(false);

			if (!FILE_SYSTEM.DoesDirectoryExist(FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_32bit_Depth/"))
				FILE_SYSTEM.MakeDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_32bit_Depth/");

			std::vector<int> ImagesIDList = CurrentCOLMAPProject->GetImagesIDList();
			for (size_t i = 0; i < ImagesIDList.size(); i++)
			{
				std::string OriginalPhotoPath = CurrentCOLMAPProject->GetPathToPhotoByImageID(ImagesIDList[i]);
				std::string OriginalPhotoFileName = FILE_SYSTEM.GetFileName(OriginalPhotoPath, false);
				CurrentCOLMAPProject->RenderViewFromImage(ImagesIDList[i], true, FE_DEPTH_EXPORT_32BIT_PFM_RAW, FILE_SYSTEM.GetCurrentWorkingPath() + "/BulkImageOut_32bit_Depth/" + OriginalPhotoFileName + "_Depth32bit" + ".pfm");
			}

			CurrentCOLMAPProject->GetCurrentViewRenderSettings()->SetAutoOpenResult(bAutoOpenFiles);
		}
	}
}

void UIInspector::RenderAnnotationInformation(AnnotationData* CurrentAnnotationData)
{
	const std::vector<AnnotationInfo>& AnnotationInfos = CurrentAnnotationData->GetAllAnnotationInfos();
	std::vector<int> RowAnnotationIDs;
	std::vector<LabeledColor> Rows;
	for (size_t i = 0; i < AnnotationInfos.size(); i++)
	{
		if (AnnotationInfos[i].ID == 0)
			continue;

		RowAnnotationIDs.push_back(AnnotationInfos[i].ID);
		Rows.push_back({ AnnotationInfos[i].Name, AnnotationInfos[i].GetColor() });
	}

	ImGui::Text("Annotations: %d", static_cast<int>(Rows.size()));
	if (Rows.empty())
	{
		ImGui::TextDisabled("No annotations.");
	}
	else
	{
		int ChangedRowIndex = UI_CORE.ShowLabeledColorTable("##AnnotationColors", Rows);
		if (ChangedRowIndex != -1)
			CurrentAnnotationData->UpdateAnnotationColor(RowAnnotationIDs[ChangedRowIndex], Rows[ChangedRowIndex].Color);
	}

	if (DEVELOPER_MODE.IsOn())
	{
		ImGui::Separator();

		bool bInEditingMode = CurrentAnnotationData->IsInEditingMode();
		ImGui::Checkbox("Editing mode", &bInEditingMode);
		CurrentAnnotationData->SetEditingMode(bInEditingMode);

		ImGui::Separator();

		PolygonPlane* CurrentPolygonPlane = CurrentAnnotationData->GetPolygonPlane();
		if (CurrentPolygonPlane != nullptr)
		{
			if (CurrentAnnotationData->IsInEditingMode())
			{
				ImGui::Text("Canvas transform:");
				UI_CORE.ShowTransformConfiguration("Debug Canvas transform", &CurrentPolygonPlane->CanvasEntity->GetComponent<FETransformComponent>());
				CurrentPolygonPlane->UpdateCanvasTrianglePositions();

				if (ImGui::Button("Begin drafting a polygon"))
					CurrentPolygonPlane->BeginDraftPolygon();

				if (ImGui::Button("Finalize drafted polygon"))
					CurrentPolygonPlane->FinalizeDraftPolygon();

				if (ImGui::Button("Clear drafted polygon"))
					CurrentPolygonPlane->ClearDraftPolygon();

				if (ImGui::Button("Annotate mesh with polygons"))
				{
					AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
					if (ActiveObject == nullptr)
						return;

					std::vector<std::pair<int, std::vector<int>>> TriangleIndicesInPolygon = CurrentPolygonPlane->GetTriangleIndicesInAllPolygons(ActiveObject);
					MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
					if (CurrentMeshAnalysisData == nullptr)
						return;

					if (!TriangleIndicesInPolygon.empty())
					{
						for (size_t i = 0; i < CurrentAnnotationData->PerElementID.size(); i++)
							CurrentAnnotationData->PerElementID[i] = -1;

						AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(ActiveObject->GetID());
						for (size_t i = 0; i < TriangleIndicesInPolygon.size(); i++)
						{
							int PolygonIndex = TriangleIndicesInPolygon[i].first;
							AnnotationInfo* AssociatedAnnotationInfo = CurrentAnnotationData->GetAnnotationInfoByPolygonIndex(PolygonIndex);
							if (AssociatedAnnotationInfo != nullptr)
							{
								for (size_t j = 0; j < TriangleIndicesInPolygon[i].second.size(); j++)
								{
									CurrentAnnotationData->PerElementID[TriangleIndicesInPolygon[i].second[j]] = AssociatedAnnotationInfo->ID;
								}
							}
						}

						if (CurrentAnnotationData->MeshBufferID == GLuint(-1))
							ANNOTATION_MANAGER.InitalizeBuffer(CurrentAnnotationData);

						ANNOTATION_MANAGER.UpdateBuffer(CurrentAnnotationData);
					}
					else
					{
						for (size_t i = 0; i < CurrentAnnotationData->PerElementID.size(); i++)
							CurrentAnnotationData->PerElementID[i] = -1;

						if (CurrentAnnotationData->MeshBufferID != GLuint(-1))
							ANNOTATION_MANAGER.UpdateBuffer(CurrentAnnotationData);
					}
				}

				CurrentPolygonPlane->RenderAdditionalVisualization();
			}
			ImGui::Separator();

			std::vector<FEPolygon> AllPolygons = CurrentPolygonPlane->GetAllPolygons();
			const std::vector<AnnotationInfo>& AllAnnotationInfos = CurrentAnnotationData->GetAllAnnotationInfos();

			auto PolygonIndexToString = [](int Index)->std::string {
				if (Index == -1)
					return "";

				return "Polygon " + std::to_string(Index);
				};

			ImGui::Text("Select polygon: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			static int SelectedPolygonIndex = -1;
			if (ImGui::BeginCombo("##ChoosePolygon", (PolygonIndexToString(SelectedPolygonIndex)).c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < AllPolygons.size(); i++)
				{
					const bool bIsSelected = i == SelectedPolygonIndex;
					if (ImGui::Selectable((PolygonIndexToString(static_cast<int>(i))).c_str(), bIsSelected))
					{
						SelectedPolygonIndex = static_cast<int>(i);
					}

					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (SelectedPolygonIndex == -1)
				ImGui::BeginDisabled();
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.6f, 0.1f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.65f, 0.2f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.75f, 0.6f, 0.1f));
			ImGui::SameLine();
			if (ImGui::Button("Delete selected polygon"))
			{
				if (SelectedPolygonIndex != -1)
				{
					CurrentPolygonPlane->DeletePolygon(SelectedPolygonIndex);
					SelectedPolygonIndex = -1;
					ImGui::PopStyleColor(3);
					return;
				}
			}
			ImGui::PopStyleColor(3);
			if (SelectedPolygonIndex == -1)
				ImGui::EndDisabled();

			if (SelectedPolygonIndex != -1)
			{
				FEPolygon SelectedPolygon = AllPolygons[SelectedPolygonIndex];
				ImGui::Text("Number of points in polygon: %d", static_cast<int>(SelectedPolygon.Points.size()));

				AnnotationInfo* AssociatedAnnotationInfo = CurrentAnnotationData->GetAnnotationInfoByPolygonIndex(SelectedPolygonIndex);
				int SelectedAnnotationIndex = -1;
				if (AssociatedAnnotationInfo != nullptr)
					SelectedAnnotationIndex = AssociatedAnnotationInfo->ID;

				auto AnnotationIndexToString = [&](int Index)->std::string {
					if (Index >= AllAnnotationInfos.size() || Index < 0)
						return "";

					return AllAnnotationInfos[Index].Name;
					};

				ImGui::Text("Select annotation: ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(190);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
				if (ImGui::BeginCombo("##ChooseAnnotation", (AnnotationIndexToString(SelectedAnnotationIndex)).c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < AllAnnotationInfos.size(); i++)
					{
						const bool bIsSelected = i == SelectedAnnotationIndex;
						if (ImGui::Selectable((AnnotationIndexToString(static_cast<int>(i))).c_str(), bIsSelected))
						{
							CurrentAnnotationData->SetPolygonIndexAnnotation(SelectedPolygonIndex, AllAnnotationInfos[i].ID);
						}

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (AssociatedAnnotationInfo != nullptr)
				{
					ImGui::Text("Annotation name: %s", AssociatedAnnotationInfo->Name.c_str());
					ImGui::Text("Annotation description: %s", AssociatedAnnotationInfo->Description.c_str());
					glm::vec4 Color = AssociatedAnnotationInfo->GetColor();
					if (ImGui::ColorEdit4("Annotation color", (float*)&Color))
						CurrentAnnotationData->UpdateAnnotationColor(AssociatedAnnotationInfo->ID, Color);

					ImGui::Text("Number of histogram entries: %d", static_cast<int>(AssociatedAnnotationInfo->HistogramData.size()));
				}
				else
				{
					ImGui::Text("No annotation data associated with this polygon.");
				}
			}
			ImGui::Separator();
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
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
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
				ImGui::TreePop();
				ImGui::PopStyleVar();
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
						if (CurrentMeshAnalysisData != nullptr)
						{
							TotalWeight = CurrentMeshAnalysisData->GetTotalArea();
							WeightUnit = "area";
						}
						
						break;
					}

					case DATA_SOURCE_TYPE::POINT_CLOUD:
					{
						PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
						if (CurrentPointCloudAnalysisData != nullptr)
						{
							TotalWeight = static_cast<double>(CurrentPointCloudAnalysisData->RawPointCloudData.size());
							WeightUnit = "points";
						}
						
						break;
					}
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

		ImGui::PopStyleVar();
	}
}

void UIInspector::UpdateMeshSelectedTrianglesRendering()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	std::vector<FELine> LinesToRender;
	float LineWidth = 0.5f;
	if (CurrentMeshAnalysisData->TriangleSelected.size() == 1)
	{
		std::vector<glm::dvec3> SelectedTrianglePoints = CurrentMeshAnalysisData->Triangles[CurrentMeshAnalysisData->TriangleSelected[0]];
		LinesToRender.push_back(FELine(SelectedTrianglePoints[0], SelectedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f), LineWidth));
		LinesToRender.push_back(FELine(SelectedTrianglePoints[0], SelectedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f), LineWidth));
		LinesToRender.push_back(FELine(SelectedTrianglePoints[1], SelectedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f), LineWidth));

		if (!CurrentMeshAnalysisData->TrianglesNormals.empty())
		{
			glm::vec3 Point = SelectedTrianglePoints[0];
			glm::vec3 Normal = CurrentMeshAnalysisData->TrianglesNormals[CurrentMeshAnalysisData->TriangleSelected[0]][0];
			LinesToRender.push_back(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f), LineWidth));

			Point = SelectedTrianglePoints[1];
			Normal = CurrentMeshAnalysisData->TrianglesNormals[CurrentMeshAnalysisData->TriangleSelected[0]][1];
			LinesToRender.push_back(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f), LineWidth));

			Point = SelectedTrianglePoints[2];
			Normal = CurrentMeshAnalysisData->TrianglesNormals[CurrentMeshAnalysisData->TriangleSelected[0]][2];
			LinesToRender.push_back(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f), LineWidth));
		}
	}
	else if (CurrentMeshAnalysisData->TriangleSelected.size() > 1)
	{
		for (size_t i = 0; i < CurrentMeshAnalysisData->TriangleSelected.size(); i++)
		{
			std::vector<glm::dvec3> SelectedTrianglePoints = CurrentMeshAnalysisData->Triangles[CurrentMeshAnalysisData->TriangleSelected[i]];
			LinesToRender.push_back(FELine(SelectedTrianglePoints[0], SelectedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f), LineWidth));
			LinesToRender.push_back(FELine(SelectedTrianglePoints[0], SelectedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f), LineWidth));
			LinesToRender.push_back(FELine(SelectedTrianglePoints[1], SelectedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f), LineWidth));
		}
	}

	MAIN_SCENE_MANAGER.AddLinesToEntity(&SelectionLinesEntity, LinesToRender);

	if (SelectionLinesEntity != nullptr && SelectionLinesEntity->GetParentEntity() != ActiveEntity)
		ActiveEntity->AttachChild(SelectionLinesEntity, false);
}

void UIInspector::CleanUpSelectionLinesComponent()
{
	MAIN_SCENE_MANAGER.ClearLinesFromEntity(SelectionLinesEntity);
}

void UIInspector::OnObjectLoad(AnalysisObject* NewObject)
{
	FEEntity* NewActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (NewActiveEntity != nullptr)
	{
		FEScene* Scene = MAIN_SCENE_MANAGER.GetMainScene();
		FENaiveSceneGraphNode* Node = Scene->SceneGraph.GetNodeByEntityID(NewActiveEntity->GetObjectID());
		OBJECT_VIEWER_WINDOW.SetNodeSelected(Scene->SceneGraph.GetNodeByEntityID(NewActiveEntity->GetObjectID()), true);
	}
}

void UIInspector::OnActiveObjectChange(AnalysisObject* NewActiveObject)
{
	UI_INSPECTOR.CleanUpSelectionLinesComponent();
}

float UIInspector::GetRadiusOfAreaToSelect()
{
	return RadiusOfAreaToSelect;
}

void UIInspector::SetRadiusOfAreaToSelect(const float NewValue)
{
	RadiusOfAreaToSelect = NewValue;
}

int UIInspector::GetMeshSelectionMode()
{
	return MeshSelectionMode;
}

void UIInspector::SetMeshSelectionMode(const int NewValue)
{
	MeshSelectionMode = NewValue;
}

void UIInspector::MouseButtonCallback(int Button, int Action, int Mods)
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

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_PRESS)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(true);
	}

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);

		//LAYER_RASTERIZATION_MANAGER.DebugMouseClick();

		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		FEMesh* ActiveMesh = nullptr;
		if (ActiveObject != nullptr && ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
			ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());

		if (ActiveMesh != nullptr)
		{
			// Check if photogrammetry element should be selected first
			COLMAPProject* CurrentProject = COLMAP_DATA_MANAGER.GetProjectByAnalysisObjectID(ActiveObject->GetID());
			float PhotogrammetryHitDistance = std::numeric_limits<float>::max();
			if (CurrentProject != nullptr)
				CurrentProject->ImageUnderMouse(&PhotogrammetryHitDistance);

			float MeshHitDistance = std::numeric_limits<float>::max();
			int TriangleIndexUnderMouse = -1;
			std::vector<int> TriangleIndexesInRadius;
			if (UI_INSPECTOR.GetMeshSelectionMode() == 1)
			{
				TriangleIndexUnderMouse = ANALYSIS_OBJECT_MANAGER.GetTriangleIndexUnderMouse(&MeshHitDistance);
			}
			else if (UI_INSPECTOR.GetMeshSelectionMode() == 2)
			{
				TriangleIndexesInRadius = ANALYSIS_OBJECT_MANAGER.GetTriangleIndexesInRadius(UI_INSPECTOR.GetRadiusOfAreaToSelect());
			}

			if (MeshHitDistance > PhotogrammetryHitDistance)
			{

			}
			else
			{
				if (UI_INSPECTOR.GetMeshSelectionMode() == 1)
				{
					ANALYSIS_OBJECT_MANAGER.SelectTriangleByIndex(TriangleIndexUnderMouse);
				}
				else if (UI_INSPECTOR.GetMeshSelectionMode() == 2)
				{
					ANALYSIS_OBJECT_MANAGER.SelectTrianglesByIndexes(TriangleIndexesInRadius);
					// FE_FIX_ME: Reenable this functionality.
					//OutputSelectedAreaInfoToFile();
				}

				UI_INSPECTOR.UpdateMeshSelectedTrianglesRendering();
			}
		}
	}
}

void UIInspector::OutputSelectedAreaInfoToFile()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	if (CurrentMeshAnalysisData->TriangleSelected.size() < 2)
		return;

	bool bCurrentSettings = LOG.IsFileOutputActive();
	if (!bCurrentSettings)
		LOG.SetFileOutput(true);

	std::string Text = "Area radius : " + std::to_string(UI_INSPECTOR.GetRadiusOfAreaToSelect());
	LOG.Add(Text, FILE_SYSTEM.GetFileName(ActiveObject->GetFilePath()));

	Text = "Area approximate center : X - ";
	const glm::vec3 Center = CurrentMeshAnalysisData->TrianglesCentroids[CurrentMeshAnalysisData->TriangleSelected[0]];
	Text += std::to_string(Center.x);
	Text += " Y - ";
	Text += std::to_string(Center.y);
	Text += " Z - ";
	Text += std::to_string(Center.z);
	LOG.Add(Text, FILE_SYSTEM.GetFileName(ActiveObject->GetFilePath()));

	for (size_t i = 0; i < ActiveObject->Layers.size(); i++)
	{
		DataLayer* CurrentLayer = ActiveObject->Layers[i];

		Text = "Layer \"" + CurrentLayer->GetCaption() + "\" : \n";
		Text += "Area average value : ";

		float TotalValue = 0.0f;
		if (CurrentLayer->GetType() == LAYER_TYPE::INTERPOLATION)
		{
			TotalValue = std::numeric_limits<float>::quiet_NaN();
		}
		else
		{
			for (size_t j = 0; j < CurrentMeshAnalysisData->TriangleSelected.size(); j++)
			{
				TotalValue += CurrentLayer->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[j]];
			}
		}

		float AverageValue = std::numeric_limits<float>::quiet_NaN();
		if (!isnan(TotalValue))
			AverageValue = TotalValue / CurrentMeshAnalysisData->TriangleSelected.size();

		Text += std::to_string(AverageValue);
		LOG.Add(Text, FILE_SYSTEM.GetFileName(ActiveObject->GetFilePath()));
	}

	if (!bCurrentSettings)
		LOG.SetFileOutput(false);
}

bool UIInspector::ExportOBJ(std::string FilePath, int LayerIndex)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return false;

	if (FilePath.empty())
		return false;

	if (FilePath.find(".obj") == std::string::npos)
		FilePath += ".obj";

	// If LayerIndex is -1, export just model without any layers.
	if (LayerIndex < 0)
	{
		return RESOURCE_MANAGER.ExportFEMeshToOBJ(ActiveMesh, FilePath.c_str());
	}
	else // Export model with layer.
	{
		if (LayerIndex >= ActiveObject->Layers.size())
			return false;

		// To export model with layer, we need to create a new mesh with vertex colors as layer data.
		DataLayer* LayerToExport = ActiveObject->Layers[LayerIndex];

		std::vector<float> ColorData;
		ColorData.resize(CurrentMeshAnalysisData->Vertices.size());
		for (size_t i = 0; i < LayerToExport->ElementsToData.size(); i++)
		{
			float CompexityValueForTriangle = LayerToExport->ElementsToData[i];
			float NormalizedValue = (CompexityValueForTriangle - LayerToExport->GetMin()) / (LayerToExport->MaxVisible - LayerToExport->GetMin());
			if (NormalizedValue > 1.0f)
				NormalizedValue = 1.0f;
			else if (NormalizedValue < 0.0f)
				NormalizedValue = 0.0f;

			glm::vec3 Color = GetTurboColorMap(NormalizedValue);

			int FirstVertexIndex = CurrentMeshAnalysisData->Indices[i * 3] * 3;
			ColorData[FirstVertexIndex] = Color.x;
			ColorData[FirstVertexIndex + 1] = Color.y;
			ColorData[FirstVertexIndex + 2] = Color.z;

			int SecondVertexIndex = CurrentMeshAnalysisData->Indices[i * 3 + 1] * 3;
			ColorData[SecondVertexIndex] = Color.x;
			ColorData[SecondVertexIndex + 1] = Color.y;
			ColorData[SecondVertexIndex + 2] = Color.z;

			int ThirdVertexIndex = CurrentMeshAnalysisData->Indices[i * 3 + 2] * 3;
			ColorData[ThirdVertexIndex] = Color.x;
			ColorData[ThirdVertexIndex + 1] = Color.y;
			ColorData[ThirdVertexIndex + 2] = Color.z;
		}

		std::vector<float> TemporaryVertices; TemporaryVertices.resize(CurrentMeshAnalysisData->Vertices.size());
		for (size_t i = 0; i < CurrentMeshAnalysisData->Vertices.size(); i++)
			TemporaryVertices[i] = static_cast<float>(CurrentMeshAnalysisData->Vertices[i]);

		FEMesh* NewMesh = RESOURCE_MANAGER.RawDataToMesh(TemporaryVertices.data(), static_cast<int>(TemporaryVertices.size()),
														 CurrentMeshAnalysisData->UVs.data(), static_cast<int>(CurrentMeshAnalysisData->UVs.size()),
														 CurrentMeshAnalysisData->Normals.data(), static_cast<int>(CurrentMeshAnalysisData->Normals.size()),
														 CurrentMeshAnalysisData->Tangents.data(), static_cast<int>(CurrentMeshAnalysisData->Tangents.size()),
														 CurrentMeshAnalysisData->Indices.data(), static_cast<int>(CurrentMeshAnalysisData->Indices.size()),
														 ColorData.data(), static_cast<int>(ColorData.size()),
														 nullptr, 0, 0,
														 "Exported model with layer");

		bool bResult = RESOURCE_MANAGER.ExportFEMeshToOBJ(NewMesh, FilePath.c_str());
		RESOURCE_MANAGER.DeleteFEMesh(NewMesh);

		return bResult;
	}

	return false;
}

void UIInspector::SetShouldTakeScreenshot(bool NewValue)
{
	bNextFrameForScreenshot = NewValue;
}

bool UIInspector::ShouldTakeScreenshot()
{
	return bNextFrameForScreenshot;
}

void UIInspector::SetUseTransparentBackground(bool NewValue)
{
	bUseTransparentBackground = NewValue;
}

bool UIInspector::ShouldUseTransparentBackground()
{
	return bUseTransparentBackground;
}

void UIInspector::RasterizationSettingsUI()
{
	if (LAYER_MANAGER.GetActiveLayer() == nullptr)
		ImGui::BeginDisabled();

	bool bNeedUpdate = false;

	const char* RasterizationModes[] = { "Min", "Max", "Mean", "Cumulative" };
	int TempInt = LAYER_RASTERIZATION_MANAGER.GetGridRasterizationMode();
	ImGui::Text("Mode: ");
	ImGui::SetNextItemWidth(128);
	if (ImGui::Combo("##Mode", &TempInt, RasterizationModes, IM_ARRAYSIZE(RasterizationModes)))
	{
		if (TempInt != LayerRasterizationManager::GridRasterizationMode::Cumulative)
			LAYER_RASTERIZATION_MANAGER.ActivateAutomaticOutliersSuppression();

		bNeedUpdate = true;
		LAYER_RASTERIZATION_MANAGER.SetGridRasterizationMode(static_cast<LayerRasterizationManager::GridRasterizationMode>(TempInt));
	}

	float TempFloat = LAYER_RASTERIZATION_MANAGER.GetResolutionInMeters();
	glm::vec2 MinMax = LAYER_RASTERIZATION_MANAGER.GetMinMaxResolutionInMeters();

	ImGui::Text("Choose resolution in meters: ");
	ImGui::Text(("Min value : "
		+ std::to_string(MinMax.y)
		+ " m \nMax value : "
		+ std::to_string(MinMax.x) + " m").c_str());
	static bool bSliderResolutionValueChanged = false;
	static float SliderResolutionNewValue = 0.0f;
	if (ImGui::SliderFloat("##Resolution in meters", &TempFloat, MinMax.y, MinMax.x))
	{
		bSliderResolutionValueChanged = true;
		SliderResolutionNewValue = TempFloat;
	}

	static char CustomResolutionInM[512];
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	ImGui::Text("Input exact value: ");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	ImGui::SetNextItemWidth(128);
	ImGui::InputText("##ResolutionInM", CustomResolutionInM, IM_ARRAYSIZE(CustomResolutionInM));
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::Button("Apply"))
	{
		TempFloat = static_cast<float>(atof(CustomResolutionInM));
		bNeedUpdate = true;
		LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(TempFloat);

		float NewResolution = LAYER_RASTERIZATION_MANAGER.GetResolutionInMeters();
		strcpy_s(CustomResolutionInM, std::to_string(NewResolution).c_str());
	}

	if (bSliderResolutionValueChanged && ImGui::IsMouseReleased(0))
	{
		bNeedUpdate = true;
		bSliderResolutionValueChanged = false;
		LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(SliderResolutionNewValue);
	}

	ImGui::Text(std::string("Output resolution: " + std::to_string(LAYER_RASTERIZATION_MANAGER.GetResolutionInPixelsThatWouldGiveSuchResolutionInMeters(TempFloat))).c_str());

	glm::vec3 ForceProjectionVector = LAYER_RASTERIZATION_MANAGER.GetProjectionVector();
	int SelectedAxis = ForceProjectionVector == glm::vec3(1.0f, 0.0f, 0.0f) ? 0 : ForceProjectionVector == glm::vec3(0.0f, 1.0f, 0.0f) ? 1 : 2;
	ImGui::Text("Select the axis along which the layer should be projected: ");
	if (ImGui::RadioButton("X", &SelectedAxis, 0))
	{
		bNeedUpdate = true;
		ForceProjectionVector = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	ImGui::SameLine();
	if (ImGui::RadioButton("Y", &SelectedAxis, 1))
	{
		bNeedUpdate = true;
		ForceProjectionVector = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	ImGui::SameLine();
	if (ImGui::RadioButton("Z", &SelectedAxis, 2))
	{
		bNeedUpdate = true;
		ForceProjectionVector = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	if (LAYER_RASTERIZATION_MANAGER.GetGridRasterizationMode() == LayerRasterizationManager::GridRasterizationMode::Cumulative)
	{
		static bool bAutomaticPercentOfAreaThatWouldBeRed = true;
		if (ImGui::Checkbox("Automatic outliers suppression", &bAutomaticPercentOfAreaThatWouldBeRed))
		{
			if (bAutomaticPercentOfAreaThatWouldBeRed)
			{
				bNeedUpdate = true;
				LAYER_RASTERIZATION_MANAGER.ActivateAutomaticOutliersSuppression();
			}
		}

		if (bAutomaticPercentOfAreaThatWouldBeRed)
			ImGui::BeginDisabled();

		TempFloat = LAYER_RASTERIZATION_MANAGER.GetCumulativeModePercentOfAreaThatWouldBeRed();
		static bool bSliderThresholdValueChanged = false;
		static float SliderThresholdNewValue = 0.0f;
		ImGui::Text("Choose percent of area that would be above color scale threshold(red): ");
		if (ImGui::SliderFloat("##Percent of area that would be above color scale threshold(red)", &TempFloat, 0.0f, 99.9f))
		{
			bSliderThresholdValueChanged = true;
			SliderThresholdNewValue = TempFloat;
		}

		if (bSliderThresholdValueChanged && ImGui::IsMouseReleased(0))
		{
			bNeedUpdate = true;
			bSliderThresholdValueChanged = false;
			LAYER_RASTERIZATION_MANAGER.SetCumulativeModePercentOfAreaThatWouldBeRed(SliderThresholdNewValue);
		}

		if (bAutomaticPercentOfAreaThatWouldBeRed)
			ImGui::EndDisabled();
	}

	std::string TextForButton = "Activate preview";
	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() == -1)
	{
		if (ImGui::Button(TextForButton.c_str()))
		{
			LAYER_RASTERIZATION_MANAGER.PrepareLayerForExport(LAYER_MANAGER.GetActiveLayer());
		}
	}

	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() != -1)
	{
		if (ImGui::Button("Save to file..."))
		{
			LAYER_RASTERIZATION_MANAGER.PromptUserForSaveLocation();
		}
	}

	if (LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID() != -1)
	{
		float CurrentWindowWidth = ImGui::GetWindowWidth() * 0.95f;
		ImGui::Text("Preview:");
		ImGui::Image((void*)(intptr_t)LAYER_RASTERIZATION_MANAGER.GetTexturePreviewID(), ImVec2(CurrentWindowWidth, CurrentWindowWidth));
	}

	if (LAYER_MANAGER.GetActiveLayer() == nullptr)
		ImGui::EndDisabled();

	if (bNeedUpdate)
	{
		LAYER_RASTERIZATION_MANAGER.PrepareLayerForExport(LAYER_MANAGER.GetActiveLayer(), ForceProjectionVector);
	}
}

void UIInspector::RenderExportTab()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	int TreeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
	if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		if (ImGui::TreeNodeEx("Mesh Export", TreeFlags))
		{
			ImGui::Text("Export model with selected layer(as vertex color):");
			if (ImGui::Button("Export to file..."))
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileSaveDialog(FilePath, MODEL_EXPORT_FILE_FILTER, 1);

				ExportOBJ(FilePath, LAYER_MANAGER.GetActiveLayerIndex());
			}

			ImGui::TreePop();
		}

		MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		//if (LayerSelectionMode == 2)
		//{
		//	ImGui::Text("Radius of area to measure: ");
		//	ImGui::SetNextItemWidth(128);
		//	ImGui::DragFloat("##RadiusOfAreaToMeasure", &RadiusOfAreaToMeasure, 0.01f);
		//	if (RadiusOfAreaToMeasure < 0.1f)
		//		RadiusOfAreaToMeasure = 0.1f;

		//	ImGui::Checkbox("Output selection data to file", &bOutputSelectionToFile);
		//}

		DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
		if (CurrentMeshAnalysisData->TriangleSelected.size() == 1 && ActiveLayer != nullptr)
		{
			ImGui::Separator();
			ImGui::Text("Selected triangle information :");

			std::vector<DataLayer*> Layers = LAYER_MANAGER.GetAllLayersOfActiveObject();
			std::string Text = "Value per layer:\n";
			for (size_t i = 0; i < Layers.size(); i++)
			{
				std::string CurrentCaption = Layers[i]->GetCaption();
				float CurrentValue = 0.0f;
				if (Layers[i]->GetType() == LAYER_TYPE::INTERPOLATION)
				{
					CurrentValue = std::numeric_limits<float>::quiet_NaN();
				}
				else
				{
					CurrentValue = Layers[i]->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[0]];
				}

				Text += CurrentCaption + " : " + std::to_string(CurrentValue) + "\n";
			}

			ImGui::Text(Text.c_str());
		}
		else if (CurrentMeshAnalysisData->TriangleSelected.size() > 1 && ActiveLayer != nullptr)
		{
			ImGui::Text("Selected area information : ");
			std::string Text = "Average values per layer:\n";

			std::vector<DataLayer*> Layers = LAYER_MANAGER.GetAllLayersOfActiveObject();
			for (size_t i = 0; i < Layers.size(); i++)
			{
				std::string CurrentCaption = Layers[i]->GetCaption();
				float TotalValue = 0.0f;
				if (Layers[i]->GetType() == LAYER_TYPE::INTERPOLATION)
				{
					TotalValue = std::numeric_limits<float>::quiet_NaN();
				}
				else
				{
					for (size_t j = 0; j < CurrentMeshAnalysisData->TriangleSelected.size(); j++)
					{
						TotalValue += Layers[i]->ElementsToData[CurrentMeshAnalysisData->TriangleSelected[j]];
					}
				}

				float AverageValue = std::numeric_limits<float>::quiet_NaN();
				if (!isnan(TotalValue))
					AverageValue = TotalValue / CurrentMeshAnalysisData->TriangleSelected.size();

				Text += CurrentCaption + " : " + std::to_string(AverageValue) + "\n";
			}

			ImGui::Text(Text.c_str());
		}
	}

	ImGui::Separator();

	ImGui::Text("Screenshoot:");
	ImGui::Checkbox("Transparent background", &bUseTransparentBackground);

	if (ImGui::Button("Take screenshoot"))
		bNextFrameForScreenshot = true;

	if (ImGui::TreeNodeEx("Export layer as image", TreeFlags))
	{
		RasterizationSettingsUI();
		ImGui::TreePop();
	}

	ImGui::PopStyleVar();
}