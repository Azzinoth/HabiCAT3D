#include "COLMAPDataManager.h"
using namespace FocalEngine;

COLMAPDataManager::COLMAPDataManager()
{
	ImagesInstancedShader = RESOURCE_MANAGER.CreateShader("ImagesInstancedShader", RESOURCE_MANAGER.LoadGLSL("SubSystems/Shaders/COLMAPShaders/ImagesInstancedShader_VS.glsl").c_str(),
																				   RESOURCE_MANAGER.LoadGLSL("SubSystems/Shaders/COLMAPShaders/ImagesInstancedShader_FS.glsl").c_str());

	ImagesInstancedShader->UpdateUniformData("HighlightedCellIndex", -1);
	ImagesInstancedShader->UpdateUniformData("SelectedCellIndex", -1);

	ImagesInstancedMaterial = RESOURCE_MANAGER.CreateMaterial("ImagesInstancedMaterial");
	ImagesInstancedMaterial->Shader = ImagesInstancedShader;

	FEMesh* PlaneMesh = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
	ImagesInstancedGameModel = RESOURCE_MANAGER.CreateGameModel(PlaneMesh, ImagesInstancedMaterial);

	APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(COLMAPDataManager::MouseButtonCallback);
}

COLMAPDataManager::~COLMAPDataManager() {}

bool COLMAPDataManager::CreateVisualsForNewProject(COLMAPProject* NewProject)
{
	if (NewProject == nullptr)
		return false;

	if (NewProject->GetPhotogrammetryAnchorEntity() == nullptr)
	{
		FEEntity* NewAnchor = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("COLMAPImagesAnchor");
		NewProject->PhotogrammetryAnchorID = NewAnchor->GetObjectID();
		FEEntity* MainEntity = MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(NewProject->ParentAnalysisObjectID);
		NewAnchor->AttachTo(MainEntity, false);
	}

	for (const auto& CurrentCameraID : NewProject->GetPhysicalCamerasIDList())
		if (!NewProject->CreateCameraSceneRepresentation(CurrentCameraID))
			return false;
	
	if (!NewProject->CreateImagesInstancedSceneRepresentation())
		return false;
	
	if (NewProject->IsTiePointsLoaded())
	{
		if (!NewProject->CreateTiePointsSceneRepresentation())
			return false;
	}

	return true;
}

COLMAPProject* COLMAPDataManager::CreateNewProject(std::string& ParentAnalysisObjectID, std::string& FolderPath, COLMAPFoundData WhatToLoad)
{
	COLMAPProject* Result = nullptr;

	AnalysisObject* ParentAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(ParentAnalysisObjectID);
	if (ParentAnalysisObject == nullptr)
		return Result;

	COLMAPProject* AlreadyExistingProject = GetProjectByAnalysisObjectID(ParentAnalysisObjectID);
	if (AlreadyExistingProject != nullptr)
		return Result;

	if (!FILE_SYSTEM.DoesDirectoryExist(FolderPath))
		return Result;

	Result = new COLMAPProject();
	Result->ParentAnalysisObjectID = ParentAnalysisObjectID;
	if (!Result->LoadFromFolder(FolderPath, WhatToLoad.bTiePointsData))
	{
		delete Result;
		Result = nullptr;
		return Result;
	}

	if (!CreateVisualsForNewProject(Result))
	{
		delete Result;
		Result = nullptr;
		return Result;
	}

	FEEntity* Anchor = Result->GetPhotogrammetryAnchorEntity();
	if (Anchor == nullptr)
	{
		delete Result;
		Result = nullptr;
		return Result;
	}

	glm::vec3 ParentShift = ParentAnalysisObject->GetAppliedShift();
	Anchor->GetComponent<FETransformComponent>().SetPosition(-ParentShift);

	Projects[Result->GetID()] = Result;
	return Result;
}

