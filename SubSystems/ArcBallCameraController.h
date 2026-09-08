#include "FocalEngine/Resources/UserScriptsData/FENativeScriptConnector.h"
using namespace FocalEngine;

// DO NOT CHANGE THIS LINE.
SET_MODULE_ID("4A7C82E16F9013ABEDC05324");

class ArcBallCameraController : public FENativeScriptCore
{
	int LastMouseX = 0;
	int LastMouseY = 0;
	bool bWasDraggingLastFrame = false;
	bool bWasPanningLastFrame = false;

	glm::dquat CurrentOrientation = glm::dquat(1.0, 0.0, 0.0, 0.0);

	bool bWasOrbitingLastFrame = false;
	double OrbitAzimuth = 0.0;

	bool GetNormalizedMousePosition(int MouseX, int MouseY, double& OutNormalizedX, double& OutNormalizedY);
	glm::dvec3 ProjectMouseToSphere(double NormalizedX, double NormalizedY);
	void UpdateOrbit(double DeltaTime);
	void UpdateViewMatrix();
public:
	void Awake() override;
	void OnUpdate(double DeltaTime) override;
	void OnDestroy() override { /* ... */ }

	float DistanceToModel = 10.0;
	glm::vec3 TargetPosition = glm::vec3(0.0f);
	float MouseWheelSensitivity = 1.0f;
	float PanSensitivity = 1.0f;

	bool bOrbitAnimation = false;
	// Degrees per second around the world up axis.
	float OrbitSpeed = 15.0f;
	// Degrees above the target plane, taken from the current view when the animation starts.
	float OrbitElevation = 30.0f;
};

REGISTER_SCRIPT(ArcBallCameraController)
RUN_IN_EDITOR_MODE(ArcBallCameraController)
REGISTER_SCRIPT_FIELD(ArcBallCameraController, float, DistanceToModel)
REGISTER_SCRIPT_FIELD(ArcBallCameraController, glm::vec3, TargetPosition)
REGISTER_SCRIPT_FIELD(ArcBallCameraController, float, MouseWheelSensitivity)
REGISTER_SCRIPT_FIELD(ArcBallCameraController, float, PanSensitivity)
REGISTER_SCRIPT_FIELD(ArcBallCameraController, bool, bOrbitAnimation)
REGISTER_SCRIPT_FIELD(ArcBallCameraController, float, OrbitSpeed)
REGISTER_SCRIPT_FIELD(ArcBallCameraController, float, OrbitElevation)