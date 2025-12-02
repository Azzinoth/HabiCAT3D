#include "UIComponents.h"

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

	AvailableRange = FLT_MAX;
}

void FEArrowScroller::Clear()
{
	StartPosition = ImVec2(0.0f, 0.0f);
	Position = ImVec2(0.0f, 0.0f);

	bSelected = false;
	bMouseHover = false;

	bWindowFlagWasAdded = false;
	OriginalWindowFlags = 0;

	LastFrameDelta = 0;
	Size = 20.0f;

	Color = ImColor(10, 10, 40, 255);
	SelectedColor = ImColor(115, 115, 255, 255);

	AvailableRange = FLT_MAX;
	RangePosition = 0.0f;
}

ImVec2 FEArrowScroller::GetPixelPosition() const
{
	return Position;
}

void FEArrowScroller::SetPixelPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;

	if (bHorizontal)
	{
		Area.left = static_cast<LONG>(GetStartPosition().x + Position.x - Size / 2.0f);
		Area.right = static_cast<LONG>(GetStartPosition().x + Position.x + Size / 2.0f);
		Area.top = static_cast<LONG>(GetStartPosition().y + Position.y - Size);
		Area.bottom = static_cast<LONG>(GetStartPosition().y + Position.y);
	}
	else
	{
		Area.left = static_cast<LONG>(GetStartPosition().x + Position.x - Size);
		Area.right = static_cast<LONG>(GetStartPosition().x + Position.x);
		Area.top = static_cast<LONG>(GetStartPosition().y + Position.y - Size / 2.0f);
		Area.bottom = static_cast<LONG>(GetStartPosition().y + Position.y + Size / 2.0f);
	}
}

ImVec2 FEArrowScroller::GetStartPosition() const
{
	return StartPosition;
}

void FEArrowScroller::SetStartPosition(ImVec2 NewValue)
{
	StartPosition = NewValue;
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
		float BottomLimitInPixels = AvailableRange * 0.0f;

		if (bHorizontal)
		{
			if (GetPixelPosition().x + LastFrameDelta <= AvailableRange - BottomLimitInPixels && GetPixelPosition().x + LastFrameDelta >= 0.0f)
			{
				SetPixelPosition(ImVec2(GetPixelPosition().x + LastFrameDelta, GetPixelPosition().y));
			}

			SetRangePosition(GetPixelPosition().x / AvailableRange);
		}
		else
		{
			if (GetPixelPosition().y + LastFrameDelta <= AvailableRange - BottomLimitInPixels && GetPixelPosition().y + LastFrameDelta >= 0.0f)
			{
				SetPixelPosition(ImVec2(GetPixelPosition().x, GetPixelPosition().y + LastFrameDelta));
			}

			SetRangePosition(GetPixelPosition().y / AvailableRange);
		}
	}
	else
	{
		SetPixelPosition(ImVec2(GetPixelPosition().x, GetPixelPosition().y));
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

float FEArrowScroller::GetAvailableRange()
{
	return AvailableRange;
}

void FEArrowScroller::SetAvailableRange(const float NewValue)
{
	AvailableRange = NewValue;
}

void FEArrowScroller::LiftRangeRestrictions()
{
	AvailableRange = FLT_MAX;
}

void FEArrowScroller::SetOrientation(const bool IsHorisontal)
{
	bHorizontal = IsHorisontal;
}

float FEArrowScroller::GetRangePosition()
{
	return RangePosition;
}

void FEArrowScroller::SetRangePosition(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	RangePosition = NewValue;
}

ImVec2 Legend::GetPosition()
{
	return Position;
}

void Legend::SetPosition(ImVec2 NewValue)
{
	Position = NewValue;
}

ImVec2 Legend::GetSize()
{
	return Size;
}

void Legend::SetSize(ImVec2 NewValue)
{
	Size = NewValue;
}

void Legend::Clear()
{
	Captions.clear();
}

void Legend::SetDummyValues()
{
	Clear();

	SetCaption(0.0, "0.0");
	SetCaption(0.25, "0.25");
	SetCaption(0.5, "0.5");
	SetCaption(0.75, "0.75");
	SetCaption(1.0, "1.0");
}

bool Legend::GetCaption(float NormalizedPosition, std::string& Result)
{
	for (size_t i = 0; i < Captions.size(); i++)
	{
		if (Captions[i].NormalizedPosition == NormalizedPosition)
		{
			Result = Captions[i].Text;
			return true;
		}
	}

	Result = "";
	return false;
}

void Legend::SetCaption(float NormalizedPosition, std::string Text)
{
	if (NormalizedPosition < 0.0f || NormalizedPosition > 1.0f)
		return;

	for (size_t i = 0; i < Captions.size(); i++)
	{
		if (Captions[i].NormalizedPosition == NormalizedPosition)
		{
			Captions[i].Text = Text;
			return;
		}
	}

	Captions.push_back(LegendItem{ NormalizedPosition, Text });
	std::sort(Captions.begin(), Captions.end(), [](const LegendItem& A, const LegendItem& B) {
		return A.NormalizedPosition < B.NormalizedPosition;
	});
}

std::vector<LegendItem> Legend::GetAllItems()
{
	return Captions;
}

void Legend::SetNormalizedPositionToVec2Impl(ImVec2(*Func)(ImVec2, ImVec2, float, std::string))
{
	NormalizedPositionToVec2Impl = Func;
}

ImVec2 Legend::NormalizedPositionToVec2(float NormalizedPosition, std::string Text)
{
	if (NormalizedPositionToVec2Impl != nullptr)
		return NormalizedPositionToVec2Impl(Position, Size, NormalizedPosition, Text);

	ImVec2 TextSize = ImGui::CalcTextSize(Text.c_str());
	return ImVec2(Position.x + Size.x * NormalizedPosition - TextSize.x / 2.0f, Position.y + Size.y);
}

void Legend::Render()
{
	for (size_t i = 0; i < Captions.size(); i++)
	{
		ImGui::SetCursorPos(NormalizedPositionToVec2(Captions[i].NormalizedPosition, Captions[i].Text));
		ImGui::Text("%s", Captions[i].Text.c_str());
	}
}

FEColorRangeAdjuster::FEColorRangeAdjuster()
{
	RangeSize = ImVec2(20, 600);
	RangePosition = ImVec2(17, 15);

	Slider.SetOrientation(false);

	Slider.SetSize(13.0f);
	Slider.SetAvailableRange(RangeSize.y - 1);
	Slider.SetStartPosition(ImVec2(15.0f, 32.0f));
	Slider.SetColor(ImColor(255, 155, 155, 255));

	Legend.SetDummyValues();
	Legend.SetNormalizedPositionToVec2Impl(FEColorRangeAdjuster::LegendCaptionsPosition);
}

ImVec2 FEColorRangeAdjuster::GetPosition() const
{
	return Position;
}

void FEColorRangeAdjuster::SetPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;
}

