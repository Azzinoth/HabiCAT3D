#pragma once

#include "UICore.h"

class SettingsWindow
{
	friend class UIManager;
public:
	SINGLETON_PUBLIC_PART(SettingsWindow)

	void Render();

	bool GetWireFrameMode();
	void SetWireFrameMode(bool NewValue);

	float GetAmbientLightFactor();
	void SetAmbientLightFactor(float NewValue);

	bool IsInModelCameraMode();
	void SetIsModelCamera(bool NewValue, glm::vec3 ModelCameraFocusPoint = glm::vec3(0.0f));
	void SwitchCameraMode(bool bModelCamera, glm::vec3 ModelCameraFocusPoint = glm::vec3(0.0f));

	void FocusCameraOnObject(AnalysisObject* Object = nullptr);

	void ShowCameraTransform();
	std::string CameraPositionToString();
	void StringToCameraPosition(std::string Text);

	std::string CameraRotationToString();
	void StringToCameraRotation(std::string Text);
private:
	SINGLETON_PRIVATE_PART(SettingsWindow)

	bool bVisible = false;

	bool bModelCamera = true;
	bool bChooseCameraFocusPointMode = false;

	bool bWireframeMode = false;
	float AmbientLightFactor = 2.2f;

	void AdjustCameraNearFarPlanes();
	void ModelCameraAdjustment(AnalysisObject* Object = nullptr);
	void FreeCameraAdjustment(AnalysisObject* Object = nullptr);
};

#define SETTINGS_WINDOW SettingsWindow::GetInstance()