COLMAPProject* COLMAPDataManager::GetProjectByID(const std::string& ProjectID)
{
	if (Projects.find(ProjectID) == Projects.end())
		return nullptr;

	return Projects[ProjectID];
}

bool COLMAPDataManager::DeleteProject(const std::string& ProjectID)
{
	COLMAPProject* ProjectToDelete = GetProjectByID(ProjectID);
	if (ProjectToDelete == nullptr)
		return false;

	delete ProjectToDelete;
	Projects.erase(ProjectID);
	return true;
}

std::vector<std::string> COLMAPDataManager::GetProjectsIDList() const
{
	std::vector<std::string> Result;
	for (const auto& CurrentProject : Projects)
		Result.push_back(CurrentProject.first);

	return Result;
}

COLMAPProject* COLMAPDataManager::GetProjectByAnalysisObjectID(const std::string& AnalysisObjectID)
{
	for (const auto& CurrentProject : Projects)
	{
		if (CurrentProject.second->ParentAnalysisObjectID == AnalysisObjectID)
			return CurrentProject.second;
	}

	return nullptr;
}

COLMAPProject* COLMAPDataManager::GetProjectByEntityID(const std::string& EntityID)
{
	// FE_TO_DO: Using slow linear search for now, optimize later if needed.
	for (const auto& CurrentProject : Projects)
	{
		if (CurrentProject.second->PhotogrammetryAnchorID == EntityID ||
			CurrentProject.second->TiePointsEntityID == EntityID ||
			CurrentProject.second->ImagesInstancedEntityID == EntityID)
			return CurrentProject.second;
	}

	return nullptr;
}

void COLMAPDataManager::MouseButtonCallback(int Button, int Action, int Mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
		return;
	}

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
	{
		for (const auto& CurrentProject : COLMAP_DATA_MANAGER.Projects)
			CurrentProject.second->MouseButtonCallback(Button, Action, Mods);
	}
}

bool COLMAPDataManager::IsPhotoFolderFound(const std::string& FolderPath) const
{
	std::string PhotoFolderFullPath = FolderPath + COLMAP_PHOTO_FOLDER;
	if (!FILE_SYSTEM.DoesDirectoryExist(PhotoFolderFullPath))
		return false;

	std::vector<std::string> PhotosNames = FILE_SYSTEM.GetFileNamesInDirectory(PhotoFolderFullPath);
	if (PhotosNames.empty())
		return false;

	// Check if there is at least one image file in the folder.
	std::vector<std::string> ImagesExtensions = { ".jpg", ".jpeg", ".png", ".bmp" };
	for (auto& CurrentPhotoName : PhotosNames)
	{
		std::string CurrentPhotoExtension = FILE_SYSTEM.GetFileExtension(CurrentPhotoName);
		std::transform(CurrentPhotoExtension.begin(), CurrentPhotoExtension.end(), CurrentPhotoExtension.begin(), ::tolower);
		if (std::find(ImagesExtensions.begin(), ImagesExtensions.end(), CurrentPhotoExtension) != ImagesExtensions.end())
			return true;
	}

	return true;
}

COLMAPFoundData COLMAPDataManager::FindCOLMAPDataInFolder(const std::string& FolderPath) const
{
	COLMAPFoundData Result;

	if (!FILE_SYSTEM.DoesDirectoryExist(FolderPath))
		return Result;

	std::string AbsoluteFolderPath = FILE_SYSTEM.GetAbsolutePath(FolderPath) + "\\";
	if (!FILE_SYSTEM.DoesDirectoryExist(AbsoluteFolderPath))
		return Result;

	Result.bCamerasData = FILE_SYSTEM.DoesFileExist(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_CAMERAS_FILE);
	Result.bImagesData = FILE_SYSTEM.DoesFileExist(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_IMAGES_FILE);
	Result.bTiePointsData = FILE_SYSTEM.DoesFileExist(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_TIE_POINTS_FILE);
	Result.bPhotos = IsPhotoFolderFound(AbsoluteFolderPath);

	return Result;
}