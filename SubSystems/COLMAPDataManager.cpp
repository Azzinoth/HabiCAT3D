#include "COLMAPDataManager.h"
using namespace FocalEngine;

COLMAPPhysicalCamera::COLMAPPhysicalCamera()
{
	ID = APPLICATION.GetUniqueHexID();
	Width = 0;
	Height = 0;
}

COLMAPPhysicalCamera::~COLMAPPhysicalCamera() {}

std::string COLMAPPhysicalCamera::GetID() const
{
	return ID;
}

std::string COLMAPPhysicalCamera::GetModel() const
{
	return Model;
}

int COLMAPPhysicalCamera::GetWidth() const
{
	return Width;
}

int COLMAPPhysicalCamera::GetHeight() const
{
	return Height;
}

double COLMAPPhysicalCamera::GetParameter(int Index) const
{
	if (Index < 0 || Index >= Parameters.size())
		return -DBL_MAX;

	return Parameters[Index];
}

FEEntity* COLMAPPhysicalCamera::GetSceneEntity() const
{
	return MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(SceneEntityID);
}

bool COLMAPPhysicalCamera::EquvialentTo(COLMAPPhysicalCamera* OtherCamera) const
{
	if (OtherCamera == nullptr)
		return false;

	if (Model != OtherCamera->Model ||
		Width != OtherCamera->Width ||
		Height != OtherCamera->Height ||
		Parameters.size() != OtherCamera->Parameters.size())
		return false;

	for (size_t i = 0; i < Parameters.size(); i++)
	{
		if (Parameters[i] != OtherCamera->Parameters[i])
			return false;
	}

	return true;
}

COLMAPCamera::COLMAPCamera() {}
COLMAPCamera::~COLMAPCamera() {}

int COLMAPCamera::GetID() const
{
	return ID;
}

COLMAPPhysicalCamera* COLMAPCamera::GetPhysicalCamera()
{
	return PhysicalCamera;
}

COLMAPImage::COLMAPImage() {}
COLMAPImage::~COLMAPImage() {}

int COLMAPImage::GetID() const
{
	return ID;
}

int COLMAPImage::GetCameraID() const
{
	return CameraID;
}

std::string COLMAPImage::GetName() const
{
	return Name;
}

glm::dquat COLMAPImage::GetOriginalRotation() const
{
	return OriginalRotation;
}

glm::dvec3 COLMAPImage::GetOriginalTranslation() const
{
	return OriginalTranslation;
}

glm::quat COLMAPImage::GetRotation() const
{
	return Rotation;
}

glm::vec3 COLMAPImage::GetPosition() const
{
	return Position;
}

FEEntity* COLMAPImage::GetSceneEntity() const
{
	return MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(SceneEntityID);
}

COLMAPProject::COLMAPProject()
{
	ID = APPLICATION.GetUniqueHexID();
}

COLMAPProject::~COLMAPProject()
{
	for (auto& CurrentCamera : Cameras)
		delete CurrentCamera.second;

	for (auto& CurrentImage : Images)
		delete CurrentImage.second;

	Cameras.clear();
	Images.clear();
	TiePoints.clear();

	if (!PhotogrammetryAnchorID.empty())
	{
		MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(PhotogrammetryAnchorID);
		PhotogrammetryAnchorID = "";
	}

	if (!TiePointsEntityID.empty())
	{
		MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(TiePointsEntityID);
		TiePointsEntityID = "";
	}
}

std::string COLMAPProject::GetID() const
{
	return ID;
}

COLMAPPhysicalCamera* COLMAPProject::FindPhysicalCameraEquvialentTo(COLMAPPhysicalCamera* OtherCamera) const
{
	for (const auto& CurrentCamera : PhysicalCameras)
	{
		if (CurrentCamera.second->EquvialentTo(OtherCamera))
			return CurrentCamera.second;
	}

	return nullptr;
}

