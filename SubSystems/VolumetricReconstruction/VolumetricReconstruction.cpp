#include "VolumetricReconstruction.h"
#include "../UI/UICore.h"
#include "../COLMAP/COLMAPDataManager.h"
using namespace FocalEngine;

VolumetricReconstruction::VolumetricReconstruction() {}
VolumetricReconstruction::~VolumetricReconstruction() {}

// Returns the color at pixel (X, Y) as a glm::vec4 normalized to the [0, 1] range.
// RawData must come from FETexture::GetRawData(), and Width / InternalFormat must match that texture.
// The byte layout of RawData depends on InternalFormat, so the read differs per format.
glm::vec4 VolumetricReconstruction::GetPixelColor(const unsigned char* RawData, int X, int Y, int Width, GLint InternalFormat)
{
	const int PixelIndex = Y * Width + X;

	// 16-bit single channel (typical for depth maps): one unsigned short per pixel, stored in red.
	if (InternalFormat == GL_R16)
	{
		const float Value = reinterpret_cast<const unsigned short*>(RawData)[PixelIndex] / 65535.0f;
		return glm::vec4(Value, Value, Value, 1.0f);
	}

	// 32-bit float single channel (PFM depth maps): FETexture::GetRawData reads GL_R32F back
	// as GL_RED / GL_FLOAT, so the buffer holds one raw float per pixel, stored in red.
	// The value is returned as is, without clamping, because raw depth values must survive the round trip.
	if (InternalFormat == GL_R32F)
	{
		const float Value = reinterpret_cast<const float*>(RawData)[PixelIndex];
		return glm::vec4(Value, Value, Value, 1.0f);
	}

	// 32-bit float RGB (color PFM files): three raw floats per pixel.
	if (InternalFormat == GL_RGB32F)
	{
		const float* Pixel = reinterpret_cast<const float*>(RawData) + PixelIndex * 3;
		return glm::vec4(Pixel[0], Pixel[1], Pixel[2], 1.0f);
	}

	// 8-bit single channel: one unsigned char per pixel, stored in red.
	if (InternalFormat == GL_RED)
	{
		const float Value = RawData[PixelIndex] / 255.0f;
		return glm::vec4(Value, Value, Value, 1.0f);
	}

	// 8-bit RGB: three unsigned chars per pixel.
	if (InternalFormat == GL_RGB)
	{
		const unsigned char* Pixel = RawData + PixelIndex * 3;
		return glm::vec4(Pixel[0] / 255.0f, Pixel[1] / 255.0f, Pixel[2] / 255.0f, 1.0f);
	}

	// 8-bit RGBA, including the DXT-compressed formats that GetRawData reads back as RGBA bytes.
	const unsigned char* Pixel = RawData + PixelIndex * 4;
	return glm::vec4(Pixel[0] / 255.0f, Pixel[1] / 255.0f, Pixel[2] / 255.0f, Pixel[3] / 255.0f);
}

glm::vec4 VolumetricReconstruction::GetValueOfDepth(int X, int Y)
{
	if (NeuralNetworkDepthMap == nullptr)
		return glm::vec4(-1.0f);

	int Width = NeuralNetworkDepthMap->GetWidth();
	int Height = NeuralNetworkDepthMap->GetHeight();
	GLint InternalFormat = NeuralNetworkDepthMap->GetInternalFormat();

	if (X < 0 || X >= Width || Y < 0 || Y >= Height)
		return glm::vec4(-1.0f);

	return GetPixelColor(NeuralNetworkDepthMapRawData, X, Y, Width, InternalFormat);
}

float VolumetricReconstruction::ConvertDepthToWorldDistance(float DepthValue, float NearPlaneValue, float FarPlaneValue)
{
	float WorldDistance = 2.0 * DepthValue - 1.0;
	WorldDistance = 2.0 * NearPlaneValue * FarPlaneValue / (FarPlaneValue + NearPlaneValue - WorldDistance * (FarPlaneValue - NearPlaneValue));
	return WorldDistance;
}

