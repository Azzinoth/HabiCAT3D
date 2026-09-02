#include "SubSystems/ConsoleJobs/ConsoleJobManager.h"
#include "SubSystems/VRManager/VRManager.h"
#include"VolumeTransferFunctionWidget.h"
#include "Tests/RunAllTests.h"
using namespace FocalEngine;

std::string HabiCATDepthFolder = "BulkImageOut_8bit_Depth/";

std::string ImageFileName = "after_cam_1_vid_1_111_111";

std::string MyTextureToLoad = HabiCATDepthFolder + ImageFileName + "_Depth8bit.png";
//std::string MyTextureToLoad = "D:/06_12_2026/BulkImageOut_8bit_Depth/after_cam_1_vid_1_109_109_Depth8bit.png";

//std::string NNTextureToLoad = "D:/aligned_depths/" + ImageFileName + ".png";
//std::string NNTextureToLoad = "D:/06_12_2026/aligned_depths/after_cam_1_vid_1_109_109.png";
//std::string NNTextureToLoad = "aligned_depths/" + ImageFileName + ".png";
std::string NNTextureToLoad = "\\rasbora\\Shared\\joe_stuff\\32bit-coral-depths\\full_32bit_multiframe-informed-hough_s100_n141120-32bit\\aligned_depths\\" + ImageFileName + ".pfm";

//std::string PhotoTextureToLoad = "C:/Users/Kindr/Downloads/Site_USED_IN_IEEE_VIS_2026 (1)/images/after_cam_1_vid_1_109_109.jpg";
std::string PhotoTextureToLoad = "//mallard/Shared/Coral_Cameras_Data/images/" + ImageFileName + ".jpg";

std::string MaskOfIgnoredPixelsTextureToLoad = "Masks/" + ImageFileName + "_Mask.png";




bool bPFMUsed = true;

FETexture* LoadedDepthMap = nullptr;
FETexture* MyDepthMap = nullptr;
FETexture* NNDepthMap = nullptr;
FETexture* PhotoTexture = nullptr;
FETexture* MaskOfIgnoredPixelsTexture = nullptr;
unsigned char* LoadedDepthMapRawData = nullptr;
const int VolumeTextureResolution = 1024;

FEEntity* LastCreatedEntity = nullptr;
bool bAccumulatePoints = true;
FEEntity* VolumetricEntity = nullptr;

