#include "VRManager.h"

VRManager::VRManager()
{
	EntitiesToManipulate.push_back(nullptr);
}

void VRManager::Initialize()
{
	FEOpenXR_INPUT.AddControllerStateChangeCallback(&VRManager::OnControllerConnectionStatusChangeCallBack);

	FEOpenXR_INPUT.SetRightTriggerPressCallBack(&VRManager::OnRightTriggerPress);
	FEOpenXR_INPUT.SetRightTriggerReleaseCallBack(&VRManager::OnRightTriggerRelease);

	FEOpenXR_INPUT.SetLeftTriggerPressCallBack(&VRManager::OnLeftTriggerPress);
	FEOpenXR_INPUT.SetLeftTriggerReleaseCallBack(&VRManager::OnLeftTriggerRelease);

	// "A" Button
	FEOpenXR_INPUT.SetLeftAButtonPressCallBack(&VRManager::OnLeftAButtonPress);
	FEOpenXR_INPUT.SetLeftAButtonReleaseCallBack(&VRManager::OnLeftAButtonRelease);

	FEOpenXR_INPUT.SetRightAButtonPressCallBack(&VRManager::OnRightAButtonPress);
	FEOpenXR_INPUT.SetRightAButtonReleaseCallBack(&VRManager::OnRightAButtonRelease);

	FEOpenXR_INPUT.SetLeftAButtonTouchActivateCallBack(&VRManager::OnLeftAButtonTouchActivate);
	FEOpenXR_INPUT.SetLeftAButtonTouchDeactivateCallBack(&VRManager::OnLeftAButtonTouchDeactivate);
	FEOpenXR_INPUT.SetRightAButtonTouchActivateCallBack(&VRManager::OnRightAButtonTouchActivate);
	FEOpenXR_INPUT.SetRightAButtonTouchDeactivateCallBack(&VRManager::OnRightAButtonTouchDeactivate);

	// "B" Button
	//FEOpenXR_INPUT.SetLeftBButtonPressCallBack(&VRManager::OnLeftBButtonPress);
	FEOpenXR_INPUT.SetLeftBButtonReleaseCallBack(&VRManager::OnLeftBButtonRelease);

	//FEOpenXR_INPUT.SetRightBButtonPressCallBack(&VRManager::OnRightBButtonPress);
	FEOpenXR_INPUT.SetRightBButtonReleaseCallBack(&VRManager::OnRightBButtonRelease);

	// Vive alternative
	FEOpenXR_INPUT.SetLeftViveSqueezePressCallBack(&VRManager::OnLeftAButtonPress);
	FEOpenXR_INPUT.SetLeftViveSqueezeReleaseCallBack(&VRManager::OnLeftAButtonRelease);

	FEOpenXR_INPUT.SetRightViveSqueezePressCallBack(&VRManager::OnRightAButtonPress);
	FEOpenXR_INPUT.SetRightViveSqueezeReleaseCallBack(&VRManager::OnRightAButtonRelease);

	FEOpenXR_INPUT.SetLeftViveMenuClickCallBack(&VRManager::OnLeftViveMenuClick);

	// Cylinder
	FEMesh* CylinderMesh = RESOURCE_MANAGER.LoadFEMesh("Resources//Cylinder.model");

	FEShader* SolidColorShader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	// FIX ME: Work of this shader is not correct.
	SolidColorShader->UpdateUniformData("BrightnessFactor", 1.0f);

	FEMaterial* GreenMaterial = RESOURCE_MANAGER.CreateMaterial();
	GreenMaterial->Shader = SolidColorShader;
	GreenMaterial->SetBaseColor(glm::vec3(0.0f, 1.0f, 0.0f));
	FEGameModel* GreenCylinderGameModel = RESOURCE_MANAGER.CreateGameModel(CylinderMesh, GreenMaterial);

	FEMaterial* RedMaterial = RESOURCE_MANAGER.CreateMaterial();
	RedMaterial->Shader = SolidColorShader;
	RedMaterial->SetBaseColor(glm::vec3(1.0f, 0.0f, 0.0f));
	FEGameModel* RedCylinderGameModel = RESOURCE_MANAGER.CreateGameModel(CylinderMesh, RedMaterial);

	FEMaterial* BlueMaterial = RESOURCE_MANAGER.CreateMaterial();
	BlueMaterial->Shader = SolidColorShader;
	BlueMaterial->SetBaseColor(glm::vec3(0.0f, 0.0f, 1.0f));
	FEGameModel* BlueCylinderGameModel = RESOURCE_MANAGER.CreateGameModel(CylinderMesh, BlueMaterial);

	FEMaterial* WhiteMaterial = RESOURCE_MANAGER.CreateMaterial();
	WhiteMaterial->Shader = SolidColorShader;
	WhiteMaterial->SetBaseColor(glm::vec3(0.9f, 0.9f, 0.9f));
	FEGameModel* WhiteCylinderGameModel = RESOURCE_MANAGER.CreateGameModel(CylinderMesh, WhiteMaterial);
	WhiteCylinderGameModel->SetName("White Cylinder Game Model");

	GreenCylinderEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Green Cylinder");
	GreenCylinderEntity->AddComponent<FEGameModelComponent>(GreenCylinderGameModel);
	FEGameModelComponent& GreenCylinderGameModelComponent = GreenCylinderEntity->GetComponent<FEGameModelComponent>();
	GreenCylinderGameModelComponent.SetReceivingShadows(false);
	GreenCylinderEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, false);
	FETransformComponent& GreenCylinderTransform = GreenCylinderEntity->GetComponent<FETransformComponent>();
	GreenCylinderTransform.SetScale(glm::vec3(0.001f, 0.01f, 0.001f));

	RedCylinderEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Red Cylinder");
	RedCylinderEntity->AddComponent<FEGameModelComponent>(RedCylinderGameModel);
	FEGameModelComponent& RedCylinderGameModelComponent = RedCylinderEntity->GetComponent<FEGameModelComponent>();
	RedCylinderGameModelComponent.SetReceivingShadows(false);
	RedCylinderEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, false);
	FETransformComponent& RedCylinderTransform = RedCylinderEntity->GetComponent<FETransformComponent>();
	RedCylinderTransform.SetScale(glm::vec3(0.001f, 0.01f, 0.001f));
	RedCylinderTransform.SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

	BlueCylinderEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Blue Cylinder");
	BlueCylinderEntity->AddComponent<FEGameModelComponent>(BlueCylinderGameModel);
	FEGameModelComponent& BlueCylinderGameModelComponent = BlueCylinderEntity->GetComponent<FEGameModelComponent>();
	BlueCylinderGameModelComponent.SetReceivingShadows(false);
	BlueCylinderEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, false);
	FETransformComponent& BlueCylinderTransform = BlueCylinderEntity->GetComponent<FETransformComponent>();
	BlueCylinderTransform.SetScale(glm::vec3(0.001f, 0.01f, 0.001f));
	BlueCylinderTransform.SetRotation(glm::vec3(0.0f, 0.0f, 90.0f));

	WhiteCylinderEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("White Cylinder");
	WhiteCylinderEntity->AddComponent<FEGameModelComponent>(WhiteCylinderGameModel);
	FEGameModelComponent& WhiteCylinderGameModelComponent = WhiteCylinderEntity->GetComponent<FEGameModelComponent>();
	WhiteCylinderGameModelComponent.SetReceivingShadows(false);
	WhiteCylinderEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, false);
	FETransformComponent& WhiteCylinderTransform = WhiteCylinderEntity->GetComponent<FETransformComponent>();
	WhiteCylinderTransform.SetScale(glm::vec3(VR_MANAGER.SelectionRayWideness, 0.1f, VR_MANAGER.SelectionRayWideness));
	WhiteCylinderTransform.SetRotation(glm::vec3(0.0f, 0.0f, 90.0f));
}

