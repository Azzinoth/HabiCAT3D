#pragma once

#include "HelpJob.h"
#include "FileLoadJob.h"
#include "FileSaveJob.h"
#include "ComplexityJob.h"
#include "EvaluationJob.h"
#include "GlobalSettingJob.h"
#include "ExportLayerAsImageJob.h"
using namespace FocalEngine;

class ConsoleJobManager
{
public:
	SINGLETON_PUBLIC_PART(ConsoleJobManager)

	void AddJob(ConsoleJob* Job);
	void Update();

	std::vector<ConsoleJob*> ConvertCommandAction(CommandLineAction Action);
	std::vector<ConsoleJob*> ConvertCommandAction(std::vector<CommandLineAction> Actions);
private:
	SINGLETON_PRIVATE_PART(ConsoleJobManager)

	std::vector<ConsoleJob*> JobsList;
	std::vector<ConsoleJob*> JobsWithFailedEvaluations;

	int EvaluationsTotalCount = 0;
	int EvaluationsFailedCount = 0;

	bool SetGridResolution(ComplexityJob* Job);
	void SetRugosityAlgorithm(ComplexityJob* Job);

	void ExecuteJob(ConsoleJob* Job);
	void WaitForJitterManager();

	void OutputConsoleTextWithColor(std::string Text, int R, int G, int B);

	struct ConsoleJobSettingsInfo
	{
		std::string Name;
		std::string Description;
		bool bIsOptional;
		std::string DefaultValue;
		std::vector<std::string> PossibleValues;
	};

	struct ConsoleJobInfo
	{
		std::string CommandName;
		std::string Purpose;
		std::vector<ConsoleJobSettingsInfo> SettingsInfo;
	};

	std::map<std::string, ConsoleJobInfo> ConsoleJobsInfo;
	void PrintCommandHelp(std::string CommandName);
	void PrintHelp(std::string CommandName = "");

	int JobsAdded = 0;
	void OnAllJobsFinished();

	bool bConvertEvaluationToUsableScript = false;
	std::vector<std::string> SavedConvertionsOfEvaluationToUsableScript;
};

#define CONSOLE_JOB_MANAGER ConsoleJobManager::getInstance()