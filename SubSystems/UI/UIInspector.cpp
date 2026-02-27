#include "UIInspector.h"
using namespace FocalEngine;

UIInspector::UIInspector()
{
	LAYER_MANAGER.AddActiveLayerChangedCallback(OnLayerChange);
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

		//LOAD_PHOTOGRAMMETRY_WINDOW.Render();

		//RenderLayerDebugInfo(DEVELOPER_MODE.GetDebugGrid());

		ImGui::End();
	}
}

void UIInspector::RenderSelectedObjectTab()
{
	FEEntity* SelectedEntity = OBJECT_VIEWER_WINDOW.GetSelectedEntity();
	if (SelectedEntity == nullptr)
	{
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::CalcTextSize("No object selected.").x / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2.0f - ImGui::CalcTextSize("No object selected.").y / 2.0f);

		ImGui::Text("No object selected.");

		return;
	}

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	std::string SelectedObjectType = "Unknown";
	if (ActiveObject != nullptr)
	{
		if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			SelectedObjectType = "3D Model";
		}
		else if (ActiveObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
		{
			SelectedObjectType = "Point Cloud";
		}
	}
	else
	{
		SelectedObjectType = "Photogrammetry";
	}

	//ImGui::Text(("Name: " + SelectedEntity->GetName()).c_str());
	ImGui::Text(("Type: " + SelectedObjectType).c_str());

	if (ActiveObject != nullptr)
	{
		if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
			if (ActiveMesh == nullptr)
				return;

			ImGui::Text("Triangle count: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(ActiveMesh->GetVertexCount() / 3).c_str());
		}
		else if (ActiveObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
		{
			FEPointCloud* ActivePointCloud = static_cast<FEPointCloud*>(ActiveObject->GetEngineResource());
			if (ActivePointCloud == nullptr)
				return;

			ImGui::Text("Point count: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(ActivePointCloud->GetPointCount()).c_str());
		}
	}

	FETransformComponent& TransformComponent = SelectedEntity->GetComponent<FETransformComponent>();
	if (DEVELOPER_MODE.IsOn())
		UI_CORE.ShowTransformConfiguration("Selected Object Transform", &TransformComponent);
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
		DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
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

		ImGui::Separator();
		LayerInterpolationData* InterpolationData = ActiveLayer->GetInterpolationData();
		if (ActiveLayer->GetType() == LAYER_TYPE::INTERPOLATION && InterpolationData != nullptr)
		{
			if (ImGui::Begin("Interpolation Settings"))
			{
				ImGui::Text("Layers blend factor:");
				float GlobalFactor = InterpolationData->GetInterpolationFactor();
				ImGui::DragFloat("##InterpolationFactor", &GlobalFactor, 0.001f, 0.0f, 1.0f, "%.3f");
				InterpolationData->SetInterpolationFactor(GlobalFactor);

				bool bUseMinMaxInterpolation = InterpolationData->IsMinMaxInterpolationEnabled();
				ImGui::Checkbox("Use Min/Max interpolation", &bUseMinMaxInterpolation);
				InterpolationData->SetMinMaxInterpolationEnabled(bUseMinMaxInterpolation);

				ImGui::End();
			}
		}
		ImGui::Separator();

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

		ImGui::Separator();
		ImGui::Text("Distribution : ");
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
	}

	DEVELOPER_MODE.ShowLayerDebugUI();
}