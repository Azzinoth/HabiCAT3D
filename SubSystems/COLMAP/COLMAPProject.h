#pragma once

#include "COLMAPImage.h"
using namespace FocalEngine;

struct COLMAPPoint3D
{
	int ID;
	glm::dvec3 Position;
	glm::ivec3 RGB;
	double Error;

	// (IMAGE_ID, POINT2D_IDX)
	std::vector<std::pair<int, int>> Track;
};

struct COLMAPViewRenderSettings
{
	friend class COLMAPProject;
private:
	COLMAPViewRenderSettings() = default;

	bool bRenderOnlyCurrentAnalysisObject = true;
	bool bRenderTiePoints = false;
	bool bRenderOtherCameras = false;

	bool bAutoOpenResult = true;
public:
	bool GetRenderOnlyCurrentAnalysisObject() const;
	void SetRenderOnlyCurrentAnalysisObject(bool Value);

	bool GetRenderTiePoints() const;
	void SetRenderTiePoints(bool Value);

	bool GetRenderOtherCameras() const;
	void SetRenderOtherCameras(bool Value);

	bool GetAutoOpenResult() const;
	void SetAutoOpenResult(bool Value);
};

class COLMAPProject
{
	friend class COLMAPDataManager;
	COLMAPProject();
	~COLMAPProject();

	std::string ID;
	std::string ParentAnalysisObjectID;
	std::string FolderPath;

	std::unordered_map<std::string, COLMAPPhysicalCamera*> PhysicalCameras;
	std::unordered_map<int, COLMAPCamera*> Cameras;
	std::unordered_map<int, COLMAPImage*> Images;
	const glm::vec4 DefaultImageColor = glm::vec4(58.0f / 255.0f, 110.0f / 255.0f, 165.0f / 255.0f, 1.0f);
	const glm::vec4 HighlightedImageColor = glm::vec4(163.0f / 255.0f, 73.0f / 255.0f, 164.0f / 255.0f, 1.0f);
	const glm::vec4 SelectedImageColor = glm::vec4(0.1f, 1.0f, 0.1f, 1.0f);
	int SelectedImageID = -1;
	void SetColorForImageInternal(int ImageID, glm::vec4 Color);
	std::vector<std::function<void(int)>> OnSelectedImageChangedCallbacks;
	bool SetSelectedImageByIDInternal(int ImageID);
	std::vector<int> HighlightedImageIDs;
	bool IsImageHighlighted(int ImageID) const;
	void ResetHighlightedImagesInternal();
	void SetHighlightedImagesByIDInternal(std::vector<int> ImageID);

	std::unordered_map<int, COLMAPPoint3D> TiePoints;

	std::string PhotogrammetryAnchorID = "";
	std::string TiePointsEntityID = "";

	GLuint ImagesColorsSSBO = GLuint(-1);
	static void BeforeRenderCallback(FEEntity* Entity);

	std::string ImagesInstancedEntityID = "";
	std::unordered_map<int, int> ImageInstanceIndexToImageID;

	bool LoadCameras(const std::string& FilePath);
	bool LoadImages(const std::string& FilePath);
	bool LoadTiePoints(const std::string& FilePath);

	bool LoadFromFolder(const std::string& FolderPath, bool bLoadTiePoints);

	bool CreateCameraSceneRepresentation(std::string CameraID);
	bool DeleteCameraSceneRepresentation(std::string CameraID);

	bool CreateImagesInstancedSceneRepresentation();
	int CameraAttachedToImageID = -1;
	bool IsCameraVisualizationAttachedToImage(int ImageID);
	bool AttachCameraVisualizationToImage(int ImageID);
	bool DetachCameraVisualizationFromImage(int ImageID);
	void MouseButtonCallback(int Button, int Action, int Mods);
	
	bool CreateTiePointsSceneRepresentation();

	COLMAPViewRenderSettings* CurrentViewRenderSettings = nullptr;
public:
	std::string GetID() const;
	std::string GetParentAnalysisObjectID() const;
	std::string GetFolderPath() const;

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
	std::string GetPathToPhotoByImageID(int ImageID);
	COLMAPCamera* GetCameraForImage(int ImageID);
	std::vector<int> GetImagesIDList() const;
	std::vector<COLMAPImage*> GetImagesContainingAABB(FEAABB AABBToTest);
	COLMAPImage* ImageUnderMouse(float* HitDistance = nullptr);
	bool SelectImageByID(int ImageID);
	void ResetSelectedImage();
	COLMAPImage* GetSelectedImage();
	void AddOnSelectedImageChangedCallback(std::function<void(int)> Callback);
	void ClearOnSelectedImageChangedCallbacks();
	COLMAPViewRenderSettings* GetCurrentViewRenderSettings();
	bool RenderViewFromImage(int ImageID, bool bDepthMap = false, FE_DEPTH_EXPORT_MODE DepthExportMode = FE_DEPTH_EXPORT_GRAYSCALE_PNG);
	void HighlightImagesThatSeeAABB(FEAABB AABBToTest, bool bSelectClosest = true);
	bool IsPhotoFolderAvailable() const;
	FEEntity* GetImagesInstancedEntity();

	bool IsTiePointsLoaded() const;
	COLMAPPoint3D* GetTiePoint(int ID);
	std::vector<FEPointCloudVertex> GetTiePointsAsFEPoints();
	std::unordered_map<int, COLMAPPoint3D>* GetTiePointsMap();
	FEEntity* GetTiePointsEntity();
};