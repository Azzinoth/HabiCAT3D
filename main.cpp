#include "SubSystems/FECGALWrapper.h"
#include "SubSystems/FEFreeCamera.h"
using namespace FocalEngine;

FEFreeCamera* currentCamera = nullptr;
bool wireframeMode = false;
FEShader* meshShader = nullptr;

static const char* const sTestVS = R"(
@In_Position@
@In_Normal@

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
} FS_IN;

@ViewMatrix@
@ProjectionMatrix@

uniform int colorMode;
uniform vec3 lightDirection;

layout (location = 0) out vec4 out_Color;

void main(void)
{
	vec3 baseColor = vec3(0.0, 0.5, 1.0);

	if (colorMode == 1)
		baseColor = FS_IN.color;

	if (colorMode == 2)
		baseColor = FS_IN.segmentsColors;

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
bool showSimplified = false;

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
	meshShader->getParameter("colorMode")->updateData(mesh->colorMode);

	FE_GL_ERROR(glBindVertexArray(mesh->getVaoID()));
	if ((mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));

	if ((mesh->vertexAttributes & FE_SEGMENTS_COLORS) == FE_SEGMENTS_COLORS) FE_GL_ERROR(glEnableVertexAttribArray(7));

	if ((mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	if ((mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount()));

	glBindVertexArray(0);
}

void testFunction();

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

	testFunction();

	while (APPLICATION.isWindowOpened())
	{
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		APPLICATION.beginFrame();

		renderTargetCenterForCamera();
		currentCamera->move(10);

		if (loadedMesh != nullptr)
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

			if (simplifiedMesh == nullptr || !showSimplified)
			{
				renderFEMesh(loadedMesh);
			}
			else if (simplifiedMesh != nullptr && showSimplified)
			{
				renderFEMesh(simplifiedMesh);
			}

			

			/*FETransformComponent* newPosition = new FETransformComponent();
			newPosition->setPosition(glm::vec3(0.0, 0.0, 5.0));
			testShader->getParameter("FEWorldMatrix")->updateData(newPosition->getTransformMatrix());
			testShader->loadDataToGPU();

			renderFEMesh(compareToMesh);*/
			
			//APPLICATION.setWindowCaption("vertexCount: " + std::to_string(loadedMesh->getVertexCount()));

			meshShader->stop();
		}
			
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

		


	

		if (loadedMesh != nullptr)
		{
			ImGui::Checkbox("Wireframe", &wireframeMode);

			if (simplifiedMesh == nullptr)
				ImGui::BeginDisabled();

			ImGui::Checkbox("Show simplified mesh", &showSimplified);

			if (simplifiedMesh == nullptr)
				ImGui::EndDisabled();

			/*static float toLeave = 50;
			ImGui::Text("Leave :");
			ImGui::SetNextItemWidth(80);
			ImGui::DragFloat("##Leave", &toLeave, 0.01f, 0.001f, 99.0f);
			ImGui::SameLine();
			ImGui::Text(" %% of vertices.");*/

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

			if (loadedMesh->minRugorsity != DBL_MAX)
				ImGui::Text(("minRugorsity: " + std::to_string(loadedMesh->minRugorsity)).c_str());

			if (loadedMesh->maxRugorsity != -DBL_MAX)
				ImGui::Text(("maxRugorsity: " + std::to_string(loadedMesh->maxRugorsity)).c_str());

			if (loadedMesh->colorCount == unsigned int(-1))
				ImGui::BeginDisabled();

			bool showRugosity = loadedMesh->colorMode == 1;
			if (ImGui::Checkbox("Show Rugosity", &showRugosity))
			{
				if (showRugosity)
				{
					loadedMesh->colorMode = 1;
				}
				else
				{
					loadedMesh->colorMode = 0;
				}
			}

			if (loadedMesh->colorCount == unsigned int(-1))
				ImGui::EndDisabled();

			if (loadedMesh->segmentsColorsCount == unsigned int(-1))
				ImGui::BeginDisabled();
				
			bool showSegments = loadedMesh->colorMode == 2;
			if (ImGui::Checkbox("Show Segments", &showSegments))
			{
				if (showSegments)
				{
					loadedMesh->colorMode = 2;
				}
				else
				{
					loadedMesh->colorMode = 0;
				}
			}

			if (loadedMesh->segmentsColorsCount == unsigned int(-1))
				ImGui::EndDisabled();
		}
		
		APPLICATION.endFrame();
	}

	return 0;
}