// Unprojects a pixel to a world position using the camera's own rendered depth buffer, Camera's framebuffer must be up to date.
// Returns false if there is no rendered depth buffer or the pixel holds no geometry.
bool VolumetricReconstruction::GetWorldPositionFromCameraDepthBuffer(FEEntity* CameraEntity, int PixelX, int PixelY, int ImageWidth, int ImageHeight, glm::dvec3& OutWorldPosition, double& OutDistanceAlongRay)
{
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
		return false;

	if (ImageWidth <= 0 || ImageHeight <= 0 || PixelX < 0 || PixelX >= ImageWidth || PixelY < 0 || PixelY >= ImageHeight)
		return false;

	FECameraRenderingData* CameraData = RENDERER.GetCameraRenderingData(CameraEntity);
	if (CameraData == nullptr || CameraData->SceneToTextureFB == nullptr)
		return false;

	FETexture* DepthTexture = CameraData->SceneToTextureFB->GetDepthAttachment();
	if (DepthTexture == nullptr)
		return false;

	// Normalized coordinates of the pixel center, Y measured from the top of the image.
	const double NormalizedX = (PixelX + 0.5) / ImageWidth;
	const double NormalizedY = (PixelY + 0.5) / ImageHeight;

	// The depth buffer resolution can differ from the displayed depth map resolution.
	const int BufferWidth = DepthTexture->GetWidth();
	const int BufferHeight = DepthTexture->GetHeight();
	int BufferX = static_cast<int>(NormalizedX * BufferWidth);
	int BufferRowFromTop = static_cast<int>(NormalizedY * BufferHeight);
	BufferX = std::max(0, std::min(BufferX, BufferWidth - 1));
	BufferRowFromTop = std::max(0, std::min(BufferRowFromTop, BufferHeight - 1));

	// glGetTexImage returns rows bottom-up, while the displayed image is top-down.
	const int BufferY = BufferHeight - 1 - BufferRowFromTop;

	unsigned char* DepthRawData = DepthTexture->GetRawData();
	if (DepthRawData == nullptr)
		return false;

	// GL_DEPTH_COMPONENT32 raw data is one float per pixel in the [0, 1] range, full precision,
	// without the per-image min/max normalization that the PNG export applies.
	const double DepthValue = reinterpret_cast<const float*>(DepthRawData)[BufferY * BufferWidth + BufferX];
	delete[] DepthRawData;

	// The far plane clear value, nothing was rendered to this pixel.
	if (DepthValue >= 1.0)
		return false;

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

	const glm::dvec4 ClipCoordinates = glm::dvec4(2.0 * NormalizedX - 1.0, 1.0 - 2.0 * NormalizedY, 2.0 * DepthValue - 1.0, 1.0);
	glm::dvec4 EyeCoordinates = glm::inverse(glm::dmat4(CameraComponent.GetProjectionMatrix())) * ClipCoordinates;
	EyeCoordinates /= EyeCoordinates.w;

	const glm::dmat4 InverseViewMatrix = glm::inverse(glm::dmat4(CameraComponent.GetViewMatrix()));
	OutWorldPosition = glm::dvec3(InverseViewMatrix * glm::dvec4(EyeCoordinates.x, EyeCoordinates.y, EyeCoordinates.z, 1.0));

	// Distance from the camera position to the reconstructed point, in other words how far to move
	// along the normalized ray through this pixel to land on the geometry.
	const glm::dvec3 CameraPosition = glm::dvec3(InverseViewMatrix[3]);
	OutDistanceAlongRay = glm::length(OutWorldPosition - CameraPosition);

	return true;
}