float VRManager::ControllerDistance()
{
	return glm::length(FEOpenXR_INPUT.GetLeftControllerPosition() - FEOpenXR_INPUT.GetRightControllerPosition());
}

void VRManager::CheckShouldScaleWorldActivate()
{
	if (bLeftControllerScalingJustGotActive && bRightControllerScalingJustGotActive)
	{
		SetScalingWithControllers(true);

		bLeftControllerScalingJustGotActive = false;
		bRightControllerScalingJustGotActive = false;

		if (VR_MANAGER.CurrentEntityToManipulate != nullptr)
		{
			InitialControllerDistanceForScale = ControllerDistance();
			FETransformComponent& EntityToManipulateTransform = VR_MANAGER.CurrentEntityToManipulate->GetComponent<FETransformComponent>();
			InitialEntityScale = EntityToManipulateTransform.GetScale();
			OriginalEntityPosition = EntityToManipulateTransform.GetPosition();

			FEEntity* LeftControllerEntity = OpenXR_MANAGER.GetLeftControllerEntity();
			FETransformComponent& LeftControllerTransform = LeftControllerEntity->GetComponent<FETransformComponent>();

			FEEntity* RightControllerEntity = OpenXR_MANAGER.GetRightControllerEntity();
			FETransformComponent& RightControllerTransform = RightControllerEntity->GetComponent<FETransformComponent>();

			OriginalPivotPosition = 0.5f * (LeftControllerTransform.GetPosition(FE_WORLD_SPACE) + RightControllerTransform.GetPosition(FE_WORLD_SPACE)) * TranslateScale;
		}
	}

	if (bLeftControllerScalingJustGotInActive || bRightControllerScalingJustGotInActive)
	{
		bLeftControllerScalingJustGotInActive = false;
		bRightControllerScalingJustGotInActive = false;

		SetScalingWithControllers(false);
	}
}

