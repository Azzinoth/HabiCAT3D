#include "FileSaveJob.h"
using namespace FocalEngine;

FileSaveJob::FileSaveJob(std::string FilePath)
{
	this->FilePath = FilePath;
	Type = "FILE_SAVE";
}

FileSaveJob* FileSaveJob::CreateInstance(CommandLineAction ActionToParse)
{
	FileSaveJob* Result = nullptr;

	if (ActionToParse.Settings.find("filepath") == ActionToParse.Settings.end())
		return Result;

	if (ActionToParse.Settings["filepath"].empty())
		return Result;

	Result = new FileSaveJob(ActionToParse.Settings["filepath"]);

	return Result;
}

ConsoleJobInfo FileSaveJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "file_save";
	Info.Purpose = "Saves the current state to a file at the specified path.";
	ConsoleJobSettingsInfo CurrentSettingInfo;
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "The path of the file to save.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	return Info;
}

bool FileSaveJob::Execute(void* InputData, void* OutputData)
{
	ANALYSIS_OBJECT_MANAGER.SaveToRUGFile(FilePath);
	OutputConsoleTextWithColor("File saved successfully.", 0, 255, 0);

	return true;
}