// FE_FIX_ME: Temporary function, jittered point cloud density should be used instead.
FETexture* VolumetricReconstruction::Create3DTextureFromPointCloud(FEPointCloud* PointCloud, FEAABB NewVolumeAABB, size_t Dimension, std::string Name)
{
	if (PointCloud == nullptr)
		return nullptr;

	const std::vector<FEPointCloudVertex>& Points = PointCloud->GetRawData();
	if (Points.empty())
		return nullptr;

	if (Dimension < 1)
		return nullptr;

	// Mirror the measurement grid layout: a cubic grid centered on the volume AABB,
	// with the side length equal to the longest axis so every voxel stays a cube.
	const float LongestAxisLength = NewVolumeAABB.GetLongestAxisLength();
	if (LongestAxisLength <= 0.0f)
		return nullptr;

	const float VoxelSize = LongestAxisLength / static_cast<float>(Dimension);
	const glm::vec3 GridMin = NewVolumeAABB.GetCenter() - glm::vec3(LongestAxisLength * 0.5f);

	// Each voxel accumulates the number of points that fall inside it - the same
	// "points in cell" density measure that PointDensityLayerProducer reads from
	// the measurement grid (GridNode::UserData = PointsInCell.size()).
	const size_t VoxelCount = Dimension * Dimension * Dimension;
	std::vector<uint32_t> DensityCounts(VoxelCount, 0);

	uint32_t MaxCount = 0;
	for (size_t PointIndex = 0; PointIndex < Points.size(); PointIndex++)
	{
		const FEPointCloudVertex& Point = Points[PointIndex];
		const glm::vec3 LocalPosition = (glm::vec3(Point.X, Point.Y, Point.Z) - GridMin) / VoxelSize;

		const int VoxelX = static_cast<int>(LocalPosition.x);
		const int VoxelY = static_cast<int>(LocalPosition.y);
		const int VoxelZ = static_cast<int>(LocalPosition.z);

		// Skip points that land outside of the requested volume.
		if (VoxelX < 0 || VoxelY < 0 || VoxelZ < 0)
			continue;

		if (VoxelX >= static_cast<int>(Dimension) || VoxelY >= static_cast<int>(Dimension) || VoxelZ >= static_cast<int>(Dimension))
			continue;

		// Texture memory is laid out with X varying fastest, then Y, then Z.
		const size_t VoxelIndex = static_cast<size_t>(VoxelX) + static_cast<size_t>(VoxelY) * Dimension + static_cast<size_t>(VoxelZ) * Dimension * Dimension;
		DensityCounts[VoxelIndex]++;

		if (DensityCounts[VoxelIndex] > MaxCount)
			MaxCount = DensityCounts[VoxelIndex];
	}

	if (MaxCount == 0)
		return nullptr;

	// Normalize the per-voxel counts into the full 16 bit range so the density field
	// keeps as much resolution as possible. FEVolumeSystem reads the texture min/max
	// and rescales a GL_R16 volume by 65535 when it binds it as "volumeTexture".
	std::vector<uint16_t> VoxelData(VoxelCount, 0);
	for (size_t VoxelIndex = 0; VoxelIndex < VoxelCount; VoxelIndex++)
	{
		const float NormalizedDensity = static_cast<float>(DensityCounts[VoxelIndex]) / static_cast<float>(MaxCount);
		VoxelData[VoxelIndex] = static_cast<uint16_t>(NormalizedDensity * 65535.0f);
	}

	FETexture* DensityTexture = RESOURCE_MANAGER.RawDataTo3DFETexture(reinterpret_cast<unsigned char*>(VoxelData.data()), static_cast<int>(Dimension), static_cast<int>(Dimension), static_cast<int>(Dimension), GL_R16, GL_RED, GL_UNSIGNED_SHORT);
	if (DensityTexture == nullptr)
		return nullptr;

	if (!Name.empty())
		DensityTexture->SetName(Name);

	return DensityTexture;
}