void LoadingJoeInfoProjection(std::string& FileName)
{
	std::string MyTextureFilePath = HabiCATDepthFolder + FileName + "_Depth8bit.png";
	//std::string NNTextureFilePath = "D:/aligned_depths/" + FileName + ".png";
	std::string NNTextureFilePath = "\\\\rasbora\\Shared\\joe_stuff\\32bit-coral-depths\\full_32bit_multiframe-informed-hough_s100_n141120-32bit\\aligned_depths\\" + FileName + ".pfm";
	std::string PhotoTextureFilePath = "//mallard/Shared/Coral_Cameras_Data/images/" + FileName + ".jpg";
	std::string MaskOfIgnoredPixelsTextureFilePath = "Masks/" + FileName + "_Mask.png";

	if (MyDepthMap != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(MyDepthMap);
	
	MyDepthMap = RESOURCE_MANAGER.LoadPNGTexture(MyTextureFilePath);

	if (NNDepthMap != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(NNDepthMap);

	std::string TextureExtension = FILE_SYSTEM.GetFileExtension(NNTextureFilePath);
	if (TextureExtension == ".png")
	{
		NNDepthMap = RESOURCE_MANAGER.LoadPNGTexture(NNTextureFilePath);
	}
	else if (TextureExtension == ".pfm")
	{
		NNDepthMap = RESOURCE_MANAGER.LoadPFMTexture(NNTextureFilePath);
	}

	if (PhotoTexture != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(PhotoTexture);
	PhotoTexture = RESOURCE_MANAGER.LoadJPGTexture(PhotoTextureFilePath);

	if (MaskOfIgnoredPixelsTexture != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(MaskOfIgnoredPixelsTexture);
	MaskOfIgnoredPixelsTexture = RESOURCE_MANAGER.LoadPNGTexture(MaskOfIgnoredPixelsTextureFilePath);

	LoadedDepthMap = NNDepthMap;
	delete[] LoadedDepthMapRawData;
	LoadedDepthMapRawData = LoadedDepthMap->GetRawData();
}

void BulkLoadingJoeProjection_Individual(std::string& FileName)
{
	std::string MyTextureFilePath = HabiCATDepthFolder + FileName + "_Depth8bit.png";
	//std::string NNTextureFilePath = "D:/aligned_depths/" + FileName + ".png";
	std::string NNTextureFilePath = "\\\\rasbora\\Shared\\joe_stuff\\32bit-coral-depths\\full_32bit_multiframe-informed-hough_s100_n141120-32bit\\aligned_depths\\" + FileName + ".pfm";
	std::string MaskOfIgnoredPixelsTextureFilePath = "coral-masks/mask_" + FileName + ".png";

	if (MyDepthMap != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(MyDepthMap);
	MyDepthMap = RESOURCE_MANAGER.LoadPNGTexture(MyTextureFilePath);

	if (NNDepthMap != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(NNDepthMap);

	std::string TextureExtension = FILE_SYSTEM.GetFileExtension(NNTextureFilePath);
	if (TextureExtension == ".png")
	{
		NNDepthMap = RESOURCE_MANAGER.LoadPNGTexture(NNTextureFilePath);
	}
	else if (TextureExtension == ".pfm")
	{
		NNDepthMap = RESOURCE_MANAGER.LoadPFMTexture(NNTextureFilePath);
	}

	PhotoTexture = nullptr;

	if (MaskOfIgnoredPixelsTexture != nullptr)
		RESOURCE_MANAGER.DeleteFETexture(MaskOfIgnoredPixelsTexture);
	MaskOfIgnoredPixelsTexture = RESOURCE_MANAGER.LoadPNGTexture(MaskOfIgnoredPixelsTextureFilePath);

	LoadedDepthMap = NNDepthMap;
	delete[] LoadedDepthMapRawData;
	LoadedDepthMapRawData = LoadedDepthMap->GetRawData();
}

// Returns the color at pixel (X, Y) as a glm::vec4 normalized to the [0, 1] range.
// RawData must come from FETexture::GetRawData(), and Width / InternalFormat must match that texture.
// The byte layout of RawData depends on InternalFormat, so the read differs per format.
glm::vec4 GetPixelColor(const unsigned char* RawData, int X, int Y, int Width, GLint InternalFormat)
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

glm::vec4 GetValueOfDepth(int X, int Y)
{
	if (LoadedDepthMap == nullptr)
		return glm::vec4(-1.0f);

	int Width = LoadedDepthMap->GetWidth();
	int Height = LoadedDepthMap->GetHeight();
	GLint InternalFormat = LoadedDepthMap->GetInternalFormat();

	if (X < 0 || X >= Width || Y < 0 || Y >= Height)
		return glm::vec4(-1.0f);

	return GetPixelColor(LoadedDepthMapRawData, X, Y, Width, InternalFormat);
}

float ConvertDepthToWorldDistance(float DepthValue, float NearPlaneValue, float FarPlaneValue)
{
	float WorldDistance = 2.0 * DepthValue - 1.0;
	WorldDistance = 2.0 * NearPlaneValue * FarPlaneValue / (FarPlaneValue + NearPlaneValue - WorldDistance * (FarPlaneValue - NearPlaneValue));
	return WorldDistance;
}

// Reconstructs the world position behind a depth map pixel, using the in-memory depth buffer
// of the camera that rendered it. Works like FEGeometry::CreateMouseRayToWorld, but feeds the
// real depth into the unprojection instead of -1.0, so the perspective divide yields the point
// on the geometry itself rather than just a ray direction.
// The camera's framebuffer must be up to date, for example through RenderViewFromImage.
// PixelX and PixelY are texel coordinates in the displayed depth map, with Y measured from the top.
// Returns false if the camera has no rendered depth buffer or the pixel holds no geometry.
bool GetWorldPositionFromDepthMapPixel(FEEntity* CameraEntity, int PixelX, int PixelY, int ImageWidth, int ImageHeight, glm::dvec3& OutWorldPosition, double& OutDistanceAlongRay)
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

// Reconstructs a world position for every pixel of the camera's in-memory depth buffer and
// places the result in a point cloud attached to a new entity in the main scene.
// Same unprojection as GetWorldPositionFromDepthMapPixel, but the depth buffer is read back
// and the matrices are inverted only once for the whole image instead of per pixel.
// Points are in world space, so the new entity keeps an identity transform.
// Points are colored by their normalized depth, matching how the exported depth map PNG looks.
// The camera's framebuffer must be up to date, for example through RenderViewFromImage.
// Returns the created entity, or nullptr if the camera has no rendered depth buffer.
FEEntity* CreatePointCloudFromCameraDepthBuffer(FEEntity* CameraEntity, std::string Name)
{
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
		return nullptr;

	FECameraRenderingData* CameraData = RENDERER.GetCameraRenderingData(CameraEntity);
	if (CameraData == nullptr || CameraData->SceneToTextureFB == nullptr)
		return nullptr;

	FETexture* DepthTexture = CameraData->SceneToTextureFB->GetDepthAttachment();
	if (DepthTexture == nullptr)
		return nullptr;

	const int BufferWidth = DepthTexture->GetWidth();
	const int BufferHeight = DepthTexture->GetHeight();
	if (BufferWidth <= 0 || BufferHeight <= 0)
		return nullptr;

	unsigned char* DepthRawData = DepthTexture->GetRawData();
	if (DepthRawData == nullptr)
		return nullptr;

	const float* DepthValues = reinterpret_cast<const float*>(DepthRawData);
	const size_t PixelCount = static_cast<size_t>(BufferWidth) * BufferHeight;

	// Min/max of the rendered depth values, used only to color the points.
	float MinDepth = std::numeric_limits<float>::max();
	float MaxDepth = std::numeric_limits<float>::lowest();
	for (size_t i = 0; i < PixelCount; i++)
	{
		if (DepthValues[i] >= 1.0f)
			continue;

		if (DepthValues[i] < MinDepth)
			MinDepth = DepthValues[i];
		if (DepthValues[i] > MaxDepth)
			MaxDepth = DepthValues[i];
	}

	// No geometry was rendered at all.
	if (MinDepth > MaxDepth)
	{
		delete[] DepthRawData;
		return nullptr;
	}

	const float DepthRange = MaxDepth - MinDepth;
	const float InverseDepthRange = DepthRange > 1e-6f ? 1.0f / DepthRange : 1.0f;

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	const glm::dmat4 InverseProjectionMatrix = glm::inverse(glm::dmat4(CameraComponent.GetProjectionMatrix()));
	const glm::dmat4 InverseViewMatrix = glm::inverse(glm::dmat4(CameraComponent.GetViewMatrix()));

	std::vector<FEPointCloudVertex> Points;
	Points.reserve(PixelCount);

	const int ColorWidth = PhotoTexture->GetWidth();
	const int ColorHeight = PhotoTexture->GetHeight();
	const GLint ColorTextureInternalFormat = PhotoTexture->GetInternalFormat();
	unsigned char* ColorTextureRawData = PhotoTexture->GetRawData();
	if (ColorTextureRawData == nullptr)
	{
		delete[] DepthRawData;
		return nullptr;
	}

	for (int BufferY = 0; BufferY < BufferHeight; BufferY++)
	{
		for (int BufferX = 0; BufferX < BufferWidth; BufferX++)
		{
			const float DepthValue = DepthValues[BufferY * BufferWidth + BufferX];

			// The far plane clear value, nothing was rendered to this pixel.
			if (DepthValue >= 1.0f)
				continue;

			// glGetTexImage returns rows bottom-up, which already matches the NDC Y direction,
			// so no flip is needed when iterating the buffer directly.
			const double NormalizedDeviceX = 2.0 * (BufferX + 0.5) / BufferWidth - 1.0;
			const double NormalizedDeviceY = 2.0 * (BufferY + 0.5) / BufferHeight - 1.0;

			const glm::dvec4 ClipCoordinates = glm::dvec4(NormalizedDeviceX, NormalizedDeviceY, 2.0 * static_cast<double>(DepthValue) - 1.0, 1.0);
			glm::dvec4 EyeCoordinates = InverseProjectionMatrix * ClipCoordinates;
			EyeCoordinates /= EyeCoordinates.w;

			const glm::dvec3 WorldPosition = glm::dvec3(InverseViewMatrix * glm::dvec4(EyeCoordinates.x, EyeCoordinates.y, EyeCoordinates.z, 1.0));

			const float NormalizedDepth = (DepthValue - MinDepth) * InverseDepthRange;
			const float ClampedDepth = std::max(0.0f, std::min(1.0f, NormalizedDepth));

			// Default to grayscale depth; replace with the photo color when it is readable.
			// GetPixelColor can't decode GL_DEPTH_COMPONENT32F, so keep gray for that format.
			glm::vec4 FinalColor(ClampedDepth, ClampedDepth, ClampedDepth, 1.0f);
			if (ColorTextureInternalFormat != GL_DEPTH_COMPONENT32F)
			{
				// The depth buffer is bottom-up, the photo is top-down: flip the row and scale to the photo size.
				const double U = (BufferX + 0.5) / BufferWidth;
				const double VFromTop = 1.0 - (BufferY + 0.5) / BufferHeight;
				int ColorX = std::max(0, std::min(static_cast<int>(U * ColorWidth), ColorWidth - 1));
				int ColorY = std::max(0, std::min(static_cast<int>(VFromTop * ColorHeight), ColorHeight - 1));
				FinalColor = GetPixelColor(ColorTextureRawData, ColorX, ColorY, ColorWidth, ColorTextureInternalFormat);
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

	delete[] DepthRawData;
	delete[] ColorTextureRawData;

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

// Builds a point cloud by combining the camera's rendered depth range with per-pixel depth
// values taken from an external depth map texture, for example one produced by Depth Anything.
// The external values are treated as if they were normalized with the same min/max that the
// engine depth render produces, then remapped back into the depth buffer range and unprojected.
// That assumption is an approximation, not exact math, but it matches how the engine exports
// depth map PNGs, which the external depth maps were aligned against.
// The camera's framebuffer must be up to date, for example through RenderViewFromImage.
// Returns the created entity, or nullptr if the depth buffer or the texture is unavailable.
FEEntity* CreatePointCloudFromExternalDepthMap(FEEntity* CameraEntity, FETexture* DepthMapForRange, FETexture* DepthMapForPosition, FETexture* ColorTextureMap, std::string Name)
{
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
		return nullptr;

	if (DepthMapForRange == nullptr || DepthMapForPosition == nullptr)
		return nullptr;

	unsigned char* DepthForRangeRawData = DepthMapForRange->GetRawData();
	if (DepthForRangeRawData == nullptr)
		return nullptr;

	const float* DepthValues = reinterpret_cast<const float*>(DepthForRangeRawData);
	const size_t DepthPixelCount = static_cast<size_t>(DepthMapForRange->GetWidth()) * DepthMapForRange->GetHeight();

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

	delete[] DepthForRangeRawData;

	if (MinDepth > MaxDepth)
		return nullptr;

	const int TextureWidth = DepthMapForPosition->GetWidth();
	const int TextureHeight = DepthMapForPosition->GetHeight();
	if (TextureWidth <= 0 || TextureHeight <= 0)
		return nullptr;

	const GLint DepthMapForPositionInternalFormat = DepthMapForPosition->GetInternalFormat();
	unsigned char* DepthMapForPositionRawData = DepthMapForPosition->GetRawData();
	if (DepthMapForPositionRawData == nullptr)
		return nullptr;

	const GLint ColorTextureInternalFormat = ColorTextureMap->GetInternalFormat();
	unsigned char* ColorTextureRawData = ColorTextureMap->GetRawData();
	if (ColorTextureRawData == nullptr)
	{
		delete[] DepthMapForPositionRawData;
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

			const float TextureValue = GetPixelColor(DepthMapForPositionRawData, TextureX, TextureY, TextureWidth, DepthMapForPositionInternalFormat).x;
			double DepthValue = 0.0;
			if (bPFMUsed)
			{
				DepthValue = static_cast<double>(TextureValue);
			}
			else
			{
				DepthValue = MinDepth + static_cast<double>(TextureValue) * (MaxDepth - MinDepth);
			}

			//const double DepthValue = MinDepth + static_cast<double>(TextureValue) * (MaxDepth - MinDepth);

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
			//const unsigned char Gray = static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, TextureValue)) * 255.0f);

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

	delete[] DepthMapForPositionRawData;
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

	//FEPointCloud* NewPointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(Points, Name, "", false, true);
	//FEEntity* NewEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity(Name);
	//NewEntity->AddComponent<FEPointCloudComponent>(NewPointCloud);

	//LastCreatedEntity = NewEntity;
	//return NewEntity;
}

// FE_FIX_ME: Temporary function, jittered point cloud density should be used instead.
FETexture* Create3DTextureFromPointCloud(FEPointCloud* PointCloud, FEAABB NewVolumeAABB, size_t Dimension, std::string Name)
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

void JoeInfoProjectionUI()
{
	if (ImGui::Begin("Projection UI"))
	{
		AnalysisObject* CurrentAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (CurrentAnalysisObject != nullptr)
		{
			COLMAPProject* CurrentProject = COLMAP_DATA_MANAGER.GetProjectByAnalysisObjectID(CurrentAnalysisObject->GetID());
			if (CurrentProject != nullptr)
			{
				if (ImGui::Button("(BULK)Create point cloud from loaded depth map"))
				{
					FEEntity* NewEntity = nullptr;
					std::vector<int> ImageIDs = CurrentProject->GetImagesIDList();
					for (size_t i = 0; i < ImageIDs.size(); i++)
					{
						if (i > 500)
							break;

						std::string PhotoPath = CurrentProject->GetPathToPhotoByImageID(ImageIDs[i]);
						if (!FILE_SYSTEM.DoesFileExist(PhotoPath))
							continue;
						std::string FileName = FILE_SYSTEM.GetFileName(PhotoPath, false);

						COLMAPCamera* ImageCamera = CurrentProject->GetCameraForImage(ImageIDs[i]);
						if (ImageCamera == nullptr)
							continue;

						COLMAPPhysicalCamera* PhysicalCamera = ImageCamera->GetPhysicalCamera();
						if (PhysicalCamera == nullptr)
							continue;

						FEEntity* CameraEntity = PhysicalCamera->GetSceneEntity();
						if (CameraEntity == nullptr)
							continue;

						BulkLoadingJoeProjection_Individual(std::string(FileName));

						if (!CurrentProject->RenderViewFromImage(ImageIDs[i], true, FE_DEPTH_EXPORT_GRAYSCALE_PNG, "", false))
							continue;

						FECameraRenderingData* CameraData = RENDERER.GetCameraRenderingData(CameraEntity);
						if (CameraData == nullptr || CameraData->SceneToTextureFB == nullptr)
							continue;

						FETexture* DepthTexture = CameraData->SceneToTextureFB->GetDepthAttachment();
						if (DepthTexture == nullptr)
							continue;

						FEEntity* CreatedEntity = CreatePointCloudFromExternalDepthMap(CameraEntity, DepthTexture, LoadedDepthMap, LoadedDepthMap, "ExternalDepthPointCloud_BULK");
						if (CreatedEntity != nullptr)
							NewEntity = CreatedEntity;
					}

					if (NewEntity != nullptr && NewEntity->HasComponent<FEPointCloudComponent>())
					{
						FEPointCloudComponent& PointCloudComponent = NewEntity->GetComponent<FEPointCloudComponent>();
						FEPointCloud* NewPointCloud = PointCloudComponent.GetPointCloud();
						RESOURCE_MANAGER.ExportFEPointCloudToLAZ(NewPointCloud, "ExternalDepthPointCloud_BULK.laz");
					}
				}
			}
		}


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

		static char FileName[1024] = "";
		ImGui::InputText("File name", FileName, sizeof(FileName));
		if (ImGui::Button("Load"))
			LoadingJoeInfoProjection/*BulkLoadingJoeProjection_Individual*/(std::string(FileName));
		
		if (LoadedDepthMap == nullptr)
		{
			ImGui::End();
			return;
		}

		static glm::vec2 SelectedPixel = glm::vec2(-1.0f);
	
		const int TextureWidth = LoadedDepthMap->GetWidth();
		const int TextureHeight = LoadedDepthMap->GetHeight();
		const ImVec2 DisplaySize = ImVec2(504, 280);

		// Top-left corner of the image in screen space, captured before the image is drawn.
		const ImVec2 ImageScreenPosition = ImGui::GetCursorScreenPos();
		ImGui::Image(LoadedDepthMap->GetTextureID(), DisplaySize);

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

						const glm::vec4 Color = GetValueOfDepth(static_cast<int>(SelectedPixel.x), static_cast<int>(SelectedPixel.y));
						float DepthValue = (Color.x + Color.y + Color.z) / 3.0f;
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
								bReconstructionValid = GetWorldPositionFromDepthMapPixel(CameraEntity, static_cast<int>(SelectedPixel.x), static_cast<int>(SelectedPixel.y), TextureWidth, TextureHeight, ReconstructedWorldPosition, DistanceAlongRay);
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

						if (ImGui::Button("Create point cloud from camera depth"))
						{
							FEEntity* NewEntity = nullptr;
							if (CurrentProject->RenderViewFromImage(SelectedImage->GetID(), true, FE_DEPTH_EXPORT_GRAYSCALE_PNG, "", false))
							{
								NewEntity = CreatePointCloudFromCameraDepthBuffer(CameraEntity, "DepthPointCloud_" + std::to_string(SelectedImage->GetID()));
								if (NewEntity != nullptr && NewEntity->HasComponent<FEPointCloudComponent>())
								{
									FEPointCloudComponent& PointCloudComponent = NewEntity->GetComponent<FEPointCloudComponent>();
									FEPointCloud* NewPointCloud = PointCloudComponent.GetPointCloud();
									RESOURCE_MANAGER.ExportFEPointCloudToLAZ(NewPointCloud, "DepthPointCloud_" + std::to_string(SelectedImage->GetID()) + ".laz");
								}
							}
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
										FEEntity* NewEntity = nullptr;
										//CreatePointCloudFromExternalDepthMap(CameraEntity, DepthTexture, LoadedDepthMap, LoadedDepthMap, "ExternalDepthPointCloud_" + std::to_string(SelectedImage->GetID()));
										NewEntity = CreatePointCloudFromExternalDepthMap(CameraEntity, DepthTexture, LoadedDepthMap, LoadedDepthMap, "ExternalDepthPointCloud_" + std::to_string(SelectedImage->GetID()));
										if (NewEntity != nullptr && NewEntity->HasComponent<FEPointCloudComponent>())
										{
											FEPointCloudComponent& PointCloudComponent = NewEntity->GetComponent<FEPointCloudComponent>();
											FEPointCloud* NewPointCloud = PointCloudComponent.GetPointCloud();
											RESOURCE_MANAGER.ExportFEPointCloudToLAZ(NewPointCloud, "ExternalDepthPointCloud_" + std::to_string(SelectedImage->GetID()) + ".laz");
										}
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

glm::vec4 ClearColor = glm::vec4(0.33f, 0.39f, 0.49f, 1.0f);

double MouseX;
double MouseY;

void MouseMoveCallback(double XPos, double YPos)
{
	MouseX = XPos;
	MouseY = YPos;
}

void LoadResource(std::string FileName);

static void DropCallback(int Count, const char** Paths);
void DropCallback(int Count, const char** Paths)
{
	if (UI.IsProgressModalPopupOpen())
		return;

	for (size_t i = 0; i < size_t(Count); i++)
	{
		LoadResource(Paths[i]);
	}
}

void AfterNewResourceLoads(AnalysisObject* NewObject)
{
	if (NewObject == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return;

	ResourceAnalysisData* AnalysisData = NewObject->GetAnalysisData();

	if (!APPLICATION.HasConsoleWindow())
	{
		ActiveEntity->GetComponent<FETransformComponent>().SetPosition(-AnalysisData->GetAABB().GetCenter());
		AnalysisData->Position->SetPosition(-AnalysisData->GetAABB().GetCenter());
	}

	if (!APPLICATION.HasConsoleWindow())
	{
		if (ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() == 1)
			SETTINGS_WINDOW.SetIsModelCamera(true);

		if (NewObject->GetType() == DATA_SOURCE_TYPE::MESH)
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("lightDirection", glm::normalize(ANALYSIS_OBJECT_MANAGER.GetAllMeshObjectsAverageNormal()));
	}

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject != nullptr &&
		ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH &&
		ActiveObject->Layers.empty())
	{
		DataLayer* NewLayer = HEIGHT_LAYER_PRODUCER.Calculate();
		if (NewLayer != nullptr)
			ActiveObject->AddLayer(NewLayer);
	}
}

void LoadResource(std::string FileName)
{
	ANALYSIS_OBJECT_MANAGER.LoadResource(FileName);
}

void MouseButtonCallback(int Button, int Action, int Mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
		return;
	}

	if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_PRESS)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(true);
	}
	else if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_RELEASE)
	{
		MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().SetActive(false);
	}
}

void WindowResizeCallback(int Width, int Height)
{
	SCREENSHOT_MANAGER.RenderTargetWasResized();
}

void AddFontOnSecondFrame()
{
	static bool bFirstTime = true;
	static bool bFontCreated = false;

	if (bFirstTime)
	{
		bFirstTime = false;
	}
	else
	{
		if (!bFontCreated)
		{
			glfwMakeContextCurrent(APPLICATION.GetMainWindow()->GetGlfwWindow());
			ImGui::SetCurrentContext(APPLICATION.GetMainWindow()->GetImGuiContext());

			bFontCreated = true;
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/Cousine-Regular.ttf", 32);
			ImGui::GetIO().Fonts->Build();
		}
	}
}

void ConsoleMainFunction()
{
	// Wait until the console window is created
	bool Success = APPLICATION.HasConsoleWindow();
	while (!Success)
	{
		Success = APPLICATION.HasConsoleWindow();
	}

	// To ensure initialisation of JITTER_MANAGER
	JITTER_MANAGER.GetInstance();

	while (true)
	{
		CONSOLE_JOB_MANAGER.Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (!APPLICATION.IsNotTerminated())
			break;
	}
}

void ConsoleThreadCode(void* InputData)
{
	// To keep console window open
	while (APPLICATION.IsNotTerminated())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

bool MarkTrianglesInRangeForAnnotation(AnalysisObject* Object, DataLayer* Layer, float LowerLevel, float UpperLevel)
{
	if (Object == nullptr || Layer == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(Object->GetID());
	if (CurrentAnnotationData == nullptr)
		return false;

	bool bAtLeastOneTriangleAnnotated = false;
	for (size_t i = 0; i < Layer->ElementsToData.size(); i++)
	{
		float CurrentValue = Layer->ElementsToData[i];
		if (CurrentValue >= LowerLevel && CurrentValue <= UpperLevel)
		{
			CurrentAnnotationData->PerElementID[i] = 1;
			bAtLeastOneTriangleAnnotated = true;
		}
	}

	if (bAtLeastOneTriangleAnnotated)
		ANNOTATION_MANAGER.UpdateBuffer(CurrentAnnotationData);

	return bAtLeastOneTriangleAnnotated;
}

void ApplyHeadLight()
{
	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	if (CameraEntity != nullptr && CameraEntity->HasComponent<FECameraComponent>() && ANALYSIS_OBJECT_MANAGER.CustomMeshShader != nullptr)
	{
		const glm::mat4 ViewMatrix = CameraEntity->GetComponent<FECameraComponent>().GetViewMatrix();
		const glm::vec3 CameraRight = glm::vec3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
		const glm::vec3 CameraUp = glm::vec3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
		const glm::vec3 CameraForward = -glm::vec3(ViewMatrix[0][2], ViewMatrix[1][2], ViewMatrix[2][2]);

		const glm::vec3 LightDirection = glm::normalize(-CameraForward + 0.4f * CameraUp - 0.2f * CameraRight);
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("lightDirection", LightDirection);
	}
}

void MainWindowRender()
{
	static bool FirstFrame = true;

	ApplyHeadLight();

	if (UI_INSPECTOR.ShouldTakeScreenshot())
	{
		ClearColor.w = UI_INSPECTOR.ShouldUseTransparentBackground() ? 0.0f : 1.0f;
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);

		UI_INSPECTOR.SetShouldTakeScreenshot(false);
		SCREENSHOT_MANAGER.TakeScreenshot();
		return;
	}

	JoeInfoProjectionUI();

	//// Arcball orientation gizmo.
	//{
	//	FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
	//	if (CameraEntity != nullptr
	//		&& CameraEntity->HasComponent<FECameraComponent>()
	//		&& CameraEntity->HasComponent<FENativeScriptComponent>())
	//	{
	//		FENativeScriptComponent& NativeScriptComponent = CameraEntity->GetComponent<FENativeScriptComponent>();
	//		float ProbeDistance = 0.0f;
	//		const bool bIsArcBallCamera = NativeScriptComponent.IsInitialized()
	//			&& NativeScriptComponent.GetVariableValue<float>("DistanceToModel", ProbeDistance);

	//		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	//		const FEViewport* Viewport = CameraComponent.GetViewport();
	//		if (bIsArcBallCamera && Viewport != nullptr)
	//		{
	//			const double ViewportWidth = static_cast<double>(Viewport->GetWidth());
	//			const double ViewportHeight = static_cast<double>(Viewport->GetHeight());
	//			const ImVec2 GizmoCenter(
	//				static_cast<float>(Viewport->GetX() + ViewportWidth * 0.5),
	//				static_cast<float>(Viewport->GetY() + ViewportHeight * 0.5));
	//			const float GizmoRadius = static_cast<float>(0.5 * (ViewportWidth < ViewportHeight ? ViewportWidth : ViewportHeight));

	//			const glm::mat4 ViewMatrix = CameraComponent.GetViewMatrix();
	//			const glm::vec3 WorldXInEye = glm::vec3(ViewMatrix[0]);
	//			const glm::vec3 WorldYInEye = glm::vec3(ViewMatrix[1]);
	//			const glm::vec3 WorldZInEye = glm::vec3(ViewMatrix[2]);

	//			ImDrawList* DrawList = ImGui::GetBackgroundDrawList();
	//			DrawList->AddCircle(GizmoCenter, GizmoRadius, IM_COL32(220, 220, 220, 90), 64, 1.0f);

	//			auto DrawGreatCircle = [&](const glm::vec3& BasisOne, const glm::vec3& BasisTwo, ImU32 Color)
	//			{
	//				const int Segments = 72;
	//				ImVec2 Points[72];
	//				for (int i = 0; i < Segments; i++)
	//				{
	//					const float Theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(Segments);
	//					const float CosTheta = cos(Theta);
	//					const float SinTheta = sin(Theta);
	//					const float ScreenX = CosTheta * BasisOne.x + SinTheta * BasisTwo.x;
	//					const float ScreenY = CosTheta * BasisOne.y + SinTheta * BasisTwo.y;
	//					Points[i] = ImVec2(GizmoCenter.x + ScreenX * GizmoRadius, GizmoCenter.y - ScreenY * GizmoRadius);
	//				}
	//				DrawList->AddPolyline(Points, Segments, Color, ImDrawFlags_Closed, 1.5f);
	//			};

	//			DrawGreatCircle(WorldYInEye, WorldZInEye, IM_COL32(255,  90,  90, 200));
	//			DrawGreatCircle(WorldZInEye, WorldXInEye, IM_COL32( 90, 220,  90, 200));
	//			DrawGreatCircle(WorldXInEye, WorldYInEye, IM_COL32(110, 130, 255, 200));
	//		}
	//	}
	//}

	static int AnnotationID = 0;
	ImGui::InputInt("Annotation ID", &VR_MANAGER.AnnotationIDToUse);

	if (ImGui::Button("Read annotations data from GPU memory"))
	{
		ANNOTATION_MANAGER.ReadBackBuffer(ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject()->GetID()));
	}

	bool bGraphDebugWindow = true;
	if (bGraphDebugWindow)
	{
		static float XValue = 0.0f;
		ImGui::InputFloat("x value of data point", &XValue, 0.1f, 1.0f, "%.3f");

		static float YValue = 0.0f;
		ImGui::InputFloat("y value of data point", &YValue, 0.1f, 1.0f, "%.3f");

		static int StackID = 0;
		ImGui::InputInt("stack ID of data point", &StackID);

		if (ImGui::Button("Add data point to a graph"))
		{
			FEGraphDataPoint NewDataPoint;
			NewDataPoint.XValue = XValue;
			NewDataPoint.YValue = YValue;
			NewDataPoint.StackID = StackID;
			UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints({ NewDataPoint });
		}

		ImGui::Separator();

		static std::string FirstChoosenActiveObjectID;
		static int FirstChoosenLayerIndex = -1;
		static std::string SecondChoosenActiveObjectID;
		static int SecondChoosenLayerIndex = -1;

		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		if (ActiveObject != nullptr)
		{
			DataLayer* ActiveLayer = ActiveObject->GetActiveLayer();
			if (ActiveLayer != nullptr)
			{
				ImGui::Separator();
				ImGui::Text("DEBUG OF ANNOTATIONS AND GRAPH:");

				float LayerMin = ActiveLayer->GetMin();
				float LayerMax = ActiveLayer->GetMax();

				static float LowerLevel = 0.0f;
				ImGui::DragFloat("Lower level of metric", &LowerLevel, 0.01f, LayerMin, LayerMax);

				static float UpperLevel = 0.0f;
				ImGui::DragFloat("Upper level of metric", &UpperLevel, 0.01f, LayerMin, LayerMax);

				if (ImGui::Button("Add annotation to triangles in this range"))
				{
					MarkTrianglesInRangeForAnnotation(ActiveObject, ActiveLayer, LowerLevel, UpperLevel);
				}

				ImGui::Separator();
			}
		}
		
		bool bHaveAnyObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectCount() > 0;
		if (bHaveAnyObject)
		{
			AnalysisObject* FirstObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(FirstChoosenActiveObjectID);
			std::string FirstActiveObjectString = "Choose active object";
			if (!FirstChoosenActiveObjectID.empty() && FirstObject != nullptr)
				FirstActiveObjectString = FirstObject->GetName();

			// First layer object combo box
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("First active object: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			if (ImGui::BeginCombo("##ChooseFirstActiveObject", FirstActiveObjectString.c_str(), ImGuiWindowFlags_None))
			{
				std::vector<std::string> AllObjectIDs = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectsIDList();
				for (size_t i = 0; i < AllObjectIDs.size(); i++)
				{
					AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AllObjectIDs[i]);
					if (CurrentObject == nullptr)
						continue;

					bool bIsSelected = (AllObjectIDs[i] == FirstChoosenActiveObjectID);
					if (ImGui::Selectable(CurrentObject->GetName().c_str(), bIsSelected))
					{
						FirstChoosenActiveObjectID = AllObjectIDs[i];
						FirstChoosenLayerIndex = -1;
					}
					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			std::string FirstLayerString = "Choose layer";
			if (FirstChoosenLayerIndex != -1 && FirstObject != nullptr)
				FirstLayerString = FirstObject->Layers[FirstChoosenLayerIndex]->GetCaption();

			// First layer combo box
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("First layer: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			if (FirstObject != nullptr)
			{
				if (ImGui::BeginCombo("##ChooseFirstLayer", FirstLayerString.c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < FirstObject->GetLayerCount(); i++)
					{
						bool bIsSelected = (i == FirstChoosenLayerIndex);
						if (ImGui::Selectable(FirstObject->Layers[i]->GetCaption().c_str(), bIsSelected))
						{
							FirstChoosenLayerIndex = static_cast<int>(i);
						}

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			AnalysisObject* SecondObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(SecondChoosenActiveObjectID);
			std::string SecondActiveObjectString = "Choose active object";
			if (!SecondActiveObjectString.empty() && SecondObject != nullptr)
				SecondActiveObjectString = SecondObject->GetName();

			// Second layer object combo box
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("Second active object: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			if (ImGui::BeginCombo("##ChooseSecondActiveObject", SecondActiveObjectString.c_str(), ImGuiWindowFlags_None))
			{
				std::vector<std::string> AllObjectIDs = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectsIDList();
				for (size_t i = 0; i < AllObjectIDs.size(); i++)
				{
					AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AllObjectIDs[i]);
					if (CurrentObject == nullptr)
						continue;
					bool bIsSelected = (AllObjectIDs[i] == SecondChoosenActiveObjectID);
					if (ImGui::Selectable(CurrentObject->GetName().c_str(), bIsSelected))
					{
						SecondChoosenActiveObjectID = AllObjectIDs[i];
						SecondChoosenLayerIndex = -1;
					}
					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			std::string SecondString = "Choose layer";
			if (SecondChoosenLayerIndex != -1 && SecondObject != nullptr)
				SecondString = SecondObject->Layers[SecondChoosenLayerIndex]->GetCaption();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
			ImGui::Text("Second layer: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(190);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
			if (SecondObject != nullptr)
			{
				if (ImGui::BeginCombo("##ChooseSecondLayer", SecondString.c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < SecondObject->Layers.size(); i++)
					{
						bool bIsSelected = (i == SecondChoosenLayerIndex);
						if (ImGui::Selectable(SecondObject->Layers[i]->GetCaption().c_str(), bIsSelected))
						{
							SecondChoosenLayerIndex = static_cast<int>(i);
						}

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			if (ImGui::Button("Update histogram data"))
			{
				if (FirstChoosenLayerIndex != -1 && SecondChoosenLayerIndex != -1 &&
					FirstObject != nullptr && SecondObject != nullptr)
				{
					UI.GetHistogramPointer()->Clear();

					DataLayer* FirstLayer = FirstObject->Layers[FirstChoosenLayerIndex];
					DataLayer* SecondLayer = SecondObject->Layers[SecondChoosenLayerIndex];

					if (FirstLayer->ValueWeightAndIndex.empty() || SecondLayer->ValueWeightAndIndex.empty())
						return;

					std::vector<double> Values;
					std::vector<double> Weights;
					int BinsCount = 128;

					for (const auto& Tuple : FirstLayer->ValueWeightAndIndex)
					{
						Values.push_back(std::get<0>(Tuple));
						Weights.push_back(std::get<1>(Tuple));
					}

					std::vector<FEGraphDataPoint> FirstGraphDataPoints = UI.GetHistogramPointer()->ConvertToDataPoints(Values, Weights, BinsCount);
					Values.clear();
					Weights.clear();

					for (const auto& Tuple : SecondLayer->ValueWeightAndIndex)
					{
						Values.push_back(std::get<0>(Tuple));
						Weights.push_back(std::get<1>(Tuple));
					}

					std::vector<FEGraphDataPoint> SecondGraphDataPoints = UI.GetHistogramPointer()->ConvertToDataPoints(Values, Weights, BinsCount);
					for (size_t i = 0; i < SecondGraphDataPoints.size(); i++)
					{
						SecondGraphDataPoints[i].StackID = 1;
					}

					UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(FirstGraphDataPoints);
					UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(SecondGraphDataPoints);
				}
			}

			if (ImGui::Button("Add Annotations to graph"))
			{
				UI.GetHistogramPointer()->Clear();

				AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
				if (ActiveObject == nullptr)
					return;

				DataLayer* ActiveLayerData = ActiveObject->GetActiveLayer();
				AnnotationData* AnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(ActiveObject->GetID());
				if (AnnotationData != nullptr && ActiveLayerData != nullptr)
				{
					std::vector<std::tuple<double, double, int>> NoAnnotationHistogramData;

					if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
					{
						MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
						if (CurrentMeshAnalysisData == nullptr)
							return;

						FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
						if (ActiveMesh == nullptr)
							return;

						for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
						{
							if (AnnotationData->PerElementID[i] == -1)
							{
								double CurrentLayerTriangleValue = ActiveLayerData->ElementsToData[i];
								NoAnnotationHistogramData.push_back(std::make_tuple(CurrentLayerTriangleValue, CurrentMeshAnalysisData->TrianglesArea[i], i));
							}
						}
					}
					else if (ActiveObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
					{
						size_t ElementCount = std::min(AnnotationData->PerElementID.size(), ActiveLayerData->ElementsToData.size());
						for (size_t i = 0; i < ElementCount; i++)
						{
							if (AnnotationData->PerElementID[i] == -1)
							{
								double CurrentLayerPointValue = ActiveLayerData->ElementsToData[i];
								// For point clouds, each point has weight equal to 1.0.
								NoAnnotationHistogramData.push_back(std::make_tuple(CurrentLayerPointValue, 1.0, static_cast<int>(i)));
							}
						}
					}
					else
					{
						return;
					}

					std::vector<double> Values;
					std::vector<double> Weights;
					int BinsCount = 128;

					for (const auto& Tuple : NoAnnotationHistogramData)
					{
						Values.push_back(std::get<0>(Tuple));
						Weights.push_back(std::get<1>(Tuple));
					}

					std::vector<FEGraphDataPoint> NoAnnotationGraphDataPoints = UI.GetHistogramPointer()->ConvertToDataPoints(Values, Weights,
																															  BinsCount, ActiveLayerData->GetMin(), ActiveLayerData->GetMax());
					UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(NoAnnotationGraphDataPoints);

					std::vector<AnnotationInfo> AllAnnotationInfo = AnnotationData->GetAllAnnotationInfos();
					std::vector<std::vector<FEGraphDataPoint>> AnnotationGraphDataPoints;
					std::unordered_map<int, int> AnnotationIDToGraphStackIndex;
					int GraphStackIndex = 1;
					for (size_t i = 0; i < AllAnnotationInfo.size(); i++)
					{
						if (AllAnnotationInfo[i].HistogramData.empty())
							continue;

						Values.clear();
						Weights.clear();
						for (const auto& Tuple : AllAnnotationInfo[i].HistogramData)
						{
							Values.push_back(std::get<0>(Tuple));
							Weights.push_back(std::get<1>(Tuple));
						}

						std::vector<FEGraphDataPoint> CurrentAnnotationGraphDataPoints = UI.GetHistogramPointer()->ConvertToDataPoints(Values, Weights,
																																	   BinsCount, ActiveLayerData->GetMin(), ActiveLayerData->GetMax());
						for (size_t j = 0; j < CurrentAnnotationGraphDataPoints.size(); j++)
							CurrentAnnotationGraphDataPoints[j].StackID = GraphStackIndex;

						if (!CurrentAnnotationGraphDataPoints.empty())
						{
							AnnotationIDToGraphStackIndex[AllAnnotationInfo[i].ID] = GraphStackIndex;
							AllAnnotationInfo[i].StackGraphIndex = GraphStackIndex;
							GraphStackIndex++;
						}
						else
						{
							AllAnnotationInfo[i].StackGraphIndex = -1;
						}

						AnnotationGraphDataPoints.push_back(CurrentAnnotationGraphDataPoints);
					}

					for (size_t i = 0; i < AnnotationGraphDataPoints.size(); i++)
					{
						UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(AnnotationGraphDataPoints[i]);
					}


					/*for (size_t i = 0; i < AnnotationGraphDataPoints.size(); i++)
					{
						int CurrentAnnotationID = AllAnnotationInfo[i].ID;
						if (AnnotationIDToGraphStackIndex.find(CurrentAnnotationID) != AnnotationIDToGraphStackIndex.end())
						{
							int CurrentGraphStackIndex = AnnotationIDToGraphStackIndex[CurrentAnnotationID];
							FEGraphStackInfo* CurrentStackInfo = UI.GetHistogramPointer()->GetGraphPointer()->GetStackInfoByID(CurrentGraphStackIndex);
							if (CurrentStackInfo != nullptr)
							{
								glm::vec4 Color = AllAnnotationInfo[i].GetColor();
								CurrentStackInfo->StartGradientColor = ImColor(Color.x, Color.y, Color.z);
								CurrentStackInfo->EndGradientColor = ImColor(Color.x, Color.y, Color.z);
							}
						}
					}*/

					for (size_t i = 0; i < AllAnnotationInfo.size(); i++)
					{
						int StackGraphIndex = AllAnnotationInfo[i].StackGraphIndex;
						if (StackGraphIndex != -1)
						{
							FEGraphStackInfo* CurrentStackInfo = UI.GetHistogramPointer()->GetGraphPointer()->GetStackInfoByID(StackGraphIndex);
							if (CurrentStackInfo != nullptr)
							{
								glm::vec4 Color = AllAnnotationInfo[i].GetColor();
								CurrentStackInfo->StartGradientColor = ImColor(Color.x, Color.y, Color.z);
								CurrentStackInfo->EndGradientColor = ImColor(Color.x, Color.y, Color.z);
							}
						}


						/*int CurrentAnnotationID = AllAnnotationInfo[i].ID;
						if (AnnotationIDToGraphStackIndex.find(CurrentAnnotationID) != AnnotationIDToGraphStackIndex.end())
						{
							int CurrentGraphStackIndex = AnnotationIDToGraphStackIndex[CurrentAnnotationID];
							FEGraphStackInfo* CurrentStackInfo = UI.GetHistogramPointer()->GetGraphPointer()->GetStackInfoByID(CurrentGraphStackIndex);
							if (CurrentStackInfo != nullptr)
							{
								glm::vec4 Color = AllAnnotationInfo[i].GetColor();
								CurrentStackInfo->StartGradientColor = ImColor(Color.x, Color.y, Color.z);
								CurrentStackInfo->EndGradientColor = ImColor(Color.x, Color.y, Color.z);
							}
						}*/
					}
					
					



				}

			}

			// Here I should have input for reordering stacks
			// It should accept string like "2,0,1" where each number is stack index
			static char OrderInputBuffer[1024] = "";
			ImGui::InputText("Stack Indices (e.g. 2,0,1)", OrderInputBuffer, sizeof(OrderInputBuffer));

			if (ImGui::Button("Reorder Stacks"))
			{
				std::vector<int> NewOrder;

				std::string StringRepresentation(OrderInputBuffer);
				std::stringstream StringStream(StringRepresentation);
				std::string Token;

				while (std::getline(StringStream, Token, ','))
				{
					try {
						// std::stoi converts string to int and handles whitespace automatically
						NewOrder.push_back(std::stoi(Token));
					}
					catch (...) {
						// Catch invalid inputs (like letters or empty strings) so the app doesn't crash
					}
				}

				UI.GetHistogramPointer()->GetGraphPointer()->ChangeStackOrder(NewOrder);
			}
		}

		ImGui::Separator();
	}

	bool bVRMode = ENGINE.IsVREnabled();
	if (ImGui::Checkbox("Enter VR mode", &bVRMode))
	{
		if (bVRMode)
		{
			if (ENGINE.EnableVR(FERenderingPipeline::Forward_Simplified))
			{
				VR_MANAGER.Initialize();
			}
		}
		else
		{
			ENGINE.DisableVR();
		}
	}

	if (bVRMode)
	{
		VR_MANAGER.Update();
		
		/*std::pair<glm::vec3, glm::vec3> EyeGazeData = FEOpenXR_INPUT.GetEyeGazeOriginAndDirection();
		float RayLength = 10.0f;
		glm::vec3 OutEnd = EyeGazeData.first + EyeGazeData.second * RayLength;
		RENDERER.DebugLineCounter = 0;
		RENDERER.DebugDrawLine(EyeGazeData.first, OutEnd, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);*/
		
		FEEntity* VRRigEntity = OpenXR_MANAGER.GetVRRigEntity();
		if (VRRigEntity != nullptr)
		{
			FETransformComponent& VRRigTransform = VRRigEntity->GetComponent<FETransformComponent>();
			glm::vec3 VRRigPosition = VRRigTransform.GetPosition();

			ImGui::Text("VRRig Position : ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##X Left Controller", &VRRigPosition[0], 0.01f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##Y Left Controller", &VRRigPosition[1], 0.01f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##Z Left Controller", &VRRigPosition[2], 0.01f);

			VRRigTransform.SetPosition(VRRigPosition);
		}
	}

	UI.Render();

	if (FirstFrame)
	{
		FirstFrame = false;
		UI.ApplyStandardWindowsSizeAndPosition();
	}
}

GLFWimage ConvertIconToGLFWImage(HICON Icon)
{
	ICONINFO IconInfo;
	GetIconInfo(Icon, &IconInfo);
	BITMAP BMP;
	GetObject(IconInfo.hbmColor, sizeof(BITMAP), &BMP);

	GLFWimage Result;
	Result.width = BMP.bmWidth;
	Result.height = BMP.bmHeight;

	int BytesPerPixel = BMP.bmBitsPixel / 8;
	int Size = Result.width * Result.height * 4;
	Result.pixels = new unsigned char[Size];

	// Get the bits from the bitmap and store them in the GLFWimage
	GetBitmapBits(IconInfo.hbmColor, Size, Result.pixels);

	// Convert BGR to RGB
	for (int i = 0; i < Size; i += 4)
	{
		std::swap(Result.pixels[i], Result.pixels[i + 2]); // Swap B and R
	}

	// Clean up
	DeleteObject(IconInfo.hbmColor);
	DeleteObject(IconInfo.hbmMask);

	return Result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//LOG.SetFileOutput(true);
	GDALAllRegister();

	const auto ProcessorCount = THREAD_POOL.GetLogicalCoreCount();
	const unsigned int HowManyToUse = ProcessorCount > 4 ? ProcessorCount - 2 : 1;

	THREAD_POOL.SetConcurrentThreadCount(HowManyToUse);

	bool bIsConsoleModeRequested = false;
	std::vector<CommandLineAction> ParsedCommandActions;

	ParsedCommandActions = APPLICATION.ParseCommandLine(lpCmdLine);
	if (!ParsedCommandActions.empty())
		std::transform(ParsedCommandActions[0].Action.begin(), ParsedCommandActions[0].Action.end(), ParsedCommandActions[0].Action.begin(), [](unsigned char c) { return std::tolower(c); });

	if (!ParsedCommandActions.empty() && ParsedCommandActions[0].Action == "console")
	{
		bIsConsoleModeRequested = true;
		ParsedCommandActions.erase(ParsedCommandActions.begin());
	}

	if (bIsConsoleModeRequested)
	{
		FEConsoleWindow* Console = APPLICATION.CreateConsoleWindow(ConsoleThreadCode);
		Console->WaitForCreation();
		Console->SetTitle("HabiCAT3D console");

		bool bRunTestsRequested = false;
		if (!ParsedCommandActions.empty())
		{
			std::string FirstAction = ParsedCommandActions[0].Action;
			std::transform(FirstAction.begin(), FirstAction.end(), FirstAction.begin(), [](unsigned char c) { return std::tolower(c); });
			if (FirstAction == "run_tests")
			{
				bRunTestsRequested = true;
				ParsedCommandActions.erase(ParsedCommandActions.begin());
			}
		}

		if (bRunTestsRequested)
		{
			testing::GTEST_FLAG(output) = "xml:HabiCAT3D_Tests.xml";
			int FakeArgc = 1;
			char FakeArgv0[] = "HabiCAT3D";
			char* FakeArgv[] = { FakeArgv0, nullptr };
			testing::InitGoogleTest(&FakeArgc, FakeArgv);

			std::cout << "Running HabiCAT3D test suite..." << std::endl;
			int TestResult = RUN_ALL_TESTS();
			std::cout << std::endl << "Test suite finished with exit code " << TestResult << "." << std::endl;
			std::cout << "Close the console window to exit." << std::endl;

			while (APPLICATION.IsNotTerminated())
			{
				APPLICATION.BeginFrame();
				APPLICATION.RenderWindows();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				APPLICATION.EndFrame();
			}

			return TestResult;
		}

		std::vector<ConsoleJob*> ParsedJobs = CONSOLE_JOB_MANAGER.ConvertCommandAction(ParsedCommandActions);
		for (size_t i = 0; i < ParsedJobs.size(); i++)
		{
			CONSOLE_JOB_MANAGER.AddJob(ParsedJobs[i]);
		}

		while (APPLICATION.IsNotTerminated())
		{
			APPLICATION.BeginFrame();

			ConsoleMainFunction();
			APPLICATION.RenderWindows();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			APPLICATION.EndFrame();
		}
	}
	else
	{
		ENGINE.InitWindow(1280, 720, "HabiCAT3D");
		// If I will directly assign result of APPLICATION.AddWindow to UI.MainWindow, then in Release build with full optimization app will crash, because of execution order.
		FEWindow* MainWindow = APPLICATION.GetMainWindow();

		GLFWimage Icon = ConvertIconToGLFWImage(LoadIcon(hInstance, MAKEINTRESOURCE(101)));
		glfwSetWindowIcon(MainWindow->GetGlfwWindow(), 1, &Icon);

		APPLICATION.GetMainWindow()->SetRenderFunction(MainWindowRender);
		APPLICATION.GetMainWindow()->AddOnDropCallback(DropCallback);
		APPLICATION.GetMainWindow()->AddOnMouseMoveCallback(MouseMoveCallback);
		APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(MouseButtonCallback);
		APPLICATION.GetMainWindow()->AddOnResizeCallback(WindowResizeCallback);

		FEEntity* CameraEntity = MAIN_SCENE_MANAGER.GetMainCamera();
		CAMERA_SYSTEM.SetCameraRenderingPipeline(CameraEntity, FERenderingPipeline::Forward_Simplified);
		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		CameraComponent.SetClearColor(glm::vec4(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w));
		CameraComponent.SetNearPlane(0.1f);
		CameraComponent.SetActive(false);

		ANALYSIS_OBJECT_MANAGER.AddOnObjectLoadCallback(AfterNewResourceLoads);

		SCREENSHOT_MANAGER.Init();
		DEVELOPER_MODE.Initialize();
		ANNOTATION_MANAGER.Initialize();

		//LoadingJoeInfoProjection();

		while (ENGINE.IsNotTerminated())
		{
			AddFontOnSecondFrame();

			ENGINE.BeginFrame();
			ENGINE.Render();
			ENGINE.EndFrame();
		}
	}

	return 0;
}