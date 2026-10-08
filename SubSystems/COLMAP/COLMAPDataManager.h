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

	std::unordered_map<FEUUID, COLMAPProject*> Projects;
	bool CreateVisualsForNewProject(COLMAPProject* NewProject);
	void RegisterOnSelectedImageChanged(COLMAPProject* Project, int ImageID);
	std::vector<std::function<void(COLMAPProject*, int)>> OnSelectedImageChangedCallbacks;

	FEShader* ImagesInstancedShader = nullptr;
	FEMaterial* ImagesInstancedMaterial = nullptr;
	FEGameModel* ImagesInstancedGameModel = nullptr;

	static void MouseButtonCallback(int Button, int Action, int Mods);
	bool IsPhotoFolderFound(const std::string& FolderPath) const;
public:
	SINGLETON_PUBLIC_PART(COLMAPDataManager)

	COLMAPProject* CreateNewProject(const FEUUID& ParentAnalysisObjectID, std::string& FolderPath, COLMAPFoundData WhatToLoad = {true, true, true, true});
	COLMAPProject* GetProjectByID(const FEUUID& ProjectID);
	COLMAPProject* GetProjectByAnalysisObjectID(const FEUUID& AnalysisObjectID);
	COLMAPProject* GetProjectByEntityID(const FEUUID& EntityID);
	bool DeleteProject(const FEUUID& ProjectID);
	std::vector<FEUUID> GetProjectsIDList() const;
	COLMAPFoundData FindCOLMAPDataInFolder(const std::string& FolderPath) const;

	void AddOnSelectedImageChangedCallback(std::function<void(COLMAPProject*, int)> Callback);
	void ClearOnSelectedImageChangedCallbacks();
};

#define COLMAP_DATA_MANAGER COLMAPDataManager::GetInstance()