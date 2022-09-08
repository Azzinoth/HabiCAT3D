#pragma once

#include "FECGALWrapper.h"

namespace FocalEngine
{
	class UIManager
	{
	public:
		SINGLETON_PUBLIC_PART(UIManager)

		void showTransformConfiguration(std::string name, FETransformComponent* transform);
		void showCameraTransform();

		void SetCamera(FEFreeCamera* newCamera);
		void SetMeshShader(FEShader* newShader);

		void RenderMainWindow(FEMesh* currentMesh);

		bool GetWireFrameMode();
		void SetWireFrameMode(bool NewValue);

		bool GetDeveloperMode();
		void SetDeveloperMode(bool NewValue);
	private:
		SINGLETON_PRIVATE_PART(UIManager)

		FEFreeCamera* currentCamera = nullptr;
		FEShader* meshShader = nullptr;

		bool wireframeMode = false;
		float TimeTookToJitter = 0.0f;

		bool DeveloperMode = false;
	};

	#define UI UIManager::getInstance()
}
