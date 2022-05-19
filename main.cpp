#include "FEBasicApplication/FEBasicApplication.h"
using namespace FocalEngine;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	APPLICATION.createWindow(1280, 720, "Rugosity Calculator");

	while (APPLICATION.isWindowOpened())
	{
		APPLICATION.beginFrame();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui::ShowDemoWindow();

		APPLICATION.endFrame();
	}

	return 0;
}