#include "UICore.h"

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

	ImVec2 GetPixelPosition() const;
	void SetPixelPosition(ImVec2 NewPosition);

	void Clear();
};

struct LegendItem
{
	float NormalizedPosition = 0.0f;
	std::string Text;
};

struct Legend
{
private:
	ImVec2 Position;
	ImVec2 Size;

	std::vector<LegendItem> Captions;
	std::function<ImVec2(ImVec2, ImVec2, float, std::string)> NormalizedPositionToVec2Impl;

	ImVec2 NormalizedPositionToVec2(float NormalizedPosition, std::string Text);
public:

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

	std::function<glm::vec3(float)> ColorRangeFunction;
	static ImVec2 LegendCaptionsPosition(ImVec2 Position, ImVec2 Size, float NormalizedPosition, std::string Caption);
public:
	Legend Legend;
	bool bRenderSlider = true;

	FEColorRangeAdjuster();

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewPosition);

	std::function<glm::vec3(float)> GetColorRangeFunction();
	void SetColorRangeFunction(std::function<glm::vec3(float)> UserFunc);

	float GetSliderValue();
	void SetSliderValue(float NewValue);

	void Render(bool bScreenshotMode);
	void Clear();
};

struct FEGraphDataPoint
{
	double XValue = 0.0;
	double YValue = 0.0;
	int StackID = 0;
};

struct FEGraphStackInfo
{
	int ID = 0;
	ImColor StartGradientColor = ImColor(11.0f / 255.0f, 11.0f / 255.0f, 11.0f / 255.0f);
	ImColor EndGradientColor = ImColor(35.0f / 255.0f, 94.0f / 255.0f, 133.0f / 255.0f);
	ImColor OutlineColor = ImColor(56.0f / 255.0f, 165.0f / 255.0f, 237.0f / 255.0f);
	std::string Name = "Stack";

	glm::dvec2 XNormalizedPositionBounds = glm::dvec2(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
	glm::dvec2 YNormalizedPositionBounds = glm::dvec2(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());

	glm::dvec2 XValueBounds = glm::dvec2(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
	glm::dvec2 YValueBounds = glm::dvec2(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
};

struct GraphQueryResult
{
	int ControlPointIndex0 = -1;
	int ControlPointIndex1 = -1;
	int ControlPointIndex2 = -1;
	int ControlPointIndex3 = -1;

	float InterpolationFactor = 0.0f;
	float GraphYNormalized = 0.0f;
	float DistanceFromGraph = 0.0f;
};

struct FEStackBounds
{
	int StackID = -1;
	float Bottom = 0.0f;
	float Top = 0.0f;
};

class FEGraphRender
{
	std::unordered_map<int, std::vector<FEGraphDataPoint>> StackIDToDataPointsMap;

	glm::dvec2 GlobalXValueBounds = glm::dvec2(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
	glm::dvec2 GlobalYValueBounds = glm::dvec2(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());

	std::vector<FEStackBounds> GetStackBoundsAtX(float NormalizedX);
	FEStackBounds GetStackBoundAtX(int StackID, float NormalizedX);

	int GetStackID(glm::vec2 NormizedPosition);

	std::vector<FEGraphStackInfo> StacksInfo;

	double GetGraphYValue(glm::vec2 NormizedPosition);
	double GetNormalizedYValue(double GraphYValue);

	ImVec2 Position = ImVec2(10, 10);
	ImVec2 Size = ImVec2(100, 100);

	int ColumnWidth = 3;
	float Ceiling = 1.0f;

	void UpdateAfterDataPointsChange();

	float GetNormalizedTotalHeightAtX(float NormalizedXPosition);

	ImVec2 GraphCanvasPosition = ImVec2(0, 0);
	ImVec2 GraphCanvasSize = ImVec2(50, 50);

	bool bCacheIsDirty = true;
	std::vector<std::vector<ImColor>> CacheGraph;
	bool bIsMouseHovering = false;

	int OutlineThickness = 3;

	bool bRenderOnlyDataPoints = false;
	bool bUseGradientColors = true;
	bool bFillGraph = true;
	
	bool bOutlineGraph = false;

	ImColor DefaultStartGradientColor = ImColor(11.0f / 255.0f, 11.0f / 255.0f, 11.0f / 255.0f);
	ImColor DefaultEndGradientColor = ImColor(35.0f / 255.0f, 94.0f / 255.0f, 133.0f / 255.0f);
	ImColor DefaultOutlineColor = ImColor(56.0f / 255.0f, 165.0f / 255.0f, 237.0f / 255.0f);

	void RenderXLegend();

	void InputUpdate();
	std::vector<std::function<void(float)>> MouseClickCallbacks;

	FEGraphStackInfo GenerateStackInfo(int ID);
	void UpdateStackInfo(std::vector<FEGraphDataPoint> NewDataPoints);

	int DebugClosestDataPointIndex = -1;
	GraphQueryResult QueryGraph(float XNormalized, float YNormalized = -1.0f, int StackID = 0, float Tolerance = 0.01f);

	int CalculatePrecisionForValues(const std::vector<float>& Values);
	std::string FormatFloatWithPrecision(float Value, int Precision);
public:
	Legend XLegend;
	void UpdateXLegend();

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewValue);

	ImVec2 GetSize() const;
	void SetSize(ImVec2 NewValue);

	float GetCeiling();
	void SetCeiling(float NewValue);

	int GetDataPointsCount();
	void AddDataPoints(std::vector<FEGraphDataPoint> NewDataPoints);

	void Render();
	void Clear();
	void InvalidateCache();

	void AddMouseClickCallback(std::function<void(float)> Func);

	std::vector<FEGraphStackInfo> GetStackInfoList();
	FEGraphStackInfo* GetStackInfoByID(int ID);
	bool ChangeStackOrder(std::vector<int> NewOrder);
};