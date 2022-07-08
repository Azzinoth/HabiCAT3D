#include "SubSystems/FECGALWrapper.h"
using namespace FocalEngine;

static const char* const sTestVS = R"(
@In_Position@
@In_Normal@

layout (location = 8) in float RugosityData;

@In_Color@
@In_Segments_colors@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

uniform int colorMode;

out VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
	float materialIndex;

	vec3 color;
	vec3 segmentsColors;

	float Rugosity;
} vs_out;

void main(void)
{
	//gl_Position = ProjectionMatrix * vec4(vPos, 1.0);

	vec4 worldPosition = FEWorldMatrix * vec4(FEPosition, 1.0);
	vs_out.worldPosition = worldPosition.xyz;
	vs_out.viewPosition = FEViewMatrix * worldPosition;
	gl_Position = FEProjectionMatrix * vs_out.viewPosition;

	vs_out.vertexNormal = normalize(vec3(FEWorldMatrix * vec4(FENormal, 0.0)));

	if (colorMode == 1)
		vs_out.color = FEColor;

	if (colorMode == 2)
		vs_out.segmentsColors = FESegmentsColors;

	vs_out.Rugosity = RugosityData;
}
)";

static const char* const sTestFS = R"(
in VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	flat vec3 vertexNormal;
	flat float materialIndex;

	flat vec3 color;
	flat vec3 segmentsColors;

	float Rugosity;
} FS_IN;

@ViewMatrix@
@ProjectionMatrix@

uniform int colorMode;
uniform vec3 lightDirection;

uniform float minRugorsity;
uniform float maxRugorsity;

layout (location = 0) out vec4 out_Color;

vec3 darkBlue = vec3(0.0f, 0.0f, 0.4f);
vec3 lightCyan = vec3(27.0f / 255.0f, 213.0f / 255.0f, 200.0f / 255.0f);
vec3 green = vec3(0.0f / 255.0f, 255.0f / 255.0f, 64.0f / 255.0f);
vec3 yellow = vec3(225.0f / 255.0f, 225.0f / 255.0f, 0.0f / 255.0f);
vec3 red = vec3(225.0f / 255.0f, 0 / 255.0f, 0.0f / 255.0f);

void main(void)
{
	vec3 baseColor = vec3(0.0, 0.5, 1.0);

	if (colorMode == 1)
		baseColor = FS_IN.color;

	if (colorMode == 2)
		baseColor = FS_IN.segmentsColors;


	float normalizedRugorsity = (FS_IN.Rugosity - minRugorsity) / (maxRugorsity - minRugorsity);
	

	if (normalizedRugorsity <= 0.125 && normalizedRugorsity > 0.0)
	{
		float DistanceToLower = abs(normalizedRugorsity - 0.0);
		float DistanceToUpper = abs(normalizedRugorsity - 0.125);

		float DistanceToLowerCof = 1.0f - DistanceToLower / abs(0.125 - 0.0);
		float DistanceToUpperCof = 1.0f - DistanceToUpper / abs(0.125 - 0.0);

		vec3 mix = darkBlue * DistanceToLowerCof + lightCyan * DistanceToUpperCof;

		baseColor = mix;
		//setColorOfFace(i, mix);
	}
	else if (normalizedRugorsity <= 0.25 && normalizedRugorsity > 0.125)
	{
		float DistanceToLower = abs(normalizedRugorsity - 0.125);
		float DistanceToUpper = abs(normalizedRugorsity - 0.25);

		float DistanceToLowerCof = 1.0f - DistanceToLower / abs(0.25 - 0.125);
		float DistanceToUpperCof = 1.0f - DistanceToUpper / abs(0.25 - 0.125);

		vec3 mix = lightCyan * DistanceToLowerCof + green * DistanceToUpperCof;

		baseColor = mix;
		//setColorOfFace(i, mix);
	}
	else if (normalizedRugorsity <= 0.5 && normalizedRugorsity > 0.25)
	{
		float DistanceToLower = abs(normalizedRugorsity - 0.25);
		float DistanceToUpper = abs(normalizedRugorsity - 0.5);

		float DistanceToLowerCof = 1.0f - DistanceToLower / abs(0.5 - 0.25);
		float DistanceToUpperCof = 1.0f - DistanceToUpper / abs(0.5 - 0.25);

		vec3 mix = green * DistanceToLowerCof + yellow * DistanceToUpperCof;

		baseColor = mix;
		//setColorOfFace(i, mix);
	}
	else if (normalizedRugorsity <= 0.75 && normalizedRugorsity > 0.5)
	{
		float DistanceToLower = abs(normalizedRugorsity - 0.5);
		float DistanceToUpper = abs(normalizedRugorsity - 0.75);

		float DistanceToLowerCof = 1.0f - DistanceToLower / abs(0.75 - 0.5);
		float DistanceToUpperCof = 1.0f - DistanceToUpper / abs(0.75 - 0.5);

		vec3 mix = yellow * DistanceToLowerCof + red * DistanceToUpperCof;

		baseColor = mix;
		//setColorOfFace(i, mix);
	}
	else
	{
		baseColor = red;
		//setColorOfFace(i, red);
	}




	float diffuseFactor = max(dot(FS_IN.vertexNormal, lightDirection), 0.15);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 2.8f;

	out_Color = vec4(ambientColor * diffuseFactor * baseColor, 1.0f);
}
)";


