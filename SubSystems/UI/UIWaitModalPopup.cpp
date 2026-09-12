#include "UIWaitModalPopup.h"
using namespace FocalEngine;

WaitModalPopup::WaitModalPopup() {}
WaitModalPopup::~WaitModalPopup() {}

void WaitModalPopup::OpenPopup(const std::string& Title, const std::string& Message, std::function<void()> AfterPopupWasRendered)
{
	CurrentPopupData.Title = Title;
	CurrentPopupData.Message = Message;
	CurrentPopupData.AfterPopupWasRendered = AfterPopupWasRendered;

	bActive = true;
	PresentedFrames = 0;
	PresentedTime = 0.0f;
}

void WaitModalPopup::Render()
{
	if (bActive && !ImGui::IsPopupOpen(CurrentPopupData.Title.c_str()))
		ImGui::OpenPopup(CurrentPopupData.Title.c_str());

	int TitleWidth = ImGui::CalcTextSize(CurrentPopupData.Title.c_str()).x + 40;
	int MessageWidth = ImGui::CalcTextSize(CurrentPopupData.Message.c_str()).x + 40;

	int PopupWidth = TitleWidth > MessageWidth ? TitleWidth : MessageWidth;
	ImGui::SetNextWindowSize(ImVec2(PopupWidth, 0));
	if (!ImGui::BeginPopupModal(CurrentPopupData.Title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	int MainWindowWidth = 0;
	int MainWindowHeight = 0;
	APPLICATION.GetMainWindow()->GetSize(&MainWindowWidth, &MainWindowHeight);
	ImGui::SetWindowPos(ImVec2(MainWindowWidth / 2.0f - ImGui::GetWindowWidth() / 2.0f, MainWindowHeight / 2.0f - ImGui::GetWindowHeight() / 2.0f));

	float TextWidth = ImGui::CalcTextSize(CurrentPopupData.Message.c_str()).x;
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - TextWidth) / 2.0f);
	ImGui::TextUnformatted(CurrentPopupData.Message.c_str());

	if (!bActive)
	{
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return;
	}

	// Frame count: ImGui hides a popup on its first frame to measure it, so wait until it was actually drawn and presented.
	// Time delay: ImGui fades the modal background in at 6.0 per second, so let it reach full strength before blocking.
	if (PresentedFrames >= 2 && PresentedTime >= 0.2f)
	{
		if (CurrentPopupData.AfterPopupWasRendered != nullptr)
			CurrentPopupData.AfterPopupWasRendered();

		CurrentPopupData.AfterPopupWasRendered = nullptr;
		bActive = false;
		ImGui::CloseCurrentPopup();
	}
	
	PresentedFrames++;
	PresentedTime += ImGui::GetIO().DeltaTime;
	ImGui::EndPopup();
}