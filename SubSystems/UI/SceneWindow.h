#pragma once

#include "../../EngineInclude.h"

class SceneWindow
{
	friend class UIManager;
public:
	SINGLETON_PUBLIC_PART(SceneWindow)

	void Render();

	bool IsHovered() const;
	bool IsBeingMoved() const;
	bool IsMouseCapturedByUI() const;
	bool MouseWasDraggedSincePress() const;

	ImVec2 GetContentPosition() const;
	ImVec2 GetContentSize() const;
private:
	SINGLETON_PRIVATE_PART(SceneWindow)

	bool bHovered = false;
	bool bBeingMoved = false;
	bool bShouldDockToCentralNode = true;

	ImVec2 ContentPosition = ImVec2(0.0f, 0.0f);
	ImVec2 ContentSize = ImVec2(0.0f, 0.0f);
};

#define SCENE_WINDOW SceneWindow::GetInstance()