void VRManager::UpdateWhiteCylinder()
{
	FETransformComponent& WhiteCylinderTransform = WhiteCylinderEntity->GetComponent<FETransformComponent>();
	if (bScalingWithControllersActive)
	{
		WhiteCylinderTransform.SetPosition(CurrentPivotPosition);
		float OriginalLength = 2.0f;

		WhiteCylinderTransform.SetScale(glm::vec3(SelectionRayWideness, ControllerDistance() / OriginalLength, SelectionRayWideness) * VRRigScale);

		// Assumes cylinder is oriented along y-axis
		glm::vec3 DefaultCylinderDirection = glm::vec3(0, 1, 0);
		glm::vec3 Direction = glm::normalize(FEOpenXR_INPUT.GetLeftControllerPosition() - FEOpenXR_INPUT.GetRightControllerPosition());
		glm::quat Rotation = glm::rotation(DefaultCylinderDirection, Direction);
		WhiteCylinderTransform.SetQuaternion(Rotation);
	}
}

void VRManager::ScaleWorldUpdate()
{
	CheckShouldScaleWorldActivate();
	UpdateWhiteCylinder();

	bool bShouldBeVisible = ((bLeftAButtonTouched && bRightAButtonTouched) || bScalingWithControllersActive) && (GetCurrentEntityToManipulate() != nullptr);
	RedCylinderEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, bShouldBeVisible);
	GreenCylinderEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, bShouldBeVisible);
	BlueCylinderEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, bShouldBeVisible);

	bool bShouldInteractionRayBeVisible = bScalingWithControllersActive && (GetCurrentEntityToManipulate() != nullptr);
	WhiteCylinderEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, bShouldInteractionRayBeVisible);

	if (bShouldBeVisible)
	{
		if (VR_MANAGER.CurrentEntityToManipulate != nullptr)
		{
			FEEntity* LeftControllerEntity = OpenXR_MANAGER.GetLeftControllerEntity();
			FETransformComponent& LeftControllerTransform = LeftControllerEntity->GetComponent<FETransformComponent>();

			FEEntity* RightControllerEntity = OpenXR_MANAGER.GetRightControllerEntity();
			FETransformComponent& RightControllerTransform = RightControllerEntity->GetComponent<FETransformComponent>();

			CurrentPivotPosition = 0.5f * (LeftControllerTransform.GetPosition(FE_WORLD_SPACE) + RightControllerTransform.GetPosition(FE_WORLD_SPACE)) * TranslateScale;

			RedCylinderEntity->GetComponent<FETransformComponent>().SetPosition(CurrentPivotPosition);
			GreenCylinderEntity->GetComponent<FETransformComponent>().SetPosition(CurrentPivotPosition);
			BlueCylinderEntity->GetComponent<FETransformComponent>().SetPosition(CurrentPivotPosition);

			if (bScalingWithControllersActive)
			{
				float Delta = ControllerDistance() - InitialControllerDistanceForScale;

				if (!isnan(exp(Delta * 10.f) * InitialEntityScale.x) &&
					!isnan(exp(Delta * 10.f) * InitialEntityScale.y) &&
					!isnan(exp(Delta * 10.f) * InitialEntityScale.z))
				{
					// Each frame if scaling is active
					glm::vec3 NewScale = glm::vec3(exp(Delta * 10.0f) * InitialEntityScale);
					// The pivot position in the original scaled space
					glm::vec3 OriginalPivotRelativePosition = (OriginalPivotPosition - OriginalEntityPosition) / InitialEntityScale;
					// The position of the pivot point in the new scaled space
					glm::vec3 ScaledPivotPosition = OriginalPivotRelativePosition * NewScale;

					FETransformComponent& EntityToManipulateTransform = CurrentEntityToManipulate->GetComponent<FETransformComponent>();
					EntityToManipulateTransform.SetScale(NewScale);

					// Adjust the PointCloudPosition to ensure the pivot point remains at the same position in the scaled space
					glm::vec3 NewPosition = OriginalPivotPosition - ScaledPivotPosition;

					// Make sure that we can move data while scaling
					NewPosition -= OriginalPivotPosition - CurrentPivotPosition;
					EntityToManipulateTransform.SetPosition(NewPosition, FE_COORDINATE_SPACE_TYPE::FE_WORLD_SPACE);
				}
			}
		}
	}
}

