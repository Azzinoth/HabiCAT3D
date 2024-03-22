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
	// TODO: Implement this
	return true;
}