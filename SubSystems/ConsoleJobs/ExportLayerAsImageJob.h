#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ExportLayerAsImageJob : public ConsoleJob
{
	friend ConsoleJobManager;

	LayerRasterizationManager::GridRasterizationMode ExportMode;
	LayerRasterizationManager::SaveMode SaveMode;

	std::string FilePath = "";
	float ResolutionInM = 0.0f;
	glm::vec3 ForceProjectionVector = glm::vec3(0.0f);
	float PersentOfAreaThatWouldBeRed = 5.0f;

	int LayerIndex = -1;
public:
	ExportLayerAsImageJob();

	LayerRasterizationManager::GridRasterizationMode GetExportMode();
	void SetExportMode(LayerRasterizationManager::GridRasterizationMode NewValue);

	LayerRasterizationManager::SaveMode GetSaveMode();
	void SetSaveMode(LayerRasterizationManager::SaveMode NewValue);

	std::string GetFilePath();
	void SetFilePath(std::string NewValue);

	float GetResolutionInM();
	void SetResolutionInM(float NewValue);

	glm::vec3 GetForceProjectionVector();
	void SetForceProjectionVector(glm::vec3 NewValue);

	float GetPersentOfAreaThatWouldBeRed();
	void SetPersentOfAreaThatWouldBeRed(float NewValue);

	int GetLayerIndex();
	void SetLayerIndex(int NewValue);
};