void VRManager::OnRightTriggerPress()
{

}

void VRManager::OnRightTriggerRelease()
{
	
}

void VRManager::OnLeftTriggerPress()
{
	FEEntity* CurrentEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (CurrentEntity == nullptr)
		return;

	VR_MANAGER.SetCurrentEntityToManipulate(CurrentEntity);

	VR_MANAGER.bLeftControllerTriggerIsPressed = true;
}

void VRManager::OnLeftTriggerRelease()
{
	VR_MANAGER.bLeftControllerTriggerIsPressed = false;
	VR_MANAGER.ReleaseEntity();
}

void VRManager::OnLeftAButtonPress()
{
	VR_MANAGER.bLeftControllerScalingJustGotActive = true;
}

void VRManager::OnRightAButtonPress()
{
	VR_MANAGER.bRightControllerScalingJustGotActive = true;
}

void VRManager::OnLeftAButtonRelease()
{
	VR_MANAGER.bLeftControllerScalingJustGotInActive = true;
}

void VRManager::OnLeftViveMenuClick()
{
}

void VRManager::OnRightAButtonRelease()
{
	VR_MANAGER.bRightControllerScalingJustGotInActive = true;

	VR_MANAGER.RightAButtonReleaseTimeStamp = TIME.GetTimeStamp();
}

void VRManager::OnLeftAButtonTouchActivate()
{
	VR_MANAGER.bLeftAButtonTouched = true;
}

