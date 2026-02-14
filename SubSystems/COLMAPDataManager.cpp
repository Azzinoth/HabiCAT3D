#include "COLMAPDataManager.h"
using namespace FocalEngine;
#include <shellapi.h>

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

bool COLMAPViewRenderSettings::GetRenderOnlyCurrentAnalysisObject() const
{
	return bRenderOnlyCurrentAnalysisObject;
}

void COLMAPViewRenderSettings::SetRenderOnlyCurrentAnalysisObject(bool Value)
{
	bRenderOnlyCurrentAnalysisObject = Value;
}

bool COLMAPViewRenderSettings::GetRenderTiePoints() const
{
	return bRenderTiePoints;
}

void COLMAPViewRenderSettings::SetRenderTiePoints(bool Value)
{
	bRenderTiePoints = Value;
}

bool COLMAPViewRenderSettings::GetRenderOtherCameras() const
{
	return bRenderOtherCameras;
}

void COLMAPViewRenderSettings::SetRenderOtherCameras(bool Value)
{
	bRenderOtherCameras = Value;
}

bool COLMAPViewRenderSettings::GetAutoOpenResult() const
{
	return bAutoOpenResult;
}

void COLMAPViewRenderSettings::SetAutoOpenResult(bool Value)
{
	bAutoOpenResult = Value;
}