bool COLMAPProject::LoadCameras(const std::string& FilePath)
{
	if (!FILE_SYSTEM.DoesFileExist(FilePath))
		return false;

	PhysicalCameras.clear();
	Cameras.clear();
	std::ifstream File(FilePath);
	std::string CurrentLine;

	while (std::getline(File, CurrentLine))
	{
		if (CurrentLine.empty() || CurrentLine[0] == '#')
			continue;

		std::istringstream LineStream(CurrentLine);
		COLMAPPhysicalCamera* NewPhysicalCamera = new COLMAPPhysicalCamera();
		COLMAPCamera* NewCamera = new COLMAPCamera();
		LineStream >> NewCamera->ID >> NewPhysicalCamera->Model >> NewPhysicalCamera->Width >> NewPhysicalCamera->Height;

		double CurrentParameter;
		while (LineStream >> CurrentParameter)
			NewPhysicalCamera->Parameters.push_back(CurrentParameter);

		COLMAPPhysicalCamera* ExistingCamera = FindPhysicalCameraEquvialentTo(NewPhysicalCamera);
		if (ExistingCamera != nullptr)
		{
			delete NewPhysicalCamera;
			NewCamera->PhysicalCamera = ExistingCamera;
		}
		else
		{
			NewCamera->PhysicalCamera = NewPhysicalCamera;
			PhysicalCameras[NewPhysicalCamera->ID] = NewPhysicalCamera;
		}
		
		Cameras[NewCamera->ID] = NewCamera;
	}

	return true;
}

size_t COLMAPProject::GetPhysicalCameraCount() const
{
	return PhysicalCameras.size();
}

COLMAPPhysicalCamera* COLMAPProject::GetPhysicalCamera(std::string ID)
{
	if (PhysicalCameras.find(ID) == PhysicalCameras.end())
		return nullptr;

	return PhysicalCameras[ID];
}

std::vector<std::string> COLMAPProject::GetPhysicalCamerasIDList() const
{
	std::vector<std::string> Result;
	for (const auto& CurrentCamera : PhysicalCameras)
		Result.push_back(CurrentCamera.first);

	return Result;
}

size_t COLMAPProject::GetCameraCount() const
{
	return Cameras.size();
}

bool COLMAPProject::CreateCameraSceneRepresentation(std::string CameraID)
{
	COLMAPPhysicalCamera* PhysicalCamera = GetPhysicalCamera(CameraID);
	if (PhysicalCamera == nullptr)
		return false;

	if (PhysicalCamera->GetSceneEntity() != nullptr)
		return false;

	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("COLMAPPhysicalCamera_" + PhysicalCamera->GetID());
	CameraEntity->AddComponent<FECameraComponent>();
	CAMERA_SYSTEM.SetCameraRenderingPipeline(CameraEntity, FERenderingPipeline::Forward_Simplified);
	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

	// Convert focal length to vertical FOV (in degrees)
	// FOV_y = 2 * atan(height / (2 * f))
	float FOVYRadians = 2.0f * std::atan(static_cast<float>(PhysicalCamera->GetHeight()) / (2.0f * static_cast<float>(PhysicalCamera->GetParameter(0))));
	float FOVYDegrees = glm::degrees(FOVYRadians);
	float AspectRatio = static_cast<float>(PhysicalCamera->GetWidth()) / static_cast<float>(PhysicalCamera->GetHeight());

	// Set up your camera component
	CameraComponent.SetFOV(FOVYDegrees);
	CameraComponent.SetAspectRatio(AspectRatio);
	CameraComponent.TryToSetViewportSize(PhysicalCamera->GetWidth(), PhysicalCamera->GetHeight());
	// Magic number, fix later
	CameraComponent.SetFarPlane(50.0f);
	CAMERA_SYSTEM.IndividualUpdate(CameraEntity, 1.0);
	CameraEntity->GetComponent<FECameraComponent>().SetActive(false);

	CameraEntity->AddComponent<FELineComponent>();
	FELineComponent& CameraLineComponent = CameraEntity->GetComponent<FELineComponent>();

	std::vector<FELine> LinesToRender;
	LinesToRender = RENDERER.GetFrustumLines(CameraEntity, glm::vec3(0.0f, 1.0f, 0.0f), 0.5f);
	FELineCollection* LineCollection = RESOURCE_MANAGER.RawDataToFELineCollection(LinesToRender);

	CameraLineComponent.SetLineCollection(LineCollection);
	PhysicalCamera->SceneEntityID = CameraEntity->GetObjectID();
	CameraEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);
	
	return true;
}