struct SDFNode
{
	float value = 0.0f;
	FEAABB AABB;
};

struct SDF
{
	std::vector< std::vector< std::vector<SDFNode*>>> data;
};

struct centroidData
{
	glm::vec3 centroid;
	glm::vec3 normal;
};

std::vector<centroidData> getTrianglesCentroids(FEMesh* mesh)
{
	std::vector<centroidData> result;

	if (mesh == nullptr)
		return result;

	std::vector<float> FEVertices;
	FEVertices.resize(mesh->getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getPositionsBufferID(), 0, sizeof(float) * FEVertices.size(), FEVertices.data()));

	std::vector<float> FENormals;
	FENormals.resize(mesh->getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getNormalsBufferID(), 0, sizeof(float) * FENormals.size(), FENormals.data()));

	std::vector<int> FEIndices;
	FEIndices.resize(mesh->getIndicesCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getIndicesBufferID(), 0, sizeof(int) * FEIndices.size(), FEIndices.data()));

	for (size_t i = 0; i < FEIndices.size(); i += 3)
	{
		int vertexPosition = FEIndices[i] * 3;
		glm::vec3 firstVertex = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		vertexPosition = FEIndices[i + 1] * 3;
		glm::vec3 secondVertex = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		vertexPosition = FEIndices[i + 2] * 3;
		glm::vec3 thirdVertex = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		glm::vec3 currentCentroid = (firstVertex + secondVertex + thirdVertex) / 3.0f;

		// We are taking index of last vertex because all verticies of triangle should have same normal.
		glm::vec3 triangleNormal = glm::vec3(FENormals[vertexPosition], FENormals[vertexPosition + 1], FENormals[vertexPosition + 2]);

		centroidData data;
		data.centroid = currentCentroid;
		data.normal = triangleNormal;
		result.push_back(data);
	}

	return result;
}

SDF* createSDF(FEMesh* mesh, int dimentions)
{
	if (dimentions < 1 || dimentions > 4096)
		return nullptr;

	// If dimentions is not power of 2, we can't continue.
	if (log2(dimentions) != int(log2(dimentions)))
		return nullptr;

	glm::vec3 center = mesh->AABB.getCenter();
	FEAABB SDFAABB = FEAABB(center - glm::vec3(mesh->AABB.getSize() / 2.0f), center + glm::vec3(mesh->AABB.getSize() / 2.0f));

	SDF* result = new SDF;
	result->data.resize(dimentions);
	for (size_t i = 0; i < dimentions; i++)
	{
		result->data[i].resize(dimentions);
		for (size_t j = 0; j < dimentions; j++)
		{
			result->data[i][j].resize(dimentions);
		}
	}

	glm::vec3 start = SDFAABB.getMin();
	glm::vec3 currentAABBMin;
	float cellSize = SDFAABB.getSize() / dimentions;

	std::vector<centroidData> centroids = getTrianglesCentroids(mesh);

	for (size_t i = 0; i < dimentions; i++)
	{
		for (size_t j = 0; j < dimentions; j++)
		{
			for (size_t k = 0; k < dimentions; k++)
			{
				currentAABBMin = start + glm::vec3(cellSize * i, cellSize * j, cellSize * k);

				result->data[i][j][k] = new SDFNode();
				result->data[i][j][k]->AABB = FEAABB(currentAABBMin, currentAABBMin + glm::vec3(cellSize));

				float minDistance = FLT_MAX;
				int centroidIndex = -1;
				glm::vec3 cellCenter = result->data[i][j][k]->AABB.getCenter();
				for (size_t p = 0; p < centroids.size(); p++)
				{
					float currentDistance = glm::distance(centroids[p].centroid, cellCenter);
					if (currentDistance < minDistance)
					{
						minDistance = currentDistance;
						centroidIndex = p;
					}
				}

				if (centroidIndex != -1)
				{
					glm::vec3 vectorToCentroid = centroids[centroidIndex].centroid - cellCenter;

					float test = glm::dot(vectorToCentroid, centroids[centroidIndex].normal);
					if (test >= 0)
					{
						minDistance = -minDistance;
					}
				}

				result->data[i][j][k]->value = minDistance;
			}
		}
	}

	return result;
}

void testFunction()
{
	FEMesh* testMesh = CGALWrapper.importOBJ("C:/Users/Kindr/Downloads/OBJ_Models/pickle_.obj", true);
	SDF* testSDF = createSDF(testMesh, 8);

	int y = 0;
	y++;
}