std::function<glm::vec3(float)> FEColorRangeAdjuster::GetColorRangeFunction()
{
	return ColorRangeFunction;
}

void FEColorRangeAdjuster::SetColorRangeFunction(std::function<glm::vec3(float)> UserFunc)
{
	ColorRangeFunction = UserFunc;
}

float FEColorRangeAdjuster::GetSliderValue()
{
	return 1.0f - Slider.GetRangePosition();
}

void FEColorRangeAdjuster::SetSliderValue(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	Slider.SetRangePosition(1.0f - NewValue);
	Slider.SetPixelPosition(ImVec2(Slider.GetPixelPosition().x, Slider.GetRangePosition() * Slider.GetAvailableRange()));
}

void FEColorRangeAdjuster::Clear()
{
	Slider.Clear();
	RangeSize = ImVec2(20, 600);
	RangePosition = ImVec2(17, 15);

	Slider.SetOrientation(false);

	Slider.SetSize(13.0f);
	Slider.SetAvailableRange(RangeSize.y - 1);
	Slider.SetStartPosition(ImVec2(15.0f, 32.0f));
	Slider.SetColor(ImColor(255, 155, 155, 255));

	Legend.SetDummyValues();
	Legend.SetNormalizedPositionToVec2Impl(FEColorRangeAdjuster::LegendCaptionsPosition);
}

ImVec2 FEColorRangeAdjuster::LegendCaptionsPosition(ImVec2 Position, ImVec2 Size, float NormalizedPosition, std::string Caption)
{
	ImVec2 Result;

	Result.x = Position.x + 10;
	Result.y = Position.y + 5 + (1.0f - NormalizedPosition) * Size.y;

	return Result;
}

void FEColorRangeAdjuster::Render(bool bScreenshotMode)
{
	float WindowX = ImGui::GetCurrentWindow()->Pos.x;
	float WindowY = ImGui::GetCurrentWindow()->Pos.y;

	ImColor CurrentColor = ImColor(155, 155, 155, 255);

	// Take max color from range and desaturate it.
	if (ColorRangeFunction != nullptr)
	{
		glm::vec3 ColorGiven = ColorRangeFunction(1.0f);
		CurrentColor = ImColor(int(ColorGiven.x * 255), int(ColorGiven.y * 255), int(ColorGiven.z * 255), 255);
		CurrentColor = ImColor(CurrentColor.Value.x * 0.5f + 0.15f, CurrentColor.Value.y * 0.5f + 0.15f, CurrentColor.Value.z * 0.5f + 0.15f);
	}

	Legend.SetPosition(RangePosition + ImVec2(15, -10));
	Legend.SetSize(RangeSize);

	if (bScreenshotMode && ImGui::GetIO().Fonts->Fonts.Size > 0)
	{
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
	}
	
	Legend.Render();

	if (bScreenshotMode && ImGui::GetIO().Fonts->Fonts.Size > 0)
	{
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}

	int UpperUnusedStart = static_cast<int>(RangeSize.y * Slider.GetRangePosition());
	if (bScreenshotMode)
		UpperUnusedStart = 0;

	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(WindowX, WindowY) + RangePosition,
		ImVec2(WindowX + RangeSize.x, WindowY + UpperUnusedStart + 1) + RangePosition,
		CurrentColor);

	for (size_t i = 0; i < RangeSize.y - UpperUnusedStart; i++)
	{
		float factor = i / float(RangeSize.y - UpperUnusedStart);

		if (ColorRangeFunction != nullptr)
		{
			glm::vec3 ColorGiven = ColorRangeFunction(factor);
			CurrentColor = ImColor(int(ColorGiven.x * 255), int(ColorGiven.y * 255), int(ColorGiven.z * 255), 255);
		}
			
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(WindowX, WindowY + RangeSize.y - i + 1) + RangePosition,
			ImVec2(WindowX + RangeSize.x, WindowY + RangeSize.y - i) + RangePosition,
			CurrentColor);
	}

	RangePosition.y = Position.y + 10;
	if (!bScreenshotMode && bRenderSlider)
		Slider.Render();
}

