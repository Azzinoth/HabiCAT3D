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

	float RelativeResolution = 0.0f;
	float ResolutionInM = 0.0f;

	std::string JitterQuality = "55";
	bool bRunOnWholeModel = false;

	std::string TriangleEdges_Mode = "MAX_LEHGTH";

	std::string Rugosity_Algorithm = "AVERAGE";
	bool bRugosity_DeleteOutliers = true;

	bool bCalculateStandardDeviation = false;
public:

	// Resolution in range of 0.0 to 1.0
	float GetRelativeResolution();
	// Resolution in range of 0.0 to 1.0
	void SetRelativeResolution(float NewValue);

	// Explicit resolution in meters, would be used first if valid
	float GetResolutionInM();
	// Explicit resolution in meters, would be used first if valid
	void SetResolutionInM(float NewValue);

	// Number of jitters, more jitters, smoother result, but slower
	std::string GetJitterQuality();
	// Number of jitters, more jitters, smoother result, but slower
	void SetJitterQuality(std::string NewValue);

	// No jitter, just whole model as input
	bool IsRunOnWholeModel();
	// No jitter, just whole model as input
	void SetRunOnWholeModel(bool NewValue);

	// Posible values: AVERAGE, MIN, LSF(CGAL)
	std::string GetRugosity_Algorithm();
	// Posible values: AVERAGE, MIN, LSF(CGAL)
	void SetRugosity_Algorithm(std::string NewValue);

	bool IsRugosity_DeleteOutliers();
	void SetRugosity_DeleteOutliers(bool NewValue);

	// Posible values: MAX_LEHGTH, MIN_LEHGTH, MEAN_LEHGTH
	std::string GetTriangleEdges_Mode();
	// Posible values: MAX_LEHGTH, MIN_LEHGTH, MEAN_LEHGTH
	void SetTriangleEdges_Mode(std::string NewValue);

	bool IsStandardDeviationNeeded();
	void SetIsStandardDeviationNeeded(bool NewValue);




	// , Jitter, algorithm, etc.
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

	void SetGridResolution(ComplexityJob* Job);
	void SetRugosityAlgorithm(ComplexityJob* Job);
	void ExecuteJob(ConsoleJob* Job);
};

#define CONSOLE_JOB_MANAGER ConsoleJobManager::getInstance()