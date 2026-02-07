#pragma once

#include "VRManager/VRManager.h"
using namespace FocalEngine;

#define COLMAP_CAMERAS_FILE "cameras.txt"
#define COLMAP_IMAGES_FILE "images.txt"
#define COLMAP_TIE_POINTS_FILE "points3D.txt"

struct COLMAPPhysicalCamera
{
	friend class COLMAPProject;
	friend class COLMAPDataManager;
private:
	COLMAPPhysicalCamera();
	~COLMAPPhysicalCamera();

	std::string ID;
	std::string Model;
	int Width, Height;
	std::vector<double> Parameters;

	std::string SceneEntityID = "";
public:
	std::string GetID() const;
	std::string GetModel() const;
	int GetWidth() const;
	int GetHeight() const;
	double GetParameter(int Index) const;

	FEEntity* GetSceneEntity() const;
	bool EquvialentTo(COLMAPPhysicalCamera* OtherCamera) const;
};

struct COLMAPCamera
{
	friend class COLMAPProject;
	friend class COLMAPDataManager;
private:
	COLMAPCamera();
	~COLMAPCamera();

	int ID;
	COLMAPPhysicalCamera* PhysicalCamera;
public:
	int GetID() const;

	COLMAPPhysicalCamera* GetPhysicalCamera();
};

struct COLMAPPoint3D
{
	int ID;
	glm::dvec3 Position;
	glm::ivec3 RGB;
	double Error;

	// (IMAGE_ID, POINT2D_IDX)
	std::vector<std::pair<int, int>> Track;
};

struct COLMAPImage
{
	friend class COLMAPProject;
	friend class COLMAPDataManager;
private:
	COLMAPImage();
	~COLMAPImage();

	int ID;
	glm::dquat OriginalRotation;
	glm::quat Rotation;
	glm::dvec3 OriginalTranslation;
	glm::vec3 Position;
	int CameraID;
	std::string Name;

	std::string SceneEntityID = "";
public:
	int GetID() const;
	int GetCameraID() const;
	std::string GetName() const;
	glm::dquat GetOriginalRotation() const;
	glm::quat GetRotation() const;
	glm::dvec3 GetOriginalTranslation() const;
	glm::vec3 GetPosition() const;

	FEEntity* GetSceneEntity() const;
};

class COLMAPProject
{
	friend class COLMAPDataManager;
	COLMAPProject();
	~COLMAPProject();

	std::string ID;
	std::string ParentAnalysisObjectID;

	std::unordered_map<std::string, COLMAPPhysicalCamera*> PhysicalCameras;
	std::unordered_map<int, COLMAPCamera*> Cameras;
	std::unordered_map<int, COLMAPImage*> Images;
	std::unordered_map<int, COLMAPPoint3D> TiePoints;

	std::string PhotogrammetryAnchorID = "";
	std::string TiePointsEntityID = "";

	bool LoadCameras(const std::string& FilePath);
	bool LoadImages(const std::string& FilePath);
	bool LoadTiePoints(const std::string& FilePath);

	bool LoadFromFolder(const std::string& FolderPath);

	bool CreateCameraSceneRepresentation(std::string CameraID);
	bool DeleteCameraSceneRepresentation(std::string CameraID);

	bool CreateImageSceneRepresentation(int ImageID);
	bool DeleteImageSceneRepresentation(int ImageID);
	bool IsCameraVisualizationAttachedToImage(int ImageID);
	bool AttachCameraVisualizationToImage(int ImageID);
	bool DetachCameraVisualizationFromImage(int ImageID);
	
	bool CreateTiePointsSceneRepresentation();
public:
	std::string GetID() const;

	FEEntity* GetPhotogrammetryAnchorEntity();

	size_t GetPhysicalCameraCount() const;
	COLMAPPhysicalCamera* GetPhysicalCamera(std::string ID);
	std::vector<std::string> GetPhysicalCamerasIDList() const;
	COLMAPPhysicalCamera* FindPhysicalCameraEquvialentTo(COLMAPPhysicalCamera* OtherCamera) const;
	std::vector<int> GetCamerasWithPhysicalCameraID(const std::string& PhysicalCameraID) const;

	size_t GetCameraCount() const;
	COLMAPCamera* GetCamera(int ID);
	std::vector<int> GetCamerasIDList() const;

	size_t GetImageCount() const;
	COLMAPImage* GetImage(int ID);
	COLMAPImage* GetImageByFilename(const std::string& Filename, bool bPartialMatch = false);
	COLMAPCamera* GetCameraForImage(int ImageID);
	std::vector<int> GetImagesIDList() const;
	std::vector<COLMAPImage*> GetImagesContainingAABB(FEAABB AABBToTest);

	COLMAPPoint3D* GetTiePoint(int ID);
	std::vector<FEPointCloudVertex> GetTiePointsAsFEPoints();
	std::unordered_map<int, COLMAPPoint3D>* GetTiePointsMap();
	FEEntity* GetTiePointsEntity();
};

class COLMAPDataManager
{
	SINGLETON_PRIVATE_PART(COLMAPDataManager)

	std::unordered_map<std::string, COLMAPProject*> Projects;

	bool CreateVisualsForNewProject(COLMAPProject* NewProject);
public:	SINGLETON_PUBLIC_PART(COLMAPDataManager)

	COLMAPProject* CreateNewProject(std::string& ParentAnalysisObjectID, std::string& FolderPath);
	COLMAPProject* GetProjectByID(const std::string& ProjectID);
	COLMAPProject* GetProjectByAnalysisObjectID(const std::string& AnalysisObjectID);
	bool DeleteProject(const std::string& ProjectID);
	std::vector<std::string> GetProjectsIDList() const;
};

#define COLMAP_DATA_MANAGER COLMAPDataManager::GetInstance()