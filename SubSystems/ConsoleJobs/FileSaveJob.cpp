#include "FileSaveJob.h"
using namespace FocalEngine;

FileSaveJob::FileSaveJob(std::string FilePath)
{
	this->FilePath = FilePath;
	Type = "FILE_SAVE";
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