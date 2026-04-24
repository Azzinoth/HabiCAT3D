#include "SettingsWindow.h"
using namespace FocalEngine;

SettingsWindow::SettingsWindow() {}
SettingsWindow::~SettingsWindow() {}

bool SettingsWindow::GetWireFrameMode()
{
	return bWireframeMode;
}

void SettingsWindow::SetWireFrameMode(const bool NewValue)
{
	bWireframeMode = NewValue;
}

float SettingsWindow::GetAmbientLightFactor()
{
	return AmbientLightFactor;
}

void SettingsWindow::SetAmbientLightFactor(float NewValue)
{
	AmbientLightFactor = NewValue;
}

void SettingsWindow::Render()
{
	if (!bVisible)
		return;

	if (ImGui::Begin("Settings_new"))
	{
		/*AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (ActiveObject != nullptr)
		{
			FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
			if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
			{*/
				ImGui::Checkbox("Wireframe", &bWireframeMode);

				ImGui::Text("Ambiant light intensity:");
				ImGui::SetNextItemWidth(150);
				ImGui::DragFloat("##AmbiantLightScale", &AmbientLightFactor, 0.025f);
				ImGui::SameLine();
				if (ImGui::Button("Reset"))
				{
					AmbientLightFactor = 2.2f;
				}
		//	}
		//}

		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (ActiveObject != nullptr)
		{
			FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
			bool bModelCameraMode = bModelCamera;
			if (ImGui::Checkbox("Model camera", &bModelCameraMode))
			{
				SetIsModelCamera(bModelCameraMode);
			}

			if (bModelCamera && bChooseCameraFocusPointMode && ImGui::IsMouseReleased(0) && ActiveEntity != nullptr)
			{
				glm::dvec3 IntersectionPoint = ANALYSIS_OBJECT_MANAGER.IntersectTriangle(MAIN_SCENE_MANAGER.GetMouseRayDirection());

				IntersectionPoint = glm::dvec3(ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(IntersectionPoint, 1.0));
				if (IntersectionPoint != glm::dvec3(0.0))
				{
					SetIsModelCamera(true, IntersectionPoint);
				}
			}

			if (bModelCamera)
			{
				if (ImGui::Button("Set point on model as a focus point"))
					bChooseCameraFocusPointMode = true;

				FENativeScriptComponent& NativeScriptComponent = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FENativeScriptComponent>();
				float ModelCameraMouseWheelSensitivity = 0.0f;
				NativeScriptComponent.GetVariableValue<float>("MouseWheelSensitivity", ModelCameraMouseWheelSensitivity);
				ImGui::DragFloat("Mouse wheel sensitivity", &ModelCameraMouseWheelSensitivity, 0.0001f, 0.000001f, 100.0f);
				NativeScriptComponent.SetVariableValue("MouseWheelSensitivity", ModelCameraMouseWheelSensitivity);
			}

			ShowCameraTransform();
		}

		ImGui::Separator();
		bool bDeveloperModeOn = DEVELOPER_MODE.IsOn();
		if (ImGui::Checkbox("Developer mode", &bDeveloperModeOn))
			DEVELOPER_MODE.SetIsOn(bDeveloperModeOn);

		if (!DEVELOPER_MODE.IsOn())
		{
			//if (DebugGrid != nullptr && DebugGrid->RenderingMode != 0)
				//UpdateRenderingMode(DebugGrid, 0);
		}
	}

	ImGui::End();
}

void SettingsWindow::ShowCameraTransform()
{
	if (!bModelCamera)
	{
		// ********* POSITION *********
		glm::vec3 CameraPosition = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE);

		ImGui::Text("Position : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##X pos", &CameraPosition[0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Y pos", &CameraPosition[1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Z pos", &CameraPosition[2], 0.1f);

		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().SetPosition(CameraPosition, FE_WORLD_SPACE);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Copy##Position"))
			APPLICATION.SetClipboardText(CameraPositionToString());

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Paste##Position"))
			StringToCameraPosition(APPLICATION.GetClipboardText());

		// ********* ROTATION *********
		glm::vec3 CameraRotation = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetRotation(FE_WORLD_SPACE);

		ImGui::Text("Rotation : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##X rot", &CameraRotation[0], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Y rot", &CameraRotation[1], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Z rot", &CameraRotation[2], 0.1f);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Copy##Rotation"))
			APPLICATION.SetClipboardText(CameraRotationToString());

		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().SetRotation(CameraRotation, FE_WORLD_SPACE);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(40);
		if (ImGui::Button("Paste##Rotation"))
			StringToCameraRotation(APPLICATION.GetClipboardText());

		float NearPlane = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().GetNearPlane();
		ImGui::Text("Near plane: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Near plane", &NearPlane, 0.01f, 0.01f, 100.0f);
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetNearPlane(NearPlane);

		float FarPlane = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().GetFarPlane();
		ImGui::Text("Far plane: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Far plane", &FarPlane, 0.01f, 0.01f, 100000.0f);
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetFarPlane(FarPlane);

		FENativeScriptComponent& NativeScriptComponent = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FENativeScriptComponent>();
		float CameraSpeed = 0.0f;
		NativeScriptComponent.GetVariableValue<float>("MovementSpeed", CameraSpeed);
		ImGui::Text("Camera speed: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Camera_speed", &CameraSpeed, 0.01f, 0.01f, 100.0f);
		NativeScriptComponent.SetVariableValue("MovementSpeed", CameraSpeed);

		if (DEVELOPER_MODE.IsOn())
		{
			ImGui::SameLine();
			ImGui::Text(("Thread count: " + std::to_string(THREAD_POOL.GetThreadCount())).c_str());
		}
	}
	else
	{
		if (DEVELOPER_MODE.IsOn())
		{
			ImGui::Text(("Thread count: " + std::to_string(THREAD_POOL.GetThreadCount())).c_str());
		}
	}
}

