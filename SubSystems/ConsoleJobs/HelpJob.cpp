#include "HelpJob.h"
using namespace FocalEngine;

std::map<std::string, ConsoleJobInfo>* HelpJob::ConsoleJobsInfo = nullptr;

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

void HelpJob::PrintCommandHelp(std::string CommandName)
{
	std::transform(CommandName.begin(), CommandName.end(), CommandName.begin(), [](unsigned char c) { return std::tolower(c); });

	std::cout << "\n\n";

	ConsoleJobInfo* Info = nullptr;
	if ((*ConsoleJobsInfo).find(CommandName) != (*ConsoleJobsInfo).end())
		Info = &(*ConsoleJobsInfo)[CommandName];

	if (Info == nullptr)
	{
		std::cout << "No help available for this command." << std::endl;
		return;
	}

	std::cout << "Help for '" << Info->CommandName << "' command:\n";
	std::cout << "Purpose:\n  " << Info->Purpose << "\n\n";
	std::cout << "Settings:\n";

	for (const auto& SettingInfo : Info->SettingsInfo)
	{
		std::cout << "  - " << SettingInfo.Name << " (" << (SettingInfo.bIsOptional ? "Optional" : "Required") << "): " << SettingInfo.Description << "\n";
		if (!SettingInfo.PossibleValues.empty())
		{
			std::cout << "      Possible Values: ";
			for (const auto& value : SettingInfo.PossibleValues)
			{
				std::cout << "'" << value << "', ";
			}
			std::cout << "\b\b \n"; // Removes the last comma and space
		}

		if (SettingInfo.bIsOptional)
		{
			std::cout << "      Default: " << SettingInfo.DefaultValue << "\n";
		}

		std::cout << "\n";
	}
}

bool HelpJob::Execute(void* InputData, void* OutputData)
{
	if (CommandName.empty())
	{
		std::cout <<
			"Command signature:\n"
			"  -[COMMAND] [OPTIONS]=[VALUE]\n\n"

			"Commands:\n"
			"-help                                           Display this help message.\n"
			"-help command_name=[COMMAND]                    Display help for a specific command along with all its settings.\n"
			"-file_load filepath=[PATH]                      Load a file from the specified path.\n"
			"-file_save filepath=[PATH]                      Save the current state to a file at the specified path.\n"
			"-run_script_file filepath=[PATH]                Execute a sequence of commands from a specified script(text) file.Each command in the file should be on a new line.\n"
			"-complexity type=[LAYER_TYPE]                   Create a complexity job with the specified settings to create a layer.\n"
			"-evaluation type=[TYPE] subtype=[WHAT_TO_TEST]  Create an evaluation job with the specified settings to test a layer or other objects.\n"
			"-global_settings type=[TYPE]                    Set a global setting for the application.\n"
			"-export_layer_as_image export_mode=[MODE]       Export a layer as an image.\n\n"

			"Examples:\n"
			"-load filepath=\"C:/data/mesh.obj\"\n"
			"-save filepath=\"C:/data/processed_mesh.rug\"\n"
			"-complexity type=RUGOSITY rugosity_algorithm = MIN jitter_quality=73\n"
			"-evaluation type=COMPLEXITY subtype=MAX_LAYER_VALUE expected_value=5.02 tolerance=0.01\n\n"

			"Notes:\n"
			"For Boolean settings, use true or false.\n"
			"Paths must be enclosed in quotes.\n\n";
	}
	else
	{
		PrintCommandHelp(CommandName);
	}

	return true;
}