bool COLMAPProject::DeleteCameraSceneRepresentation(std::string CameraID)
{
	COLMAPPhysicalCamera* PhysicalCamera = GetPhysicalCamera(CameraID);
	if (PhysicalCamera == nullptr)
		return false;

	FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
	if (CameraEntity == nullptr)
		return false;

	MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(CameraEntity->GetObjectID());
	PhysicalCamera->SceneEntityID = "";
	return true;
}

std::vector<int> COLMAPProject::GetCamerasWithPhysicalCameraID(const std::string& PhysicalCameraID) const
{
	std::vector<int> Result;
	for (const auto& CurrentCamera : Cameras)
	{
		if (CurrentCamera.second->PhysicalCamera->GetID() == PhysicalCameraID)
			Result.push_back(CurrentCamera.first);
	}

	return Result;
}

bool COLMAPProject::LoadImages(const std::string& FilePath)
{
	if (!FILE_SYSTEM.DoesFileExist(FilePath))
		return false;

	Images.clear();
	std::ifstream File(FilePath);
	std::string CurrentLine;

	while (std::getline(File, CurrentLine))
	{
		if (CurrentLine.empty() || CurrentLine[0] == '#')
			continue;

		std::istringstream LineStream(CurrentLine);
		COLMAPImage* NewImage = new COLMAPImage();

		LineStream >> NewImage->ID >> NewImage->OriginalRotation.w >> NewImage->OriginalRotation.x >> NewImage->OriginalRotation.y >> NewImage->OriginalRotation.z;
		// COLMAP stores world to camera rotation, inverting to get camera to world.
		NewImage->Rotation = glm::inverse(NewImage->OriginalRotation);
		// COLMAP convention: X-right, Y-down, Z-forward (into the scene)
		// OpenGL convention: X-right, Y-up, Z-backward (out of the screen)
		// Rotate 180° around local X-axis to flip Y and Z
		glm::quat FlipYZ = glm::quat(0.0f, 1.0f, 0.0f, 0.0f);
		NewImage->Rotation = NewImage->Rotation * FlipYZ;

		LineStream >> NewImage->OriginalTranslation.x >> NewImage->OriginalTranslation.y >> NewImage->OriginalTranslation.z;
		// COLMAP stores translation as world to camera, converting to camera center in world coords: -R^T * T
		glm::dmat3 OriginalRotationMatrix = glm::mat3_cast(NewImage->OriginalRotation);
		NewImage->Position = -glm::transpose(OriginalRotationMatrix) * NewImage->OriginalTranslation;

		LineStream >> NewImage->CameraID >> NewImage->Name;

		Images[NewImage->ID] = NewImage;

		// Skip the POINTS2D
		std::getline(File, CurrentLine);
	}

	return true;
}

bool COLMAPProject::LoadTiePoints(const std::string& FilePath)
{
	if (!FILE_SYSTEM.DoesFileExist(FilePath))
		return false;

	TiePoints.clear();
	std::ifstream File(FilePath);
	std::string CurrentLine;

	while (std::getline(File, CurrentLine))
	{
		if (CurrentLine.empty() || CurrentLine[0] == '#') continue;

		std::istringstream LineStream(CurrentLine);
		COLMAPPoint3D CurrentPoint;
		int R, G, B;

		LineStream >> CurrentPoint.ID
				   >> CurrentPoint.Position.x >> CurrentPoint.Position.y >> CurrentPoint.Position.z
				   >> R >> G >> B
				   >> CurrentPoint.Error;

		CurrentPoint.RGB = glm::ivec3(R, G, B);

		int ImageID, Point2DIndex;
		while (LineStream >> ImageID >> Point2DIndex)
			CurrentPoint.Track.emplace_back(ImageID, Point2DIndex);

		TiePoints[CurrentPoint.ID] = CurrentPoint;
	}

	return true;
}

COLMAPCamera* COLMAPProject::GetCamera(int ID)
{
	if (Cameras.find(ID) == Cameras.end())
		return nullptr;

	return Cameras[ID];
}

