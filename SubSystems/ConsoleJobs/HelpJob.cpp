#include "HelpJob.h"
using namespace FocalEngine;

HelpJob::HelpJob(std::string CommandName)
{
	this->CommandName = CommandName;
	Type = "HELP_JOB";
}

HelpJob* HelpJob::CreateInstance(CommandLineAction ActionToParse)
{
	HelpJob* Result = nullptr;

	std::string CommandName;
	if (ActionToParse.Settings.find("command_name") != ActionToParse.Settings.end())
		CommandName = ActionToParse.Settings["command_name"];

	Result = new HelpJob(CommandName);
	return Result;
}

std::string HelpJob::GetCommandName()
{
	return CommandName;
}

ConsoleJobInfo HelpJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "help";
	Info.Purpose = "Prints help for all commands or for a specific command.";

	ConsoleJobSettingsInfo CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "command_name";
	CurrentSettingInfo.Description = "The name of the command to print help for.";
	CurrentSettingInfo.bIsOptional = true;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	return Info;
}

bool HelpJob::Execute(void* InputData, void* OutputData)
{
	// TODO: Implement this
	return true;
}