#include "../SubSystems/ComplexityCore/Layers/RugosityLayerProducer.h"

class FEArrowScroller
{
	bool bHorizontal;

	ImVec2 StartPosition;
	ImVec2 Position;
	bool bSelected;
	bool bMouseHover;

	bool bWindowFlagWasAdded;
	int OriginalWindowFlags;

	RECT Area;
	float Size;

	ImColor Color;
	ImColor SelectedColor;

	float LastFrameMouseX;
	float LastFrameMouseY;
	float LastFrameDelta;

	float AvailableRange;
	float RangePosition = 0.0f;
	//float RangeBottomLimit = 1.0f;
public:
	FEArrowScroller(bool Horizontal = true);

	ImVec2 GetStartPosition() const;
	void SetStartPosition(ImVec2 NewValue);

	float GetSize() const;
	void SetSize(float NewValue);

	bool IsSelected() const;
	void SetSelected(bool NewValue);

	ImColor GetColor() const;
	void SetColor(ImColor NewValue);

	ImColor GetSelectedColor() const;
	void SetSelectedColor(ImColor NewValue);

	float GetLastFrameDelta() const;

	void Render();

	float GetAvailableRange();
	void SetAvailableRange(float NewValue);
	void LiftRangeRestrictions();

	void SetOrientation(bool IsHorisontal);

	float GetRangePosition();
	void SetRangePosition(float NewValue);

	float GetRangeBottomLimit();
	void SetRangeBottomLimit(float NewValue);

	ImVec2 GetPixelPosition() const;
	void SetPixelPosition(ImVec2 NewPosition);

	void Clear();
};

struct Legend
{
private:
	ImVec2 Position;
	ImVec2 Size;

	std::unordered_map<float, std::string> Captions;
	std::function<ImVec2(ImVec2, ImVec2, float, std::string)> NormalizedPositionToVec2Impl;

	ImVec2 NormalizedPositionToVec2(float NormalizedPosition, std::string Text);
public:
	struct LegendItem
	{
		float NormalizedPosition;
		std::string Text;
	};

	ImVec2 GetPosition();
	void SetPosition(ImVec2 NewValue);

	ImVec2 GetSize();
	void SetSize(ImVec2 NewValue);

	void Clear();
	void SetDummyValues();
	bool GetCaption(float NormalizedPosition, std::string& Result);
	void SetCaption(float NormalizedPosition, std::string Text);
	std::vector<LegendItem> GetAllItems();

	void SetNormalizedPositionToVec2Impl(ImVec2(*Func)(ImVec2, ImVec2, float, std::string));

	void Render();
};

class FEColorRangeAdjuster
{
	ImVec2 Position;
	FEArrowScroller Slider;

	ImVec2 RangeSize;
	ImVec2 RangePosition;

	std::function<ImColor(float)> ColorRangeFunction;
	static ImVec2 LegendCaptionsPosition(ImVec2 Position, ImVec2 Size, float NormalizedPosition, std::string Caption);
public:
	Legend Legend;
	bool bRenderSlider = true;

	FEColorRangeAdjuster();

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewPosition);

	std::function<ImColor(float)> GetColorRangeFunction();
	void SetColorRangeFunction(std::function<ImColor(float)> UserFunc);

	float GetSliderValue();
	void SetSliderValue(float NewValue);

	float GetRangeBottomLimit();
	void SetRangeBottomLimit(float NewValue);

	void Render(bool bScreenshotMode);
	void Clear();
};

class FEGraphRender
{
	ImVec2 Position = ImVec2(10, 10);
	ImVec2 Size = ImVec2(100, 100);

	std::vector<float> DataPonts;
	std::vector<double> NormalizedDataPonts;

	int ColumnWidth = 3;
	float Ceiling = 1.0f;

	std::vector<double> NormalizeArray(std::vector<float> Array);

	float GetValueAtPosition(float NormalizedPosition);

	ImVec2 GraphCanvasPosition = ImVec2(0, 0);
	ImVec2 GraphCanvasSize = ImVec2(50, 50);

	bool bInterpolation = true;

	ImColor StartGradientColor = ImColor(11.0f / 255.0f, 11.0f / 255.0f, 11.0f / 255.0f);
	ImColor EndGradientColor = ImColor(35.0f / 255.0f, 94.0f / 255.0f, 133.0f / 255.0f);
	ImColor OutlineColor = ImColor(56.0f / 255.0f, 165.0f / 255.0f, 237.0f / 255.0f);

	float GraphHeightAtPixel(int PixelX);
	void RenderOneColumn(int XPosition, ImVec2 WindowPosition);
	int OutlineThickness = 2;
	bool ShouldOutline(int XPosition, int YPosition);

	void RenderBottomLegend();

	void InputUpdate();
	std::vector<std::function<void(float)>> MouseClickCallbacks;

	std::vector<std::vector<ImColor>> CacheGraph;
	bool bCacheIsDirty = true;
public:
	Legend Legend;

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewValue);

	ImVec2 GetSize() const;
	void SetSize(ImVec2 NewValue);

	float GetCeiling();
	void SetCeiling(float NewValue);

	int GetDataPointsCount();
	std::vector<float> GetDataPoints() const;
	void SetDataPoints(std::vector<float> NewValue);

	bool IsUsingInterpolation();
	void SetIsUsingInterpolation(bool NewValue);

	void Render();
	void Clear();

	void AddMouseClickCallback(std::function<void(float)> Func);
};