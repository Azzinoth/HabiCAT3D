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
	int ResolutionInPixels = 0;
	glm::vec3 ForceProjectionVector = glm::vec3(0.0f);
	float PercentOfAreaThatWouldBeRed = 5.0f;

	int LayerIndex = -1;

	static ConsoleJobInfo GetInfo();
	bool Execute(void* InputData = nullptr, void* OutputData = nullptr);
public:
	ExportLayerAsImageJob();
	static ExportLayerAsImageJob* CreateInstance(CommandLineAction ActionToParse);

	LayerRasterizationManager::GridRasterizationMode GetExportMode();
	void SetExportMode(LayerRasterizationManager::GridRasterizationMode NewValue);

	LayerRasterizationManager::SaveMode GetSaveMode();
	void SetSaveMode(LayerRasterizationManager::SaveMode NewValue);

	std::string GetFilePath();
	void SetFilePath(std::string NewValue);

	float GetResolutionInM();
	void SetResolutionInM(float NewValue);

	int GetResolutionInPixels();
	void SetResolutionInPixels(int NewValue);

	glm::vec3 GetForceProjectionVector();
	void SetForceProjectionVector(glm::vec3 NewValue);

	float GetPercentOfAreaThatWouldBeRed();
	void SetPercentOfAreaThatWouldBeRed(float NewValue);

	int GetLayerIndex();
	void SetLayerIndex(int NewValue);
};