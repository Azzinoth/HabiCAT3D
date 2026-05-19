#include "ArcBallCameraController.h"

void ArcBallCameraController::Awake()
{

}

bool ArcBallCameraController::GetNormalizedMousePosition(int MouseX, int MouseY, double& OutNormalizedX, double& OutNormalizedY)
{
	if (ParentEntity == nullptr || !ParentEntity->HasComponent<FECameraComponent>())
		return false;

	FECameraComponent& CameraComponent = ParentEntity->GetComponent<FECameraComponent>();
	const FEViewport* Viewport = CameraComponent.GetViewport();
	if (Viewport == nullptr)
		return false;

	const double ViewportWidth = static_cast<double>(Viewport->GetWidth());
	const double ViewportHeight = static_cast<double>(Viewport->GetHeight());
	const double ViewportCenterX = Viewport->GetX() + ViewportWidth * 0.5;
	const double ViewportCenterY = Viewport->GetY() + ViewportHeight * 0.5;
	const double SphereRadius = 0.5 * (ViewportWidth < ViewportHeight ? ViewportWidth : ViewportHeight);
	if (SphereRadius <= 0.0)
		return false;

	OutNormalizedX = (MouseX - ViewportCenterX) / SphereRadius;
	OutNormalizedY = -(MouseY - ViewportCenterY) / SphereRadius;
	return true;
}

glm::dvec3 ArcBallCameraController::ProjectMouseToSphere(double NormalizedX, double NormalizedY)
{
	const double DistanceFromCenterSquared = NormalizedX * NormalizedX + NormalizedY * NormalizedY;
	if (DistanceFromCenterSquared <= 0.5)
		return glm::dvec3(NormalizedX, NormalizedY, sqrt(1.0 - DistanceFromCenterSquared));

	return glm::normalize(glm::dvec3(NormalizedX, NormalizedY, 0.5 / sqrt(DistanceFromCenterSquared)));
}

void ArcBallCameraController::UpdateViewMatrix()
{
	if (ParentEntity == nullptr || !ParentEntity->HasComponent<FECameraComponent>())
		return;

	FETransformComponent& TransformComponent = ParentEntity->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = ParentEntity->GetComponent<FECameraComponent>();

	const glm::dvec3 LocalCameraOffset = glm::dvec3(0.0, 0.0, DistanceToModel);
	const glm::dvec3 WorldCameraOffset = CurrentOrientation * LocalCameraOffset;
	const glm::dvec3 CameraPosition = glm::dvec3(TargetPosition) + WorldCameraOffset;
	const glm::dvec3 UpVector = CurrentOrientation * glm::dvec3(0.0, 1.0, 0.0);

	const glm::dmat4 NewViewMatrix = glm::lookAt(CameraPosition, glm::dvec3(TargetPosition), UpVector);

	glm::dvec3 DecomposedPosition;
	glm::dquat DecomposedRotation;
	glm::dvec3 DecomposedScale;
	GEOMETRY.DecomposeMatrixToTranslationRotationScale(NewViewMatrix, DecomposedPosition, DecomposedRotation, DecomposedScale);

	TransformComponent.SetPosition(CameraPosition);
	TransformComponent.SetQuaternion(DecomposedRotation);
	TransformComponent.ForceSetWorldMatrix(TransformComponent.GetLocalMatrix());

	CameraComponent.SetViewMatrix(NewViewMatrix);
}

void ArcBallCameraController::OnUpdate(double DeltaTime)
{
	if (ParentEntity == nullptr || !ParentEntity->HasComponent<FECameraComponent>())
		return;

	FECameraComponent& CameraComponent = ParentEntity->GetComponent<FECameraComponent>();

	FEInputMouseState MouseState = INPUT.GetMouseState();
	const int MouseX = static_cast<int>(MouseState.X);
	const int MouseY = static_cast<int>(MouseState.Y);

	bool bLeftButtonHeld = false;
	auto LeftButtonIterator = MouseState.Buttons.find(FE_MOUSE_BUTTON_LEFT);
	if (LeftButtonIterator != MouseState.Buttons.end())
		bLeftButtonHeld = LeftButtonIterator->second.State != FE_RELEASED;

	bool bRightButtonHeld = false;
	auto RightButtonIterator = MouseState.Buttons.find(FE_MOUSE_BUTTON_RIGHT);
	if (RightButtonIterator != MouseState.Buttons.end())
		bRightButtonHeld = RightButtonIterator->second.State != FE_RELEASED;

	if (CameraComponent.IsActive() && DeltaTime > 0.0)
	{
		if (bLeftButtonHeld && bWasDraggingLastFrame && (MouseX != LastMouseX || MouseY != LastMouseY))
		{
			double PreviousNormalizedX, PreviousNormalizedY;
			double CurrentNormalizedX, CurrentNormalizedY;
			if (GetNormalizedMousePosition(LastMouseX, LastMouseY, PreviousNormalizedX, PreviousNormalizedY) &&
				GetNormalizedMousePosition(MouseX, MouseY, CurrentNormalizedX, CurrentNormalizedY))
			{
				const glm::dvec3 PreviousOnSphere = ProjectMouseToSphere(PreviousNormalizedX, PreviousNormalizedY);
				const glm::dvec3 CurrentOnSphere = ProjectMouseToSphere(CurrentNormalizedX, CurrentNormalizedY);

				glm::dquat DeltaRotation;
				DeltaRotation.w = glm::dot(PreviousOnSphere, CurrentOnSphere);
				const glm::dvec3 RotationAxis = glm::cross(PreviousOnSphere, CurrentOnSphere);
				DeltaRotation.x = RotationAxis.x;
				DeltaRotation.y = RotationAxis.y;
				DeltaRotation.z = RotationAxis.z;
				DeltaRotation = glm::normalize(DeltaRotation);

				CurrentOrientation = CurrentOrientation * glm::inverse(DeltaRotation);
				CurrentOrientation = glm::normalize(CurrentOrientation);
			}
		}

		if (bRightButtonHeld && bWasPanningLastFrame && (MouseX != LastMouseX || MouseY != LastMouseY))
		{
			double PreviousNormalizedX, PreviousNormalizedY;
			double CurrentNormalizedX, CurrentNormalizedY;
			if (GetNormalizedMousePosition(LastMouseX, LastMouseY, PreviousNormalizedX, PreviousNormalizedY) &&
				GetNormalizedMousePosition(MouseX, MouseY, CurrentNormalizedX, CurrentNormalizedY))
			{
				const double DeltaNormalizedX = CurrentNormalizedX - PreviousNormalizedX;
				const double DeltaNormalizedY = CurrentNormalizedY - PreviousNormalizedY;

				const glm::dvec3 WorldRight = CurrentOrientation * glm::dvec3(1.0, 0.0, 0.0);
				const glm::dvec3 WorldUp = CurrentOrientation * glm::dvec3(0.0, 1.0, 0.0);
				const double PanScale = static_cast<double>(DistanceToModel) * static_cast<double>(PanSensitivity);

				const glm::dvec3 PanDelta = (DeltaNormalizedX * WorldRight + DeltaNormalizedY * WorldUp) * PanScale;
				TargetPosition -= glm::vec3(PanDelta);
			}
		}

		bWasDraggingLastFrame = bLeftButtonHeld;
		bWasPanningLastFrame = bRightButtonHeld;
	}
	else
	{
		bWasDraggingLastFrame = false;
		bWasPanningLastFrame = false;
	}

	LastMouseX = MouseX;
	LastMouseY = MouseY;

	UpdateViewMatrix();
}