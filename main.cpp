#include "SubSystems/FECGALWrapper.h"
#include "SubSystems/FEFreeCamera.h"
using namespace FocalEngine;

FEFreeCamera* currentCamera = nullptr;
bool wireframeMode = false;

static const char* const sTestVS = R"(
@In_Position@
@In_Normal@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
	float materialIndex;
} vs_out;

void main(void)
{
	//gl_Position = ProjectionMatrix * vec4(vPos, 1.0);

	vec4 worldPosition = FEWorldMatrix * vec4(FEPosition, 1.0);
	vs_out.worldPosition = worldPosition.xyz;
	vs_out.viewPosition = FEViewMatrix * worldPosition;
	gl_Position = FEProjectionMatrix * vs_out.viewPosition;

	vs_out.vertexNormal = normalize(vec3(FEWorldMatrix * vec4(FENormal, 0.0)));
}
)";

static const char* const sTestFS = R"(
in VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
	flat float materialIndex;
} FS_IN;

@ViewMatrix@
@ProjectionMatrix@

uniform vec3 lightDirection;

layout (location = 0) out vec4 out_Color;

void main(void)
{
	vec3 baseColor = vec3(0.0, 0.5, 1.0);

	//vec3 lightDirection = normalize(vec3(0.0, 1.0, 0.2));
	float diffuseFactor = max(dot(FS_IN.vertexNormal, lightDirection), 0.15);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 2.8f;

	out_Color = vec4(ambientColor * diffuseFactor * baseColor, 1.0f);
}
)";

void renderTargetCenterForCamera()
{
	int centerX, centerY = 0;
	int shiftX, shiftY = 0;

	int xpos, ypos;
	APPLICATION.getWindowPosition(&xpos, &ypos);

	//if (renderTargetMode == FE_GLFW_MODE)
	//{
		int windowW, windowH = 0;
		APPLICATION.getWindowSize(&windowW, &windowH);
		centerX = xpos + (windowW / 2);
		centerY = ypos + (windowH / 2);

		shiftX = xpos;
		shiftY = ypos;
	/*}
	else if (renderTargetMode == FE_CUSTOM_MODE)
	{
		centerX = xpos + renderTargetXShift + (renderTargetW / 2);
		centerY = ypos + renderTargetYShift + (renderTargetH / 2);

		shiftX = renderTargetXShift + xpos;
		shiftY = renderTargetYShift + ypos;
	}*/

	currentCamera->setRenderTargetCenterX(centerX);
	currentCamera->setRenderTargetCenterY(centerY);

	currentCamera->setRenderTargetShiftX(shiftX);
	currentCamera->setRenderTargetShiftY(shiftY);
}

FEMesh* loadedMesh = nullptr;
FEMesh* simplifiedMesh = nullptr;

static void dropCallback(int count, const char** paths);
void dropCallback(int count, const char** paths)
{
	for (size_t i = 0; i < size_t(count); i++)
	{
		if (FILE_SYSTEM.isFolder(paths[i]) && count == 1)
		{
			/*if (PROJECT_MANAGER.getCurrent() == nullptr)
			{
				PROJECT_MANAGER.setProjectsFolder(paths[i]);
			}*/
		}

		if (!FILE_SYSTEM.checkFile(paths[i]))
		{
			//LOG.add("Can't locate file: " + std::string(fileName) + " in FEResourceManager::importAsset", FE_LOG_ERROR, FE_LOG_LOADING);
			continue;
		}

		std::string fileExtention = FILE_SYSTEM.getFileExtension(paths[i]);
		if (fileExtention == ".obj")
		{
			loadedMesh = CGALWrapper.importOBJ(paths[i], true);
		}

		//if (PROJECT_MANAGER.getCurrent() != nullptr)
		//{
		//	std::vector<FEObject*> loadedObjects = RESOURCE_MANAGER.importAsset(paths[i]);
		//	for (size_t i = 0; i < loadedObjects.size(); i++)
		//	{
		//		if (loadedObjects[i] != nullptr)
		//		{
		//			if (loadedObjects[i]->getType() == FE_ENTITY)
		//			{
		//				//SCENE.addEntity(reinterpret_cast<FEEntity*>(loadedObjects[i]));
		//			}
		//			else
		//			{
		//				VIRTUAL_FILE_SYSTEM.createFile(loadedObjects[i], VIRTUAL_FILE_SYSTEM.getCurrentPath());
		//				PROJECT_MANAGER.getCurrent()->setModified(true);
		//				PROJECT_MANAGER.getCurrent()->addUnSavedObject(loadedObjects[i]);
		//			}
		//		}
		//	}
		//}
	}
}

void mouseMoveCallback(double xpos, double ypos)
{
	if (currentCamera != nullptr)
		currentCamera->mouseMoveInput(xpos, ypos);
}

void keyButtonCallback(int key, int scancode, int action, int mods)
{
	currentCamera->keyboardInput(key, scancode, action, mods);
}

void mouseButtonCallback(int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		currentCamera->setIsInputActive(false);
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		currentCamera->setIsInputActive(true);
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		currentCamera->setIsInputActive(false);
	}
}