ImVec2 FEGraphRender::GetPosition() const
{
	return Position;
}

void FEGraphRender::SetPosition(ImVec2 NewValue)
{
	if (NewValue.x < 0)
		NewValue.x = 0;

	if (NewValue.y < 0)
		NewValue.y = 0;

	Position = NewValue;
}

ImVec2 FEGraphRender::GetSize() const
{
	return Size;
}

void FEGraphRender::SetSize(ImVec2 NewValue)
{
	if (NewValue.x < 10)
		NewValue.x = 10;

	if (NewValue.y < 10)
		NewValue.y = 10;

	Size = NewValue;

	if (!StackIDToDataPointsMap.empty())
		ColumnWidth = static_cast<int>(Size.x / StackIDToDataPointsMap.size());

	InvalidateCache();
	UpdateXLegend();
}

float FEGraphRender::GetCeiling()
{
	return Ceiling;
}

void FEGraphRender::SetCeiling(float NewValue)
{
	Ceiling = NewValue;
}

void FEGraphRender::AddDataPoints(std::vector<FEGraphDataPoint> NewDataPoints)
{
	InvalidateCache();

	if (NewDataPoints.empty())
		return;

	for (size_t i = 0; i < NewDataPoints.size(); i++)
		StackIDToDataPointsMap[NewDataPoints[i].StackID].push_back(NewDataPoints[i]);

	UpdateStackInfo(NewDataPoints);
	UpdateAfterDataPointsChange();
}

void FEGraphRender::UpdateAfterDataPointsChange()
{
	Ceiling = FLT_MAX;

	GlobalXValueBounds = glm::dvec2(DBL_MAX, -DBL_MAX);
	GlobalYValueBounds = glm::dvec2(DBL_MAX, -DBL_MAX);

	auto MapIterator = StackIDToDataPointsMap.begin();
	while (MapIterator != StackIDToDataPointsMap.end())
	{
		std::vector<FEGraphDataPoint>& DataPointsForStack = MapIterator->second;
		FEGraphStackInfo* CurrentStackInfo = GetStackInfoByID(MapIterator->first);

		double CurrentStackMinYValue = DBL_MAX;
		double CurrentStackMaxYValue = -DBL_MAX;

		for (size_t i = 0; i < DataPointsForStack.size(); i++)
		{
			GlobalXValueBounds.x = std::min(GlobalXValueBounds.x, DataPointsForStack[i].XValue);
			GlobalXValueBounds.y = std::max(GlobalXValueBounds.y, DataPointsForStack[i].XValue);

			CurrentStackMinYValue = std::min(CurrentStackMinYValue, DataPointsForStack[i].YValue);
			CurrentStackMaxYValue = std::max(CurrentStackMaxYValue, DataPointsForStack[i].YValue);

			CurrentStackInfo->XValueBounds.x = std::min(CurrentStackInfo->XValueBounds.x, DataPointsForStack[i].XValue);
			CurrentStackInfo->XValueBounds.y = std::max(CurrentStackInfo->XValueBounds.y, DataPointsForStack[i].XValue);

			CurrentStackInfo->YValueBounds.x = std::min(CurrentStackInfo->YValueBounds.x, DataPointsForStack[i].YValue);
			CurrentStackInfo->YValueBounds.y = std::max(CurrentStackInfo->YValueBounds.y, DataPointsForStack[i].YValue);
		}

		GlobalYValueBounds.x = std::min(GlobalYValueBounds.x, CurrentStackMinYValue);
		GlobalYValueBounds.y = GlobalYValueBounds.y == -DBL_MAX ? CurrentStackMaxYValue : GlobalYValueBounds.y + CurrentStackMaxYValue;

		MapIterator++;
	}

	// We have global MinXValue, MaxXValue, MinYValue, MaxYValue now we will calculate normalized positions for each stack.
	MapIterator = StackIDToDataPointsMap.begin();
	while (MapIterator != StackIDToDataPointsMap.end())
	{
		FEGraphStackInfo* CurrentStackInfo = GetStackInfoByID(MapIterator->first);

		CurrentStackInfo->XNormalizedPositionBounds.x = (CurrentStackInfo->XValueBounds.x - GlobalXValueBounds.x) / (GlobalXValueBounds.y - GlobalXValueBounds.x);
		CurrentStackInfo->XNormalizedPositionBounds.y = (CurrentStackInfo->XValueBounds.y - GlobalXValueBounds.x) / (GlobalXValueBounds.y - GlobalXValueBounds.x);

		CurrentStackInfo->YNormalizedPositionBounds.x = (CurrentStackInfo->YValueBounds.x - GlobalYValueBounds.x) / (GlobalYValueBounds.y - GlobalYValueBounds.x);
		CurrentStackInfo->YNormalizedPositionBounds.y = (CurrentStackInfo->YValueBounds.y - GlobalYValueBounds.x) / (GlobalYValueBounds.y - GlobalYValueBounds.x);
		MapIterator++;
	}

	if (GlobalYValueBounds.x != DBL_MAX && GlobalYValueBounds.y != -DBL_MAX)
		Ceiling = static_cast<float>(GlobalYValueBounds.y * 1.1);

	UpdateXLegend();
}

