#pragma once

#include "FECGALWrapper.h"

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

		FEMesh* TestMesh = nullptr;
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
		//void TestCGALVariant();
	};

	#define UI UIManager::getInstance()
}