COLMAPProject::COLMAPProject()
{
	ID = APPLICATION.GetUniqueHexID();
	CurrentViewRenderSettings = new COLMAPViewRenderSettings();
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

std::string COLMAPProject::GetParentAnalysisObjectID() const
{
	return ParentAnalysisObjectID;
}

std::string COLMAPProject::GetFolderPath() const
{
	return FolderPath;
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

std::string COLMAPProject::GetPathToPhotoByImageID(int ImageID)
{
	std::string Result = "";
	COLMAPImage* Image = GetImage(ImageID);
	if (Image == nullptr)
		return Result;

	Result = FolderPath + "/images/" + Image->Name;
	if (!FILE_SYSTEM.DoesFileExist(Result))
		Result = "";

	return Result;
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

COLMAPViewRenderSettings* COLMAPProject::GetCurrentViewRenderSettings()
{
	return CurrentViewRenderSettings;
}

bool COLMAPProject::RenderViewFromImage(int ImageID)
{
	COLMAPImage* Image = GetImage(ImageID);
	if (Image == nullptr)
		return false;

	COLMAPCamera* ImageCamera = GetCameraForImage(Image->GetID());
	if (ImageCamera == nullptr)
		return false;

	COLMAPPhysicalCamera* PhysicalCamera = ImageCamera->GetPhysicalCamera();
	if (PhysicalCamera == nullptr)
		return false;

	FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
	if (CameraEntity == nullptr)
		return false;

	FEScene* MainScene = MAIN_SCENE_MANAGER.GetMainScene();

	// We should hide current physical camera, so that it wouldn't be visible in the rendered result(as green lines on coners).
	CameraEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);

	std::unordered_map<std::string, bool> PreviousAnalysisObjectsVisibility;
	if (CurrentViewRenderSettings->bRenderOnlyCurrentAnalysisObject)
	{
		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObject(ParentAnalysisObjectID);
		
		std::vector<std::string> AllAnalysisObjectIDs = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectsIDList();
		for (size_t i = 0; i < AllAnalysisObjectIDs.size(); i++)
		{
			if (ActiveObject->GetID() == AllAnalysisObjectIDs[i])
				continue;

			AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObject(AllAnalysisObjectIDs[i]);
			FEEntity* CurrentEntity = CurrentObject->GetEntity();
			if (CurrentEntity != nullptr)
			{
				PreviousAnalysisObjectsVisibility[CurrentObject->GetID()] = CurrentObject->IsRenderedInScene();
				CurrentObject->SetRenderInScene(false);
				CurrentEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);

				COLMAPProject* OtherProject = COLMAP_DATA_MANAGER.GetProjectByAnalysisObjectID(CurrentObject->GetID());
				if (OtherProject != nullptr)
				{
					// FE_FIX_ME: Handle special case if some other AnalysisObject have photogrammetry anchor attached, we should hide all.
				}
			}
		}
	}

	FEEntity* TiePointsEntity = MainScene->GetEntity(TiePointsEntityID);
	bool bWasTiePointsEntityInitiallyVisible = false;
	if (TiePointsEntity != nullptr)
	{
		bWasTiePointsEntityInitiallyVisible = TiePointsEntity->IsComponentVisible(ComponentVisibilityType::ALL);
		TiePointsEntity->SetComponentVisible(ComponentVisibilityType::ALL, CurrentViewRenderSettings->bRenderTiePoints);
	}
	
	FEEntity* AllImagesInstancedEntity = MainScene->GetEntity(ImagesInstancedEntityID);
	bool bWasImagesInstancedEntityInitiallyVisible = false;
	if (AllImagesInstancedEntity != nullptr)
	{
		bWasImagesInstancedEntityInitiallyVisible = AllImagesInstancedEntity->IsComponentVisible(ComponentVisibilityType::ALL);
		AllImagesInstancedEntity->SetComponentVisible(ComponentVisibilityType::ALL, CurrentViewRenderSettings->bRenderOtherCameras);
	}
	
	FEEntity* CurrentMainCamera = CAMERA_SYSTEM.GetMainCamera(MainScene);
	CAMERA_SYSTEM.SetMainCamera(CameraEntity);
	CAMERA_SYSTEM.IndividualUpdate(CameraEntity, 0.0);
	RENDERER.Render(MainScene);

	FETexture* CameraResult = RENDERER.GetCameraResult(CameraEntity);
	RESOURCE_MANAGER.ExportFETextureToPNG(CameraResult, "CameraView.png");

	CAMERA_SYSTEM.SetMainCamera(CurrentMainCamera);

	CameraEntity->SetComponentVisible(ComponentVisibilityType::ALL, true);

	if (CurrentViewRenderSettings->bRenderOnlyCurrentAnalysisObject)
	{
		for (const auto& CurrentEntityVisibilityPair : PreviousAnalysisObjectsVisibility)
		{
			AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObject(CurrentEntityVisibilityPair.first);
			if (CurrentObject != nullptr)
				CurrentObject->SetRenderInScene(CurrentEntityVisibilityPair.second);

			FEEntity* CurrentEntity = CurrentObject->GetEntity();
			if (CurrentEntity != nullptr)
				CurrentEntity->SetComponentVisible(ComponentVisibilityType::ALL, CurrentEntityVisibilityPair.second);
		}
	}

	if (TiePointsEntity != nullptr)
	{
		if (TiePointsEntity->IsComponentVisible(ComponentVisibilityType::ALL) != bWasTiePointsEntityInitiallyVisible)
			TiePointsEntity->SetComponentVisible(ComponentVisibilityType::ALL, bWasTiePointsEntityInitiallyVisible);
	}

	if (AllImagesInstancedEntity != nullptr)
	{
		if (AllImagesInstancedEntity->IsComponentVisible(ComponentVisibilityType::ALL) != bWasImagesInstancedEntityInitiallyVisible)
			AllImagesInstancedEntity->SetComponentVisible(ComponentVisibilityType::ALL, bWasImagesInstancedEntityInitiallyVisible);
	}

	std::string FullPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/CameraView.png";
	if (FILE_SYSTEM.DoesFileExist(FullPath) && CurrentViewRenderSettings->bAutoOpenResult)
		ShellExecute(NULL, "open", FullPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

	return true;
}

void COLMAPProject::BeforeRenderCallback(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEInstancedComponent>())
		return;

	// This function is static so we need to get the current project instance by Entity.
	COLMAPProject* CurrentProject = COLMAP_DATA_MANAGER.GetProjectByEntityID(Entity->GetObjectID());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CurrentProject->ImagesColorsSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

bool COLMAPProject::CreateImagesInstancedSceneRepresentation()
{
	FEEntity* ImagesInstancedEntity = MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(ImagesInstancedEntityID);
	if (ImagesInstancedEntity != nullptr)
		MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(ImagesInstancedEntityID);

	ImagesInstancedEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("COLMAPImagesInstanced");
	ImagesInstancedEntityID = ImagesInstancedEntity->GetObjectID();

	ImagesInstancedEntity->AddComponent<FEGameModelComponent>(COLMAP_DATA_MANAGER.ImagesInstancedGameModel);
	ImagesInstancedEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	//CurrentImagesInstancedEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, true);
	ImagesInstancedEntity->AddComponent<FEInstancedComponent>();


	FEInstancedComponent& ImagesInstancedComponent = ImagesInstancedEntity->GetComponent<FEInstancedComponent>();
	ImagesInstancedComponent.SetCullingType(FE_CULLING_TYPE::FE_CULLING_NONE);

	INSTANCED_RENDERING_SYSTEM.AddBeforeRenderCallback(ImagesInstancedEntity, COLMAPProject::BeforeRenderCallback);

	int InstanceIndex = 0;
	for (const auto& [ImageID, CurrentImage] : Images)
	{
		if (Cameras.find(CurrentImage->CameraID) == Cameras.end())
		{
			MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(ImagesInstancedEntityID);
			return false;
		}

		COLMAPCamera* CurrentCamera = Cameras[CurrentImage->CameraID];
		COLMAPPhysicalCamera* PhysicalCamera = CurrentCamera->GetPhysicalCamera();
		if (PhysicalCamera == nullptr)
		{
			MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(ImagesInstancedEntityID);
			return false;
		}

		float ScaleX = PhysicalCamera->GetWidth() * 0.00003f;
		float ScaleY = PhysicalCamera->GetHeight() * 0.00003f;

		glm::vec3 Position = CurrentImage->GetPosition();

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position);
		ModelMatrix = ModelMatrix * glm::mat4_cast(CurrentImage->GetRotation());
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(ScaleX, ScaleY, 0.00003f));
		
		INSTANCED_RENDERING_SYSTEM.AddIndividualInstance(ImagesInstancedEntity, ModelMatrix);
		ImageInstanceIndexToImageID[InstanceIndex] = ImageID;
		InstanceIndex++;
	}

	FE_GL_ERROR(glGenBuffers(1, &ImagesColorsSSBO));
	FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ImagesColorsSSBO));
	std::vector<glm::vec4> InstanceColors(InstanceIndex, glm::vec4(58.0f / 255.0f, 110.0f / 255.0f, 165.0f / 255.0f, 1.0f));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceColors.size() * sizeof(glm::vec4), InstanceColors.data(), GL_DYNAMIC_DRAW));

	ImagesInstancedEntity->AttachTo(GetPhotogrammetryAnchorEntity(), false);
	return true;
}

