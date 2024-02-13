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
	//RangeBottomLimit = 1.0f;
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
		float BottomLimitInPixels = AvailableRange * 0.0f/** (1.0f - RangeBottomLimit)*/;

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

float FEArrowScroller::GetRangeBottomLimit()
{
	return 0.0f;
	//return RangeBottomLimit;
}

void FEArrowScroller::SetRangeBottomLimit(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	//RangeBottomLimit = NewValue;
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
	if (Captions.find(NormalizedPosition) == Captions.end())
	{
		Result = "";
		return false;
	}

	Result = Captions[NormalizedPosition];
	return true;
}

void Legend::SetCaption(float NormalizedPosition, std::string Text)
{
	if (NormalizedPosition < 0.0f || NormalizedPosition > 1.0f)
		return;

	Captions[NormalizedPosition] = Text;
}

std::vector<Legend::LegendItem> Legend::GetAllItems()
{
	std::vector<Legend::LegendItem> Result;
	Result.resize(Captions.size());

	size_t Index = 0;
	auto LegendIt = Captions.begin();
	while (LegendIt != Captions.end())
	{
		Result[Index].NormalizedPosition = LegendIt->first;
		Result[Index].Text = LegendIt->second;

		LegendIt++;
	}

	return Result;
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
	auto LegendIt = Captions.begin();
	while (LegendIt != Captions.end())
	{
		ImGui::SetCursorPos(NormalizedPositionToVec2(LegendIt->first, LegendIt->second));
		ImGui::Text(LegendIt->second.c_str());

		LegendIt++;
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

std::function<ImColor(float)> FEColorRangeAdjuster::GetColorRangeFunction()
{
	return ColorRangeFunction;
}

void FEColorRangeAdjuster::SetColorRangeFunction(std::function<ImColor(float)> UserFunc)
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

float FEColorRangeAdjuster::GetRangeBottomLimit()
{
	return 1.0f - Slider.GetRangeBottomLimit();
}

void FEColorRangeAdjuster::SetRangeBottomLimit(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	Slider.SetRangeBottomLimit(1.0f - NewValue);
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
		CurrentColor = ColorRangeFunction(1.0f);
		CurrentColor = ImColor(CurrentColor.Value.x * 0.5f + 0.15f, CurrentColor.Value.y * 0.5f + 0.15f, CurrentColor.Value.z * 0.5f + 0.15f);
	}

	Legend.SetPosition(RangePosition + ImVec2(15, -10));
	Legend.SetSize(RangeSize);

	if (bScreenshotMode)
	{
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
	}
	
	Legend.Render();

	if (bScreenshotMode)
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
			CurrentColor = ColorRangeFunction(factor);
			
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

	if (!DataPonts.empty())
	{
		ColumnWidth = Size.x / DataPonts.size();
	}

	bCacheIsDirty = true;
}

std::vector<double> FEGraphRender::NormalizeArray(std::vector<float> Array)
{
	std::vector<double> Result;

	double MinValue = DBL_MAX;
	double MaxValue = -DBL_MAX;

	for (size_t i = 0; i < Array.size(); i++)
	{
		MinValue = std::min(MinValue, double(Array[i]));
		MaxValue = std::max(MaxValue, double(Array[i]));
	}

	float CurrentCeiling = Ceiling == -FLT_MAX ? MaxValue : Ceiling;
	for (size_t i = 0; i < Array.size(); i++)
	{
		Result.push_back((Array[i] - MinValue) / CurrentCeiling);
	}

	return Result;
}

float FEGraphRender::GetCeiling()
{
	return Ceiling;
}

void FEGraphRender::SetCeiling(float NewValue)
{
	Ceiling = NewValue;
	NormalizedDataPonts = NormalizeArray(DataPonts);
}

std::vector<float> FEGraphRender::GetDataPoints() const
{
	return DataPonts;
}

void FEGraphRender::SetDataPoints(std::vector<float> NewValue)
{
	Ceiling = -FLT_MAX;
	DataPonts = NewValue;
	NormalizedDataPonts = NormalizeArray(DataPonts);
	bCacheIsDirty = true;
}

float FEGraphRender::GetValueAtPosition(float NormalizedPosition)
{
	if (DataPonts.empty())
		return 0.0f;

	float NormalizedDataPointWidth = 1.0f / DataPonts.size();
	float CurrentPosition = 0.0f;

	for (int i = 0; i < DataPonts.size(); i++)
	{
		if (NormalizedPosition >= CurrentPosition && NormalizedPosition < CurrentPosition + NormalizedDataPointWidth)
		{
			float NormalizedPositionBetweenDataPoints = (NormalizedPosition - CurrentPosition) / NormalizedDataPointWidth;

			float result = 0.0f;
			if (bInterpolation)
			{
				if (i + 1 >= DataPonts.size())
				{
					result = NormalizedDataPonts[i];
					return result;
				}
				result = NormalizedDataPonts[i] * (1.0f - NormalizedPositionBetweenDataPoints) + NormalizedDataPonts[i + 1] * NormalizedPositionBetweenDataPoints;
			}
			else
			{
				result = NormalizedDataPonts[i];
			}

			return result;
		}

		CurrentPosition += NormalizedDataPointWidth;
	}
}

float FEGraphRender::GraphHeightAtPixel(int PixelX)
{
	if (PixelX < 0 || PixelX > Size.x)
		return -1.0f;

	int GraphBottom = Size.y + Position.y;

	float NormalizedPosition = PixelX / Size.x;
	if (NormalizedPosition > 1.0f)
		NormalizedPosition = 1.0f;

	float ValueAtThisPixel = GetValueAtPosition(NormalizedPosition);
	float ColumnHeight = GraphBottom - float(Size.y) * ValueAtThisPixel;

	return ColumnHeight;
}

bool FEGraphRender::ShouldOutline(int XPosition, int YPosition)
{
	if (YPosition < GraphHeightAtPixel(XPosition) + OutlineThickness)
		return true;

	for (int i = -OutlineThickness; i < OutlineThickness; i++)
	{
		if (YPosition <= GraphHeightAtPixel(XPosition + i) + OutlineThickness)
			return true;
	}

	return false;
}

void FEGraphRender::RenderOneColumn(int XPosition, ImVec2 WindowPosition)
{
	int GraphBottom = Size.y + Position.y;

	if (bCacheIsDirty)
	{
		int ColumnTop = GraphHeightAtPixel(XPosition);

		if (abs(GraphBottom - ColumnTop) == 0)
		{
			ColumnTop -= 1;
		}
		else if (ColumnTop < Position.y)
		{
			ColumnTop = Position.y + OutlineThickness + 1;
		}

		int CurrentIndex = 0;
		for (int i = GraphBottom; i > ColumnTop; i--)
		{
			float CombineFactor = (float(GraphBottom) - float(i)) / (float(GraphBottom) - float(Position.y));

			ImColor FirstPart = ImColor(EndGradientColor.Value.x * CombineFactor,
				EndGradientColor.Value.y * CombineFactor,
				EndGradientColor.Value.z * CombineFactor);

			ImColor SecondPart = ImColor(StartGradientColor.Value.x * (1.0f - CombineFactor),
				StartGradientColor.Value.y * (1.0f - CombineFactor),
				StartGradientColor.Value.z * (1.0f - CombineFactor));

			ImColor CurrentColor = ImColor(FirstPart.Value.x + SecondPart.Value.x,
				FirstPart.Value.y + SecondPart.Value.y,
				FirstPart.Value.z + SecondPart.Value.z);

			// Outline of graph
			if (ShouldOutline(XPosition, i))
				CurrentColor = OutlineColor;

			CacheGraph[XPosition][CurrentIndex++] = CurrentColor;
		}
	}
	else
	{
		int CurrentIndex = 0;
		for (int i = GraphBottom; i > Position.y; i--)
		{
			ImColor CurrentColor = CacheGraph[XPosition][CurrentIndex++];

			if (CurrentColor.Value == ImColor(0.0f, 0.0f, 0.0f, 0.0f).Value)
				break;

			ImVec2 MinPosition = ImVec2(Position.x + XPosition, i - 1);
			ImVec2 MaxPosition = ImVec2(Position.x + XPosition + 1, i);
			ImGui::GetWindowDrawList()->AddRectFilled(WindowPosition + MinPosition,
													  WindowPosition + MaxPosition,
													  CurrentColor);
		}
	}
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

	for (int i = 0; i < SizeX; i++)
	{
		RenderOneColumn(i, WindowPosition);
	}

	RenderBottomLegend();

	bCacheIsDirty = false;

	//ImGuiWindow* CurrentWindow = ImGui::GetCurrentWindow();
	//if (CurrentWindow != nullptr)
	//{
	//	// Debug functionality
	//	ImGui::GetWindowDrawList()->AddRectFilled(CurrentWindow->Pos + Position + GraphCanvasPosition,
	//											  CurrentWindow->Pos + Position + Size,
	//											  ImColor(56.0f / 255.0f, 165.0f / 255.0f, 237.0f / 255.0f, 45.0f / 255.0f));
	//}
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
}

void FEGraphRender::AddMouseClickCallback(std::function<void(float)> Func)
{
	MouseClickCallbacks.push_back(Func);
}

bool FEGraphRender::IsUsingInterpolation()
{
	return bInterpolation;
}

void FEGraphRender::SetIsUsingInterpolation(bool NewValue)
{
	bInterpolation = NewValue;
	bCacheIsDirty = true;
}

void FEGraphRender::RenderBottomLegend()
{
	Legend.SetPosition(Position);
	Legend.SetSize(Size);

	Legend.Render();
}

void FEGraphRender::Clear()
{
	DataPonts.clear();
	NormalizedDataPonts.clear();

	OutlineThickness = 2;
	bInterpolation = true;

	Legend.Clear();
	bCacheIsDirty = true;
}

int FEGraphRender::GetDataPointsCount()
{
	return DataPonts.size();
}