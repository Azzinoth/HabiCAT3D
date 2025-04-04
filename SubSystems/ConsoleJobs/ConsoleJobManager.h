#pragma once

#include "ExitJob.h"
#include "HelpJob.h"
#include "FileLoadJob.h"
#include "FileSaveJob.h"
#include "ComplexityJob.h"
#include "EvaluationJob.h"
#include "GlobalSettingJob.h"
#include "ExportLayerAsImageJob.h"
#include "QueryJob.h"
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

	void ExecuteJob(ConsoleJob* Job);

	std::map<std::string, ConsoleJobInfo> ConsoleJobsInfo;

	int JobsAdded = 0;
	void OnAllJobsFinished();
	void OutputEvaluationResults();

	bool bConvertEvaluationToUsableScript = false;
	std::vector<std::string> SavedConversionsOfEvaluationToUsableScript;

	bool bOutputLogToFile = false;
};

#define CONSOLE_JOB_MANAGER ConsoleJobManager::GetInstance()