COLMAPImage* COLMAPProject::ImageUnderMouse()
{
	COLMAPImage* Result = nullptr;

	FEEntity* ImagesInstancedEntity = MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(ImagesInstancedEntityID);
	if (ImagesInstancedEntity == nullptr)
		return Result;

	FEEntity* MainCameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	glm::vec3 SelectionRayDirection = glm::vec3(CAMERA_SYSTEM.GetMouseRayToWorld(MainCameraEntity));
	glm::vec3 SelectionRayOrigin = MainCameraEntity->GetComponent<FETransformComponent>().GetPosition();

	FEInstancedComponent& ImagesInstancedComponent = ImagesInstancedEntity->GetComponent<FEInstancedComponent>();
	if (ImagesInstancedComponent.GetInstanceCount() == 0)
		return Result;

	float Distance = 0.0f;
	std::vector<pair<int, int>> InstanceIndexAndHitDistance;
	for (size_t i = 0; i < ImagesInstancedComponent.IndividualInstancedAABB.size(); i++)
	{
		FEAABB IndividualAABB = ImagesInstancedComponent.IndividualInstancedAABB[i];
		IndividualAABB = IndividualAABB.Transform(ImagesInstancedEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
		if (IndividualAABB.RayIntersect(SelectionRayOrigin, SelectionRayDirection, Distance))
		{
			InstanceIndexAndHitDistance.emplace_back(i, Distance);
		}
	}

	// Find the closest hit.
	float HitDistance = FLT_MAX;
	for (const auto& CurrentInstance : InstanceIndexAndHitDistance)
	{
		if (CurrentInstance.second < HitDistance)
		{
			HitDistance = CurrentInstance.second;
			int ImageID = ImageInstanceIndexToImageID[CurrentInstance.first];
			Result = GetImage(ImageID);
		}
	}

	return Result;
}

void COLMAPProject::MouseButtonCallback(int Button, int Action, int Mods)
{
	COLMAPImage* CurrentImageUnderMouse = ImageUnderMouse();
	if (CurrentImageUnderMouse != nullptr)
		SetSelectedImageByIDInternal(CurrentImageUnderMouse->GetID());
}

bool COLMAPProject::SetSelectedImageByIDInternal(int ImageID)
{
	COLMAPImage* ImageToSelect = GetImage(ImageID);
	if (ImageToSelect == nullptr)
		return false;

	int PreviouslySelectedImageID = SelectedImageID;
	DetachCameraVisualizationFromImage(PreviouslySelectedImageID);
	SelectedImageID = ImageID;
	AttachCameraVisualizationToImage(ImageID);

	FEEntity* ImageEntity = MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(ImagesInstancedEntityID);
	if (ImageEntity != nullptr)
	{
		int NewSelectionInstanceIndex = -1;
		for (const auto& CurrentInstance : ImageInstanceIndexToImageID)
		{
			if (CurrentInstance.second == ImageID)
			{
				NewSelectionInstanceIndex = CurrentInstance.first;
				break;
			}
		}

		if (NewSelectionInstanceIndex != -1)
		{
			int PreviouslySelectedInstanceIndex = -1;
			for (const auto& CurrentInstance : ImageInstanceIndexToImageID)
			{
				if (CurrentInstance.second == PreviouslySelectedImageID)
				{
					PreviouslySelectedInstanceIndex = CurrentInstance.first;
					break;
				}
			}
			
			FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ImagesColorsSSBO));
			if (PreviouslySelectedInstanceIndex != -1)
				FE_GL_ERROR(glBufferSubData(GL_SHADER_STORAGE_BUFFER, PreviouslySelectedInstanceIndex * sizeof(glm::vec4), sizeof(glm::vec4), &DefaultImageColor));
			FE_GL_ERROR(glBufferSubData(GL_SHADER_STORAGE_BUFFER, NewSelectionInstanceIndex * sizeof(glm::vec4), sizeof(glm::vec4), &SelectedImageColor));
			FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
		}
	}
	
	return true;
}

