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
public:
	std::string Type;         // ......Type, what to check (min, max, average, mean)
	float ExpectedValue;
	float ActualValue;
	float Tolerance = 0.0f;
	
	bool Failed();
private:
	bool bFailed = true;
};

class ComplexityJobSettings
{
	friend ConsoleJobManager;

	float RelativeResolution = 0.0f;
	float ResolutionInM = 0.0f;

	std::string JitterQuality = "55";
	bool bRunOnWholeModel = false;

	std::string TriangleEdges_Mode = "MAX_LEHGTH";

	std::string Rugosity_Algorithm = "AVERAGE";
	bool bUniqueProjectedArea = false;
	bool bRugosity_DeleteOutliers = true;
	std::string Rugosity_MinAlgorithm_Quality = "91";

	bool bFractalDimension_FilterValues = true;

	bool bCalculateStandardDeviation = false;

	int Compare_FirstLayerIndex = -1;
	int Compare_SecondLayerIndex = -1;
	bool bCompare_Normalize = true;

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

	// Number of reference planes, more planes better results, but slower
	std::string GetRugosity_MinAlgorithm_Quality();
	// Number of reference planes, more planes better results, but slower
	void SetRugosity_MinAlgorithm_Quality(std::string NewValue);

	// Is unique projected area used, it yields very accurate results, but this method is very slow.
	bool GetRugosity_IsUsingUniqueProjectedArea();
	// Is unique projected area used, it yields very accurate results, but this method is very slow.
	void SetRugosity_IsUsingUniqueProjectedArea(bool NewValue);

	bool IsRugosity_DeleteOutliers();
	void SetRugosity_DeleteOutliers(bool NewValue);

	// Should app filter values that are less that 2.0
	bool GetFractalDimension_ShouldFilterValues();
	// Should app filter values that are less that 2.0
	void SetFractalDimension_ShouldFilterValues(bool NewValue);

	// Posible values: MAX_LEHGTH, MIN_LEHGTH, MEAN_LEHGTH
	std::string GetTriangleEdges_Mode();
	// Posible values: MAX_LEHGTH, MIN_LEHGTH, MEAN_LEHGTH
	void SetTriangleEdges_Mode(std::string NewValue);

	bool IsStandardDeviationNeeded();
	void SetIsStandardDeviationNeeded(bool NewValue);

	// Index of the first layer to compare
	// ID is not used, because could be unknown at the time of the job creation
	int GetCompare_FirstLayerIndex();
	// Index of the first layer to compare
	// ID is not used, because could be unknown at the time of the job creation
	void SetCompare_FirstLayerIndex(int NewValue);

	// Index of the second layer to compare
	// ID is not used, because could be unknown at the time of the job creation
	int GetCompare_SecondLayerIndex();
	// Index of the second layer to compare
	// ID is not used, because could be unknown at the time of the job creation
	void SetCompare_SecondLayerIndex(int NewValue);

	// Should app normalize the layers before comparing
	bool IsCompare_Normalize();
	// Should app normalize the layers before comparing
	void SetCompare_Normalize(bool NewValue);
};

class ComplexityJob : public ConsoleJob
{
public:
	friend ConsoleJobManager;

	ComplexityJob();
	ComplexityJob(std::string ComplexityType, ComplexityJobSettings Settings, std::vector<ComplexityJobEvaluation> Evaluations);

	std::string ComplexityType;

	ComplexityJobSettings Settings;
	std::vector<ComplexityJobEvaluation> Evaluations;
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
	}
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
	std::vector<ConsoleJob*> JobsWithFailedEvaluations;

	void SetGridResolution(ComplexityJob* Job);
	void SetRugosityAlgorithm(ComplexityJob* Job);
	void RunEvaluations(ComplexityJob* Job);

	void ExecuteJob(ConsoleJob* Job);
	void WaitForJitterManager();
};

#define CONSOLE_JOB_MANAGER ConsoleJobManager::getInstance()