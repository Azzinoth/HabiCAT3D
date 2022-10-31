#include "FEModelViewCamera.h"
using namespace FocalEngine;

FEModelViewCamera::FEModelViewCamera(std::string name) : FEBasicCamera (name)
{

}

FEModelViewCamera::~FEModelViewCamera()
{

}

void FEModelViewCamera::move(float deltaTime)
{
	updateViewMatrix();

	if (clientOnUpdateImpl)
		clientOnUpdateImpl(this);
}

void FEModelViewCamera::reset()
{
	CurrentPolarAngle = 90.0;
	CurrentAzimutAngle = 90.0;

	FEBasicCamera::reset();
}

void FEModelViewCamera::mouseMoveInput(double xpos, double ypos)
{
	int mouseX = int(xpos);
	int mouseY = int(ypos);

	if (!isInputActive)
	{
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		return;
	}

	if (lastMouseX != mouseX)
	{
		CurrentAzimutAngle += (mouseX - lastMouseX) * 0.1f;
	}

	if (lastMouseY != mouseY )
	{
		CurrentPolarAngle -= (mouseY - lastMouseY) * 0.1f;
	}

	glm::vec3 NewPosition = PolarToCartesian(CurrentPolarAngle, CurrentAzimutAngle, 40.0);


	setPosition(NewPosition);

	lastMouseX = mouseX;
	lastMouseY = mouseY;
}

void FEModelViewCamera::keyboardInput(int key, int scancode, int action, int mods)
{
	if (!isInputActive)
		return;
}

glm::dvec3 FEModelViewCamera::PolarToCartesian(double PolarAngle, double AzimutAngle, double R)
{
	PolarAngle *= glm::pi<double>() / 180.0;
	AzimutAngle *= glm::pi<double>() / 180.0;

	double X = R * sin(PolarAngle) * cos(AzimutAngle);
	double Y = R * sin(PolarAngle) * sin(AzimutAngle);
	double Z = R * cos(PolarAngle);

	return glm::dvec3(X, Z, Y);
}

void FEModelViewCamera::updateViewMatrix()
{
	viewMatrix = glm::mat4(1.0f);

	position = PolarToCartesian(CurrentPolarAngle, CurrentAzimutAngle, DistancetoModel);
	viewMatrix = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0, 1, 0));
}

double FEModelViewCamera::GetDistanceToModel()
{
	return DistancetoModel;
}

void FEModelViewCamera::SetDistanceToModel(double NewValue)
{
	if (NewValue < 0.0)
		NewValue = 0.1;

	DistancetoModel = NewValue;
	updateViewMatrix();
}