void showTransformConfiguration(std::string name, FETransformComponent* transform)
{
	// ********************* POSITION *********************
	glm::vec3 position = transform->getPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + name).c_str(), &position[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + name).c_str(), &position[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + name).c_str(), &position[2], 0.1f);
	transform->setPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = transform->getRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + name).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + name).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + name).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	transform->setRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &transform->uniformScaling);
	glm::vec3 scale = transform->getScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + name).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + name).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + name).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);

	glm::vec3 oldScale = transform->getScale();
	transform->changeXScaleBy(scale[0] - oldScale[0]);
	transform->changeYScaleBy(scale[1] - oldScale[1]);
	transform->changeZScaleBy(scale[2] - oldScale[2]);
}

void showCameraTransform(FEFreeCamera* camera)
{
	// ********* POSITION *********
	glm::vec3 cameraPosition = camera->getPosition();

	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##X pos", &cameraPosition[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Y pos", &cameraPosition[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Z pos", &cameraPosition[2], 0.1f);

	camera->setPosition(cameraPosition);

	// ********* ROTATION *********
	glm::vec3 cameraRotation = glm::vec3(camera->getYaw(), camera->getPitch(), camera->getRoll());

	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##X rot", &cameraRotation[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Y rot", &cameraRotation[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Z rot", &cameraRotation[2], 0.1f);

	camera->setYaw(cameraRotation[0]);
	camera->setPitch(cameraRotation[1]);
	camera->setRoll(cameraRotation[2]);

	float cameraSpeed = camera->getMovementSpeed();
	ImGui::Text("Camera speed in m/s : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Camera_speed", &cameraSpeed, 0.01f, 0.01f, 100.0f);
	camera->setMovementSpeed(cameraSpeed);
}

FEFreeCamera* currentCamera = nullptr;
bool wireframeMode = false;
FEShader* meshShader = nullptr;

double mouseX;
double mouseY;

SDF* currentSDF = nullptr;
bool showCellsWithTriangles = false;
bool showTrianglesInCells = true;