std::vector<int> COLMAPProject::GetCamerasIDList() const
{
	std::vector<int> Result;
	for (const auto& CurrentCamera : Cameras)
		Result.push_back(CurrentCamera.first);

	return Result;
}

size_t COLMAPProject::GetImageCount() const
{
	return Images.size();
}

COLMAPImage* COLMAPProject::GetImage(int ID)
{
	auto ImageMapIterator = Images.begin();
	while (ImageMapIterator != Images.end())
	{
		if (ImageMapIterator->second->ID == ID)
			return ImageMapIterator->second;

		ImageMapIterator++;
	}

	return nullptr;
}

COLMAPImage* COLMAPProject::GetImageByFilename(const std::string& Filename, bool bPartialMatch)
{
	auto ImageMapIterator = Images.begin();
	while (ImageMapIterator != Images.end())
	{
		if (bPartialMatch)
		{
			if (ImageMapIterator->second->Name.find(Filename) != std::string::npos)
				return ImageMapIterator->second;
		}
		else
		{
			if (ImageMapIterator->second->Name == Filename)
				return ImageMapIterator->second;
		}

		ImageMapIterator++;
	}

	return nullptr;
}

COLMAPCamera* COLMAPProject::GetCameraForImage(int ImageID)
{
	COLMAPImage* Image = GetImage(ImageID);
	if (Image == nullptr)
		return nullptr;

	return GetCamera(Image->CameraID);
}

std::vector<int> COLMAPProject::GetImagesIDList() const
{
	std::vector<int> Result;
	for (const auto& CurrentImage : Images)
		Result.push_back(CurrentImage.first);

	return Result;
}

bool COLMAPProject::CreateImageSceneRepresentation(int ImageID)
{
	COLMAPImage* CurrentImage = GetImage(ImageID);
	if (CurrentImage == nullptr)
		return false;

	if (CurrentImage->GetSceneEntity() != nullptr)
		return false;

	if (Cameras.find(CurrentImage->CameraID) == Cameras.end())
		return false;

	COLMAPCamera* CurrentCamera = Cameras[CurrentImage->CameraID];
	COLMAPPhysicalCamera* PhysicalCamera = CurrentCamera->GetPhysicalCamera();
	if (PhysicalCamera == nullptr)
		return false;

	if (GetPhotogrammetryAnchorEntity() == nullptr)
	{
		FEEntity* NewAnchor = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("COLMAPImagesAnchor");
		PhotogrammetryAnchorID = NewAnchor->GetObjectID();
		FEEntity* MainEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
		NewAnchor->AttachTo(MainEntity, false);
	}
	FEEntity* Anchor = GetPhotogrammetryAnchorEntity();

	FEEntity* MainCameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	FEEntity* ImageVisualizationEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("COLMAPImage_" + std::to_string(CurrentImage->ID));
	FETransformComponent& ImageTransform = ImageVisualizationEntity->GetComponent<FETransformComponent>();
	ImageTransform.SetPosition(CurrentImage->GetPosition());
	ImageTransform.SetQuaternion(CurrentImage->GetRotation());

	ImageVisualizationEntity->AddComponent<FELineComponent>();
	FELineComponent& ImageLineComponent = ImageVisualizationEntity->GetComponent<FELineComponent>();

	// Create rectangle based on camera width and height
	// Scale factor to make the visualization reasonable size in world space
	float Scale = 0.00003f;
	float HalfWidth = (PhysicalCamera->GetWidth() * Scale) / 2.0f;
	float HalfHeight = (PhysicalCamera->GetHeight() * Scale) / 2.0f;

	// Define the 4 corners of the rectangle (in local camera space, on XY plane)
	glm::vec3 TopLeft = glm::vec3(-HalfWidth, HalfHeight, 0.0f);
	glm::vec3 TopRight = glm::vec3(HalfWidth, HalfHeight, 0.0f);
	glm::vec3 BottomRight = glm::vec3(HalfWidth, -HalfHeight, 0.0f);
	glm::vec3 BottomLeft = glm::vec3(-HalfWidth, -HalfHeight, 0.0f);

	// Create 4 lines forming the rectangle
	std::vector<FELine> LinesToRender;
	LinesToRender.reserve(4);

	FELine TopLine, RightLine, BottomLine, LeftLine;

	// Top edge
	TopLine.StartPoint = TopLeft;
	TopLine.EndPoint = TopRight;
	TopLine.Color = glm::vec3(1.0f, 0.0f, 0.0f);
	TopLine.Width = 0.5f;
	LinesToRender.push_back(TopLine);

	// Right edge
	RightLine.StartPoint = TopRight;
	RightLine.EndPoint = BottomRight;
	RightLine.Color = glm::vec3(1.0f, 0.0f, 0.0f);
	RightLine.Width = 0.5f;
	LinesToRender.push_back(RightLine);

	// Bottom edge
	BottomLine.StartPoint = BottomRight;
	BottomLine.EndPoint = BottomLeft;
	BottomLine.Color = glm::vec3(1.0f, 0.0f, 0.0f);
	BottomLine.Width = 0.5f;
	LinesToRender.push_back(BottomLine);

	// Left edge
	LeftLine.StartPoint = BottomLeft;
	LeftLine.EndPoint = TopLeft;
	LeftLine.Color = glm::vec3(1.0f, 0.0f, 0.0f);
	LeftLine.Width = 0.5f;
	LinesToRender.push_back(LeftLine);

	FELineCollection* LineCollection = RESOURCE_MANAGER.RawDataToFELineCollection(LinesToRender);
	ImageLineComponent.SetLineCollection(LineCollection);

	CurrentImage->SceneEntityID = ImageVisualizationEntity->GetObjectID();
	ImageVisualizationEntity->AttachTo(Anchor, false);

	return true;
}