void renderFEMesh(FEMesh* mesh)
{
	FE_GL_ERROR(glBindVertexArray(mesh->getVaoID()));
	if ((mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));

	if ((mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	if ((mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount()));

	glBindVertexArray(0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	
	//Surface_mesh surface_mesh;

	//std::ifstream objFile("C:/Users/kandr/Downloads/sphere.obj");
	//std::vector<Point_3> points;
	//std::vector<Polygon_3> faces;

	//bool result = CGAL::IO::read_OBJ(objFile, points, faces);


	////PMP::orient_polygon_soup(points, faces); // optional if your mesh is not correctly oriented
	//Surface_mesh sm;
	//try
	//{
	//	PMP::polygon_soup_to_polygon_mesh(points, faces, sm);
	//}
	//catch (const std::exception& e)
	//{
	//	int y = 0;
	//	y++;
	//}
	//
	//// In this example, the simplification stops when the number of undirected edges
	//// drops below 10% of the initial count
	//double stop_ratio = 0.5;
	//SMS::Count_ratio_stop_predicate<Surface_mesh> stop(stop_ratio);

	//int r = SMS::edge_collapse(sm, stop);

	

	//saveSurfaceMeshToOBJFile("C:/Users/kandr/Downloads/sphereR_.obj", sm);

	

	//APPLICATION.createWindow(1280, 720, "Rugosity Calculator");
	APPLICATION.createWindow(1920, 1080, "Rugosity Calculator");
	APPLICATION.setDropCallback(dropCallback);
	APPLICATION.setKeyCallback(keyButtonCallback);
	APPLICATION.setMouseMoveCallback(mouseMoveCallback);
	APPLICATION.setMouseButtonCallback(mouseButtonCallback);

	glClearColor(153.0f / 255.0f, 217.0f / 255.0f, 234.0f / 255.0f, 1.0f);
	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));

	FEShader* testShader = new FEShader("mainShader", sTestVS, sTestFS);
	testShader->getParameter("lightDirection")->updateData(glm::vec3(0.0, 1.0, 0.2));

	static int FEWorldMatrix_hash = int(std::hash<std::string>{}("FEWorldMatrix"));
	static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));

	FETransformComponent* position = new FETransformComponent();


	currentCamera = new FEFreeCamera("mainCamera");
	currentCamera->setIsInputActive(false);
	currentCamera->setAspectRatio(1280.0f / 720.0f);


	/*FEMesh* test = surfaceMeshToFEMesh(sm);
	loadedMesh = test;

	saveSurfaceMeshToOBJFile("C:/Users/kandr/Downloads/sphereR_.obj", FEMeshTosurfaceMesh(test));*/


	while (APPLICATION.isWindowOpened())
	{
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		APPLICATION.beginFrame();

		renderTargetCenterForCamera();
		currentCamera->move(10);

		if (loadedMesh != nullptr)
		{
			testShader->start();

			auto iterator = testShader->parameters.begin();
			while (iterator != testShader->parameters.end())
			{
				if (iterator->second.nameHash == FEWorldMatrix_hash)
					iterator->second.updateData(position->getTransformMatrix());

				if (iterator->second.nameHash == FEViewMatrix_hash)
					iterator->second.updateData(currentCamera->getViewMatrix());

				if (iterator->second.nameHash == FEProjectionMatrix_hash)
					iterator->second.updateData(currentCamera->getProjectionMatrix());

				iterator++;
			}

			testShader->loadDataToGPU();

			if (wireframeMode)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			if (simplifiedMesh == nullptr)
			{
				renderFEMesh(loadedMesh);
			}
			else
			{
				renderFEMesh(simplifiedMesh);
			}
				


			//APPLICATION.setWindowCaption("vertexCount: " + std::to_string(loadedMesh->getVertexCount()));

			testShader->stop();
		}
			
		// ********************* LIGHT DIRECTION *********************
		glm::vec3 position = *reinterpret_cast<glm::vec3*>(testShader->getParameter("lightDirection")->data);
		ImGui::Text("Light direction : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat((std::string("##X pos : ")).c_str(), &position[0], 0.1f);
		
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat((std::string("##Y pos : ")).c_str(), &position[1], 0.1f);
		
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat((std::string("##Z pos : ")).c_str(), &position[2], 0.1f);
	
		testShader->getParameter("lightDirection")->updateData(position);

		ImGui::Checkbox("Wireframe", &wireframeMode);

		if (loadedMesh != nullptr)
		{
			/*if (simplifiedMesh == nullptr)
			{*/
				static float toLeave = 50;
				ImGui::Text("Leave :");
				ImGui::SetNextItemWidth(80);
				ImGui::DragFloat("##Leave", &toLeave, 0.01f, 0.001f, 99.0f);
				ImGui::SameLine();
				ImGui::Text(" %% of vertices.");

				if (ImGui::Button("Apply"))
				{
					simplifiedMesh = CGALWrapper.SurfaceMeshSimplification(loadedMesh, toLeave / 100.0);
				}
			/*}
			else
			{

			}*/
		}
		
		
		//ImGui::ShowDemoWindow();

		APPLICATION.endFrame();
	}

	return 0;
}