glm::dvec3 mouseRay(double mouseX, double mouseY)
{
	int W, H;
	APPLICATION.getWindowSize(&W, &H);

	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / W - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / H;

	glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(currentCamera->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(currentCamera->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

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

void addLinesOFSDF(SDF* SDF)
{
	for (size_t i = 0; i < SDF->data.size(); i++)
	{
		for (size_t j = 0; j < SDF->data[i].size(); j++)
		{
			for (size_t k = 0; k < SDF->data[i][j].size(); k++)
			{
				bool render = false;
				SDF->data[i][j][k].wasRenderedLastFrame = false;

				if (SDF->data[i][j][k].trianglesInCell.size() > 0)
					render = true;

				if (render)
				{
					glm::vec3 color = glm::vec3(0.1f, 0.6f, 0.1f);
					if (SDF->data[i][j][k].selected)
						color = glm::vec3(0.9f, 0.1f, 0.1f);

					LINE_RENDERER.RenderAABB(SDF->data[i][j][k].AABB, color);

					SDF->data[i][j][k].wasRenderedLastFrame = true;

					if (showTrianglesInCells && SDF->data[i][j][k].selected)
					{
						for (size_t l = 0; l < SDF->data[i][j][k].trianglesInCell.size(); l++)
						{
							auto currentTriangle = SDF->mesh->Triangles[SDF->data[i][j][k].trianglesInCell[l]];
							
							LINE_RENDERER.AddLineToBuffer(FELine(currentTriangle[0], currentTriangle[1], glm::vec3(1.0f, 1.0f, 0.0f)));
							LINE_RENDERER.AddLineToBuffer(FELine(currentTriangle[0], currentTriangle[2], glm::vec3(1.0f, 1.0f, 0.0f)));
							LINE_RENDERER.AddLineToBuffer(FELine(currentTriangle[1], currentTriangle[2], glm::vec3(1.0f, 1.0f, 0.0f)));
						}
					}
				}
			}
		}
	}
}

FEMesh* loadedMesh = nullptr;
FEMesh* simplifiedMesh = nullptr;
bool showSimplified = false;

FEMesh* currentMesh = nullptr;
std::vector<std::string> dimentionsList;
int SDFDimention = 16;

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
			currentMesh = loadedMesh;
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

	mouseX = xpos;
	mouseY = ypos;
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

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		if (currentMesh != nullptr)
		{
			if (!showCellsWithTriangles)
			{
				currentMesh->SelectTriangle(mouseRay(mouseX, mouseY), currentCamera);
				LINE_RENDERER.clearAll();

				if (currentMesh->TriangleSelected != -1)
				{
					LINE_RENDERER.AddLineToBuffer(FELine(currentMesh->Triangles[currentMesh->TriangleSelected][0], currentMesh->Triangles[currentMesh->TriangleSelected][1], glm::vec3(1.0f, 1.0f, 0.0f)));
					LINE_RENDERER.AddLineToBuffer(FELine(currentMesh->Triangles[currentMesh->TriangleSelected][0], currentMesh->Triangles[currentMesh->TriangleSelected][2], glm::vec3(1.0f, 1.0f, 0.0f)));
					LINE_RENDERER.AddLineToBuffer(FELine(currentMesh->Triangles[currentMesh->TriangleSelected][1], currentMesh->Triangles[currentMesh->TriangleSelected][2], glm::vec3(1.0f, 1.0f, 0.0f)));

					if (!currentMesh->TrianglesNormals.empty())
					{
						glm::vec3 Point = currentMesh->Triangles[currentMesh->TriangleSelected][0];
						glm::vec3 Normal = currentMesh->TrianglesNormals[currentMesh->TriangleSelected][0];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

						Point = currentMesh->Triangles[currentMesh->TriangleSelected][1];
						Normal = currentMesh->TrianglesNormals[currentMesh->TriangleSelected][1];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

						Point = currentMesh->Triangles[currentMesh->TriangleSelected][2];
						Normal = currentMesh->TrianglesNormals[currentMesh->TriangleSelected][2];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
					}

					if (!currentMesh->originalTrianglesToSegments.empty() && !currentMesh->segmentsNormals.empty())
					{
						glm::vec3 Centroid = (currentMesh->Triangles[currentMesh->TriangleSelected][0] +
							currentMesh->Triangles[currentMesh->TriangleSelected][1] +
							currentMesh->Triangles[currentMesh->TriangleSelected][2]) / 3.0f;

						glm::vec3 Normal = currentMesh->segmentsNormals[currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected]];
						LINE_RENDERER.AddLineToBuffer(FELine(Centroid, Centroid + Normal, glm::vec3(1.0f, 0.0f, 0.0f)));
					}

					LINE_RENDERER.SyncWithGPU();
				}
			}
			else
			{
				currentSDF->mouseClick(mouseX, mouseY);

				LINE_RENDERER.clearAll();

				if (showCellsWithTriangles)
					addLinesOFSDF(currentSDF);

				LINE_RENDERER.SyncWithGPU();
			}
			
		}
	}
}