bool COLMAPProject::DeleteImageSceneRepresentation(int ImageID)
{
	COLMAPImage* CurrentImage = GetImage(ImageID);
	if (CurrentImage == nullptr)
		return false;

	FEEntity* ImageEntity = CurrentImage->GetSceneEntity();
	if (ImageEntity != nullptr)
		return false;

	MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(ImageEntity);
	return true;
}

FEEntity* COLMAPProject::GetPhotogrammetryAnchorEntity()
{
	return MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(PhotogrammetryAnchorID);
}

bool COLMAPProject::IsCameraVisualizationAttachedToImage(int ImageID)
{
	COLMAPImage* CurrentImage = GetImage(ImageID);
	if (CurrentImage == nullptr)
		return false;

	COLMAPCamera* CurrentCamera = GetCameraForImage(ImageID);
	if (CurrentCamera == nullptr)
		return false;

	COLMAPPhysicalCamera* PhysicalCamera = CurrentCamera->GetPhysicalCamera();
	if (PhysicalCamera == nullptr)
		return false;

	FEEntity* ImageEntity = CurrentImage->GetSceneEntity();
	FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
	if (ImageEntity == nullptr || CameraEntity == nullptr)
		return false;

	FEEntity* CameraParent = CameraEntity->GetParentEntity();
	if (CameraParent == nullptr)
		return false;

	return (CameraParent->GetObjectID() == ImageEntity->GetObjectID());
}

bool COLMAPProject::AttachCameraVisualizationToImage(int ImageID)
{
	COLMAPImage* CurrentImage = GetImage(ImageID);
	if (CurrentImage == nullptr)
		return false;

	COLMAPCamera* CurrentCamera = GetCameraForImage(ImageID);
	if (CurrentCamera == nullptr)
		return false;

	COLMAPPhysicalCamera* PhysicalCamera = CurrentCamera->GetPhysicalCamera();
	if (PhysicalCamera == nullptr)
		return false;

	FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
	if (CameraEntity == nullptr)
	{
		if (!CreateCameraSceneRepresentation(PhysicalCamera->GetID()))
			return false;

		CameraEntity = PhysicalCamera->GetSceneEntity();
		if (CameraEntity == nullptr)
			return false;
	}

	FEEntity* ImageEntity = CurrentImage->GetSceneEntity();
	if (ImageEntity == nullptr || CameraEntity == nullptr)
		return false;

	CameraEntity->AttachTo(ImageEntity, false);
	CameraEntity->SetComponentVisible(ComponentVisibilityType::ALL, true);
	return true;
}