void VRManager::OnLeftAButtonTouchDeactivate()
{
	VR_MANAGER.bLeftAButtonTouched = false;
}

void VRManager::OnRightAButtonTouchActivate()
{
	VR_MANAGER.bRightAButtonTouched = true;
}

void VRManager::OnRightAButtonTouchDeactivate()
{
	VR_MANAGER.bRightAButtonTouched = false;
}

void VRManager::OnLeftBButtonRelease()
{
	
}

void VRManager::OnRightBButtonRelease()
{

}

void VRManager::Update()
{
	if (OpenXR_MANAGER.GetVRRigEntity() == nullptr)
		return;
	
	ScaleWorldUpdate();
}

std::pair<glm::vec3, glm::vec3> VRManager::GetControllerInteractionRay(bool bLeftController) const
{
	FEEntity* ControllerEntity = bLeftController ? OpenXR_MANAGER.GetLeftControllerEntity() : OpenXR_MANAGER.GetRightControllerEntity();
	if (ControllerEntity == nullptr)
		return std::make_pair(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

	FETransformComponent& ControllerTransform = ControllerEntity->GetComponent<FETransformComponent>();
	glm::vec3 ControllerPosition = ControllerTransform.GetPosition(FE_WORLD_SPACE);
	glm::quat ControllerOrientation = ControllerTransform.GetQuaternion();
	glm::vec3 InteractionRay = glm::vec3(0.0f, 0.0f, -1.0f);
	InteractionRay = glm::vec3(glm::toMat4(ControllerOrientation) * glm::vec4(InteractionRay, 1.0f));

	return std::make_pair(ControllerPosition, InteractionRay);
}

bool VRManager::IsControllerSelectionModeActive() const
{
	return bControllerSelectionMode;
}

void VRManager::SetControllerSelectionModeActive(bool bActive)
{
	bControllerSelectionMode = bActive;
}

FEEntity* VRManager::GetCurrentEntityToManipulate() const
{
	return CurrentEntityToManipulate;
}

void VRManager::SetCurrentEntityToManipulate(FEEntity* NewEntity)
{
	if (CurrentEntityToManipulate != nullptr)
		ReleaseEntity();
	
	OpenXR_MANAGER.GetLeftControllerEntity()->AttachChild(NewEntity);
	CurrentEntityToManipulate = NewEntity;
}

void VRManager::ReleaseEntity()
{
	if (CurrentEntityToManipulate == nullptr)
		return;

	CurrentEntityToManipulate->Detach(true);
}

void VRManager::SetScalingWithControllers(bool bNewValue)
{
	if (bScalingWithControllersActive && !bNewValue)
		LastTimeControllerScalingWasActive = TIME.GetTimeStamp();
	
	bScalingWithControllersActive = bNewValue;
}

void VRManager::OnControllerDisconnected(bool bLeftController)
{
	if (bLeftController)
	{
		
	}
	else
	{
		
	}
}

void VRManager::OnControllerReConnected(bool bLeftController)
{
	if (bLeftController)
	{
		
	}
	else
	{

	}
}

void VRManager::OnControllerConnected(bool bLeftController)
{

}

void VRManager::OnControllerConnectionStatusChangeCallBack(bool bLeft, FE_VR_CONTROLLER_STATE_CHANGE Change)
{
	if (Change == FE_VR_CONTROLLER_STATE_CHANGE::CONNECTED)
	{
		VR_MANAGER.OnControllerConnected(bLeft);
	}
	else if (Change == FE_VR_CONTROLLER_STATE_CHANGE::DISCONNECTED)
	{
		VR_MANAGER.OnControllerDisconnected(bLeft);
	}
	else if (Change == FE_VR_CONTROLLER_STATE_CHANGE::RECONNECTED)
	{
		VR_MANAGER.OnControllerReConnected(bLeft);
	}
}