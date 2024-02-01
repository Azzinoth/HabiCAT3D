#pragma once

#include "../UI/UIManager.h"
#include "../ScreenshotManager.h"

using namespace FocalEngine;

// ConsoleMessages: on Start, on Finish, on Progress







// There will be a queue of ConsoleJobs
// Console commands would be converted to ConsoleJobs and added to the queue
// Command line arguments would be converted to ConsoleJobs and added to the queue
// All of that would be working only if -console argument is passed to the application

// There will be a special script file that can represent a list of ConsoleJobs, but also it would have 
// a way to define a ComplexityJobEvaluation, so it can be used for QA.

class ConsoleJobManager;
class ConsoleJob      // Class ConsoleJob
{
	friend ConsoleJobManager;
	std::string ID;   // ID (unique)
protected:
	std::string Type; // ..Type (FileLoad, FileSave, ComplexityJob) // BulkFile, BulkFolder will be extracted and converted to list of ConsoleJobs

	ConsoleJob();
};

class ComplexityJobEvaluation // ....Class ComplexityJobEvaluation, ussually empty, but can be used for QA.
{
	friend ConsoleJobManager;

	std::string Type;         // ......Type, what to check (min, max, average, mean)
	float Value;			  // ......Value, what to check against
	float Tolerance;          // ......Tolerance, how much can be off
};

class ComplexityJobSettings  // ..Class ComplexityJobSettings
{
	friend ConsoleJobManager;
	// ....Resolution(in range of 0.0 to 1.0, or explicit M), Jitter, algorithm, etc.
	float Resolution;
};

class ComplexityJob : public ConsoleJob              // Class ComplexityJob child of ConsoleJobs
{
public:
	friend ConsoleJobManager;

	ComplexityJob();
	ComplexityJob(std::string ComplexityType, ComplexityJobSettings Settings, ComplexityJobEvaluation* Evaluation);

	std::string ComplexityType;                      // ..Type (TriangleCount, VectorDispersion, FractalDimension, etc.)

	ComplexityJobSettings Settings;
	ComplexityJobEvaluation* Evaluation = nullptr;
};

class FileLoadJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string FilePath;

public:
	FileLoadJob(std::string FilePath)
	{
		this->FilePath = FilePath;
		Type = "FILE_LOAD";
	};
};

class FileSaveJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string FilePath;

public:
	FileSaveJob(std::string FilePath)
	{
		this->FilePath = FilePath;
		Type = "FILE_SAVE";
	};
};

class ConsoleJobManager
{
public:
	SINGLETON_PUBLIC_PART(ConsoleJobManager)

	void AddJob(ConsoleJob* Job);
	void Update();
private:
	SINGLETON_PRIVATE_PART(ConsoleJobManager)

	std::vector<ConsoleJob*> JobsList;

	void ExecuteJob(ConsoleJob* Job);
};

#define CONSOLE_JOB_MANAGER ConsoleJobManager::getInstance()