std::string SettingsWindow::CameraPositionToString()
{
	const glm::vec3 CameraPosition = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE);
	return "( X:" + std::to_string(CameraPosition.x) + " Y:" + std::to_string(CameraPosition.y) + " Z:" + std::to_string(CameraPosition.z) + " )";
}

void SettingsWindow::StringToCameraPosition(std::string Text)
{
	size_t StartPosition = Text.find("( X:");
	if (StartPosition == std::string::npos)
		return;

	size_t EndPosition = Text.find(" Y:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("( X:") < 0 ||
		StartPosition + strlen("( X:") + EndPosition - (StartPosition + strlen("( X:")) >= Text.size())
		return;

	std::string temp = Text.substr(StartPosition + strlen("( X:"), EndPosition - (StartPosition + strlen("( X:")));

	if (temp.empty())
		return;

	const float X = float(atof(temp.c_str()));

	StartPosition = Text.find("Y:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = Text.find(" Z:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Y:") < 0 ||
		StartPosition + strlen("Y:") + EndPosition - (StartPosition + strlen("Y:")) >= Text.size())
		return;

	temp = Text.substr(StartPosition + strlen("Y:"), EndPosition - (StartPosition + strlen("Y:")));

	if (temp.empty())
		return;

	const float Y = float(atof(temp.c_str()));

	StartPosition = Text.find("Z:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = Text.find(" )");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Z:") < 0 ||
		StartPosition + strlen("Z:") + EndPosition - (StartPosition + strlen("Z:")) >= Text.size())
		return;

	temp = Text.substr(StartPosition + strlen("Z:"), EndPosition - (StartPosition + strlen("Z:")));

	if (temp.empty())
		return;

	const float Z = float(atof(temp.c_str()));

	MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().SetPosition(glm::vec3(X, Y, Z), FE_WORLD_SPACE);
}

std::string SettingsWindow::CameraRotationToString()
{
	const glm::vec3 CameraRotation = MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetRotation(FE_WORLD_SPACE);
	return "( X:" + std::to_string(CameraRotation.x) + " Y:" + std::to_string(CameraRotation.y) + " Z:" + std::to_string(CameraRotation.z) + " )";
}

void SettingsWindow::StringToCameraRotation(std::string Text)
{
	size_t StartPosition = Text.find("( X:");
	if (StartPosition == std::string::npos)
		return;

	size_t EndPosition = Text.find(" Y:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("( X:") < 0 ||
		StartPosition + strlen("( X:") + EndPosition - (StartPosition + strlen("( X:")) >= Text.size())
		return;

	std::string temp = Text.substr(StartPosition + strlen("( X:"), EndPosition - (StartPosition + strlen("( X:")));

	if (temp.empty())
		return;

	const float X = float(atof(temp.c_str()));

	StartPosition = Text.find("Y:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = Text.find(" Z:");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Y:") < 0 ||
		StartPosition + strlen("Y:") + EndPosition - (StartPosition + strlen("Y:")) >= Text.size())
		return;

	temp = Text.substr(StartPosition + strlen("Y:"), EndPosition - (StartPosition + strlen("Y:")));

	if (temp.empty())
		return;

	const float Y = float(atof(temp.c_str()));

	StartPosition = Text.find("Z:");
	if (StartPosition == std::string::npos)
		return;

	EndPosition = Text.find(" )");
	if (EndPosition == std::string::npos)
		return;

	if (StartPosition + strlen("Z:") < 0 ||
		StartPosition + strlen("Z:") + EndPosition - (StartPosition + strlen("Z:")) >= Text.size())
		return;

	temp = Text.substr(StartPosition + strlen("Z:"), EndPosition - (StartPosition + strlen("Z:")));

	if (temp.empty())
		return;

	const float Z = float(atof(temp.c_str()));

	MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().SetRotation(glm::vec3(X, Y, Z), FE_WORLD_SPACE);
}

bool SettingsWindow::IsInModelCameraMode()
{
	return bModelCamera;
}

void SettingsWindow::FocusCameraOnObject(AnalysisObject* Object)
{
	IsInModelCameraMode() ? ModelCameraAdjustment(Object) : FreeCameraAdjustment(Object);
}