bool COLMAPProject::SelectImageByID(int ImageID)
{
	COLMAPImage* ImageToSelect = GetImage(ImageID);
	if (ImageToSelect == nullptr)
		return false;

	SetSelectedImageByIDInternal(ImageID);
	return true;
}

COLMAPImage* COLMAPProject::GetSelectedImage()
{
	return GetImage(SelectedImageID);
}

FEEntity* COLMAPProject::GetPhotogrammetryAnchorEntity()
{
	return MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(PhotogrammetryAnchorID);
}

bool COLMAPProject::IsCameraVisualizationAttachedToImage(int ImageID)
{
	return CameraAttachedToImageID == ImageID;
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

	int InstanceIndex = -1;
	auto ImageIDIterator = ImageInstanceIndexToImageID.begin();
	while (ImageIDIterator != ImageInstanceIndexToImageID.end())
	{
		if (ImageIDIterator->second == ImageID)
			InstanceIndex = ImageIDIterator->first;

		ImageIDIterator++;
	}

	FEScene* MainScene = MAIN_SCENE_MANAGER.GetMainScene();
	FEEntity* ImagesInstancedEntity = MainScene->GetEntity(ImagesInstancedEntityID);
	CameraEntity->GetComponent<FETransformComponent>().SetPosition(CurrentImage->GetPosition());
	CameraEntity->GetComponent<FETransformComponent>().SetQuaternion(CurrentImage->GetRotation());
	CameraEntity->AttachTo(ImagesInstancedEntity, false);
	CameraEntity->SetComponentVisible(ComponentVisibilityType::ALL, true);

	CameraAttachedToImageID = ImageID;
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

	FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
	CameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0f));
	CameraEntity->GetComponent<FETransformComponent>().SetQuaternion(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
	CameraEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);

	CameraAttachedToImageID = -1;
	return true;
}

