#include "ConsoleJobManager.h"
using namespace FocalEngine;

ConsoleJob::ConsoleJob()
{
	ID = APPLICATION.GetUniqueHexID();
}

ComplexityJob::ComplexityJob()
{
	Type = "COMPLEXITY_JOB";
	ComplexityType = "NONE";
};

ComplexityJob::ComplexityJob(std::string ComplexityType, ComplexityJobSettings Settings, ComplexityJobEvaluation* Evaluation)
{
	this->ComplexityType = ComplexityType;
	this->Settings = Settings;
	this->Evaluation = Evaluation;
	Type = "COMPLEXITY_JOB";
};

ConsoleJobManager* ConsoleJobManager::Instance = nullptr;

ConsoleJobManager::ConsoleJobManager() {}
ConsoleJobManager::~ConsoleJobManager() {}

void ConsoleJobManager::AddJob(ConsoleJob* Job)
{
	JobsList.push_back(Job);
}

void ConsoleJobManager::ExecuteJob(ConsoleJob* Job)
{
	if (Job->Type == "FILE_LOAD")
	{
		FileLoadJob* FileJob = reinterpret_cast<FileLoadJob*>(Job);
		
		std::cout << "Initiating file load process for: " << FileJob->FilePath << std::endl;
		if (FILE_SYSTEM.CheckFile(FileJob->FilePath.c_str()))
		{
			std::cout << "File found. Loading file: " << FileJob->FilePath << std::endl;

			COMPLEXITY_METRIC_MANAGER.ImportOBJ(FileJob->FilePath.c_str(), true);
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->bDummyVariableForConsole = true;
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->UpdateAverageNormal();

			std::cout << "Successfully completed loading file: " << FileJob->FilePath << std::endl;
		}
		else
		{
			std::string ErrorMessage = "Error: File not found - " + FileJob->FilePath + ". Please check the file path and try again.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			std::cout << ErrorMessage << std::endl;
		}
	}
	else if (Job->Type == "FILE_SAVE")
	{
		COMPLEXITY_METRIC_MANAGER.SaveToRUGFile();
		std::cout << "File saved successfully." << std::endl;
	}
	else if (Job->Type == "COMPLEXITY_JOB")
	{
		if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		{
			std::string ErrorMessage = "Error: No file loaded. Please load a file before attempting to calculate complexity.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			std::cout << ErrorMessage << std::endl;
			return;
		}

		ComplexityJob* CurrentComplexityJob = reinterpret_cast<ComplexityJob*>(Job);
		if (CurrentComplexityJob->ComplexityType == "VECTOR_DISPERSION")
		{
			VECTOR_DISPERSION_LAYER_PRODUCER.CalculateWithJitterAsync(true);
			
			while (JITTER_MANAGER.GetJitterDoneCount() != JITTER_MANAGER.GetJitterToDoCount())
			{
				float Progress = float(JITTER_MANAGER.GetJitterDoneCount()) / float(JITTER_MANAGER.GetJitterToDoCount());
				std::cout << "\rProgress: " << std::to_string(Progress * 100.0f) << " %" << std::flush;


				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				THREAD_POOL.Update();
			}

			std::cout << std::endl;
		}
	}
}

void ConsoleJobManager::Update()
{
	if (!JobsList.empty())
	{
		ConsoleJob* CurrentJob = JobsList.front();
		if (CurrentJob != nullptr)
		{
			ExecuteJob(CurrentJob);

			delete CurrentJob;
			JobsList.erase(JobsList.begin());
		}
	}
}