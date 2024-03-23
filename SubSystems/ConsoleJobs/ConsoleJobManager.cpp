#include "ConsoleJobManager.h"
using namespace FocalEngine;

ConsoleJobManager* ConsoleJobManager::Instance = nullptr;

ConsoleJobManager::ConsoleJobManager()
{
	ConsoleJobSettingsInfo CurrentSettingInfo;

	ConsoleJobsInfo["help"] = HelpJob::GetInfo();
	ConsoleJobsInfo["file_load"] = FileLoadJob::GetInfo();
	ConsoleJobsInfo["file_save"] = FileSaveJob::GetInfo();

	ConsoleJobsInfo["run_script_file"].CommandName = "run_script_file";
	ConsoleJobsInfo["run_script_file"].Purpose = "Executes a sequence of commands from a specified script(text) file. Each command in the file should be on a new line.";
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "The path of the script file to execute.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["run_script_file"].SettingsInfo.push_back(CurrentSettingInfo);

	ConsoleJobsInfo["complexity"] = ComplexityJob::GetInfo();
	ConsoleJobsInfo["evaluation"] = ComplexityEvaluationJob::GetInfo();
	ConsoleJobsInfo["global_settings"] = GlobalSettingJob::GetInfo();
	ConsoleJobsInfo["export_layer_as_image"] = ExportLayerAsImageJob::GetInfo();

	HelpJob::ConsoleJobsInfo = &ConsoleJobsInfo;
}

ConsoleJobManager::~ConsoleJobManager() {}

void ConsoleJobManager::AddJob(ConsoleJob* Job)
{
	JobsList.push_back(Job);
	JobsAdded++;
}

void ConsoleJobManager::ExecuteJob(ConsoleJob* Job)
{
	void* InputData = nullptr;
	if (Job->Type == "EVALUATION_JOB" && bConvertEvaluationToUsableScript)
		InputData = (void*)(&SavedConvertionsOfEvaluationToUsableScript);

	if (Job->Type == "GLOBAL_SETTINGS_JOB")
		InputData = (void*)(&bConvertEvaluationToUsableScript);

	if (Job->Execute(InputData, nullptr))
	{
		if (Job->Type == "EVALUATION_JOB")
		{
			EvaluationsTotalCount++;

			if (reinterpret_cast<EvaluationJob*>(Job)->Failed())
				EvaluationsFailedCount++;
		}
	}
	else
	{
		if (Job->Type == "FILE_LOAD")
		{
			FileLoadJob* FileJob = reinterpret_cast<FileLoadJob*>(Job);

			std::string ErrorMessage = "Error: File not found - " + FileJob->FilePath + ". Please check the file path and try again.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			ConsoleJob::OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);

			JobsList.clear();
		}
		else if (Job->Type == "EVALUATION_JOB")
		{
			EvaluationsTotalCount++;
			EvaluationsFailedCount++;
		}
	}
}

void ConsoleJobManager::Update()
{
	if (JobsList.empty() && JobsAdded != 0)
	{
		OnAllJobsFinished();
		JobsAdded = 0;
	}
		
	if (!JobsList.empty())
	{
		ConsoleJob* CurrentJob = JobsList.front();
		if (CurrentJob != nullptr)
		{
			ExecuteJob(CurrentJob);

			if (!APPLICATION.IsNotTerminated())
			{
				JobsList.clear();
				return;
			}

			if (JobsList.empty())
				return;

			delete CurrentJob;
			JobsList.erase(JobsList.begin());
		}
	}
	else
	{
		std::string NewCommand;
		std::cout << "Enter command: ";
		std::getline(std::cin, NewCommand);
		std::vector<CommandLineAction> Actions = APPLICATION.ParseCommandLine(NewCommand);
		std::vector<ConsoleJob*> NewJobs = ConvertCommandAction(Actions);
		for (size_t i = 0; i < NewJobs.size(); i++)
		{
			AddJob(NewJobs[i]);
		}
	}
}

