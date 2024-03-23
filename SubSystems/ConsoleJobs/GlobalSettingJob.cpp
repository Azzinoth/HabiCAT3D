#include "GlobalSettingJob.h"
using namespace FocalEngine;

GlobalSettingJob::GlobalSettingJob()
{
	Type = "GLOBAL_SETTINGS_JOB";
	GlobalSettingType = "EVALUATION_JOB_TO_SCRIPT";
}

GlobalSettingJob* GlobalSettingJob::CreateInstance(CommandLineAction ActionToParse)
{
	GlobalSettingJob* Result = nullptr;

	if (ActionToParse.Settings.find("type") == ActionToParse.Settings.end())
		return Result;

	auto Iterator = ActionToParse.Settings.begin();
	while (Iterator != ActionToParse.Settings.end())
	{
		std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
		Iterator++;
	}

	Result = new GlobalSettingJob();
	Result->SetGlobalSettingType(ActionToParse.Settings["type"]);

	if (ActionToParse.Settings.find("int_value") != ActionToParse.Settings.end())
	{
		Result->SetIntValue(std::stoi(ActionToParse.Settings["int_value"]));
	}

	if (ActionToParse.Settings.find("float_value") != ActionToParse.Settings.end())
	{
		Result->SetFloatValue(std::stof(ActionToParse.Settings["float_value"]));
	}

	if (ActionToParse.Settings.find("bool_value") != ActionToParse.Settings.end())
	{
		Result->SetBoolValue(ActionToParse.Settings["bool_value"] == "TRUE" ? true : false);
	}

	return Result;
}

ConsoleJobInfo GlobalSettingJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "global_settings";
	Info.Purpose = "Sets a global setting for the application.";
	ConsoleJobSettingsInfo CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "type";
	CurrentSettingInfo.Description = "Specifies the type of global setting.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "EVALUATION_JOB_TO_SCRIPT" };
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "int_value";
	CurrentSettingInfo.Description = "Specifies the integer value for the global setting.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "0";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "float_value";
	CurrentSettingInfo.Description = "Specifies the float value for the global setting.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "0.0";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "bool_value";
	CurrentSettingInfo.Description = "Specifies the boolean value for the global setting.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	return Info;
}

std::string GlobalSettingJob::GetGlobalSettingType()
{
	return GlobalSettingType;
}

void GlobalSettingJob::SetGlobalSettingType(std::string NewValue)
{
	GlobalSettingType = NewValue;
}

int GlobalSettingJob::GetIntValue()
{
	return IntValue;
}

void GlobalSettingJob::SetIntValue(int NewValue)
{
	IntValue = NewValue;
}

float GlobalSettingJob::GetFloatValue()
{
	return FloatValue;
}

void GlobalSettingJob::SetFloatValue(float NewValue)
{
	FloatValue = NewValue;
}

bool GlobalSettingJob::GetBoolValue()
{
	return bValue;
}

void GlobalSettingJob::SetBoolValue(bool NewValue)
{
	bValue = NewValue;
}

bool GlobalSettingJob::Execute(void* InputData, void* OutputData)
{
	if (InputData != nullptr)
	{
		if (GlobalSettingType == "EVALUATION_JOB_TO_SCRIPT")
			*reinterpret_cast<bool*>(InputData) = GetBoolValue();
		
		return true;
	}

	return false;
}