std::vector<COLMAPImage*> COLMAPProject::GetImagesContainingAABB(FEAABB AABBToTest)
{
	std::vector<COLMAPImage*> Result;
	for (auto& CurrentImage : Images)
	{
		COLMAPPhysicalCamera* PhysicalCamera = GetCameraForImage(CurrentImage.second->ID)->GetPhysicalCamera();
		if (PhysicalCamera == nullptr)
			continue;

		FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
		if (CameraEntity == nullptr)
			CreateCameraSceneRepresentation(PhysicalCamera->GetID());

		CameraEntity = PhysicalCamera->GetSceneEntity();
		if (CameraEntity == nullptr)
			continue;

		AttachCameraVisualizationToImage(CurrentImage.second->ID);
		
		FEEntity* ImagesInstancedEntity = MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(ImagesInstancedEntityID);
		// Updating starting from parent to ensure world transform is correct
		TRANSFORM_SYSTEM.ForceUpdateTransformComponent(ImagesInstancedEntity);

		CAMERA_SYSTEM.IndividualUpdate(CameraEntity, 0.0);
		FECameraComponent& CurrentCameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		CurrentCameraComponent.UpdateFrustum();

		FEFrustum CameraFrustum = CurrentCameraComponent.GetFrustum();

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

	NewEntity->AttachTo(GetPhotogrammetryAnchorEntity(), false);
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

	std::string AbsoluteFolderPath = FILE_SYSTEM.GetAbsolutePath(FolderPath);
	if (!FILE_SYSTEM.DoesDirectoryExist(AbsoluteFolderPath))
		return false;

	if (!FILE_SYSTEM.DoesFileExist(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_CAMERAS_FILE) ||
		!FILE_SYSTEM.DoesFileExist(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_IMAGES_FILE) ||
		!FILE_SYSTEM.DoesFileExist(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_TIE_POINTS_FILE))
		return false;

	this->FolderPath = AbsoluteFolderPath;
	
	FEEntity* Anchor = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("PhotogrammetryAnchor_" + this->GetID());
	PhotogrammetryAnchorID = Anchor->GetObjectID();
	FEEntity* MainEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	Anchor->AttachTo(MainEntity, false);

	LoadCameras(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_CAMERAS_FILE);
	LoadImages(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_IMAGES_FILE);
	LoadTiePoints(AbsoluteFolderPath + COLMAP_SPARSE_MODEL_FOLDER + COLMAP_TIE_POINTS_FILE);

	return true;
}

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

	/*for (const auto& CurrentImageID : NewProject->GetImagesIDList())
		if (!NewProject->CreateImageSceneRepresentation(CurrentImageID))
			return false;*/
	
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