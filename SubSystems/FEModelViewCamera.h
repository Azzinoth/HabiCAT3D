#pragma once

#include "FEBasicCamera.h"

namespace FocalEngine
{
	class FEModelViewCamera : public FEBasicCamera
	{
	public:
		FEModelViewCamera(std::string name);
		~FEModelViewCamera();

		void keyboardInput(int key, int scancode, int action, int mods) final;
		void mouseMoveInput(double xpos, double ypos) final;
		void move(float deltaTime = 0.0f);

		void reset() override;
		void updateViewMatrix();

		double CurrentPolarAngle = 90.0;
		double CurrentAzimutAngle = 90.0;

		double GetDistanceToModel();
		void SetDistanceToModel(double NewValue);
	private:
		int lastMouseX = 0;
		int lastMouseY = 0;

		double DistancetoModel = 10.0;
		glm::dvec3 PolarToCartesian(double PolarAngle, double AzimutAngle, double R = 1.0);
	};
}