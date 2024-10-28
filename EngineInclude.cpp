#include "EngineInclude.h"
using namespace FocalEngine;

MainSceneManager::MainSceneManager()
{
	MainScene = SCENE_MANAGER.CreateScene("Main scene", "", FESceneFlag::Active | FESceneFlag::Renderable | FESceneFlag::GameMode);
	MainCamera = MainScene->CreateEntity("Main camera");

	FEPrefab* ModelViewCameraPrefab = RESOURCE_MANAGER.GetPrefab("14745A482D1B2C328C268027");
	std::vector<FEEntity*> AddedEntities = SCENE_MANAGER.InstantiatePrefab(ModelViewCameraPrefab, MainScene, true);
	MainCamera = AddedEntities[0];
	CAMERA_SYSTEM.SetMainCamera(MainCamera);
	CAMERA_SYSTEM.SetCameraViewport(MainCamera, ENGINE.GetDefaultViewport()->GetID());
}

MainSceneManager::~MainSceneManager()
{
	SCENE_MANAGER.DeleteScene(MainScene);
}

FEScene* MainSceneManager::GetMainScene()
{
	return MainScene;
}

FEEntity* MainSceneManager::GetMainCamera()
{
	return MainCamera;
}

glm::dvec3 MainSceneManager::GetMouseRayDirection()
{
	FETransformComponent& CameraTransformComponent = MainCamera->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = MainCamera->GetComponent<FECameraComponent>();

	FEViewport* CurrentViewport = CAMERA_SYSTEM.GetMainCameraViewport(MainScene);
	glm::ivec2 ViewportPosition = glm::ivec2(CurrentViewport->GetX(), CurrentViewport->GetY());
	glm::ivec2 ViewportSize = glm::ivec2(CurrentViewport->GetWidth(), CurrentViewport->GetHeight());

	return GEOMETRY.CreateMouseRayToWorld(INPUT.GetMouseX(), INPUT.GetMouseY(),
										  CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
										  ViewportPosition, ViewportSize);
}