int FEGraphRender::CalculatePrecisionForValues(const std::vector<float>& Values)
{
	if (Values.size() < 2)
		return 2;

	// Find minimum difference between adjacent values
	float MinDifference = FLT_MAX;
	for (size_t i = 1; i < Values.size(); i++)
	{
		float Difference = std::abs(Values[i] - Values[i - 1]);
		if (Difference > 0.0f && Difference < MinDifference)
			MinDifference = Difference;
	}

	if (MinDifference == FLT_MAX || MinDifference == 0.0f)
		return 2;

	// Determine how many decimal places needed to show this difference
	int RequiredPrecision = 0;
	if (MinDifference >= 1.0f)
	{
		RequiredPrecision = 1;
	}
	else
	{
		float Magnitude = MinDifference;
		while (Magnitude < 1.0f && RequiredPrecision < 10)
		{
			Magnitude *= 10.0f;
			RequiredPrecision++;
		}

		// One extra digit for clarity.
		RequiredPrecision++; 
	}

	return std::clamp(RequiredPrecision, 1, 10);
}

std::string FEGraphRender::FormatFloatWithPrecision(float Value, int Precision)
{
	std::ostringstream Stream;
	Stream << std::fixed << std::setprecision(Precision) << Value;
	return Stream.str();
}

void FEGraphRender::UpdateXLegend()
{
	XLegend.Clear();

	std::vector<LegendItem>* Captions = &XLegend.GetAllItems();

	if (Size.x <= 0.0f)
		return;

	if (GlobalXValueBounds.x == DBL_MAX || GlobalXValueBounds.y == -DBL_MAX)
		return;

	float ValueRange = static_cast<float>(GlobalXValueBounds.y - GlobalXValueBounds.x);
	if (ValueRange == 0.0f)
		return;

	int EstimatedLabelCount = 2;
	std::vector<float> GraphXValues;
	GraphXValues.reserve(EstimatedLabelCount);

	for (int i = 0; i < EstimatedLabelCount; i++)
	{
		float NormalizedPosition = static_cast<float>(i) / (EstimatedLabelCount - 1);
		float GraphXValue = static_cast<float>(GlobalXValueBounds.x) + ValueRange * NormalizedPosition;
		GraphXValues.push_back(GraphXValue);
	}

	int RequiredPrecision = CalculatePrecisionForValues(GraphXValues);

	// Calculate actual text width for the longest label
	std::string SampleText = FormatFloatWithPrecision(static_cast<float>(GlobalXValueBounds.y), RequiredPrecision);
	ImVec2 TextSize = ImGui::CalcTextSize(SampleText.c_str());
	float MinSpacingBetweenLabels = TextSize.x + 30.0f; // Add padding between labels

	// Recalculate label count based on actual text width
	int XLegendIntervalCount = std::max(2, static_cast<int>(Size.x / MinSpacingBetweenLabels));

	// Second pass: add captions with proper spacing
	for (int i = 0; i < XLegendIntervalCount; i++)
	{
		float NormalizedPosition = static_cast<float>(i) / (XLegendIntervalCount - 1);
		float GraphXValue = static_cast<float>(GlobalXValueBounds.x + ValueRange * NormalizedPosition);
		std::string FormattedValue = FormatFloatWithPrecision(GraphXValue, RequiredPrecision);

		// Offset normalized position to center text on its tick mark
		float TextWidthNormalized = (TextSize.x * 0.5f) / Size.x;
		float AdjustedPosition = NormalizedPosition;

		// Prevent first label from going off left edge
		if (i == 0)
			AdjustedPosition = TextWidthNormalized;
		// Prevent last label from going off right edge
		else if (i == XLegendIntervalCount - 1)
			AdjustedPosition = 1.0f - TextWidthNormalized;

		XLegend.SetCaption(AdjustedPosition, FormattedValue);
	}
}

double FEGraphRender::GetGraphYValue(glm::vec2 NormizedPosition)
{
	FEGraphStackInfo* StackInfo = GetStackInfoByID(GetStackID(NormizedPosition));
	if (StackInfo == nullptr)
		return -DBL_MAX;

	FEStackBounds Bound = GetStackBoundAtX(StackInfo->ID, NormizedPosition.x);
	float CurrentStackYNormalizedBottom = Bound.Bottom;
	float CurrentStackYNormalizedTop = Bound.Top;

	double CurrentStackYNormalizedPosition = (NormizedPosition.y - CurrentStackYNormalizedBottom) / (CurrentStackYNormalizedTop - CurrentStackYNormalizedBottom);
	double CurrentStackYValueRange = (StackInfo->YValueBounds.y - StackInfo->YValueBounds.x);
	double CurrentStackYValueAtPosition = StackInfo->YValueBounds.x + CurrentStackYValueRange * CurrentStackYNormalizedPosition;

	return CurrentStackYValueAtPosition;
}

double FEGraphRender::GetNormalizedYValue(double GraphYValue)
{
	float CurrentCeiling = static_cast<float>(Ceiling == -FLT_MAX ? GlobalYValueBounds.y : Ceiling);
	return (GraphYValue - GlobalYValueBounds.x) / CurrentCeiling;
}

float FEGraphRender::GetNormalizedTotalHeightAtX(float NormalizedXPosition)
{
	float Result = 0.0f;
	if (StackIDToDataPointsMap.empty())
		return Result;

	auto MapIterator = StackIDToDataPointsMap.begin();
	while (MapIterator != StackIDToDataPointsMap.end())
	{
		int StackID = MapIterator->first;

		GraphQueryResult QueryResult = QueryGraph(NormalizedXPosition, -1.0f, StackID);
		Result += QueryResult.GraphYNormalized;

		MapIterator++;
	}

	return Result;
}

