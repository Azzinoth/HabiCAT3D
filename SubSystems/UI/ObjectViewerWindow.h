#include "../AnalysisObjectManager.h"

class ObjectViewerWindow
{
	SINGLETON_PRIVATE_PART(ObjectViewerWindow)

	bool bVisible = true;
public:
	SINGLETON_PUBLIC_PART(ObjectViewerWindow)

	bool IsVisible() const;
	void SetVisible(bool NewValue);

	void Render();
};

#define OBJECT_VIEWER_WINDOW ObjectViewerWindow::GetInstance()