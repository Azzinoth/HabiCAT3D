#include "SceneWindow.h"
#include "UIManager.h"
using namespace FocalEngine;

SceneWindow::SceneWindow() {}
SceneWindow::~SceneWindow() {}

void SceneWindow::Render()
{
	bHovered = false;
	bBeingMoved = false;

	ImGuiID DockspaceID = UI.GetDockspaceID();
	if (DockspaceID != 0 && bShouldDockToCentralNode)
	{
		ImGuiDockNode* CentralNode = ImGui::DockBuilderGetCentralNode(DockspaceID);
		if (CentralNode != nullptr)
		{
			ImGui::SetNextWindowDockID(CentralNode->ID, ImGuiCond_Appearing);
			bShouldDockToCentralNode = false;
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		bBeingMoved = ImGui::GetCurrentContext()->MovingWindow == Window;

		ImRect ContentRegion = Window->ContentRegionRect;
		ContentPosition = ContentRegion.Min;
		ContentSize = ContentRegion.GetSize();

		bHovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(ContentRegion.Min, ContentRegion.Max) && !ImGui::IsAnyItemHovered();

		FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
		if (CameraEntity != nullptr && CameraEntity->HasComponent<FECameraComponent>())
		{
			FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
			if (CameraComponent.GetViewport() == nullptr || CameraComponent.GetViewport()->GetType() != FE_VIEWPORT_IMGUI_WINDOW)
			{
				std::string ViewportID = ENGINE.CreateViewport(ImGui::GetCurrentWindow());
				CAMERA_SYSTEM.SetCameraViewport(CameraEntity, ViewportID);
			}

			FETexture* CameraResult = RENDERER.GetCameraResult(CameraEntity);
			if (CameraResult != nullptr)
				ImGui::Image(CameraResult->GetTextureID(), ContentSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(2);
}

bool SceneWindow::IsHovered() const
{
	return bHovered;
}

bool SceneWindow::IsBeingMoved() const
{
	return bBeingMoved;
}

bool SceneWindow::IsMouseCapturedByUI() const
{
	return ImGui::GetIO().WantCaptureMouse && (!bHovered || bBeingMoved);
}

// Valid inside a mouse release callback: ImGui applies the release on its next frame, so the drag state of the press is still available.
bool SceneWindow::MouseWasDraggedSincePress() const
{
	return ImGui::IsMouseDragging(ImGuiMouseButton_Left);
}

ImVec2 SceneWindow::GetContentPosition() const
{
	return ContentPosition;
}

ImVec2 SceneWindow::GetContentSize() const
{
	return ContentSize;
}