void renderFEMesh(FEMesh* mesh)
{
	meshShader->getParameter("colorMode")->updateData(mesh->colorMode);

	meshShader->getParameter("minRugorsity")->updateData(float(mesh->minRugorsity));
	meshShader->getParameter("maxRugorsity")->updateData(float(mesh->maxRugorsity));

	FE_GL_ERROR(glBindVertexArray(mesh->getVaoID()));
	if ((mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));

	if ((mesh->vertexAttributes & FE_SEGMENTS_COLORS) == FE_SEGMENTS_COLORS) FE_GL_ERROR(glEnableVertexAttribArray(7));
	// Rugosity
	if (!mesh->rugosityData.empty())
		FE_GL_ERROR(glEnableVertexAttribArray(8));

	if ((mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	if ((mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount()));

	glBindVertexArray(0);
}

void calculateSDF(FEMesh* mesh, int dimentions)
{
	FEAABB finalAABB = mesh->AABB;

	currentSDF = new SDF(mesh, dimentions, finalAABB, currentCamera);

	currentSDF->fillCellsWithTriangleInfo();
	currentSDF->calculateRugosity();

	currentSDF->fillMeshWithRugosityData();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	/*Point_3 a = Point_3(1.0, 5.0, 1.0);
	Point_3 b = Point_3(-2.0, 15.0, 4.0);
	Point_3 c = Point_3(-2.0, 5.0, -2.0);

	double area = sqrt(CGAL::squared_area(a, b, c));

	Plane_3 testPlane = Plane_3(Point_3(0.0, 0.0, 0.0), Direction_3(0.0, 1.0, 0.0));

	Point_3 aProjection = testPlane.projection(a);
	Point_3 bProjection = testPlane.projection(b);
	Point_3 cProjection = testPlane.projection(c);

	double projectionArea = sqrt(CGAL::squared_area(aProjection, bProjection, cProjection));
	double rugosity = area / projectionArea;


	int y = 0;
	y++;*/

	//Surface_mesh surface_mesh;

	//std::ifstream objFile("C:/Users/kandr/Downloads/OBJ_Models/sphere.obj");
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

	

	//saveSurfaceMeshToOBJFile("C:/Users/kandr/Downloads/OBJ_Models/sphereR_.obj", sm);

	

	//APPLICATION.createWindow(1280, 720, "Rugosity Calculator");
	APPLICATION.createWindow(1920, 1080, "Rugosity Calculator");
	APPLICATION.setDropCallback(dropCallback);
	APPLICATION.setKeyCallback(keyButtonCallback);
	APPLICATION.setMouseMoveCallback(mouseMoveCallback);
	APPLICATION.setMouseButtonCallback(mouseButtonCallback);

	glClearColor(153.0f / 255.0f, 217.0f / 255.0f, 234.0f / 255.0f, 1.0f);
	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));

	meshShader = new FEShader("mainShader", sTestVS, sTestFS);
	meshShader->getParameter("lightDirection")->updateData(glm::vec3(0.0, 1.0, 0.2));

	static int FEWorldMatrix_hash = int(std::hash<std::string>{}("FEWorldMatrix"));
	static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));

	FETransformComponent* position = new FETransformComponent();


	currentCamera = new FEFreeCamera("mainCamera");
	currentCamera->setIsInputActive(false);
	currentCamera->setAspectRatio(1280.0f / 720.0f);

	dimentionsList.push_back("4");
	dimentionsList.push_back("8");
	dimentionsList.push_back("16");
	dimentionsList.push_back("32");
	dimentionsList.push_back("64");
	dimentionsList.push_back("128");
	dimentionsList.push_back("256");
	dimentionsList.push_back("512");
	dimentionsList.push_back("1024");
	dimentionsList.push_back("2048");
	dimentionsList.push_back("4096");

	while (APPLICATION.isWindowOpened())
	{
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		APPLICATION.beginFrame();

		renderTargetCenterForCamera();
		currentCamera->move(10);

		if (currentMesh != nullptr)
		{
			meshShader->start();

			auto iterator = meshShader->parameters.begin();
			while (iterator != meshShader->parameters.end())
			{
				if (iterator->second.nameHash == FEWorldMatrix_hash)
					iterator->second.updateData(position->getTransformMatrix());

				if (iterator->second.nameHash == FEViewMatrix_hash)
					iterator->second.updateData(currentCamera->getViewMatrix());

				if (iterator->second.nameHash == FEProjectionMatrix_hash)
					iterator->second.updateData(currentCamera->getProjectionMatrix());

				iterator++;
			}

			meshShader->loadDataToGPU();

			if (wireframeMode)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			renderFEMesh(currentMesh);

			/*FETransformComponent* newPosition = new FETransformComponent();
			newPosition->setPosition(glm::vec3(0.0, 0.0, 5.0));
			testShader->getParameter("FEWorldMatrix")->updateData(newPosition->getTransformMatrix());
			testShader->loadDataToGPU();

			renderFEMesh(compareToMesh);*/
			
			//APPLICATION.setWindowCaption("vertexCount: " + std::to_string(loadedMesh->getVertexCount()));

			meshShader->stop();
		}

		LINE_RENDERER.Render(currentCamera);
			
		// ********************* LIGHT DIRECTION *********************
		glm::vec3 position = *reinterpret_cast<glm::vec3*>(meshShader->getParameter("lightDirection")->data);
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
	
		meshShader->getParameter("lightDirection")->updateData(position);

		



		showCameraTransform(currentCamera);




	

		if (currentMesh != nullptr)
		{
			ImGui::Checkbox("Wireframe", &wireframeMode);

			if (simplifiedMesh == nullptr)
				ImGui::BeginDisabled();

			if (ImGui::Checkbox("Show simplified mesh", &showSimplified))
			{
				if (currentMesh != nullptr)
				{
					currentMesh->TriangleSelected = -1;
					LINE_RENDERER.clearAll();
				}

				if (showSimplified)
				{
					currentMesh = simplifiedMesh;
				}
				else
				{
					currentMesh = loadedMesh;
				}
			}

			if (simplifiedMesh == nullptr)
				ImGui::EndDisabled();

			/*static float toLeave = 50;
			ImGui::Text("Leave :");
			ImGui::SetNextItemWidth(80);
			ImGui::DragFloat("##Leave", &toLeave, 0.01f, 0.001f, 99.0f);
			ImGui::SameLine();
			ImGui::Text(" %% of vertices.");*/

			if (currentMesh == simplifiedMesh)
				ImGui::BeginDisabled();

			static int maxSegments = 100;
			ImGui::Text("Max Segments :");
			ImGui::SetNextItemWidth(80);
			ImGui::DragInt("##MaxSegments", &maxSegments, 0.01f, 0.001f, 99.0f);
			ImGui::SameLine();
			//ImGui::Text(" %% of vertices.");

			if (ImGui::Button("Apply"))
			{
				simplifiedMesh = CGALWrapper.SurfaceMeshApproximation(loadedMesh, maxSegments);
			}

			if (currentMesh == simplifiedMesh)
				ImGui::EndDisabled();

			if (currentMesh->minRugorsity != DBL_MAX)
				ImGui::Text(("minRugorsity: " + std::to_string(currentMesh->minRugorsity)).c_str());

			if (currentMesh->maxRugorsity != -DBL_MAX)
				ImGui::Text(("maxRugorsity: " + std::to_string(currentMesh->maxRugorsity)).c_str());

			if (currentMesh->colorCount == unsigned int(-1))
				ImGui::BeginDisabled();

			bool showRugosity = currentMesh->colorMode == 1;
			if (ImGui::Checkbox("Show Rugosity", &showRugosity))
			{
				if (showRugosity)
				{
					currentMesh->colorMode = 1;
				}
				else
				{
					currentMesh->colorMode = 0;
				}
			}

			if (currentMesh->colorCount == unsigned int(-1))
				ImGui::EndDisabled();

			if (currentMesh->segmentsColorsCount == unsigned int(-1))
				ImGui::BeginDisabled();
				
			bool showSegments = currentMesh->colorMode == 2;
			if (ImGui::Checkbox("Show Segments", &showSegments))
			{
				if (showSegments)
				{
					currentMesh->colorMode = 2;
				}
				else
				{
					currentMesh->colorMode = 0;
				}
			}

			if (currentMesh->segmentsColorsCount == unsigned int(-1))
				ImGui::EndDisabled();


			if (currentMesh->TriangleSelected != -1)
			{
				ImGui::Separator();
				ImGui::Text("Selected triangle information :");

				std::string Text = "Index : " + std::to_string(currentMesh->TriangleSelected);
				ImGui::Text(Text.c_str());

				Text = "First vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].z);
				ImGui::Text(Text.c_str());

				Text = "Second vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].z);
				ImGui::Text(Text.c_str());

				Text = "Third vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].z);
				ImGui::Text(Text.c_str());

				Text = "Segment ID : ";
				if (!currentMesh->originalTrianglesToSegments.empty())
				{
					Text += std::to_string(currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected]);
				}
				else
				{
					Text += "No information.";
				}
				ImGui::Text(Text.c_str());

				Text = "Segment normal : ";
				if (!currentMesh->originalTrianglesToSegments.empty() && !currentMesh->segmentsNormals.empty())
				{
					glm::vec3 normal = currentMesh->segmentsNormals[currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected]];

					Text += "x: " + std::to_string(normal.x);
					Text += " y: " + std::to_string(normal.y);
					Text += " z: " + std::to_string(normal.z);
				}
				else
				{
					Text += "No information.";
				}
				ImGui::Text(Text.c_str());
			}

			if (currentSDF == nullptr)
			{
				if (ImGui::Button("Generate SDF"))
					calculateSDF(currentMesh, SDFDimention);
			}
			else if (currentSDF != nullptr)
			{
				if (ImGui::Button("Delete SDF"))
				{
					//currentSDF->mesh clear all rugosity info
					delete currentSDF;
					currentSDF = nullptr;
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(128);
			if (ImGui::BeginCombo("##ChooseSDFDimention", std::to_string(SDFDimention).c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < dimentionsList.size(); i++)
				{
					bool is_selected = (std::to_string(SDFDimention) == dimentionsList[i]);
					if (ImGui::Selectable(dimentionsList[i].c_str(), is_selected))
					{
						SDFDimention = atoi(dimentionsList[i].c_str());
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (currentSDF != nullptr)
			{
				float maxRugorsity = currentMesh->maxRugorsity;
				ImGui::DragFloat("Max rugorsity", &maxRugorsity, 0.01f);
				if (maxRugorsity < currentMesh->minRugorsity)
					maxRugorsity = currentMesh->minRugorsity + 0.1f;
				currentMesh->maxRugorsity = maxRugorsity;

				float TotalTime = 0.0f;

				std::string debugTimers = "Building of SDF grid : " + std::to_string(currentSDF->TimeTookToGenerateInMS) + " ms";
				debugTimers += "\n";
				TotalTime += currentSDF->TimeTookToGenerateInMS;

				debugTimers += "Fill cells with triangle info : " + std::to_string(currentSDF->TimeTookFillCellsWithTriangleInfo) + " ms";
				debugTimers += "\n";
				TotalTime += currentSDF->TimeTookFillCellsWithTriangleInfo;

				debugTimers += "Calculate rugosity : " + std::to_string(currentSDF->TimeTookCalculateRugosity) + " ms";
				debugTimers += "\n";
				TotalTime += currentSDF->TimeTookCalculateRugosity;

				debugTimers += "Assign colors to mesh : " + std::to_string(currentSDF->TimeTookFillMeshWithRugosityData) + " ms";
				debugTimers += "\n";
				TotalTime += currentSDF->TimeTookFillMeshWithRugosityData;

				debugTimers += "Total time : " + std::to_string(TotalTime) + " ms";
				debugTimers += "\n";

				ImGui::Text((debugTimers).c_str());

				ImGui::Text(("debugTotalTrianglesInCells: " + std::to_string(currentSDF->debugTotalTrianglesInCells)).c_str());
				



				if (showRugosity)
				{
					currentMesh->colorMode = 1;
				}
				else
				{
					currentMesh->colorMode = 0;
				}

				
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				if (ImGui::Checkbox("Show cells with triangles", &showCellsWithTriangles))
				{
					LINE_RENDERER.clearAll();

					if (showCellsWithTriangles)
						addLinesOFSDF(currentSDF);

					LINE_RENDERER.SyncWithGPU();
				}
				
			}

			ImGui::Separator();
			if (currentSDF != nullptr && currentSDF->selectedCell != glm::vec3(0.0))
			{
				SDFNode currentlySelectedCell = currentSDF->data[currentSDF->selectedCell.x][currentSDF->selectedCell.y][currentSDF->selectedCell.z];

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text("Selected cell info: ");

				std::string indexes = "i: " + std::to_string(currentSDF->selectedCell.x);
				indexes += " j: " + std::to_string(currentSDF->selectedCell.y);
				indexes += " k: " + std::to_string(currentSDF->selectedCell.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((indexes).c_str());

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text(("value: " + std::to_string(currentlySelectedCell.value)).c_str());

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text(("distanceToTrianglePlane: " + std::to_string(currentlySelectedCell.distanceToTrianglePlane)).c_str());

				ImGui::Separator();

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text("Rugosity info: ");

				std::string text = "averageCellNormal x: " + std::to_string(currentlySelectedCell.averageCellNormal.x);
				text += " y: " + std::to_string(currentlySelectedCell.averageCellNormal.y);
				text += " z: " + std::to_string(currentlySelectedCell.averageCellNormal.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((text).c_str());


				text = "CellTrianglesCentroid x: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.x);
				text += " y: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.y);
				text += " z: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((text).c_str());

				if (currentlySelectedCell.approximateProjectionPlane != nullptr)
				{
					//glm::vec3 NormalStart = currentlySelectedCell.approximateProjectionPlane->PointOnPlane + choosenEntity->transform.getPosition();
					//glm::vec3 NormalEnd = NormalStart + currentlySelectedCell.approximateProjectionPlane->Normal;
					//RENDERER.drawLine(NormalStart, NormalEnd, glm::vec3(0.1f, 0.1f, 0.9f), 0.25f);
				}

				ImGui::Text(("Rugosity : " + std::to_string(currentlySelectedCell.rugosity)).c_str());

				static std::string debugText;

				if (ImGui::Button("Show debug info"))
				{
					currentSDF->calculateCellRugosity(&currentSDF->data[currentSDF->selectedCell.x][currentSDF->selectedCell.y][currentSDF->selectedCell.z], &debugText);
				}

				ImGui::Text(debugText.c_str());
			}
		}
		
		APPLICATION.endFrame();
	}

	return 0;
}