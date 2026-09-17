#pragma once

#include "../SubSystems/VolumetricReconstruction/VolumeTransferFunctionWidget.h"

class VolumetricReconstruction
{
public:
	SINGLETON_PUBLIC_PART(VolumetricReconstruction)

	glm::vec4 GetPixelColor(const unsigned char* RawData, int X, int Y, int Width, GLint InternalFormat);
	glm::vec4 GetValueOfDepth(int X, int Y);
	float ConvertDepthToWorldDistance(float DepthValue, float NearPlaneValue, float FarPlaneValue);
	bool GetWorldPositionFromCameraDepthBuffer(FEEntity* CameraEntity, int PixelX, int PixelY, int ImageWidth, int ImageHeight, glm::dvec3& OutWorldPosition, double& OutDistanceAlongRay);

	// FE_FIX_ME: Temporary function, jittered point cloud density should be used instead.
	FETexture* Create3DTextureFromPointCloud(FEPointCloud* PointCloud, FEAABB NewVolumeAABB, size_t Dimension, std::string Name);

	FEEntity* CreatePointCloudFromExternalDepthMap(FEEntity* CameraEntity, FETexture* CameraDepthBuffer, FETexture* ExternalDepthMap, FETexture* ColorTexture, std::string Name);

	void LoadNeuralNetworkDepthReconstructionInputs(const std::string& FileName);

	void RenderUI();
private:
	SINGLETON_PRIVATE_PART(VolumetricReconstruction)

	std::string MasksFolder;
	std::string DepthReconstructionFolder;

	FETexture* NeuralNetworkDepthMap = nullptr;
	unsigned char* NeuralNetworkDepthMapRawData = nullptr;

	FEEntity* LastCreatedEntity = nullptr;
	bool bAccumulatePoints = true;
	FEEntity* VolumetricEntity = nullptr;

	const int VolumeTextureResolution = 1024;

	FETexture* MaskOfIgnoredPixelsTexture = nullptr;
};

#define VOLUMETRIC_RECONSTRUCTION VolumetricReconstruction::GetInstance()