#include "UIManager.h"
using namespace FocalEngine;
#include "../ScreenshotManager.h"

UIManager::UIManager()
{
	HeatMapColorRange.SetPosition(ImVec2(0, 20));

	Histogram.SetSize(ImVec2(300, 180));
	Histogram.SetPosition(ImVec2(20, 60));

	HistogramSelectRegionMin.SetColor(ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
	HistogramSelectRegionMin.SetOrientation(true);

	HistogramSelectRegionMax.SetColor(ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
	HistogramSelectRegionMax.SetOrientation(true);

	APPLICATION.GetMainWindow()->AddOnResizeCallback(UIManager::WindowResizeCallback);

	JITTER_MANAGER.SetOnCalculationsStartCallback(OnJitterCalculationsStart);
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);

	ANALYSIS_OBJECT_MANAGER.AddOnObjectLoadCallback(UIManager::OnNewObjectLoaded);
	LAYER_MANAGER.AddActiveLayerChangedCallback(UIManager::OnLayerChange);

	LAYER_RASTERIZATION_MANAGER.SetOnCalculationsStartCallback(OnLayerRasterizationCalculationsStart);
	LAYER_RASTERIZATION_MANAGER.SetOnCalculationsEndCallback(OnLayerRasterizationCalculationsEnd);

	AddNewLayerIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/AddNewLayer.png");
}

UIManager::~UIManager() {}

void UIManager::Render()
{
	if (APPLICATION.GetMainWindow() == nullptr)
		return;
	
	if (bPreviousFrameWindowWasNull)
		bPreviousFrameWindowWasNull = false;

	if (SCREENSHOT_MANAGER.IsActive())
	{
		RenderLegend();
		return;
	}

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Load..."))
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileOpenDialog(FilePath, RUGOSITY_LOAD_FILE_FILTER, 1);

				if (!FilePath.empty())
					ANALYSIS_OBJECT_MANAGER.LoadResource(FilePath);
			}

			size_t ObjectCount = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount();
			if (ObjectCount == 0)
				ImGui::BeginDisabled();

			if (ImGui::MenuItem("Save..."))
				ANALYSIS_OBJECT_MANAGER.SaveToRUGFileAskForFilePath();

			if (ObjectCount == 0)
				ImGui::EndDisabled();

			ImGui::Separator();

			if (ImGui::MenuItem("Close all"))
				ANALYSIS_OBJECT_MANAGER.ClearAll();

			if (ImGui::MenuItem("Exit"))
				APPLICATION.Close();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Inspector", nullptr, UI_INSPECTOR.bVisible))
			{
				UI_INSPECTOR.bVisible = !UI_INSPECTOR.bVisible;
			}

			if (ImGui::MenuItem("Settings", nullptr, SETTINGS_WINDOW.bVisible))
			{
				SETTINGS_WINDOW.bVisible = !SETTINGS_WINDOW.bVisible;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings"))
		{
			bool bTemporary = IsApplyStandardLayoutOnResize();
			if (ImGui::MenuItem("Reset Window Layout on Resize", nullptr, bTemporary))
				SetApplyStandardLayoutOnResize(!IsApplyStandardLayoutOnResize());
			
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Info"))
		{
			if (ImGui::MenuItem("About..."))
				ShowAboutDialog();

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	OBJECT_VIEWER_WINDOW.Render();

	DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
	if (ActiveLayer != nullptr)
	{
		if (ActiveLayer->GetType() == LAYER_TYPE::INTERPOLATION)
			UI.HeatMapColorRange.bRenderSlider = !ActiveLayer->GetInterpolationData()->IsMinMaxInterpolationEnabled();
	}

	UI_INSPECTOR.Render();
	SETTINGS_WINDOW.Render();
	RenderLegend();
	RenderLayerTabs();
	RenderHistogramWindow();
	RenderAboutWindow();

	NEW_LAYER_WINDOW.Render();
	LOAD_PHOTOGRAMMETRY_WINDOW.Render();

	if (UI.bShouldOpenProgressPopup)
	{
		UI.bShouldOpenProgressPopup = false;
		ImGui::OpenPopup("Calculating...");
	}

	ImGui::SetNextWindowSize(ImVec2(300, 65));
	if (ImGui::BeginPopupModal("Calculating...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		int WindowW = 0;
		int WindowH = 0;
		APPLICATION.GetMainWindow()->GetSize(&WindowW, &WindowH);

		ImGui::SetWindowPos(ImVec2(WindowW / 2.0f - ImGui::GetWindowWidth() / 2.0f, WindowH / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		UpdateProgressModalPopupCurrentValue();
		std::string ProgressText = "Progress: " + std::to_string(ProgressModalPopupCurrentValue * 100.0f);
		ProgressText += " %%";
		ImGui::SetCursorPosX(90);
		ImGui::Text(ProgressText.c_str());

		std::string TimeToFinish = "Time left: " + TIME.TimeToFormattedString(JITTER_MANAGER.ApproximateTimeToFinishInMS, FE_TIME_RESOLUTION_MILLISECONDS);
		int TextWidth = static_cast<int>(ImGui::CalcTextSize(TimeToFinish.c_str()).x);
		ImGui::SetCursorPosX(300 / 2.0f - TextWidth / 2.0f);
		ImGui::Text(TimeToFinish.c_str());

		if (bShouldCloseProgressPopup)
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void UIManager::OnNewObjectLoaded(AnalysisObject* NewObject)
{
	SETTINGS_WINDOW.AdjustCameraNearFarPlanes();
	SETTINGS_WINDOW.FocusCameraOnObject();

	UI.Histogram.Clear();
	UI.HeatMapColorRange.Clear();

	LAYER_RASTERIZATION_MANAGER.ClearAllData();
	float ResolutionInMeters = LAYER_RASTERIZATION_MANAGER.GetResolutionInMetersBasedOnResolutionInPixels(512);
	if (ResolutionInMeters > 0.0f)
		LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(ResolutionInMeters);
}

void UIManager::OnJitterCalculationsStart()
{
	UI.bJitterCalculationsInProgress = true;
	UI.bShouldCloseProgressPopup = false;
	UI.bShouldOpenProgressPopup = true;
}

void UIManager::OnJitterCalculationsEnd(DataLayer* NewLayer)
{
	UI.bShouldCloseProgressPopup = true;
	UI.bJitterCalculationsInProgress = false;
}

static auto CompareColormapValue = [](float Value) {

	Value = Value * 2.0f - 1.0f;

	static auto Mix = [](glm::vec3 FirstColor, glm::vec3 SecondColor, float Factor) {
		return glm::vec3(FirstColor.x + (SecondColor.x - FirstColor.x) * Factor,
						 FirstColor.y + (SecondColor.y - FirstColor.y) * Factor,
						 FirstColor.z + (SecondColor.z - FirstColor.z) * Factor);
	};

	// Define the colors
	glm::vec3 ColorNegative = glm::vec3(0.0, 0.0, 1.0); // Blue for negative
	glm::vec3 ColorNeutral = glm::vec3(1.0, 1.0, 1.0);  // White for zero
	glm::vec3 ColorPositive = glm::vec3(1.0, 0.0, 0.0); // Red for positive

	glm::vec3 FinalColor;
	// Interpolate between the colors based on the factor
	if (Value < 0)
	{
		// Interpolate between blue and white for negative values
		FinalColor = Mix(ColorNeutral, ColorNegative, -Value);
	}
	else
	{
		// Interpolate between white and red for positive values
		FinalColor = Mix(ColorNeutral, ColorPositive, Value);
	}

	return glm::vec3(FinalColor.x, FinalColor.y, FinalColor.z);
};

static auto RainbowScaledColor = [](float Value) {
	Value = 1.0f - Value;
	Value *= 6.0f;
	const int sextant = int(Value);
	const float vsf = Value - sextant;
	const float mid1 = vsf;
	const float mid2 = 1.0f - vsf;

	glm::vec3 result = glm::vec3(1, 0, 0);

	switch (sextant)
	{
	case 0:
		result.x = 1;
		result.y = 0;
		result.z = 0;
		break;
	case 1:
		result.x = 1;
		result.y = mid1;
		result.z = 0;
		break;
	case 2:
		result.x = mid2;
		result.y = 1;
		result.z = 0;
		break;
	case 3:
		result.x = 0;
		result.y = 1;
		result.z = mid1;
		break;
	case 4:
		result.x = 0;
		result.y = mid2;
		result.z = 1;
		break;
	case 5:
		result.x = mid1;
		result.y = 0;
		result.z = 1;
		break;
	}

	return ImColor(int(result.x * 255), int(result.y * 255), int(result.z * 255), 255);
};

void UIManager::RenderLegend()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(ImVec2(2, 20));
	ImGui::SetNextWindowSize(ImVec2(150, 670));
	ImGui::Begin("Heat map legend", nullptr,
									ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoCollapse |
									ImGuiWindowFlags_NoScrollbar |
									(SCREENSHOT_MANAGER.IsActive() ? ImGuiWindowFlags_NoBackground : ImGuiWindowFlags_None) |
									(SCREENSHOT_MANAGER.IsActive() ? ImGuiWindowFlags_NoTitleBar : ImGuiWindowFlags_None));

	if (HeatMapColorRange.GetColorRangeFunction() == nullptr)
		HeatMapColorRange.SetColorRangeFunction(GetTurboColorMap);

	static bool bLastFrameActiveLayerWasValid = false;
	if (bLastFrameActiveLayerWasValid && LAYER_MANAGER.GetActiveLayer() == nullptr)
		HeatMapColorRange.Clear();

	bLastFrameActiveLayerWasValid = LAYER_MANAGER.GetActiveLayer() != nullptr;

	if (SCREENSHOT_MANAGER.IsActive() && IsActiveObjectAndLayerValid())
	{
		DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
		if (CurrentLayer->GetMin() == CurrentLayer->GetMax())
		{
			HeatMapColorRange.Legend.SetDummyValues();
		}
		else
		{
			HeatMapColorRange.Legend.Clear();
			HeatMapColorRange.Legend.SetCaption(1.0f, UI_CORE.TruncateAfterDot(std::to_string(CurrentLayer->GetMin() + (CurrentLayer->GetMax() - CurrentLayer->GetMin()) * HeatMapColorRange.GetSliderValue())));
			const float MiddleOfUsedRange = (HeatMapColorRange.GetSliderValue() + CurrentLayer->MinVisible / CurrentLayer->GetMax()) / 2.0f;
			HeatMapColorRange.Legend.SetCaption(0.0f, UI_CORE.TruncateAfterDot(std::to_string(CurrentLayer->MinVisible)));
		}
	}

	HeatMapColorRange.Render(SCREENSHOT_MANAGER.IsActive());

	if (SCREENSHOT_MANAGER.IsActive())
	{
		ImGui::End();
		ImGui::PopStyleVar(2);
		return;
	}

	static char CurrentRugosityMax[1024];
	static float LastValue = HeatMapColorRange.GetSliderValue();
	if (IsActiveObjectAndLayerValid())
	{
		DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
		if (CurrentLayer->GetMin() == CurrentLayer->GetMax())
		{
			HeatMapColorRange.Legend.SetDummyValues();
		}
		else
		{
			if (abs(CurrentLayer->GetMax()) < 100000 && LastValue != HeatMapColorRange.GetSliderValue())
			{
				LastValue = HeatMapColorRange.GetSliderValue();
				strcpy_s(CurrentRugosityMax, UI_CORE.TruncateAfterDot(std::to_string(CurrentLayer->GetMin() + (CurrentLayer->GetMax() - CurrentLayer->GetMin()) * HeatMapColorRange.GetSliderValue())).c_str());
			}

			HeatMapColorRange.Legend.Clear();
			HeatMapColorRange.Legend.SetCaption(1.0f, "max: " + UI_CORE.TruncateAfterDot(std::to_string(CurrentLayer->GetMax())));

			HeatMapColorRange.Legend.SetCaption(HeatMapColorRange.GetSliderValue(), /*"current: " +*/ UI_CORE.TruncateAfterDot(std::to_string(CurrentLayer->GetMin() + (CurrentLayer->GetMax() - CurrentLayer->GetMin()) * HeatMapColorRange.GetSliderValue())));

			const float MiddleOfUsedRange = (HeatMapColorRange.GetSliderValue() + CurrentLayer->MinVisible / CurrentLayer->GetMax()) / 2.0f;
			HeatMapColorRange.Legend.SetCaption(0.0f, "min: " + UI_CORE.TruncateAfterDot(std::to_string(CurrentLayer->MinVisible)));

			CurrentLayer->MaxVisible = CurrentLayer->GetMin() + (CurrentLayer->GetMax() - CurrentLayer->GetMin()) * HeatMapColorRange.GetSliderValue();
		}
	}

	bool bShouldDisable = false;
	if (IsActiveObjectAndLayerValid())
	{
		DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
		if (CurrentLayer->GetType() == LAYER_TYPE::INTERPOLATION)
		{
			LayerInterpolationData* InterpolationData = CurrentLayer->GetInterpolationData();
			if (InterpolationData != nullptr)
				bShouldDisable = InterpolationData->IsMinMaxInterpolationEnabled();
		}
	}

	if (!IsActiveObjectAndLayerValid() || bShouldDisable)
		ImGui::BeginDisabled();

	ImGui::SetCursorPosX(10);
	ImGui::SetCursorPosY(642);
	ImGui::SetNextItemWidth(62);
	if (ImGui::InputText("##CurrentRugosityMax", CurrentRugosityMax, IM_ARRAYSIZE(CurrentRugosityMax), ImGuiInputTextFlags_EnterReturnsTrue) ||
		ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##CurrentRugosityMax"))
	{
		
	}

	ImGui::SameLine();
	if (ImGui::Button("Set", ImVec2(62, 19)))
	{
		DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();

		float NewValue = float(atof(CurrentRugosityMax));
		if (NewValue < ActiveLayer->GetMin())
			NewValue = ActiveLayer->GetMin();

		HeatMapColorRange.SetSliderValue((NewValue - ActiveLayer->GetMin()) / float(ActiveLayer->GetMax() - ActiveLayer->GetMin()));
	}

	if (!IsActiveObjectAndLayerValid() || bShouldDisable)
		ImGui::EndDisabled();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

void UIManager::GetUsableSpaceForLayerList(ImVec2& UsableSpaceStart, ImVec2& UsableSpaceEnd)
{
	ImGuiWindow* SettingsWindow = ImGui::FindWindowByName("Settings");
	ImGuiWindow* LegendWindow = ImGui::FindWindowByName("Heat map legend");

	UsableSpaceStart = ImVec2(0.0f, 0.0f);
	UsableSpaceEnd = ImVec2(static_cast<float>(APPLICATION.GetMainWindow()->GetWidth()), static_cast<float>(APPLICATION.GetMainWindow()->GetHeight()));
	if (SettingsWindow != nullptr && LegendWindow != nullptr)
	{
		UsableSpaceStart.x = LegendWindow->Pos.x + LegendWindow->SizeFull.x;
		UsableSpaceStart.y = 20;

		UsableSpaceEnd.x = SettingsWindow->Pos.x;
		UsableSpaceEnd.y = static_cast<float>(APPLICATION.GetMainWindow()->GetHeight() - 20);
	}
}

ImVec2 UIManager::GetLayerListButtonSize(std::string ButtonText)
{
	ImVec2 Result;

	ImVec2 TextSize = ImGui::CalcTextSize(ButtonText.c_str());
	Result.x = TextSize.x + GImGui->Style.FramePadding.x * 2.0f;
	Result.y = TextSize.y + GImGui->Style.FramePadding.y * 2.0f;

	return TextSize;
}

int UIManager::TotalWidthNeededForLayerList(int ButtonUsed)
{
	int Result = 0;
	const int ButtonSpacing = 6;
	const int FirstLastButtonPadding = 4;

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return Result;

	ButtonUsed = static_cast<int>(std::min(size_t(ButtonUsed), ActiveObject->Layers.size()));

	if (ButtonUsed == 0)
		return Result;

	Result += static_cast<int>(GetLayerListButtonSize("No Layer").x + 16);
	for (int i = 0; i < ButtonUsed; i++)
	{
		Result += static_cast<int>(GetLayerListButtonSize(ActiveObject->Layers[i]->GetCaption()).x + ButtonSpacing * 2.0f + 4);
	}

	Result += static_cast<int>(FirstLastButtonPadding * 2.0f);
	
	return Result;
}

void UIManager::RenderLayerTabs()
{
	static int RowCount = 1;
	const int ButtonSpacing = 6;
	const int FirstLastButtonPadding = 4;
	const int RowHeight = 26;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImVec2 UsableSpaceStart, UsableSpaceEnd;
	GetUsableSpaceForLayerList(UsableSpaceStart, UsableSpaceEnd);

	int UsableSpaceCenter = static_cast<int>(UsableSpaceStart.x + (UsableSpaceEnd.x - UsableSpaceStart.x) / 2.0f);
	int UsableSpaceWidth = static_cast<int>(UsableSpaceEnd.x - UsableSpaceStart.x);

	int TotalWidthNeeded = 0;
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject != nullptr)
		TotalWidthNeeded = TotalWidthNeededForLayerList(static_cast<int>(ActiveObject->Layers.size()));

	if (TotalWidthNeeded == 0)
	{
		if (ActiveObject == nullptr)
		{
			TotalWidthNeeded = static_cast<int>(ImGui::CalcTextSize(NoDataText.c_str()).x + 18);
		}
		else
		{
			TotalWidthNeeded = static_cast<int>(GetLayerListButtonSize("No Layer").x + 16);
			TotalWidthNeeded += static_cast<int>(FirstLastButtonPadding * 2.0f);
		}
	}
	else if (TotalWidthNeeded > UsableSpaceWidth - 10.0f)
	{
		TotalWidthNeeded = static_cast<int>(UsableSpaceWidth - 10.0f);
	}

	const float CurrentWindowW = static_cast<float>(TotalWidthNeeded);
	const float CurrentWindowH = 6.0f + RowHeight * RowCount;

	ImVec2 LayerListWindowPosition = ImVec2(UsableSpaceCenter - CurrentWindowW / 2.0f, 21);
	ImGui::SetNextWindowPos(LayerListWindowPosition);
	ImGui::SetNextWindowSize(ImVec2(CurrentWindowW, CurrentWindowH));
	ImGui::Begin("Layers", nullptr, ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoCollapse |
									ImGuiWindowFlags_NoScrollbar |
									ImGuiWindowFlags_NoTitleBar);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
	if (ActiveObject == nullptr)
	{
		ImVec2 TextSize = ImGui::CalcTextSize(NoDataText.c_str());

		ImGui::SetCursorPos(ImVec2(CurrentWindowW / 2.0f - TextSize.x / 2.0f, CurrentWindowH / 2.0f - TextSize.y / 2.0f));
		ImGui::Text(NoDataText.c_str());

		ImGui::PopStyleVar(3);
		ImGui::End();

		return;
	}

	int CurrentRow = 0;
	int PreviousButtonWidth = 0;
	int YPosition = static_cast<int>(ImGui::GetCursorPosY() + 7);
	for (int i = -1; i < int(ActiveObject->Layers.size()); i++)
	{
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 0.6f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.5f, 0.7f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.0f, 1.6f, 0.6f));

		if (LAYER_MANAGER.GetActiveLayerIndex() == i)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(0.0f, 1.0f, 0.5f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1.0f, 1.6f, 1.6f));
		}

		if (i == -1)
		{
			ImGui::SetCursorPosY(static_cast<float>(YPosition));
			if (ImGui::Button("No Layer"))
				ActiveObject->ClearActiveLayer();
		}
		else
		{
			if (ImGui::GetCursorPosX() + GetLayerListButtonSize(ActiveObject->Layers[i]->GetCaption()).x + ButtonSpacing * 2.0f + 4 > (UsableSpaceWidth - 10))
			{
				ImGui::SetCursorPosX(FirstLastButtonPadding * 2.0f);
				CurrentRow++;
			}

			ImGui::SetCursorPosY(static_cast<float>(YPosition + CurrentRow * RowHeight));
			if (ImGui::Button((ActiveObject->Layers[i]->GetCaption() + "##" + std::to_string(i)).c_str()))
				ActiveObject->SetActiveLayer(ActiveObject->Layers[i]->GetID());
		}
		
		ImGui::PopStyleColor(4);
	}

	RowCount = CurrentRow + 1;

	ImGui::PopStyleVar(3);
	ImGui::End();

	LayerListWindowPosition.x += CurrentWindowW / 2.0f - 48 / 2.0f;
	LayerListWindowPosition.y += CurrentWindowH;
	ImGui::SetNextWindowPos(LayerListWindowPosition);
	ImGui::SetNextWindowSize(ImVec2(48, 48));
	if (ImGui::Begin("AddNewLayerIconWindow", nullptr, ImGuiWindowFlags_NoMove |
													   ImGuiWindowFlags_NoResize |
													   ImGuiWindowFlags_NoCollapse |
													   ImGuiWindowFlags_NoScrollbar |
													   ImGuiWindowFlags_NoTitleBar |
	                                                   ImGuiWindowFlags_NoBackground))
	{

		ImVec4 Transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, Transparent);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		if (ImGui::ImageButton("AddNewLayerButton", AddNewLayerIcon->GetTextureID(), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1)))
		{
			NEW_LAYER_WINDOW.Show();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Add new layer");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
	ImGui::End();
}

void UIManager::UpdateHistogramData(DataLayer* FromLayer, int NewBinCount)
{
	std::vector<double> Values;
	std::vector<double> Weights;

	if (FromLayer->ValueWeightAndIndex.empty())
		return;

	for (const auto& Tuple : FromLayer->ValueWeightAndIndex)
	{
		Values.push_back(std::get<0>(Tuple));
		Weights.push_back(std::get<1>(Tuple));
	}

	Histogram.FillDataBins(Values, Weights, NewBinCount);
}

void UIManager::UpdateHistogramData(DataLayer* FirstLayer, DataLayer* SecondLayer, int NewBinCount)
{
	std::vector<double> Values;
	std::vector<double> Weights;

	if (FirstLayer->ValueWeightAndIndex.empty() || SecondLayer->ValueWeightAndIndex.empty())
		return;

	for (const auto& Tuple : FirstLayer->ValueWeightAndIndex)
	{
		Values.push_back(std::get<0>(Tuple));
		Weights.push_back(std::get<1>(Tuple));
	}

	for (const auto& Tuple : SecondLayer->ValueWeightAndIndex)
	{
		Values.push_back(std::get<0>(Tuple));
		Weights.push_back(std::get<1>(Tuple));
	}

	Histogram.FillDataBins(Values, Weights, NewBinCount);
}

void UIManager::RenderHistogramWindow()
{
	bool bLayerWithOneValue = false;
	if (IsActiveObjectAndLayerValid())
	{
		if (LAYER_MANAGER.GetActiveLayer()->GetMin() == LAYER_MANAGER.GetActiveLayer()->GetMax())
			bLayerWithOneValue = true;
	}

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
	if (ActiveLayer == nullptr || ActiveLayer->GetType() == LAYER_TYPE::INTERPOLATION)
		return;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());

	static float LastWindowW = 0.0f;
	static float LastWindowH = 0.0f;
	static float Epsilon = 0.001f;

	const ImGuiWindow* HistogramWindow = ImGui::FindWindowByName("Histogram");
	if (HistogramWindow != nullptr)
	{
		if (std::abs(LastWindowW - HistogramWindow->SizeFull.x) > Epsilon || std::abs(LastWindowH - HistogramWindow->SizeFull.y) > Epsilon)
		{
			Histogram.SetSize(ImVec2(HistogramWindow->SizeFull.x - 40, HistogramWindow->SizeFull.y - Histogram.GetPosition().y - 50.0f));

			HistogramSelectRegionMin.SetAvailableRange(Histogram.GetSize().x - 1);
			HistogramSelectRegionMax.SetAvailableRange(Histogram.GetSize().x - 1);

			HistogramSelectRegionMin.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMin.GetRangePosition(), 0.0f));
			HistogramSelectRegionMax.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMax.GetRangePosition(), 0.0f));

			if (ActiveMesh != nullptr &&
				bHistogramPixelBins &&
				LAYER_MANAGER.GetActiveLayerIndex() != -1)
			{
				UpdateHistogramData(LAYER_MANAGER.GetActiveLayer(), Histogram.GetBinCount());
			}

			LastWindowW = HistogramWindow->SizeFull.x;
			LastWindowH = HistogramWindow->SizeFull.y;
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Histogram", nullptr))
	{
		if (!IsActiveObjectAndLayerValid() || bLayerWithOneValue)
			ImGui::BeginDisabled();
		
		ImGui::SetCursorPos(ImVec2(12.0f, 30.0f));
		if (ImGui::Checkbox("Select region mode", &bHistogramSelectRegionMode))
		{
			if (ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
			{
				Histogram.SetPosition(ImVec2(Histogram.GetPosition().x, bHistogramSelectRegionMode ? 80.0f : 60.0f));
				Histogram.SetSize(ImVec2(HistogramWindow->SizeFull.x - 40, HistogramWindow->SizeFull.y - Histogram.GetPosition().y - 50.0f));

				if (bHistogramSelectRegionMode)
				{
					HistogramSelectRegionMin.SetStartPosition(Histogram.GetPosition());
					HistogramSelectRegionMin.SetAvailableRange(Histogram.GetSize().x - 1);

					HistogramSelectRegionMax.SetStartPosition(Histogram.GetPosition() + ImVec2(1.0f, 0.0f));
					HistogramSelectRegionMax.SetAvailableRange(Histogram.GetSize().x - 1);
					HistogramSelectRegionMax.SetRangePosition(1.0f);
					HistogramSelectRegionMax.SetPixelPosition(ImVec2(Histogram.GetSize().x, 0.0f));
				}
				else
				{
					HistogramSelectRegionMin.SetRangePosition(0.0f);
					HistogramSelectRegionMin.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMin.GetRangePosition(), 0.0f));

					HistogramSelectRegionMax.SetRangePosition(1.0f);
					HistogramSelectRegionMax.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMax.GetRangePosition(), 0.0f));

					LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMin(0.0f);
					LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMax(0.0f);
				}
			}
		}

		if (IsActiveObjectAndLayerValid() && !bLayerWithOneValue)
			Histogram.Render();

		if (bHistogramSelectRegionMode && ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
		{
			HistogramSelectRegionMin.Render();
			HistogramSelectRegionMax.Render();

			if (HistogramSelectRegionMin.GetRangePosition() + 0.001f >= HistogramSelectRegionMax.GetRangePosition())
			{
				HistogramSelectRegionMin.SetRangePosition(HistogramSelectRegionMax.GetRangePosition() - 0.001f);
				HistogramSelectRegionMin.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMin.GetRangePosition(), 0.0f));
			}

			if (HistogramSelectRegionMax.GetRangePosition() - 0.001f < HistogramSelectRegionMin.GetRangePosition())
			{
				HistogramSelectRegionMax.SetRangePosition(HistogramSelectRegionMax.GetRangePosition() + 0.001f);
				HistogramSelectRegionMax.SetPixelPosition(ImVec2(Histogram.GetSize().x * HistogramSelectRegionMax.GetRangePosition(), 0.0f));
			}

			// Show the percentage selected area/points.
			float MinValueSelected = LAYER_MANAGER.GetActiveLayer()->GetMin() + (LAYER_MANAGER.GetActiveLayer()->GetMax() - LAYER_MANAGER.GetActiveLayer()->GetMin()) * HistogramSelectRegionMin.GetRangePosition();
			float MaxValueSelected = LAYER_MANAGER.GetActiveLayer()->GetMin() + (LAYER_MANAGER.GetActiveLayer()->GetMax() - LAYER_MANAGER.GetActiveLayer()->GetMin()) * HistogramSelectRegionMax.GetRangePosition();

			glm::vec2 MinValueDistribution = UI_INSPECTOR.CalculateWeightDistributionAtValue(LAYER_MANAGER.GetActiveLayer(), MinValueSelected);
			glm::vec2 MaxValueDistribution = UI_INSPECTOR.CalculateWeightDistributionAtValue(LAYER_MANAGER.GetActiveLayer(), MaxValueSelected);

			AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
			float PercentageOfWeightSelected = 0.0f;
			std::string WeightLabel = "weight";

			if (ActiveObject != nullptr)
			{
				double TotalWeight = 0.0;
				switch (ActiveObject->GetType())
				{
					case DATA_SOURCE_TYPE::MESH:
					{
						MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
						if (CurrentMeshAnalysisData != nullptr)
						{
							TotalWeight = CurrentMeshAnalysisData->GetTotalArea();
							WeightLabel = "area";
						}
						break;
					}

					case DATA_SOURCE_TYPE::POINT_CLOUD:
					{
						PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
						if (CurrentPointCloudAnalysisData != nullptr)
						{
							TotalWeight = static_cast<double>(CurrentPointCloudAnalysisData->RawPointCloudData.size());
							WeightLabel = "points";
						}
						break;
					}
				}

				if (TotalWeight > 0.0)
				{
					double PercentageAtMax = (MaxValueDistribution.x / TotalWeight) * 100.0;
					double PercentageAtMin = (MinValueDistribution.x / TotalWeight) * 100.0;
					PercentageOfWeightSelected = static_cast<float>(PercentageAtMax - PercentageAtMin);
				}
			}

			ImGui::SetCursorPos(ImVec2(200.0f, 33.0f));
			std::string CurrentText = "Selected " + WeightLabel + ": " + UI_CORE.TruncateAfterDot(std::to_string(PercentageOfWeightSelected), 3) + " %%";
			ImGui::Text(CurrentText.c_str());
			// Show the percentage selected area/points END.

			// Render text that corresponds to the min value
			ImGui::SetCursorPos(Histogram.GetPosition() + HistogramSelectRegionMin.GetPixelPosition() - ImVec2(HistogramSelectRegionMin.GetSize() * 0.90f, HistogramSelectRegionMin.GetSize() * 1.65f));
			std::string MinValue = UI_CORE.TruncateAfterDot(std::to_string(MinValueSelected), 2);
			ImGui::Text(MinValue.c_str());

			// Line that corresponds to the min value
			ImVec2 ArrowPosition = HistogramWindow->Pos + Histogram.GetPosition() + HistogramSelectRegionMin.GetPixelPosition();
			ImGui::GetWindowDrawList()->AddRectFilled(ArrowPosition - ImVec2(1.0f, 0.0f),
													  ArrowPosition + ImVec2(1.0f, Histogram.GetSize().y - 1.0f),
													  ImColor(56.0f / 255.0f, 205.0f / 255.0f, 137.0f / 255.0f, 165.0f / 255.0f));

			// Render text that corresponds to the min value
			ImGui::SetCursorPos(Histogram.GetPosition() + HistogramSelectRegionMax.GetPixelPosition() - ImVec2(HistogramSelectRegionMax.GetSize() * 0.90f, HistogramSelectRegionMax.GetSize() * 1.65f));
			std::string MaxValue = UI_CORE.TruncateAfterDot(std::to_string(MaxValueSelected), 2);
			ImGui::Text(MaxValue.c_str());

			// Line that corresponds to the max value
			ArrowPosition = HistogramWindow->Pos + Histogram.GetPosition() + HistogramSelectRegionMax.GetPixelPosition();
			ImGui::GetWindowDrawList()->AddRectFilled(ArrowPosition - ImVec2(1.0f, 0.0f),
													  ArrowPosition + ImVec2(1.0f, Histogram.GetSize().y - 1.0f),
													  ImColor(156.0f / 255.0f, 105.0f / 255.0f, 137.0f / 255.0f, 165.0f / 255.0f));
			
			DataLayer* CurrentLayer = LAYER_MANAGER.GetActiveLayer();
			if (CurrentLayer != nullptr)
			{
				CurrentLayer->SetSelectedRangeMin(HistogramSelectRegionMin.GetRangePosition());
				CurrentLayer->SetSelectedRangeMax(HistogramSelectRegionMax.GetRangePosition());
			}
		}

		if (HistogramWindow != nullptr)
			ImGui::SetCursorPos(ImVec2(10.0f, Histogram.GetPosition().y + Histogram.GetSize().y + 20.0f));
		
		if (HistogramWindow != nullptr)
			ImGui::SetCursorPos(ImVec2(130.0f, Histogram.GetPosition().y + Histogram.GetSize().y + 20.0f));
		if (ImGui::Checkbox("BinCount = Pixels", &bHistogramPixelBins))
		{
			int NewBinCount = 128;
			if (bHistogramPixelBins)
				NewBinCount = static_cast<int>(HistogramWindow->SizeFull.x - 20);

			UpdateHistogramData(LAYER_MANAGER.GetActiveLayer(), NewBinCount);
		}

		static char CurrentBinCountChar[1024] = "128";
		if (!bHistogramPixelBins)
		{
			ImGui::SetCursorPosX(290.0f);
			if (HistogramWindow != nullptr)
				ImGui::SetCursorPosY(Histogram.GetPosition().y + Histogram.GetSize().y + 20.0f);
			ImGui::SetNextItemWidth(62);
			if (ImGui::InputText("##BinCount", CurrentBinCountChar, IM_ARRAYSIZE(CurrentBinCountChar), ImGuiInputTextFlags_EnterReturnsTrue) ||
				ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##CurrentBinCountChar"))
			{
				int TempInt = atoi(CurrentBinCountChar);
				if (TempInt <= 0)
					TempInt = 1;

				if (HistogramWindow != nullptr)
				{
					if (TempInt > HistogramWindow->SizeFull.x - 20)
						TempInt = static_cast<int>(HistogramWindow->SizeFull.x - 20);
				}

				if (Histogram.GetBinCount() < TempInt)
				{
					if (ActiveMesh != nullptr && LAYER_MANAGER.GetActiveLayerIndex() != -1)
						UpdateHistogramData(LAYER_MANAGER.GetActiveLayer(), TempInt);
				}
			}
		}

		if (!IsActiveObjectAndLayerValid() || bLayerWithOneValue)
			ImGui::EndDisabled();
	}

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

