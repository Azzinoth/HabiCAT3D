#include "FEBasicApplication/FEBasicApplication.h"
using namespace FocalEngine;

#define GLM_FORCE_XYZW_ONLY
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.inl"
#include "glm/gtx/quaternion.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	APPLICATION.createWindow(1280, 720, "Rugosity Calculator");

	glm::vec3 testVector = glm::vec3(0.0);

	while (APPLICATION.isWindowOpened())
	{
		APPLICATION.beginFrame();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui::ShowDemoWindow();

		APPLICATION.endFrame();
	}

	return 0;
}