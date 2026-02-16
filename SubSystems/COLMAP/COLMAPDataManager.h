#pragma once

#include "COLMAPProject.h"
using namespace FocalEngine;

struct COLMAPFoundData
{
	bool bCamerasData = false;
	bool bImagesData = false;
	bool bTiePointsData = false;
	bool bPhotos = false;
};

class COLMAPDataManager
{
	friend class COLMAPProject;
	SINGLETON_PRIVATE_PART(COLMAPDataManager)

	std::unordered_map<std::string, COLMAPProject*> Projects;
	bool CreateVisualsForNewProject(COLMAPProject* NewProject);

	FEShader* ImagesInstancedShader = nullptr;
	FEMaterial* ImagesInstancedMaterial = nullptr;
	FEGameModel* ImagesInstancedGameModel = nullptr;

	COLMAPProject* GetProjectByEntityID(const std::string& EntityID);

	static void MouseButtonCallback(int Button, int Action, int Mods);
	bool IsPhotoFolderFound(const std::string& FolderPath) const;
public:
	SINGLETON_PUBLIC_PART(COLMAPDataManager)

	COLMAPProject* CreateNewProject(std::string& ParentAnalysisObjectID, std::string& FolderPath, COLMAPFoundData WhatToLoad = {true, true, true, true});
	COLMAPProject* GetProjectByID(const std::string& ProjectID);
	COLMAPProject* GetProjectByAnalysisObjectID(const std::string& AnalysisObjectID);
	bool DeleteProject(const std::string& ProjectID);
	std::vector<std::string> GetProjectsIDList() const;
	COLMAPFoundData FindCOLMAPDataInFolder(const std::string& FolderPath) const;
};

#define COLMAP_DATA_MANAGER COLMAPDataManager::GetInstance()