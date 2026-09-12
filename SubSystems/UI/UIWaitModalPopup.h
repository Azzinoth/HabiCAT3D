#pragma once

#include "../../EngineInclude.h"

struct WaitModalPopupData
{
	std::string Title;
	std::string Message;

	std::function<void()> AfterPopupWasRendered = nullptr;
};

class WaitModalPopup
{
public:
	SINGLETON_PUBLIC_PART(WaitModalPopup)

	void OpenPopup(const std::string& Title, const std::string& Message, std::function<void()> AfterPopupWasRendered = nullptr);
	void Render();
	
private:
	SINGLETON_PRIVATE_PART(WaitModalPopup)

	WaitModalPopupData CurrentPopupData;
	bool bActive = false;
	int PresentedFrames = 0;
	float PresentedTime = 0.0f;
};

#define WAIT_MODAL_POPUP WaitModalPopup::GetInstance()