bool UIManager::GetOutputSelectionToFile()
{
	return bOutputSelectionToFile;
}

void UIManager::SetOutputSelectionToFile(const bool NewValue)
{
	bOutputSelectionToFile = NewValue;
}

void UIManager::WindowResizeCallback(int Width, int Height)
{
	if (UI.IsApplyStandardLayoutOnResize())
		UI.ApplyStandardWindowsSizeAndPosition();
}

bool UIManager::IsApplyStandardLayoutOnResize() const
{
	return bApplyStandardLayoutOnResize;
}

void UIManager::SetApplyStandardLayoutOnResize(bool NewValue)
{
	bApplyStandardLayoutOnResize = NewValue;
}

void UIManager::ApplyStandardWindowsSizeAndPosition()
{
	ImGuiWindow* Window = ImGui::FindWindowByName("Histogram");
	if (Window != nullptr)
	{
		Window->SizeFull.x = APPLICATION.GetMainWindow()->GetWidth() * 0.5f;
		Window->Pos.x = APPLICATION.GetMainWindow()->GetWidth() / 2.0f - Window->SizeFull.x / 2.0f;

		Window->SizeFull.y = APPLICATION.GetMainWindow()->GetHeight() * 0.35f;
		Window->Pos.y = APPLICATION.GetMainWindow()->GetHeight() - 10.0f - Window->SizeFull.y;
	}

	Window = ImGui::FindWindowByName("Settings");
	if (Window != nullptr)
	{
		Window->SizeFull.x = APPLICATION.GetMainWindow()->GetWidth() * 0.3f;
		Window->SizeFull.y = APPLICATION.GetMainWindow()->GetHeight() * 0.7f;
	}
}