void SettingsWindow::ModelCameraAdjustment(AnalysisObject* Object)
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr)
		return;

	if (ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() == 0)
		return;

	FEAABB AABBToWorkWith = Object == nullptr ? ANALYSIS_OBJECT_MANAGER.GetAllObjectsAABB() : Object->GetAnalysisData()->GetAABB();

	FENativeScriptComponent& NativeScriptComponent = CameraEntity->GetComponent<FENativeScriptComponent>();
	NativeScriptComponent.SetVariableValue("DistanceToModel", AABBToWorkWith.GetLongestAxisLength() * 1.5f);
	NativeScriptComponent.SetVariableValue("MouseWheelSensitivity", AABBToWorkWith.GetLongestAxisLength() * 0.1f);
}

void SettingsWindow::FreeCameraAdjustment(AnalysisObject* Object)
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr)
		return;

	if (ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() == 0)
		return;

	FEAABB AABBToWorkWith = Object == nullptr ? ANALYSIS_OBJECT_MANAGER.GetAllObjectsAABB() : Object->GetAnalysisData()->GetAABB();

	FETransformComponent& TransformComponent = CameraEntity->GetComponent<FETransformComponent>();
	TransformComponent.SetPosition(glm::vec3(0.0f, 0.0f, AABBToWorkWith.GetLongestAxisLength() * 1.5f));
	TransformComponent.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

	FENativeScriptComponent& NativeScriptComponent = CameraEntity->GetComponent<FENativeScriptComponent>();
	NativeScriptComponent.SetVariableValue("MovementSpeed", AABBToWorkWith.GetLongestAxisLength() / 5.0f);
}

void SettingsWindow::AdjustCameraNearFarPlanes()
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr)
		return;

	if (ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() == 0)
		return;

	FEAABB AllObjectsAABB = ANALYSIS_OBJECT_MANAGER.GetAllObjectsAABB();

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.SetNearPlane(AllObjectsAABB.GetLongestAxisLength() * 0.001f);
	CameraComponent.SetFarPlane(AllObjectsAABB.GetLongestAxisLength() * 5.0f);
}

void SettingsWindow::SwitchCameraMode(bool bModelCamera, glm::vec3 ModelCameraFocusPoint)
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity == nullptr)
		return;

	if (bModelCamera)
	{
		std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Model view camera prefab");
		if (CameraPrefab.empty())
			return;

		std::vector<std::string> EntitiesIDList = CameraPrefab[0]->GetScene()->GetEntityIDList();
		if (EntitiesIDList.empty())
			return;

		for (size_t i = 0; i < EntitiesIDList.size(); i++)
		{
			FEEntity* CurrentEntity = CameraPrefab[0]->GetScene()->GetEntity(EntitiesIDList[i]);
			if (CurrentEntity == nullptr)
				continue;

			if (CurrentEntity->HasComponent<FECameraComponent>() && CurrentEntity->HasComponent<FENativeScriptComponent>())
			{
				CameraEntity->RemoveComponent<FENativeScriptComponent>();
				CameraEntity->AddComponent<FENativeScriptComponent>();
				NATIVE_SCRIPT_SYSTEM.InitializeScriptComponent(CameraEntity, CurrentEntity->GetComponent<FENativeScriptComponent>().GetModuleID(), "ModelViewCameraController");
				FENativeScriptComponent& NativeScriptComponent = CameraEntity->GetComponent<FENativeScriptComponent>();
				NativeScriptComponent.SetVariableValue("TargetPosition", ModelCameraFocusPoint);

				CameraEntity->GetComponent<FECameraComponent>().SetActive(false);
				return;
			}
		}
	}
	else
	{
		std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Free camera prefab");
		if (CameraPrefab.empty())
			return;

		std::vector<std::string> EntitiesIDList = CameraPrefab[0]->GetScene()->GetEntityIDList();
		if (EntitiesIDList.empty())
			return;

		for (size_t i = 0; i < EntitiesIDList.size(); i++)
		{
			FEEntity* CurrentEntity = CameraPrefab[0]->GetScene()->GetEntity(EntitiesIDList[i]);
			if (CurrentEntity == nullptr)
				continue;

			if (CurrentEntity->HasComponent<FECameraComponent>() && CurrentEntity->HasComponent<FENativeScriptComponent>())
			{
				CameraEntity->RemoveComponent<FENativeScriptComponent>();
				CameraEntity->AddComponent<FENativeScriptComponent>();
				NATIVE_SCRIPT_SYSTEM.InitializeScriptComponent(CameraEntity, CurrentEntity->GetComponent<FENativeScriptComponent>().GetModuleID(), "FreeCameraController");

				CameraEntity->GetComponent<FECameraComponent>().SetActive(false);
				return;
			}
		}
	}
}

void SettingsWindow::SetIsModelCamera(const bool NewValue, glm::vec3 ModelCameraFocusPoint)
{
	bChooseCameraFocusPointMode = false;

	SwitchCameraMode(NewValue, ModelCameraFocusPoint);
	AdjustCameraNearFarPlanes();
	FocusCameraOnObject();

	bModelCamera = NewValue;
}