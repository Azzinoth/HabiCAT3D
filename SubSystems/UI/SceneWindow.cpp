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

			VR_MAIN_WINDOW_RENDERING_MODE VRRenderingMode = SETTINGS_WINDOW.GetVRMainWindowRenderingMode();
			if (ENGINE.IsVREnabled() && VRRenderingMode == VR_MAIN_WINDOW_RENDERING_MODE::MIRROR_VR_VIEW)
			{
				// Headset result holds the last rendered eye. It is letterboxed instead of stretched, the eye aspect ratio rarely matches the window.
				FETexture* HeadsetResult = RENDERER.GetCameraResult(OpenXR_MANAGER.GetVRHeadsetEntity());
				if (HeadsetResult != nullptr && HeadsetResult->GetWidth() > 0 && HeadsetResult->GetHeight() > 0)
				{
					float TextureAspectRatio = static_cast<float>(HeadsetResult->GetWidth()) / static_cast<float>(HeadsetResult->GetHeight());
					ImVec2 ImageSize = ContentSize;
					if (ContentSize.x / ContentSize.y > TextureAspectRatio)
					{
						ImageSize.x = ContentSize.y * TextureAspectRatio;
					}
					else
					{
						ImageSize.y = ContentSize.x / TextureAspectRatio;
					}

					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (ContentSize.x - ImageSize.x) / 2.0f, ImGui::GetCursorPosY() + (ContentSize.y - ImageSize.y) / 2.0f));
					ImGui::Image(HeadsetResult->GetTextureID(), ImageSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				}
			}
			else if (ENGINE.IsVREnabled() && VRRenderingMode == VR_MAIN_WINDOW_RENDERING_MODE::DISABLED)
			{
				const char* MessageText = "Scene window rendering is disabled while in VR mode.";
				ImVec2 TextSize = ImGui::CalcTextSize(MessageText);
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (ContentSize.x - TextSize.x) / 2.0f, ImGui::GetCursorPosY() + (ContentSize.y - TextSize.y) / 2.0f));
				ImGui::TextDisabled("%s", MessageText);
			}
			else
			{
				FETexture* CameraResult = RENDERER.GetCameraResult(CameraEntity);
				if (CameraResult != nullptr)
					ImGui::Image(CameraResult->GetTextureID(), ContentSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			}
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