bool COLMAPProject::DetachCameraVisualizationFromImage(int ImageID)
{
	COLMAPImage* CurrentImage = GetImage(ImageID);
	if (CurrentImage == nullptr)
		return false;

	COLMAPCamera* CurrentCamera = GetCameraForImage(ImageID);
	if (CurrentCamera == nullptr)
		return false;

	COLMAPPhysicalCamera* PhysicalCamera = CurrentCamera->GetPhysicalCamera();
	if (PhysicalCamera == nullptr)
		return false;

	FEEntity* ImageEntity = CurrentImage->GetSceneEntity();
	FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
	if (ImageEntity == nullptr || CameraEntity == nullptr)
		return false;

	CameraEntity->Detach(false);
	CameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0f));
	return true;
}

std::vector<COLMAPImage*> COLMAPProject::GetImagesContainingAABB(FEAABB AABBToTest)
{
	std::vector<COLMAPImage*> Result;
	for (auto& CurrentImage : Images)
	{
		FEEntity* ImageEntity = CurrentImage.second->GetSceneEntity();
		if (ImageEntity == nullptr)
			continue;

		COLMAPPhysicalCamera* PhysicalCamera = GetCameraForImage(CurrentImage.second->ID)->GetPhysicalCamera();
		if (PhysicalCamera == nullptr)
			continue;

		FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
		if (CameraEntity == nullptr)
			CreateCameraSceneRepresentation(PhysicalCamera->GetID());

		CameraEntity = PhysicalCamera->GetSceneEntity();
		if (CameraEntity == nullptr)
			continue;

		FEEntity* OldParent = CameraEntity->GetParentEntity();
		CameraEntity->AttachTo(ImageEntity, false);
		// Updating starting from parent to ensure world transform is correct
		TRANSFORM_SYSTEM.ForceUpdateTransformComponent(ImageEntity);

		CAMERA_SYSTEM.IndividualUpdate(CameraEntity, 0.0);
		FECameraComponent& CurrentCameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		CurrentCameraComponent.UpdateFrustum();

		FEFrustum CameraFrustum = CurrentCameraComponent.GetFrustum();
		if (OldParent != nullptr)
		{
			CameraEntity->AttachTo(OldParent, false);
		}
		else
		{
			CameraEntity->Detach(false);
		}

		if (CameraFrustum.IntersectsAABB(AABBToTest) || CameraFrustum.ContainsAABB(AABBToTest))
		{
			Result.push_back(CurrentImage.second);
		}
	}

	return Result;
}

COLMAPPoint3D* COLMAPProject::GetTiePoint(int ID)
{
	if (TiePoints.find(ID) == TiePoints.end())
		return nullptr;

	return &TiePoints[ID];
}

std::unordered_map<int, COLMAPPoint3D>* COLMAPProject::GetTiePointsMap()
{
	return &TiePoints;
}

std::vector<FEPointCloudVertex> COLMAPProject::GetTiePointsAsFEPoints()
{
	std::vector<FEPointCloudVertex> Result;
	for (const auto& [ID, COLMAPPoint] : TiePoints)
	{
		FEPointCloudVertex NewPoint;
		NewPoint.X = static_cast<float>(COLMAPPoint.Position.x);
		NewPoint.Y = static_cast<float>(COLMAPPoint.Position.y);
		NewPoint.Z = static_cast<float>(COLMAPPoint.Position.z);
		NewPoint.R = static_cast<unsigned char>(COLMAPPoint.RGB.x);
		NewPoint.G = static_cast<unsigned char>(COLMAPPoint.RGB.y);
		NewPoint.B = static_cast<unsigned char>(COLMAPPoint.RGB.z);
		NewPoint.A = 255;
		Result.push_back(NewPoint);
	}

	return Result;
}