FEGraphStackInfo* FEGraphRender::GetStackInfoByID(int ID)
{
	for (size_t i = 0; i < StacksInfo.size(); i++)
	{
		if (StacksInfo[i].ID == ID)
			return &StacksInfo[i];
	}

	return nullptr;
}

int FEGraphRender::GetStackID(glm::vec2 NormizedPosition)
{
	int Result = -1;

	if (NormizedPosition.x < 0.0f || NormizedPosition.x > 1.0f ||
		NormizedPosition.y < 0.0f || NormizedPosition.y > 1.0f)
		return Result;

	std::vector<FEStackBounds> Bounds = GetStackBoundsAtX(NormizedPosition.x);
	for (const auto& CurrentBound : Bounds)
	{
		if (NormizedPosition.y >= CurrentBound.Bottom && NormizedPosition.y <= CurrentBound.Top)
			return CurrentBound.StackID;
	}

	return Result;
}

float CatmullRomInterpolation(float Point0, float Point1, float Point2, float Point3, float NormalizedT)
{
	float TSquared = NormalizedT * NormalizedT;
	float TCubed = TSquared * NormalizedT;

	float Result = 0.5f * (
		(2.0f * Point1) +
		(-Point0 + Point2) * NormalizedT +
		(2.0f * Point0 - 5.0f * Point1 + 4.0f * Point2 - Point3) * TSquared +
		(-Point0 + 3.0f * Point1 - 3.0f * Point2 + Point3) * TCubed);

	return Result;
}

GraphQueryResult FEGraphRender::QueryGraph(float XNormalized, float YNormalized, int StackID, float Tolerance)
{
	GraphQueryResult Result;

	if (StackIDToDataPointsMap.find(StackID) == StackIDToDataPointsMap.end())
		return Result;

	std::vector<FEGraphDataPoint>& DataPoints = StackIDToDataPointsMap[StackID];
	if (DataPoints.empty())
		return Result;

	FEGraphStackInfo* StackInfo = GetStackInfoByID(StackID);
	if (XNormalized < StackInfo->XNormalizedPositionBounds.x || XNormalized > StackInfo->XNormalizedPositionBounds.y)
		return Result;

	// Find left neighbor.
	int LeftNeighborIndex = 0;
	for (int j = 0; j < int(DataPoints.size()) - 1; j++)
	{
		double XValue = DataPoints[j].XValue;
		double DataPointXNormalized = (XValue - GlobalXValueBounds.x) / (GlobalXValueBounds.y - GlobalXValueBounds.x);
		if (DataPointXNormalized <= XNormalized)
		{
			LeftNeighborIndex = j;
		}
		else
		{
			break;
		}
	}

	Result.ControlPointIndex0 = std::max(0, LeftNeighborIndex - 1);
	Result.ControlPointIndex1 = LeftNeighborIndex;
	Result.ControlPointIndex2 = std::min(int(DataPoints.size() - 1), LeftNeighborIndex + 1);
	Result.ControlPointIndex3 = std::min(int(DataPoints.size() - 1), LeftNeighborIndex + 2);

	float ControlPoint1NormalizedX = static_cast<float>((DataPoints[Result.ControlPointIndex1].XValue - GlobalXValueBounds.x) / (GlobalXValueBounds.y - GlobalXValueBounds.x));
	float ControlPoint2NormalizedX = static_cast<float>((DataPoints[Result.ControlPointIndex2].XValue - GlobalXValueBounds.x) / (GlobalXValueBounds.y - GlobalXValueBounds.x));
	Result.InterpolationFactor = (ControlPoint2NormalizedX != ControlPoint1NormalizedX) ? (XNormalized - ControlPoint1NormalizedX) / (ControlPoint2NormalizedX - ControlPoint1NormalizedX) : 0.0f;
	Result.InterpolationFactor = std::clamp(Result.InterpolationFactor, 0.0f, 1.0f);

	float ControlPoint0NormalizedY = static_cast<float>(GetNormalizedYValue(DataPoints[Result.ControlPointIndex0].YValue));
	float ControlPoint1NormalizedY = static_cast<float>(GetNormalizedYValue(DataPoints[Result.ControlPointIndex1].YValue));
	float ControlPoint2NormalizedY = static_cast<float>(GetNormalizedYValue(DataPoints[Result.ControlPointIndex2].YValue));
	float ControlPoint3NormalizedY = static_cast<float>(GetNormalizedYValue(DataPoints[Result.ControlPointIndex3].YValue));
	Result.GraphYNormalized = CatmullRomInterpolation(ControlPoint0NormalizedY, ControlPoint1NormalizedY, ControlPoint2NormalizedY, ControlPoint3NormalizedY, Result.InterpolationFactor);

	// Position check (only if Y provided)
	if (YNormalized >= 0.0f)
		Result.DistanceFromGraph = YNormalized - Result.GraphYNormalized;
	
	return Result;
}

std::vector<FEStackBounds> FEGraphRender::GetStackBoundsAtX(float NormalizedX)
{
	std::vector<FEStackBounds> Result;
	float CumulativeHeight = 0.0f;

	for (const auto& StackInfo : StacksInfo)
	{
		GraphQueryResult Query = QueryGraph(NormalizedX, -1.0f, StackInfo.ID);

		FEStackBounds Bounds;
		Bounds.StackID = StackInfo.ID;
		Bounds.Bottom = CumulativeHeight;
		Bounds.Top = CumulativeHeight + Query.GraphYNormalized;

		Result.push_back(Bounds);
		CumulativeHeight = Bounds.Top;
	}

	return Result;
}

