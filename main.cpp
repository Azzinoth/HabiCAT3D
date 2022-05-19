#include "FEObjLoader.h"
using namespace FocalEngine;

int vertexCount = 0;

void importOBJ(const char* fileName, bool forceOneMesh)
{
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.forceOneMesh = forceOneMesh;
	objLoader.readFile(fileName);

	//std::vector<FEObject*> result;
	for (size_t i = 0; i < objLoader.loadedObjects.size(); i++)
	{
		//std::string name = getFileNameFromFilePath(fileName) + "_" + std::to_string(i);


		/*result.push_back(rawDataToMesh(objLoader.loadedObjects[i]->fVerC.data(), int(objLoader.loadedObjects[i]->fVerC.size()),
			objLoader.loadedObjects[i]->fTexC.data(), int(objLoader.loadedObjects[i]->fTexC.size()),
			objLoader.loadedObjects[i]->fNorC.data(), int(objLoader.loadedObjects[i]->fNorC.size()),
			objLoader.loadedObjects[i]->fTanC.data(), int(objLoader.loadedObjects[i]->fTanC.size()),
			objLoader.loadedObjects[i]->fInd.data(), int(objLoader.loadedObjects[i]->fInd.size()),
			objLoader.loadedObjects[i]->matIDs.data(), int(objLoader.loadedObjects[i]->matIDs.size()), int(objLoader.loadedObjects[i]->materialRecords.size()), name));*/

		vertexCount = objLoader.loadedObjects[i]->fVerC.size();


		// in rawDataToMesh() hidden FEMesh allocation and it will go to hash table so we need to use setMeshName() not setName.
		//result.back()->setName(name);
		//meshes[result.back()->getObjectID()] = reinterpret_cast<FEMesh*>(result.back());
	}

	//createMaterialsFromOBJData(result);
}

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
			importOBJ(paths[i], true);
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	APPLICATION.createWindow(1280, 720, "Rugosity Calculator");
	APPLICATION.setDropCallback(dropCallback);

	while (APPLICATION.isWindowOpened())
	{
		APPLICATION.beginFrame();

		APPLICATION.setWindowCaption("vertexCount: " + std::to_string(vertexCount));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui::ShowDemoWindow();

		APPLICATION.endFrame();
	}

	return 0;
}