void UIManager::ShowAboutDialog()
{
	bShouldOpenAboutWindow = true;
}

void UIManager::RenderAboutWindow()
{
	if (bShouldOpenAboutWindow)
	{
		ImGui::OpenPopup("About");
		bShouldOpenAboutWindow = false;
	}

	

	ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
	if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		int WindowW = 0;
		int WindowH = 0;
		APPLICATION.GetMainWindow()->GetSize(&WindowW, &WindowH);

		ImGui::SetWindowPos(ImVec2(WindowW / 2.0f - ImGui::GetWindowWidth() / 2.0f, WindowH / 2.0f - ImGui::GetWindowHeight() / 2.0f));

		float ContentW = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
		auto CenteredText = [ContentW](const std::string& Text) {
			ImVec2 TextSize = ImGui::CalcTextSize(Text.c_str());
			ImGui::SetCursorPosX((ContentW - TextSize.x) / 2.0f + ImGui::GetWindowContentRegionMin().x);
			ImGui::Text("%s", Text.c_str());
		};
		CenteredText(UI_CORE.GetFullVersion());

		ImGui::Separator();
		ImGui::Text("Modules:");

		CenteredText(APPLICATION.GetFullVersion());
		CenteredText(ENGINE.GetFullVersion());
		CenteredText(OBJECT_VIEWER_WINDOW.SceneGraphUI->GetFullVersion());

		ImGui::Separator();

		CenteredText("To submit a bug report or provide feedback,");
		CenteredText("please email me at kberegovyi@ccom.unh.edu.");
		CenteredText("University of New Hampshire CCOM");

		ImGui::Separator();

		float ButtonW = 210.0f;
		ImGui::SetCursorPosX((ContentW - ButtonW) / 2.0f + ImGui::GetWindowContentRegionMin().x);
		if (ImGui::Button("Close", ImVec2(ButtonW, 25.0f)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void UIManager::OnLayerChange()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	LAYER_RASTERIZATION_MANAGER.ClearAllData();

	UI.bHistogramSelectRegionMode = false;
	UI.Histogram.Clear();
	UI.HeatMapColorRange.Clear();

	DEVELOPER_MODE.ClearDebugGrid();

	DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
	if (ActiveLayer == nullptr)
		return;

	LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMin(0.0f);
	LAYER_MANAGER.GetActiveLayer()->SetSelectedRangeMax(0.0f);

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (ActiveLayer->GetMin() != ActiveLayer->GetMax())
	{
		UI.UpdateHistogramData(ActiveLayer, UI.Histogram.GetBinCount());

		if (CurrentMeshAnalysisData != nullptr)
			CurrentMeshAnalysisData->SetHeatMapType(5);
		UI.HeatMapColorRange.SetColorRangeFunction(GetTurboColorMap);
		UI.HeatMapColorRange.bRenderSlider = true;

		float MiddleOfRange = ActiveLayer->GetMin() + (ActiveLayer->GetMax() - ActiveLayer->GetMin()) / 2.0f;
		UI.HeatMapColorRange.SetSliderValue(MiddleOfRange / ActiveLayer->GetMax());

		if (ActiveLayer->GetType() == LAYER_TYPE::COMPARE)
		{
			UI.HeatMapColorRange.SetColorRangeFunction(CompareColormapValue);
			if (CurrentMeshAnalysisData != nullptr)
				CurrentMeshAnalysisData->SetHeatMapType(6);

			UI.HeatMapColorRange.bRenderSlider = false;
			UI.HeatMapColorRange.SetSliderValue(1.0f);
		}
	}
	else
	{
		if (CurrentMeshAnalysisData != nullptr)
			CurrentMeshAnalysisData->SetHeatMapType(-1);
	}
}

bool UIManager::IsActiveObjectAndLayerValid()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return false;

	if (ActiveObject->GetActiveLayer() == nullptr)
		return false;

	return true;
}

void UIManager::UpdateProgressModalPopupCurrentValue()
{
	if (bJitterCalculationsInProgress)
	{
		ProgressModalPopupCurrentValue = JITTER_MANAGER.GetProgress();
	}
	else if (bLayerRasterizationCalculationsInProgress)
	{
		ProgressModalPopupCurrentValue = LAYER_RASTERIZATION_MANAGER.GetProgress();
	}
}

void UIManager::OnLayerRasterizationCalculationsStart()
{
	UI.bLayerRasterizationCalculationsInProgress = true;
	UI.bShouldCloseProgressPopup = false;
	UI.bShouldOpenProgressPopup = true;
}

void UIManager::OnLayerRasterizationCalculationsEnd()
{
	UI.bShouldCloseProgressPopup = true;
	UI.bLayerRasterizationCalculationsInProgress = false;
}

bool UIManager::IsProgressModalPopupOpen()
{
	return !bShouldCloseProgressPopup;
}

FEWeightedHistogram* UIManager::GetHistogramPointer()
{
	return &Histogram;
}