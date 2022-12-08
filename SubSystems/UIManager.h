#pragma once

#include "ExcelWrapper.h"

namespace FocalEngine
{
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
		float RangeBottomLimit = 1.0f;
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
	};

	class FEColorRangeAdjuster
	{
		ImVec2 Position;
		FEArrowScroller Ceiling;

		ImVec2 RangeSize;
		ImVec2 RangePosition;

		std::function<ImColor(float)> ColorRangeFunction;
	public:
		FEColorRangeAdjuster();

		ImVec2 GetPosition() const;
		void SetPosition(ImVec2 NewPosition);

		std::function<ImColor(float)> GetColorRangeFunction();
		void SetColorRangeFunction(std::function<ImColor(float)> UserFunc);

		float GetCeilingValue();
		void SetCeilingValue(float NewValue);

		float GetRangeBottomLimit();
		void SetRangeBottomLimit(float NewValue);

		void Render();

		std::unordered_map<float, std::string> RangeValueLabels;
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
		void RenderOneColumn(int XPosition);
		int OutlineThickness = 5;
		bool ShouldOutline(int XPosition, int YPosition);

		void RenderBottomLegend();
	public:
		ImVec2 GetPosition() const;
		void SetPosition(ImVec2 NewValue);

		ImVec2 GetSize() const;
		void SetSize(ImVec2 NewValue);

		float GetCeiling();
		void SetCeiling(float NewValue);

		std::vector<float> GetDataPoints() const;
		void SetDataPoints(std::vector<float> NewValue);

		bool IsUsingInterpolation();
		void SetIsUsingInterpolation(bool NewValue);

		void Render();
	};

	class UIManager
	{
	public:
		SINGLETON_PUBLIC_PART(UIManager)

		void showTransformConfiguration(std::string name, FETransformComponent* transform);
		void showCameraTransform();

		void SetCamera(FEBasicCamera* newCamera);
		void SetMeshShader(FEShader* newShader);

		void RenderMainWindow(FEMesh* currentMesh);

		bool GetWireFrameMode();
		void SetWireFrameMode(bool NewValue);

		bool GetDeveloperMode();
		void SetDeveloperMode(bool NewValue);

		std::string CameraPositionToStr();
		void StrToCameraPosition(std::string text);

		std::string CameraRotationToStr();
		void StrToCameraRotation(std::string text);

		void updateCurrentMesh(FEMesh* NewMesh);

		float GetAreaToMeasureRugosity();
		void SetAreaToMeasureRugosity(float NewValue);

		int GetRugositySelectionMode();
		void SetRugositySelectionMode(int NewValue);

		bool GetIsModelCamera();
		void SetIsModelCamera(bool NewValue);

		//FEMesh* TestMesh = nullptr;
		static void(*SwapCameraImpl)(bool);
	private:
		SINGLETON_PRIVATE_PART(UIManager)

		FEBasicCamera* currentCamera = nullptr;
		FEShader* meshShader = nullptr;

		bool wireframeMode = false;
		float TimeTookToJitter = 0.0f;

		bool DeveloperMode = false;
		bool bModelCamera = true;
		FEMesh* currentMesh = nullptr;

		float AreaToMeasureRugosity = 1.0f;
		int RugositySelectionMode = 0;

		FEColorRangeAdjuster RugosityColorRange;

		void RenderLegend();
		void ShowRugosityRangeSettings();
		void RenderVisualModeWindow();

		FEGraphRender Graph;
		void FillGraphDataPoints(int BinsCount);
		void RenderRugosityHistogram();
		float FillGraphDataPoints_TotalTime = 0.0f;
		float SetDataPoints = 0.0f;
		float AreaWithRugosities_TotalTime = 0.0f;
		
		static void OnRugosityCalculationsEnd();
		//void TestCGALVariant();
	};

	#define UI UIManager::getInstance()
}