bool COLMAPProject::CreateTiePointsSceneRepresentation()
{
	if (TiePoints.empty())
		return false;

	std::vector<FEPointCloudVertex> FEPoints = GetTiePointsAsFEPoints();
	if (FEPoints.empty())
		return false;
	

	//glm::vec3 Min = glm::vec3(FLT_MAX);
	//glm::vec3 Max = glm::vec3(-FLT_MAX);

	//for (size_t i = 0; i < FEPoints.size(); i++)
	//{
	//	if (FEPoints[i].X < Min.x)
	//		Min.x = FEPoints[i].X;

	//	if (FEPoints[i].X > Max.x)
	//		Max.x = FEPoints[i].X;

	//	if (FEPoints[i].Y < Min.y)
	//		Min.y = FEPoints[i].Y;

	//	if (FEPoints[i].Y > Max.y)
	//		Max.y = FEPoints[i].Y;

	//	if (FEPoints[i].Z < Min.z)
	//		Min.z = FEPoints[i].Z;

	//	if (FEPoints[i].Z > Max.z)
	//		Max.z = FEPoints[i].Z;
	//}

	//NewPointCloud->AABB = FEAABB(Min, Max);
	//if (bCenterPositions)
	//{
	//	glm::vec3 Extent = Max - Min;
	//	glm::vec3 Center = Min + Extent / 2.0f;

	//	for (size_t i = 0; i < RawPointCloudData.size(); i++)
	//	{
	//		RawPointCloudData[i].X = RawPointCloudData[i].X - Center.x;
	//		RawPointCloudData[i].Y = RawPointCloudData[i].Y - Center.y;
	//		RawPointCloudData[i].Z = RawPointCloudData[i].Z - Center.z;
	//	}

	//	NewPointCloud->AABB = FEAABB(Min - Center, Max - Center);
	//}
	



	FEPointCloud* NewPointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(FEPoints, "", "", false, true);
	FEEntity* NewEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Point cloud entity");
	TiePointsEntityID = NewEntity->GetObjectID();
	NewEntity->AddComponent<FEPointCloudComponent>(NewPointCloud);

	FEEntity* Anchor = GetPhotogrammetryAnchorEntity();
	NewEntity->AttachTo(Anchor, false);

	return true;
}

FEEntity* COLMAPProject::GetTiePointsEntity()
{
	return MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(TiePointsEntityID);
}

bool COLMAPProject::LoadFromFolder(const std::string& FolderPath)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(FolderPath))
		return false;

	if (!FILE_SYSTEM.DoesFileExist(FolderPath + COLMAP_CAMERAS_FILE) ||
		!FILE_SYSTEM.DoesFileExist(FolderPath + COLMAP_IMAGES_FILE) ||
		!FILE_SYSTEM.DoesFileExist(FolderPath + COLMAP_TIE_POINTS_FILE))
		return false;
	
	FEEntity* Anchor = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("PhotogrammetryAnchor_" + this->GetID());
	PhotogrammetryAnchorID = Anchor->GetObjectID();
	FEEntity* MainEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	Anchor->AttachTo(MainEntity, false);

	LoadCameras(FolderPath + COLMAP_CAMERAS_FILE);
	LoadImages(FolderPath + COLMAP_IMAGES_FILE);
	LoadTiePoints(FolderPath + COLMAP_TIE_POINTS_FILE);

	return true;
}

COLMAPDataManager::COLMAPDataManager() {}
COLMAPDataManager::~COLMAPDataManager() {}

bool COLMAPDataManager::CreateVisualsForNewProject(COLMAPProject* NewProject)
{
	if (NewProject == nullptr)
		return false;

	for (const auto& CurrentCameraID : NewProject->GetPhysicalCamerasIDList())
		if (!NewProject->CreateCameraSceneRepresentation(CurrentCameraID))
			return false;
	
	for (const auto& CurrentImageID : NewProject->GetImagesIDList())
		if (!NewProject->CreateImageSceneRepresentation(CurrentImageID))
			return false;
	
	if (!NewProject->CreateTiePointsSceneRepresentation())
		return false;

	return true;
}

COLMAPProject* COLMAPDataManager::CreateNewProject(std::string& ParentAnalysisObjectID, std::string& FolderPath)
{
	COLMAPProject* Result = nullptr;

	AnalysisObject* ParentAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObject(ParentAnalysisObjectID);
	if (ParentAnalysisObject == nullptr)
		return Result;

	if (!FILE_SYSTEM.DoesDirectoryExist(FolderPath))
		return Result;

	Result = new COLMAPProject();
	Result->ParentAnalysisObjectID = ParentAnalysisObjectID;
	if (!Result->LoadFromFolder(FolderPath))
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