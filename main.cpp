#include "FEMesh.h"
#include "SubSystems/FEFreeCamera.h"
using namespace FocalEngine;

#include "ThirdParty/CGAL/Simple_cartesian.h"
#include "ThirdParty/CGAL/Surface_mesh.h"
#include "ThirdParty/CGAL/Surface_mesh_simplification/edge_collapse.h"
#include "ThirdParty/CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h"

#include "ThirdParty/CGAL/boost/graph/IO/OBJ.h"
//#include "ThirdParty/CGAL/Polygon_mesh_processing/orient_polygon_soup.h"
#include "ThirdParty/CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h"
#include "ThirdParty/CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h"
#include "ThirdParty/CGAL/Polygon_mesh_processing/repair_polygon_soup.h"

//#include "ThirdParty/CGAL/IO/OBJ.h"

typedef CGAL::Simple_cartesian<double>  Kernel;
typedef Kernel::Point_3                 Point_3;
typedef CGAL::Surface_mesh<Point_3>		Surface_mesh;

typedef std::vector<std::size_t>		Polygon_3;

namespace SMS = CGAL::Surface_mesh_simplification;
namespace PMP = CGAL::Polygon_mesh_processing;

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

FEMesh* rawDataToMesh(float* positions, int posSize,
					  float* UV, int UVSize,
				      float* normals, int normSize,
					  float* tangents, int tanSize,
					  int* indices, int indexSize,
					  float* matIndexs, int matIndexsSize, int matCount,
					  std::string Name)
{
	int vertexType = FE_POSITION | FE_INDEX;

	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint indicesBufferID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &indicesBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBufferID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexSize, indices, GL_STATIC_DRAW));

	GLuint positionsBufferID;
	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &positionsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, positionsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * posSize, positions, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint normalsBufferID = 0;
	if (normals != nullptr)
	{
		vertexType |= FE_UV;	
		// normals
		FE_GL_ERROR(glGenBuffers(1, &normalsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, normalsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normSize, normals, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint tangentsBufferID = 0;
	if (tangents != nullptr)
	{
		vertexType |= FE_TANGENTS;
		// tangents
		FE_GL_ERROR(glGenBuffers(1, &tangentsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, tangentsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tanSize, tangents, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint UVBufferID = 0;
	if (UV != nullptr)
	{
		// UV
		FE_GL_ERROR(glGenBuffers(1, &UVBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, UVBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVSize, UV, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	FEMesh* newMesh = new FEMesh(vaoID, indexSize, vertexType, Name);
	newMesh->indicesCount = indexSize;
	newMesh->indicesBufferID = indicesBufferID;

	newMesh->positionsCount = posSize;
	newMesh->positionsBufferID = positionsBufferID;

	newMesh->normalsCount = normSize;
	newMesh->normalsBufferID = normalsBufferID;

	newMesh->tangentsCount = tanSize;
	newMesh->tangentsBufferID = tangentsBufferID;

	newMesh->UVCount = UVSize;
	newMesh->UVBufferID = UVBufferID;

	return newMesh;
}

FEMesh* importOBJ(const char* fileName, bool forceOneMesh)
{
	FEMesh* result = nullptr;
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.forceOneMesh = forceOneMesh;
	objLoader.readFile(fileName);

	if (objLoader.loadedObjects.size() > 0)
	{
		result = rawDataToMesh(objLoader.loadedObjects[0]->fVerC.data(), int(objLoader.loadedObjects[0]->fVerC.size()),
								objLoader.loadedObjects[0]->fTexC.data(), int(objLoader.loadedObjects[0]->fTexC.size()),
								objLoader.loadedObjects[0]->fNorC.data(), int(objLoader.loadedObjects[0]->fNorC.size()),
								objLoader.loadedObjects[0]->fTanC.data(), int(objLoader.loadedObjects[0]->fTanC.size()),
								objLoader.loadedObjects[0]->fInd.data(), int(objLoader.loadedObjects[0]->fInd.size()),
								objLoader.loadedObjects[0]->matIDs.data(), int(objLoader.loadedObjects[0]->matIDs.size()), int(objLoader.loadedObjects[0]->materialRecords.size()), "");
	}

	//createMaterialsFromOBJData(result);

	return result;
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
			loadedMesh = importOBJ(paths[i], true);
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

FEMesh* surfaceMeshToFEMesh(Surface_mesh mesh)
{
	FEMesh* result = nullptr;

	// Extracting data from Surface_mesh.
	std::vector<Point_3> extractedPoints;
	std::vector<Polygon_3> extractedFaces;
	PMP::polygon_mesh_to_polygon_soup(mesh, extractedPoints, extractedFaces);
	PMP::repair_polygon_soup(extractedPoints, extractedFaces);

	// Formating data to FE format.
	std::vector<int> FEIndices;
	for (size_t i = 0; i < extractedFaces.size(); i++)
	{
		FEIndices.push_back(extractedFaces[i][0]);
		FEIndices.push_back(extractedFaces[i][1]);
		FEIndices.push_back(extractedFaces[i][2]);
	}

	std::vector<float> FEVertices;
	for (size_t i = 0; i < extractedPoints.size(); i++)
	{
		FEVertices.push_back(extractedPoints[i][0]);
		FEVertices.push_back(extractedPoints[i][1]);
		FEVertices.push_back(extractedPoints[i][2]);
	}

	result = rawDataToMesh(FEVertices.data(), int(FEVertices.size()),
						   nullptr, 0, nullptr, 0,nullptr, 0,
						   FEIndices.data(), int(FEIndices.size()),
						   nullptr, 0, 0, "");

	return result;
}

Surface_mesh FEMeshToSurfaceMesh(FEMesh* mesh)
{
	// Extracting data from FEMesh.
	std::vector<float> FEVertices;
	FEVertices.resize(mesh->getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getPositionsBufferID(), 0, sizeof(float) * FEVertices.size(), FEVertices.data()));

	std::vector<int> FEIndices;
	FEIndices.resize(mesh->getIndicesCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getIndicesBufferID(), 0, sizeof(int) * FEIndices.size(), FEIndices.data()));

	// Formating data to CGAL format.
	std::vector<Polygon_3> CGALFaces;
	CGALFaces.resize(FEIndices.size() / 3);
	int count = 0;
	for (size_t i = 0; i < FEIndices.size(); i+=3)
	{
		CGALFaces[count].push_back(FEIndices[i]);
		CGALFaces[count].push_back(FEIndices[i + 1]);
		CGALFaces[count].push_back(FEIndices[i + 2]);
		count++;
	}

	
	std::vector<Point_3> CGALPoints;
	for (size_t i = 0; i < FEVertices.size(); i += 3)
	{
		CGALPoints.push_back(Point_3(FEVertices[i], FEVertices[i + 1], FEVertices[i + 2]));
	}

	Surface_mesh result;

	PMP::repair_polygon_soup(CGALPoints, CGALFaces);
	PMP::polygon_soup_to_polygon_mesh(CGALPoints, CGALFaces, result);
	
	return result;
}

void saveSurfaceMeshToOBJFile(std::string fileName, Surface_mesh mesh)
{
	std::vector<Point_3> extractedPoints;
	std::vector<Polygon_3> extractedFaces;
	PMP::polygon_mesh_to_polygon_soup(mesh, extractedPoints, extractedFaces);
	PMP::repair_polygon_soup(extractedPoints, extractedFaces);

	CGAL::IO::write_OBJ(fileName, extractedPoints, extractedFaces);
}

FEMesh* simplify(FEMesh* originalMesh, double verticesLeftInPersent)
{
	if (verticesLeftInPersent == 0)
		return nullptr;

	Surface_mesh surface_mesh = FEMeshToSurfaceMesh(originalMesh);

	// In this example, the simplification stops when the number of undirected edges
	// drops below 10% of the initial count
	double stop_ratio = verticesLeftInPersent;
	SMS::Count_ratio_stop_predicate<Surface_mesh> stop(stop_ratio);

	int r = SMS::edge_collapse(surface_mesh, stop);

	return surfaceMeshToFEMesh(surface_mesh);
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
					simplifiedMesh = simplify(loadedMesh, toLeave / 100.0);
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