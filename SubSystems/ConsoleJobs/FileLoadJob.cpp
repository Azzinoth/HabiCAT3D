#include "FileLoadJob.h"
using namespace FocalEngine;

FileLoadJob::FileLoadJob(std::string FilePath)
{
	this->FilePath = FilePath;
	Type = "FILE_LOAD";
}

FileLoadJob* FileLoadJob::CreateInstance(CommandLineAction ActionToParse)
{
	FileLoadJob* Result = nullptr;

	if (ActionToParse.Settings.find("filepath") == ActionToParse.Settings.end())
		return Result;

	if (ActionToParse.Settings["filepath"].empty())
		return Result;
	
	Result = new FileLoadJob(ActionToParse.Settings["filepath"]);

	return Result;
}

ConsoleJobInfo FileLoadJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "file_load";
	Info.Purpose = "Loads a file from the specified path.";
	ConsoleJobSettingsInfo CurrentSettingInfo;
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "The path of the file to load.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	return Info;
}

bool FileLoadJob::Execute(void* InputData, void* OutputData)
{
	std::cout << "Initiating file load process for: " << FilePath << std::endl;

	if (!FILE_SYSTEM.DoesFileExist(FilePath.c_str()))
		return false;

	std::cout << "File found. Loading file: " << FilePath << std::endl;

	COMPLEXITY_METRIC_MANAGER.ImportOBJ(FilePath.c_str(), true);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->UpdateAverageNormal();

	OutputConsoleTextWithColor("Successfully completed loading file: ", 0, 255, 0);
	OutputConsoleTextWithColor(FilePath, 0, 255, 0);

	LAYER_RASTERIZATION_MANAGER.ClearAllData();
	float ResolutionInMeters = LAYER_RASTERIZATION_MANAGER.GetResolutionInMetersBasedOnResolutionInPixels(512);
	if (ResolutionInMeters > 0.0f)
		LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(ResolutionInMeters);

	return true;
}