void VolumetricReconstruction::LoadNeuralNetworkDepthReconstructionInputs(const std::string& FileName)
{
	if (FileName.empty() || DepthReconstructionFolder.empty() || MasksFolder.empty())
		return;

	// A name without extension means a PFM file, a typed extension (.pfm or .png) is used as is.
	std::string NeuralNetworkDepthMapFilePath = DepthReconstructionFolder + FileName;
	if (FILE_SYSTEM.GetFileExtension(NeuralNetworkDepthMapFilePath).empty())
		NeuralNetworkDepthMapFilePath += ".pfm";
	std::string MaskOfIgnoredPixelsTextureFilePath = MasksFolder + FileName + "_Mask.png";

	if (NeuralNetworkDepthMap != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(NeuralNetworkDepthMap);

	std::string TextureExtension = FILE_SYSTEM.GetFileExtension(NeuralNetworkDepthMapFilePath);
	if (TextureExtension == ".png")
	{
		NeuralNetworkDepthMap = RESOURCE_MANAGER.LoadPNGTexture(NeuralNetworkDepthMapFilePath);
	}
	else if (TextureExtension == ".pfm")
	{
		NeuralNetworkDepthMap = RESOURCE_MANAGER.LoadPFMTexture(NeuralNetworkDepthMapFilePath);
	}

	if (MaskOfIgnoredPixelsTexture != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(MaskOfIgnoredPixelsTexture);
	MaskOfIgnoredPixelsTexture = RESOURCE_MANAGER.LoadPNGTexture(MaskOfIgnoredPixelsTextureFilePath);

	delete[] NeuralNetworkDepthMapRawData;
	NeuralNetworkDepthMapRawData = NeuralNetworkDepthMap->GetRawData();
}

// Builds a point cloud by unprojecting per-pixel depth from an external depth map, remapped into the range of the camera's depth buffer.
// That remap assumes the external map was normalized the same way as the engine's depth, an approximation. Camera's framebuffer must be up to date.
// Returns the created entity, or nullptr if the depth buffer or a texture is unavailable.
FEEntity* VolumetricReconstruction::CreatePointCloudFromExternalDepthMap(FEEntity* CameraEntity, FETexture* CameraDepthBuffer, FETexture* ExternalDepthMap, FETexture* ColorTexture, std::string Name)
{
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
		return nullptr;

	if (CameraDepthBuffer == nullptr || ExternalDepthMap == nullptr || ColorTexture == nullptr)
		return nullptr;

	unsigned char* CameraDepthBufferRawData = CameraDepthBuffer->GetRawData();
	if (CameraDepthBufferRawData == nullptr)
		return nullptr;

	const float* DepthValues = reinterpret_cast<const float*>(CameraDepthBufferRawData);
	const size_t DepthPixelCount = static_cast<size_t>(CameraDepthBuffer->GetWidth()) * CameraDepthBuffer->GetHeight();

	float MinDepth = std::numeric_limits<float>::max();
	float MaxDepth = std::numeric_limits<float>::lowest();
	for (size_t i = 0; i < DepthPixelCount; i++)
	{
		if (DepthValues[i] >= 1.0f)
			continue;

		if (DepthValues[i] < MinDepth)
			MinDepth = DepthValues[i];
		if (DepthValues[i] > MaxDepth)
			MaxDepth = DepthValues[i];
	}

	delete[] CameraDepthBufferRawData;

	if (MinDepth > MaxDepth)
		return nullptr;

	const int TextureWidth = ExternalDepthMap->GetWidth();
	const int TextureHeight = ExternalDepthMap->GetHeight();
	if (TextureWidth <= 0 || TextureHeight <= 0)
		return nullptr;

	const GLint ExternalDepthMapInternalFormat = ExternalDepthMap->GetInternalFormat();
	// Float textures (PFM) hold raw depth, normalized integer textures (PNG) need remapping into the camera depth range.
	const bool bRawDepth = ExternalDepthMapInternalFormat == GL_R32F || ExternalDepthMapInternalFormat == GL_RGB32F;
	unsigned char* ExternalDepthMapRawData = ExternalDepthMap->GetRawData();
	if (ExternalDepthMapRawData == nullptr)
		return nullptr;

	const GLint ColorTextureInternalFormat = ColorTexture->GetInternalFormat();
	unsigned char* ColorTextureRawData = ColorTexture->GetRawData();
	if (ColorTextureRawData == nullptr)
	{
		delete[] ExternalDepthMapRawData;
		delete[] ColorTextureRawData;
		return nullptr;
	}

	bool bHaveMask = MaskOfIgnoredPixelsTexture != nullptr;
	unsigned char* MaskRawData = nullptr;
	if (bHaveMask)
	{
		MaskRawData = MaskOfIgnoredPixelsTexture->GetRawData();
		if (MaskRawData == nullptr)
			bHaveMask = false;
	}

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	const glm::dmat4 InverseProjectionMatrix = glm::inverse(glm::dmat4(CameraComponent.GetProjectionMatrix()));
	const glm::dmat4 InverseViewMatrix = glm::inverse(glm::dmat4(CameraComponent.GetViewMatrix()));

	std::vector<FEPointCloudVertex> Points;
	Points.reserve(static_cast<size_t>(TextureWidth) * TextureHeight);

	for (int TextureY = 0; TextureY < TextureHeight; TextureY++)
	{
		for (int TextureX = 0; TextureX < TextureWidth; TextureX++)
		{
			if (bHaveMask && MaskRawData != nullptr)
			{
				const int MaskIndex = (TextureY * TextureWidth + TextureX) * 4;
				if (MaskRawData[MaskIndex + 0] == 0 && MaskRawData[MaskIndex + 1] == 0 && MaskRawData[MaskIndex + 2] == 0)
					continue;
			}

			const float TextureValue = GetPixelColor(ExternalDepthMapRawData, TextureX, TextureY, TextureWidth, ExternalDepthMapInternalFormat).x;
			double DepthValue = 0.0;
			if (bRawDepth)
			{
				DepthValue = static_cast<double>(TextureValue);
			}
			else
			{
				DepthValue = MinDepth + static_cast<double>(TextureValue) * (MaxDepth - MinDepth);
			}

			// The texture rows are top-down, unlike the depth buffer, so Y is flipped for NDC.
			const double NormalizedDeviceX = 2.0 * (TextureX + 0.5) / TextureWidth - 1.0;
			const double NormalizedDeviceY = 1.0 - 2.0 * (TextureY + 0.5) / TextureHeight;

			const glm::dvec4 ClipCoordinates = glm::dvec4(NormalizedDeviceX, NormalizedDeviceY, 2.0 * DepthValue - 1.0, 1.0);
			glm::dvec4 EyeCoordinates = InverseProjectionMatrix * ClipCoordinates;
			EyeCoordinates /= EyeCoordinates.w;

			const glm::dvec3 WorldPosition = glm::dvec3(InverseViewMatrix * glm::dvec4(EyeCoordinates.x, EyeCoordinates.y, EyeCoordinates.z, 1.0));

			const glm::vec4 Color = GetPixelColor(ColorTextureRawData, TextureX, TextureY, TextureWidth, ColorTextureInternalFormat);
			glm::vec4 FinalColor = Color;
			if (ColorTextureInternalFormat == GL_DEPTH_COMPONENT32F)
			{
				FinalColor.x = std::max(0.0f, std::min(1.0f, TextureValue));
				FinalColor.y = FinalColor.x;
				FinalColor.z = FinalColor.x;
			}

			FEPointCloudVertex NewPoint;
			NewPoint.X = static_cast<float>(WorldPosition.x);
			NewPoint.Y = static_cast<float>(WorldPosition.y);
			NewPoint.Z = static_cast<float>(WorldPosition.z);
			NewPoint.R = static_cast<unsigned char>(FinalColor.x * 255.0f);
			NewPoint.G = static_cast<unsigned char>(FinalColor.y * 255.0f);
			NewPoint.B = static_cast<unsigned char>(FinalColor.z * 255.0f);
			NewPoint.A = static_cast<unsigned char>(FinalColor.w * 255.0f);
			Points.push_back(NewPoint);
		}
	}

	delete[] ExternalDepthMapRawData;
	delete[] ColorTextureRawData;
	delete[] MaskRawData;

	if (Points.empty())
		return nullptr;

	FEEntity* EntityToReturn = nullptr;
	if (bAccumulatePoints && LastCreatedEntity != nullptr)
	{
		if (LastCreatedEntity->HasComponent<FEPointCloudComponent>())
		{
			FEPointCloudComponent& PointCloudComponent = LastCreatedEntity->GetComponent<FEPointCloudComponent>();
			FEPointCloud* OldPointCloud = PointCloudComponent.GetPointCloud();
			std::vector<FEPointCloudVertex> AccumulatedPoints = OldPointCloud->GetRawData();
			AccumulatedPoints.insert(AccumulatedPoints.end(), Points.begin(), Points.end());

			FEPointCloud* NewPointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(AccumulatedPoints, Name, "", false, true);
			PointCloudComponent.SetPointCloud(NewPointCloud);

			RESOURCE_MANAGER.DeleteFEPointCloud(OldPointCloud);

			EntityToReturn = LastCreatedEntity;
			return EntityToReturn;
		}
	}

	FEPointCloud* NewPointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(Points, Name, "", false, true);
	EntityToReturn = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity(Name);
	EntityToReturn->AddComponent<FEPointCloudComponent>(NewPointCloud);

	LastCreatedEntity = EntityToReturn;
	return EntityToReturn;
}

void VolumetricReconstruction::RenderUI()
{
	if (!DEVELOPER_MODE.IsOn())
		return;

	if (ImGui::Begin("Volumetric Reconstruction"))
	{
		AnalysisObject* CurrentAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (LastCreatedEntity != nullptr)
		{
			if (ImGui::Button("Create Volumetric representation for point cloud"))
			{
				AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
				if (ActiveObject != nullptr && ActiveObject->GetAnalysisData() != nullptr && LastCreatedEntity->HasComponent<FEPointCloudComponent>())
				{
					FEPointCloud* PointCloud = LastCreatedEntity->GetComponent<FEPointCloudComponent>().GetPointCloud();
					const FEAABB VolumeAABB = ActiveObject->GetAnalysisData()->GetAABB();

					FETexture* VolumeTexture = Create3DTextureFromPointCloud(PointCloud, VolumeAABB, VolumeTextureResolution, "VolumeTexture");
					if (VolumeTexture != nullptr && !VOLUME_SYSTEM.GetVolumetricShaders().empty())
					{
						if (VolumetricEntity == nullptr)
							VolumetricEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("VolumetricEntity");

						if (!VolumetricEntity->HasComponent<FEVolumeComponent>())
							VolumetricEntity->AddComponent<FEVolumeComponent>();

						FENewMaterial* MaterialFor3DTextures = new FENewMaterial("VolumeMaterial");
						MaterialFor3DTextures->SetMaterialType(FEMaterialType::Volumetric);
						MaterialFor3DTextures->SetBlendMode(FEMaterialBlendMode::Additive);
						MaterialFor3DTextures->SetShader(VOLUME_SYSTEM.GetVolumetricShaders()[0]);
						MaterialFor3DTextures->SetTextureOverride("VolumeTexture", VolumeTexture->GetObjectID());

						FEVolumeComponent& VolumeComponent = VolumetricEntity->GetComponent<FEVolumeComponent>();
						VolumeComponent.SetMaterial(MaterialFor3DTextures);
					}
				}
			}

			// Show transform controls for the volumetric entity
			if (VolumetricEntity != nullptr && VolumetricEntity->HasComponent<FETransformComponent>())
			{
				ImGui::SeparatorText("Volumetric Entity Transform");

				FETransformComponent& VolumeTransform = VolumetricEntity->GetComponent<FETransformComponent>();
				UI_CORE.ShowTransformConfiguration(VolumetricEntity->GetObjectID(), &VolumeTransform);
			}

			// TEMP / DEBUG: volumetric shader-parameter and transfer-function editor,
			// ported from the FocalEngine volumetric editor's inspector.
			if (VolumetricEntity != nullptr && VolumetricEntity->HasComponent<FEVolumeComponent>())
			{
				static VolumeTransferFunctionWidget TransferFunctionWidget;

				FEVolumeComponent& VolumeComponent = VolumetricEntity->GetComponent<FEVolumeComponent>();
				FENewMaterial* VolumeMaterial = VolumeComponent.GetMaterial();

				// Tweakable shader parameters. NOTE: this edits the shader's uniforms directly, so the values
				// are shared by every volume using this shader. Per-volume overrides will come with a 3D material.
				if (VolumeMaterial != nullptr)
				{
					// Uniforms the engine fills in each frame, not meant to be edited by hand.
					auto IsEngineManagedUniform = [](const std::string& UniformName) {
						return UniformName == "NearPlane" || UniformName == "FarPlane" ||
							UniformName == "invViewMatrix" || UniformName == "invProjectionMatrix" ||
							UniformName == "FEWorldMatrix" || UniformName == "FECameraPosition";
						};

					const std::vector<std::string> UniformNames = VolumeMaterial->GetUniformOverrideNameList();
					bool bShownParametersHeader = false;
					for (size_t i = 0; i < UniformNames.size(); i++)
					{
						const std::string& UniformName = UniformNames[i];
						if (IsEngineManagedUniform(UniformName))
							continue;

						FEShaderUniformValue* UniformValue = VolumeMaterial->GetUniformOverride(UniformName);
						if (UniformValue == nullptr)
							continue;

						// Only scalar/vector uniforms are editable here; samplers and matrices are skipped by type.
						const bool bIsEditableType = UniformValue->IsType<float>() || UniformValue->IsType<int>() ||
							UniformValue->IsType<bool>() || UniformValue->IsType<glm::vec2>() ||
							UniformValue->IsType<glm::vec3>() || UniformValue->IsType<glm::vec4>();
						if (!bIsEditableType)
							continue;

						if (!bShownParametersHeader)
						{
							ImGui::Separator();
							ImGui::Text("Shader parameters : ");
							bShownParametersHeader = true;
						}

						const std::string WidgetID = "##VolumeUniform_" + UniformName;

						if (UniformValue->IsType<float>())
						{
							float Data = UniformValue->GetValue<float>();
							ImGui::Text("%s", UniformName.c_str());
							if (ImGui::DragFloat(WidgetID.c_str(), &Data, 0.01f))
								VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
						}
						else if (UniformValue->IsType<int>())
						{
							int Data = UniformValue->GetValue<int>();
							ImGui::Text("%s", UniformName.c_str());
							if (ImGui::DragInt(WidgetID.c_str(), &Data, 1.0f, 1, 4096))
								VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
						}
						else if (UniformValue->IsType<bool>())
						{
							bool Data = UniformValue->GetValue<bool>();
							if (ImGui::Checkbox(UniformName.c_str(), &Data))
								VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
						}
						else if (UniformValue->IsType<glm::vec2>())
						{
							glm::vec2 Data = UniformValue->GetValue<glm::vec2>();
							ImGui::Text("%s", UniformName.c_str());
							if (ImGui::DragFloat2(WidgetID.c_str(), &Data.x, 0.01f))
								VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
						}
						else if (UniformValue->IsType<glm::vec3>())
						{
							glm::vec3 Data = UniformValue->GetValue<glm::vec3>();
							ImGui::Text("%s", UniformName.c_str());
							if (ImGui::DragFloat3(WidgetID.c_str(), &Data.x, 0.01f))
								VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
						}
						else if (UniformValue->IsType<glm::vec4>())
						{
							glm::vec4 Data = UniformValue->GetValue<glm::vec4>();
							ImGui::Text("%s", UniformName.c_str());
							if (ImGui::DragFloat4(WidgetID.c_str(), &Data.x, 0.01f))
								VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
						}
					}

					// Transfer function editor, only for shaders that sample the LUT (TransferFunctionTexture).
					if (VOLUME_SYSTEM.DoesVolumeComponentHaveTransferFunction(VolumeComponent))
					{
						ImGui::Separator();
						ImGui::Text("Transfer function :");

						// Show volume non-normalized value range.
						FETexture* VolumeTexture = VolumeMaterial->GetTextureOverride("volumeTexture");
						const float DataValueLow = VolumeTexture != nullptr ? VolumeTexture->GetMinValue().x : 0.0f;
						const float DataValueHigh = VolumeTexture != nullptr ? VolumeTexture->GetMaxValue().x : 1.0f;
						TransferFunctionWidget.Render(VolumetricEntity, DataValueLow, DataValueHigh);
					}
				}
			}
		}

		ImGui::Text("Depth maps folder: %s", DepthReconstructionFolder.empty() ? "not set" : DepthReconstructionFolder.c_str());
		if (ImGui::Button("Select depth maps folder..."))
		{
			std::string SelectedFolder;
			FILE_SYSTEM.ShowFolderOpenDialog(SelectedFolder);
			if (!SelectedFolder.empty())
				DepthReconstructionFolder = SelectedFolder + "/";
		}

		ImGui::Text("Masks folder: %s", MasksFolder.empty() ? "not set" : MasksFolder.c_str());
		if (ImGui::Button("Select masks folder..."))
		{
			std::string SelectedFolder;
			FILE_SYSTEM.ShowFolderOpenDialog(SelectedFolder);
			if (!SelectedFolder.empty())
				MasksFolder = SelectedFolder + "/";
		}

		const bool bFoldersAreSet = !DepthReconstructionFolder.empty() && !MasksFolder.empty();
		if (!bFoldersAreSet)
		{
			ImGui::TextDisabled("Select both folders before loading an image.");
			ImGui::BeginDisabled();
		}

		static char FileName[1024] = "";
		ImGui::InputText("File name", FileName, sizeof(FileName));
		if (ImGui::Button("Load"))
			LoadNeuralNetworkDepthReconstructionInputs(FileName);

		if (!bFoldersAreSet)
			ImGui::EndDisabled();

		if (NeuralNetworkDepthMap == nullptr)
		{
			ImGui::End();
			return;
		}

		static glm::vec2 SelectedPixel = glm::vec2(-1.0f);

		const int TextureWidth = NeuralNetworkDepthMap->GetWidth();
		const int TextureHeight = NeuralNetworkDepthMap->GetHeight();
		// Fit the image to the window width, keeping the texture aspect ratio.
		const float DisplayWidth = std::max(1.0f, ImGui::GetContentRegionAvail().x);
		const ImVec2 DisplaySize = ImVec2(DisplayWidth, DisplayWidth * TextureHeight / TextureWidth);

		// Top-left corner of the image in screen space, captured before the image is drawn.
		const ImVec2 ImageScreenPosition = ImGui::GetCursorScreenPos();
		ImGui::Image(NeuralNetworkDepthMap->GetTextureID(), DisplaySize);

		// Map a left click inside the image back to texel coordinates.
		// The image is stretched to DisplaySize, so X and Y are scaled independently.
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			const ImVec2 MousePosition = ImGui::GetIO().MousePos;
			const float RelativeX = (MousePosition.x - ImageScreenPosition.x) / DisplaySize.x;
			const float RelativeY = (MousePosition.y - ImageScreenPosition.y) / DisplaySize.y;

			int PixelX = static_cast<int>(RelativeX * TextureWidth);
			int PixelY = static_cast<int>(RelativeY * TextureHeight);
			PixelX = std::max(0, std::min(PixelX, TextureWidth - 1));
			PixelY = std::max(0, std::min(PixelY, TextureHeight - 1));

			SelectedPixel = glm::vec2(PixelX, PixelY);
		}

		// Draw a marker over the center of the selected texel.
		if (SelectedPixel.x >= 0.0f && SelectedPixel.y >= 0.0f)
		{
			const float MarkerX = ImageScreenPosition.x + (SelectedPixel.x + 0.5f) / TextureWidth * DisplaySize.x;
			const float MarkerY = ImageScreenPosition.y + (SelectedPixel.y + 0.5f) / TextureHeight * DisplaySize.y;
			ImGui::GetWindowDrawList()->AddCircle(ImVec2(MarkerX, MarkerY), 5.0f, IM_COL32(255, 0, 0, 255), 0, 2.0f);
		}

		// Show info about the selected pixel.
		if (SelectedPixel.x >= 0.0f && SelectedPixel.y >= 0.0f)
		{
			const glm::vec4 Color = GetValueOfDepth(static_cast<int>(SelectedPixel.x), static_cast<int>(SelectedPixel.y));
			ImGui::Text("Selected pixel: %d, %d", static_cast<int>(SelectedPixel.x), static_cast<int>(SelectedPixel.y));
			ImGui::Text("Color (RGBA): %.4f, %.4f, %.4f, %.4f", Color.x, Color.y, Color.z, Color.w);
			ImGui::Text("Depth (red channel): %.4f", Color.x);

			AnalysisObject* CurrentAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
			if (CurrentAnalysisObject != nullptr)
			{
				COLMAPProject* CurrentProject = COLMAP_DATA_MANAGER.GetProjectByAnalysisObjectID(CurrentAnalysisObject->GetID());
				if (CurrentProject != nullptr)
				{
					COLMAPImage* SelectedImage = CurrentProject->GetSelectedImage();
					if (SelectedImage != nullptr)
					{
						COLMAPCamera* ImageCamera = CurrentProject->GetCameraForImage(SelectedImage->GetID());
						COLMAPPhysicalCamera* PhysicalCamera = ImageCamera->GetPhysicalCamera();
						FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
						FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

						float NearPlane = CameraComponent.GetNearPlane();
						float FarPlane = CameraComponent.GetFarPlane();

						float DepthValue = Color.x;
						float WorldDistance = ConvertDepthToWorldDistance(DepthValue, NearPlane, FarPlane);

						ImGui::Text("World distance: %.4f", WorldDistance);

						// The COLMAP camera's framebuffer is only filled when the engine renders from that
						// camera, so force a render (without file export) before reading its depth buffer.
						// Both the render and the readback are expensive, so only redo them when the selection changes.
						static glm::vec2 LastReconstructedPixel = glm::vec2(-2.0f);
						static int LastReconstructedImageID = -1;
						static bool bReconstructionValid = false;
						static glm::dvec3 ReconstructedWorldPosition = glm::dvec3(0.0);
						static double DistanceAlongRay = 0.0;

						if (LastReconstructedPixel != SelectedPixel || LastReconstructedImageID != SelectedImage->GetID())
						{
							LastReconstructedPixel = SelectedPixel;
							LastReconstructedImageID = SelectedImage->GetID();

							bReconstructionValid = false;
							if (CurrentProject->RenderViewFromImage(SelectedImage->GetID(), true, FE_DEPTH_EXPORT_GRAYSCALE_PNG, "", false))
								bReconstructionValid = GetWorldPositionFromCameraDepthBuffer(CameraEntity, static_cast<int>(SelectedPixel.x), static_cast<int>(SelectedPixel.y), TextureWidth, TextureHeight, ReconstructedWorldPosition, DistanceAlongRay);
						}

						if (bReconstructionValid)
						{
							ImGui::Text("World position: %.4f, %.4f, %.4f", ReconstructedWorldPosition.x, ReconstructedWorldPosition.y, ReconstructedWorldPosition.z);
							ImGui::Text("Distance along ray from camera: %.4f", DistanceAlongRay);
						}
						else
						{
							ImGui::Text("World position: no geometry at this pixel or depth render failed.");
						}

						if (ImGui::Button("Create point cloud from loaded depth map"))
						{
							if (CurrentProject->RenderViewFromImage(SelectedImage->GetID(), true, FE_DEPTH_EXPORT_GRAYSCALE_PNG, "", false))
							{
								FECameraRenderingData* CameraData = RENDERER.GetCameraRenderingData(CameraEntity);
								if (CameraData != nullptr && CameraData->SceneToTextureFB != nullptr)
								{
									FETexture* DepthTexture = CameraData->SceneToTextureFB->GetDepthAttachment();
									if (DepthTexture != nullptr)
									{
										CreatePointCloudFromExternalDepthMap(CameraEntity, DepthTexture, NeuralNetworkDepthMap, NeuralNetworkDepthMap, "ExternalDepthPointCloud_" + std::to_string(SelectedImage->GetID()));
									}
								}
							}
						}
					}
				}
			}

			if (LastCreatedEntity != nullptr)
			{
				bool bIsVisible = LastCreatedEntity->IsVisible();
				ImGui::Checkbox("Show created point cloud", &bIsVisible);
				LastCreatedEntity->SetVisible(bIsVisible);

				if (ImGui::Button("Delete created point cloud"))
				{
					MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(LastCreatedEntity->GetObjectID());
					LastCreatedEntity = nullptr;
				}
			}
		}
		else
		{
			ImGui::Text("Click the image to select a pixel.");
		}
	}

	ImGui::End();
}