void ConsoleJobManager::OnAllJobsFinished()
{
	if (EvaluationsTotalCount > 1)
	{
		std::cout << "All jobs finished." << std::endl;

		if (EvaluationsFailedCount == 0)
		{
			ConsoleJob::OutputConsoleTextWithColor("All evaluations passed: " + std::to_string(EvaluationsTotalCount) + " out of " + std::to_string(EvaluationsTotalCount), 0, 255, 0);
		}
		else if (EvaluationsFailedCount > 0 && EvaluationsFailedCount < EvaluationsTotalCount)
		{
			ConsoleJob::OutputConsoleTextWithColor("Some evaluations failed, only: " + std::to_string(EvaluationsTotalCount - EvaluationsFailedCount) + " out of " + std::to_string(EvaluationsTotalCount) + " passed.", 255, 255, 0);
		}
		else if (EvaluationsFailedCount == EvaluationsTotalCount)
		{
			ConsoleJob::OutputConsoleTextWithColor("All evaluations failed: " + std::to_string(EvaluationsFailedCount) + " out of " + std::to_string(EvaluationsTotalCount), 255, 0, 0);
		}

		EvaluationsTotalCount = 0;
		EvaluationsFailedCount = 0;
	}

	if (bConvertEvaluationToUsableScript)
	{
		std::cout << "Requested convertions of evaluations to usable scripts with actual values:" << std::endl;
		for (const auto& Script : SavedConvertionsOfEvaluationToUsableScript)
		{
			ConsoleJob::OutputConsoleTextWithColor(Script, 0, 255, 255);
		}
		std::cout << std::endl;
	}

	SavedConvertionsOfEvaluationToUsableScript.clear();
}

std::vector<ConsoleJob*> ConsoleJobManager::ConvertCommandAction(CommandLineAction ActionToParse)
{
	std::vector<ConsoleJob*> Result;

	std::transform(ActionToParse.Action.begin(), ActionToParse.Action.end(), ActionToParse.Action.begin(), [](unsigned char c) { return std::tolower(c); });

	if (ActionToParse.Action == "run_script_file")
	{
		if (ActionToParse.Settings.find("filepath") != ActionToParse.Settings.end())
		{
			std::string FilePath = ActionToParse.Settings["filepath"];
			if (FILE_SYSTEM.CheckFile(FilePath.c_str()))
			{
				std::ifstream File(FilePath);
				std::string Line;
				while (std::getline(File, Line))
				{
					std::vector<CommandLineAction> Actions = APPLICATION.ParseCommandLine(Line);
					std::vector<ConsoleJob*> NewJobs = ConvertCommandAction(Actions);
					for (size_t i = 0; i < NewJobs.size(); i++)
					{
						Result.push_back(NewJobs[i]);
					}
				}

				File.close();
				ConsoleJob::OutputConsoleTextWithColor("Script file read successfully. Jobs added to the queue: " + std::to_string(Result.size()), 0, 255, 0);
			}
			else
			{
				std::string ErrorMessage = "Error: File not found - " + FilePath + ". Please check the file path and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				ConsoleJob::OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			}
		}
	}

	ConsoleJob* NewJobToAdd = nullptr;

	if (ActionToParse.Action == "load")
	{
		NewJobToAdd = FileLoadJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "save")
	{
		NewJobToAdd = FileSaveJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "complexity")
	{
		NewJobToAdd = ComplexityJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "evaluation")
	{
		NewJobToAdd = ComplexityEvaluationJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "global_settings")
	{
		NewJobToAdd = GlobalSettingJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "export_layer_as_image")
	{
		NewJobToAdd = ExportLayerAsImageJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "help")
	{
		NewJobToAdd = HelpJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "exit")
	{
		NewJobToAdd = ExitJob::CreateInstance(ActionToParse);
	}

	if (NewJobToAdd != nullptr)
		Result.push_back(NewJobToAdd);

	return Result;
}

std::vector<ConsoleJob*> ConsoleJobManager::ConvertCommandAction(std::vector<CommandLineAction> Actions)
{
	std::vector<ConsoleJob*> Result;

	for (size_t i = 0; i < Actions.size(); i++)
	{
		std::vector<ConsoleJob*> NewJobs = ConvertCommandAction(Actions[i]);
		for (size_t j = 0; j < NewJobs.size(); j++)
		{
			Result.push_back(NewJobs[j]);
		}
	}

	return Result;
}