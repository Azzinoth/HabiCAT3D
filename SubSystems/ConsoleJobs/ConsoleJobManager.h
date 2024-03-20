#pragma once

#include "../UI/UIManager.h"
#include "../ScreenshotManager.h"

using namespace FocalEngine;

class ConsoleJobManager;
class ConsoleJob
{
	friend ConsoleJobManager;
	std::string ID;
protected:
	std::string Type;

	ConsoleJob();
public:
	std::string GetID();
};

class HelpJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string CommandName = "";
public:
	HelpJob(std::string CommandName = "");
	std::string GetCommandName();
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
	ComplexityJob(std::string ComplexityType, ComplexityJobSettings Settings);

	std::string ComplexityType;

	ComplexityJobSettings Settings;
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

class EvaluationJob : public ConsoleJob
{
	friend ConsoleJobManager;

	bool bFailed = true;
protected:
	std::string EvaluationType;
public:

	bool Failed();
};

class ComplexityEvaluationJob : public EvaluationJob
{
	friend ConsoleJobManager;

	std::string EvaluationSubType;

	float ExpectedValue = 0.0f;
	float ActualValue = 0.0f;
	float Tolerance = 0.0f;

	int LayerIndex = -1;
public:
	ComplexityEvaluationJob();

	float GetExpectedValue();
	void SetExpectedValue(float NewValue);

	float GetActualValue();
	void SetActualValue(float NewValue);

	float GetTolerance();
	void SetTolerance(float NewValue);

	int GetLayerIndex();
	void SetLayerIndex(int NewValue);

	std::string GetEvaluationSubType();
	void SetEvaluationSubType(std::string NewValue);
};

class GlobalSettingJob : public EvaluationJob
{
	friend ConsoleJobManager;

	std::string GlobalSettingType;

	int IntValue = 0;
	float FloatValue = 0.0f;
	bool bValue = false;
public:
	GlobalSettingJob();

	std::string GetGlobalSettingType();
	void SetGlobalSettingType(std::string NewValue);

	int GetIntValue();
	void SetIntValue(int NewValue);

	float GetFloatValue();
	void SetFloatValue(float NewValue);

	bool GetBoolValue();
	void SetBoolValue(bool NewValue);
};

class ExportLayerAsImageJob : public EvaluationJob
{
	friend ConsoleJobManager;

	LayerRasterizationManager::GridRasterizationMode ExportMode;
	LayerRasterizationManager::SaveMode SaveMode;

	std::string FilePath = "";
	float ResolutionInM = 0.0f;
	glm::vec3 ForceProjectionVector = glm::vec3(0.0f);
	float PersentOfAreaThatWouldBeRed = 5.0f;

	int LayerIndex = -1;
public:
	ExportLayerAsImageJob();

	LayerRasterizationManager::GridRasterizationMode GetExportMode();
	void SetExportMode(LayerRasterizationManager::GridRasterizationMode NewValue);

	LayerRasterizationManager::SaveMode GetSaveMode();
	void SetSaveMode(LayerRasterizationManager::SaveMode NewValue);

	std::string GetFilePath();
	void SetFilePath(std::string NewValue);

	float GetResolutionInM();
	void SetResolutionInM(float NewValue);

	glm::vec3 GetForceProjectionVector();
	void SetForceProjectionVector(glm::vec3 NewValue);

	float GetPersentOfAreaThatWouldBeRed();
	void SetPersentOfAreaThatWouldBeRed(float NewValue);

	int GetLayerIndex();
	void SetLayerIndex(int NewValue);
};

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

	void SetGridResolution(ComplexityJob* Job);
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