FEStackBounds FEGraphRender::GetStackBoundAtX(int StackID, float NormalizedX)
{
	std::vector<FEStackBounds> AllBounds = GetStackBoundsAtX(NormalizedX);
	for (const auto& CurrentBound : AllBounds)
	{
		if (CurrentBound.StackID == StackID)
			return CurrentBound;
	}

	return FEStackBounds();
}

void FEGraphRender::Render()
{
	ImVec2 WindowPosition = ImVec2(0.0f, 0.0f);
	ImGuiWindow* CurrentWindow = ImGui::GetCurrentWindow();
	if (CurrentWindow != nullptr)
		WindowPosition = CurrentWindow->Pos;

	int SizeX = static_cast<int>(Size.x);

	if (bCacheIsDirty)
	{
		int SizeY = static_cast<int>(Size.y);

		CacheGraph.clear();
		CacheGraph.resize(SizeX);
		
		for (size_t i = 0; i < SizeX; i++)
		{
			CacheGraph[i].resize(SizeY);
			for (size_t j = 0; j < SizeY; j++)
			{
				CacheGraph[i][j] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
	}

	InputUpdate();

	if (!StackIDToDataPointsMap.empty())
	{
		if (!bRenderOnlyDataPoints)
		{
			int GraphBottom = static_cast<int>(Size.y + Position.y);

			for (size_t i = 0; i < StacksInfo.size(); i++)
			{
				auto StackDataPoints = StackIDToDataPointsMap.find(StacksInfo[i].ID);
				int PreviousYPosition = -1;
				FEGraphStackInfo* StackInfo = GetStackInfoByID(StacksInfo[i].ID);
				ImColor CurrentColor = StackInfo != nullptr ? StackInfo->EndGradientColor : DefaultEndGradientColor;

				float CurrentStackNormalizedGlobalBottom = static_cast<float>(StackInfo->YNormalizedPositionBounds.x);
				float CurrentStackNormalizedGlobalTop = static_cast<float>(StackInfo->YNormalizedPositionBounds.y);

				for (size_t j = 0; j < Size.x; j++)
				{
					float XNormalized = j / Size.x;

					// Code to ensure proper stack order and height.
					FEStackBounds Bound = GetStackBoundAtX(StacksInfo[i].ID, XNormalized);
					float CurrentStackYNormalizedTop = Bound.Top;
					float CurrentStackYNormalizedBottom = Bound.Bottom;


					bool bShouldDraw = false;
					if (StacksInfo[i].XNormalizedPositionBounds.x > XNormalized ||
						StacksInfo[i].XNormalizedPositionBounds.y < XNormalized)
						continue;

					int XPosition = static_cast<int>(j);
					int CurrentStackYPixelTop = GraphBottom - static_cast<int>(CurrentStackYNormalizedTop * Size.y);

					if (PreviousYPosition == -1)
						PreviousYPosition = CurrentStackYPixelTop;

					// Draw vertical line from previous Y to current Y
					int YStart = std::min(PreviousYPosition, CurrentStackYPixelTop);
					int YEnd = std::max(PreviousYPosition, CurrentStackYPixelTop);

					for (int y = YStart; y <= YEnd; y++)
					{
						ImVec2 MinPosition = ImVec2(Position.x + XPosition, static_cast<float>(y));
						ImVec2 MaxPosition = ImVec2(Position.x + XPosition + 1, static_cast<float>(y + 1));
						ImGui::GetWindowDrawList()->AddRectFilled(WindowPosition + MinPosition,
							WindowPosition + MaxPosition,
							CurrentColor);
					}

					int CurrentStackYPixelBottom = GraphBottom - static_cast<int>(CurrentStackYNormalizedBottom * Size.y);
					if (bFillGraph)
					{
						for (int k = CurrentStackYPixelBottom; k > CurrentStackYPixelTop; k--)
						{
							float YNormalized = (float(GraphBottom) - float(k)) / Size.y;
							if (bUseGradientColors)
							{
								// Calculate gradient factor useing CurrentStackYNormalizedTop, CurrentStackYNormalizedBottom and YNormalized
								float GradientFactor = (YNormalized - CurrentStackNormalizedGlobalBottom) / (CurrentStackNormalizedGlobalTop - CurrentStackNormalizedGlobalBottom);
								ImColor StartGradientColor = StackInfo != nullptr ? StackInfo->StartGradientColor : DefaultStartGradientColor;
								ImColor EndGradientColor = StackInfo != nullptr ? StackInfo->EndGradientColor : DefaultEndGradientColor;

								if (isnan(GradientFactor) || isinf(GradientFactor))
									GradientFactor = 0.0f;

								if (GradientFactor < 0.0f)
									GradientFactor = 0.0f;

								if (GradientFactor > 1.0f)
									GradientFactor = 1.0f;

								CurrentColor = ImLerp(StartGradientColor.Value, EndGradientColor.Value, GradientFactor);
							}

							ImVec2 MinPosition = ImVec2(Position.x + XPosition, static_cast<float>(k - 1));
							ImVec2 MaxPosition = ImVec2(Position.x + XPosition + 1, static_cast<float>(k));
							ImGui::GetWindowDrawList()->AddRectFilled(WindowPosition + MinPosition,
																	  WindowPosition + MaxPosition,
																	  CurrentColor);
						}
					}

					PreviousYPosition = CurrentStackYPixelTop;
				}
			}
		}
		else
		{
			int GraphBottom = static_cast<int>(Size.y + Position.y);

			for (size_t i = 0; i < StacksInfo.size(); i++)
			{
				auto StackDataPoints = StackIDToDataPointsMap.find(StacksInfo[i].ID);
				int PreviousYPosition = -1;
				ImColor CurrentColor = GetStackInfoByID(StacksInfo[i].ID) != nullptr ? GetStackInfoByID(StacksInfo[i].ID)->EndGradientColor : DefaultEndGradientColor;

				for (size_t j = 0; j < StackDataPoints->second.size(); j++)
				{
					float XNormalized = static_cast<float>((StackDataPoints->second[j].XValue - GlobalXValueBounds.x) / (GlobalXValueBounds.y - GlobalXValueBounds.x));

					// Code to ensure proper stack order and height.
					std::vector<FEStackBounds> Bounds = GetStackBoundsAtX(XNormalized);
					float CurrentGraphYHeight = 0.0f;
					for (const auto& CurrentBound : Bounds)
					{
						if (CurrentBound.StackID == StacksInfo[i].ID)
						{
							CurrentGraphYHeight = CurrentBound.Top;
							break;
						}
					}

					int XPosition = static_cast<int>(XNormalized * Size.x);
					int YPosition = GraphBottom - static_cast<int>(CurrentGraphYHeight * Size.y);

					ImVec2 MinPosition = ImVec2(static_cast<float>(Position.x + XPosition), static_cast<float>(YPosition));
					ImVec2 MaxPosition = ImVec2(static_cast<float>(Position.x + XPosition + 1), static_cast<float>(YPosition + 1));
					// I need to use cached graph here.
					ImGui::GetWindowDrawList()->AddRectFilled(WindowPosition + MinPosition, WindowPosition + MaxPosition, CurrentColor);
				}
			}
		}
	}
	
	RenderXLegend();

	bCacheIsDirty = false;

	if (bIsMouseHovering)
	{
		ImVec2 WindowPosition = ImVec2(0.0f, 0.0f);
		ImGuiWindow* CurrentWindow = ImGui::GetCurrentWindow();
		if (CurrentWindow != nullptr)
			WindowPosition = CurrentWindow->Pos;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
		ImGui::BeginTooltip();

		int PixelX = static_cast<int>(ImGui::GetIO().MousePos.x - WindowPosition.x - Position.x);
		float NormilizedX = PixelX / Size.x;
		ImGui::Text("X Position: %d", PixelX);
		ImGui::Text("Normalized X Position: %.4f", NormilizedX);
		ImGui::Text("X Value: %.4f", GlobalXValueBounds.x + (GlobalXValueBounds.y - GlobalXValueBounds.x) * NormilizedX);

		ImGui::Separator();
		int PixelY = static_cast<int>(ImGui::GetIO().MousePos.y - WindowPosition.y - Position.y);
		float NormilizedY = 1.0f - (PixelY) / Size.y;
		ImGui::Text("Y Position: %d", PixelY);
		ImGui::Text("Normalized Y Position: %.4f", NormilizedY);
		double YValue = GetGraphYValue(glm::vec2(NormilizedX, NormilizedY));
		if (YValue == -DBL_MAX)
			ImGui::Text("Y Value: N/A");
		else
			ImGui::Text("Y Value: %.4f", YValue);

		ImGui::Separator();
		int StackedIDUnderMouse = GetStackID(glm::vec2(NormilizedX, NormilizedY));
		ImGui::Text("GlobaMaxYValue : %.3f", GlobalYValueBounds.y);
		ImGui::Text("Ceiling : %.3f", Ceiling);
		ImGui::Text("Stack ID under mouse: %d", StackedIDUnderMouse);
		ImGui::Separator();

		ImGui::EndTooltip();
		ImGui::PopStyleVar();
	}
}

void FEGraphRender::InputUpdate()
{
	if (ImGui::IsMouseClicked(0))
	{
		ImVec2 MousePosition = ImGui::GetMousePos();
		ImVec2 LocalMousePosition = MousePosition - ImGui::GetCurrentWindow()->Pos - Position;

		float NormalizedPosition = LocalMousePosition.x / Size.x;
		if (NormalizedPosition >= 0.0f && NormalizedPosition <= 1.0f)
		{
			for (size_t i = 0; i < MouseClickCallbacks.size(); i++)
			{
				if (MouseClickCallbacks[i] != nullptr)
					MouseClickCallbacks[i](NormalizedPosition);
			}
		}
	}

	bIsMouseHovering = false;
	if (ImGui::IsMouseHoveringRect(ImGui::GetCurrentWindow()->Pos + Position,
		ImGui::GetCurrentWindow()->Pos + Position + Size))
	{
		bIsMouseHovering = true;
	}
}

void FEGraphRender::AddMouseClickCallback(std::function<void(float)> Func)
{
	MouseClickCallbacks.push_back(Func);
}

void FEGraphRender::RenderXLegend()
{
	XLegend.SetPosition(Position);
	XLegend.SetSize(Size);

	XLegend.Render();
}

void FEGraphRender::Clear()
{
	StackIDToDataPointsMap.clear();
	StacksInfo.clear();
	XLegend.Clear();
	InvalidateCache();

	OutlineThickness = 3;
}

int FEGraphRender::GetDataPointsCount()
{
	int Result = 0;
	auto MapIterator = StackIDToDataPointsMap.begin();
	while (MapIterator != StackIDToDataPointsMap.end())
	{
		Result += static_cast<int>(MapIterator->second.size());
		MapIterator++;
	}

	return Result;
}

std::vector<FEGraphStackInfo> FEGraphRender::GetStackInfoList()
{
	return StacksInfo;
}

void FEGraphRender::InvalidateCache()
{
	bCacheIsDirty = true;
}

bool FEGraphRender::ChangeStackOrder(std::vector<int> NewOrder)
{
	if (NewOrder.size() != StacksInfo.size())
		return false;

	// Check for duplicate IDs in NewOrder.
	std::set<int> SeenIDs;
	for (size_t i = 0; i < NewOrder.size(); i++)
	{
		if (SeenIDs.find(NewOrder[i]) != SeenIDs.end())
			return false;

		SeenIDs.insert(NewOrder[i]);
	}

	// Check if NewOrder contains all existing Stack IDs.
	std::set<int> ExistingStackIDs;
	for (size_t i = 0; i < StacksInfo.size(); i++)
		ExistingStackIDs.insert(StacksInfo[i].ID);
	
	for (size_t i = 0; i < NewOrder.size(); i++)
	{
		if (ExistingStackIDs.find(NewOrder[i]) == ExistingStackIDs.end())
			return false;
	}

	std::vector<FEGraphStackInfo> NewStacksInfo;
	for (int StackID : NewOrder)
	{
		FEGraphStackInfo* Info = GetStackInfoByID(StackID);
		if (Info)
			NewStacksInfo.push_back(*Info);
	}
	StacksInfo = std::move(NewStacksInfo);

	InvalidateCache();

	return true;
}

FEGraphStackInfo FEGraphRender::GenerateStackInfo(int ID)
{
	FEGraphStackInfo Result;
	Result.ID = ID;
	Result.Name = "Stack " + std::to_string(ID);

	// Later it would be replaced with brighter color.
	Result.StartGradientColor = ImColor(11.0f / 255.0f, 11.0f / 255.0f, 11.0f / 255.0f);

	struct ColorTheme
	{
		ImColor End;
		ImColor Outline;
	};

	std::vector<ColorTheme> Palette;
	// 1. Default Blue
	Palette.push_back({ ImColor(35.0f / 255.0f, 94.0f / 255.0f, 133.0f / 255.0f), ImColor(56.0f / 255.0f, 165.0f / 255.0f, 237.0f / 255.0f) });
	// 2. Red
	Palette.push_back({ ImColor(133.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f), ImColor(237.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f) });
	// 3. Green
	Palette.push_back({ ImColor(40.0f / 255.0f, 133.0f / 255.0f, 40.0f / 255.0f), ImColor(60.0f / 255.0f, 237.0f / 255.0f, 60.0f / 255.0f) });
	// 4. Yellow/Orange
	Palette.push_back({ ImColor(133.0f / 255.0f, 133.0f / 255.0f, 40.0f / 255.0f), ImColor(237.0f / 255.0f, 237.0f / 255.0f, 60.0f / 255.0f) });
	// 5. Purple
	Palette.push_back({ ImColor(133.0f / 255.0f, 40.0f / 255.0f, 133.0f / 255.0f), ImColor(237.0f / 255.0f, 60.0f / 255.0f, 237.0f / 255.0f) });
	// 6. Cyan
	Palette.push_back({ ImColor(40.0f / 255.0f, 133.0f / 255.0f, 133.0f / 255.0f), ImColor(60.0f / 255.0f, 237.0f / 255.0f, 237.0f / 255.0f) });

	int SelectedIndex = -1;

	// Attempt to find a color theme that isn't currently used by any stack
	for (size_t i = 0; i < Palette.size(); i++)
	{
		bool bIsTaken = false;
		for (const auto& ExistingStack : StacksInfo)
		{
			// Check if the EndGradientColor matches. Casting to ImU32 ensures robust comparison.
			if ((ImU32)ExistingStack.EndGradientColor == (ImU32)Palette[i].End)
			{
				bIsTaken = true;
				break;
			}
		}

		if (!bIsTaken)
		{
			SelectedIndex = static_cast<int>(i);
			break;
		}
	}

	// If all palette colors are taken, cycle through them based on the total count
	if (SelectedIndex == -1)
		SelectedIndex = static_cast<int>(StacksInfo.size() % Palette.size());
	
	// Now we can choose better start gradient color based on the selected end color.
	Result.StartGradientColor = ImLerp(Result.StartGradientColor.Value, Palette[SelectedIndex].End.Value, 0.3f);
	Result.EndGradientColor = Palette[SelectedIndex].End;
	Result.OutlineColor = Palette[SelectedIndex].Outline;

	return Result;
}

void FEGraphRender::UpdateStackInfo(std::vector<FEGraphDataPoint> NewDataPoints)
{
	for (size_t i = 0; i < NewDataPoints.size(); i++)
	{
		bool bFound = false;
		for (size_t j = 0; j < StacksInfo.size(); j++)
		{
			if (StacksInfo[j].ID == NewDataPoints[i].StackID)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			FEGraphStackInfo NewStackInfo = GenerateStackInfo(NewDataPoints[i].StackID);
			StacksInfo.push_back(NewStackInfo);
		}
	}
}