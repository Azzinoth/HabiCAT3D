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

ComplexityJob::ComplexityJob(std::string ComplexityType, ComplexityJobSettings Settings)
{
	this->ComplexityType = ComplexityType;
	this->Settings = Settings;

	Type = "COMPLEXITY_JOB";
};

bool EvaluationJob::Failed()
{
	return bFailed;
}

ComplexityEvaluationJob::ComplexityEvaluationJob()
{
	Type = "EVALUATION_JOB";
	EvaluationType = "COMPLEXITY";
}

float ComplexityEvaluationJob::GetExpectedValue()
{
	return ExpectedValue;
}

void ComplexityEvaluationJob::SetExpectedValue(float NewValue)
{
	ExpectedValue = NewValue;
}

float ComplexityEvaluationJob::GetActualValue()
{
	return ActualValue;
}

void ComplexityEvaluationJob::SetActualValue(float NewValue)
{
	ActualValue = NewValue;
}

float ComplexityEvaluationJob::GetTolerance()
{
	return Tolerance;
}

void ComplexityEvaluationJob::SetTolerance(float NewValue)
{
	Tolerance = NewValue;
}

int ComplexityEvaluationJob::GetLayerIndex()
{
	return LayerIndex;
}

void ComplexityEvaluationJob::SetLayerIndex(int NewValue)
{
	LayerIndex = NewValue;
}

void ComplexityEvaluationJob::SetEvaluationSubType(std::string NewValue)
{
	EvaluationSubType = NewValue;
}

std::string ComplexityEvaluationJob::GetEvaluationSubType()
{
	return EvaluationSubType;
}

// Resolution in range of 0.0 to 1.0
float ComplexityJobSettings::GetRelativeResolution()
{
	return RelativeResolution;
}

// Resolution in range of 0.0 to 1.0
void ComplexityJobSettings::SetRelativeResolution(float NewValue)
{
	if (NewValue >= 0.0f && NewValue <= 1.0f)
		RelativeResolution = NewValue;
}

// Explicit resolution in meters, would be used first if valid
float ComplexityJobSettings::GetResolutionInM()
{
	return ResolutionInM;
}

// Explicit resolution in meters, would be used first if valid
void ComplexityJobSettings::SetResolutionInM(float NewValue)
{
	if (NewValue > 0.0f)
		ResolutionInM = NewValue;
}

// Number of jitters, more jitters, smoother result, but slower
std::string ComplexityJobSettings::GetJitterQuality()
{
	return JitterQuality;
}

// Number of jitters, more jitters, smoother result, but slower
void ComplexityJobSettings::SetJitterQuality(std::string NewValue)
{
	auto& ListOFValidValues = JITTER_MANAGER.GetJitterVectorSetNames();
	for (auto& Value : ListOFValidValues)
	{
		if (Value == NewValue)
		{
			JitterQuality = NewValue;
			return;
		}
	}
}

// No jitter, just whole model as input
bool ComplexityJobSettings::IsRunOnWholeModel()
{
	return bRunOnWholeModel;
}

// No jitter, just whole model as input
void ComplexityJobSettings::SetRunOnWholeModel(bool NewValue)
{
	bRunOnWholeModel = NewValue;
}

// Posible values: AVERAGE, MIN, LSF(CGAL)
std::string ComplexityJobSettings::GetRugosity_Algorithm()
{
	return Rugosity_Algorithm;
}

// Posible values: AVERAGE, MIN, LSF(CGAL)
void ComplexityJobSettings::SetRugosity_Algorithm(std::string NewValue)
{
	if (NewValue != "AVERAGE" && NewValue != "MIN" && NewValue != "LSF(CGAL)")
		return;

	Rugosity_Algorithm = NewValue;
}

// Number of reference planes, more planes better results, but slower
std::string ComplexityJobSettings::GetRugosity_MinAlgorithm_Quality()
{
	return Rugosity_MinAlgorithm_Quality;

}

// Is unique projected area used, it yields very accurate results, but this method is very slow.
bool ComplexityJobSettings::GetRugosity_IsUsingUniqueProjectedArea()
{
	return bUniqueProjectedArea;
}

// Is unique projected area used, it yields very accurate results, but this method is very slow.
void ComplexityJobSettings::SetRugosity_IsUsingUniqueProjectedArea(bool NewValue)
{
	bUniqueProjectedArea = NewValue;
}

// Number of reference planes, more planes better results, but slower
void ComplexityJobSettings::SetRugosity_MinAlgorithm_Quality(std::string NewValue)
{
	Rugosity_MinAlgorithm_Quality = NewValue;
}

bool ComplexityJobSettings::IsRugosity_DeleteOutliers()
{
	return bRugosity_DeleteOutliers;
}

void ComplexityJobSettings::SetRugosity_DeleteOutliers(bool NewValue)
{
	bRugosity_DeleteOutliers = NewValue;
}

// Should app filter values that are less that 2.0
bool ComplexityJobSettings::GetFractalDimension_ShouldFilterValues()
{
	return bFractalDimension_FilterValues;
}

// Should app filter values that are less that 2.0
void ComplexityJobSettings::SetFractalDimension_ShouldFilterValues(bool NewValue)
{
	bFractalDimension_FilterValues = NewValue;
}

// Posible values: MAX_LEHGTH, MIN_LEHGTH, MEAN_LEHGTH
std::string ComplexityJobSettings::GetTriangleEdges_Mode()
{
	return TriangleEdges_Mode;
}

// Posible values: MAX_LEHGTH, MIN_LEHGTH, MEAN_LEHGTH
void ComplexityJobSettings::SetTriangleEdges_Mode(std::string NewValue)
{
	if (NewValue != "MAX_LEHGTH" && NewValue != "MIN_LEHGTH" && NewValue != "MEAN_LEHGTH")
		return;

	TriangleEdges_Mode = NewValue;
}

bool ComplexityJobSettings::IsStandardDeviationNeeded()
{
	return bCalculateStandardDeviation;
}

void ComplexityJobSettings::SetIsStandardDeviationNeeded(bool NewValue)
{
	bCalculateStandardDeviation = NewValue;
}

// Index of the first layer to compare
// ID is not used, because could be unknown at the time of the job creation
int ComplexityJobSettings::GetCompare_FirstLayerIndex()
{
	return Compare_FirstLayerIndex;
}

// Index of the first layer to compare
// ID is not used, because could be unknown at the time of the job creation
void ComplexityJobSettings::SetCompare_FirstLayerIndex(int NewValue)
{
	Compare_FirstLayerIndex = NewValue;
}

// Index of the second layer to compare
// ID is not used, because could be unknown at the time of the job creation
int ComplexityJobSettings::GetCompare_SecondLayerIndex()
{
	return Compare_SecondLayerIndex;
}

// Index of the second layer to compare
// ID is not used, because could be unknown at the time of the job creation
void ComplexityJobSettings::SetCompare_SecondLayerIndex(int NewValue)
{
	Compare_SecondLayerIndex = NewValue;
}

// Should app normalize the layers before comparing
bool ComplexityJobSettings::IsCompare_Normalize()
{
	return bCompare_Normalize;
}

// Should app normalize the layers before comparing
void ComplexityJobSettings::SetCompare_Normalize(bool NewValue)
{
	bCompare_Normalize = NewValue;
}

ConsoleJobManager* ConsoleJobManager::Instance = nullptr;

ConsoleJobManager::ConsoleJobManager() {}
ConsoleJobManager::~ConsoleJobManager() {}

void ConsoleJobManager::AddJob(ConsoleJob* Job)
{
	JobsList.push_back(Job);
}

void ConsoleJobManager::SetGridResolution(ComplexityJob* Job)
{
	JITTER_MANAGER.SetResolutonInM(JITTER_MANAGER.GetLowestPossibleResolution());

	if (Job->Settings.ResolutionInM != 0.0f &&
		Job->Settings.ResolutionInM >= JITTER_MANAGER.GetLowestPossibleResolution() &&
		Job->Settings.ResolutionInM <= JITTER_MANAGER.GetHigestPossibleResolution())
	{
		JITTER_MANAGER.SetResolutonInM(Job->Settings.ResolutionInM);
	}

	if (Job->Settings.ResolutionInM == 0.0f && Job->Settings.RelativeResolution != 0.0f)
	{
		float Range = JITTER_MANAGER.GetHigestPossibleResolution() - JITTER_MANAGER.GetLowestPossibleResolution();
		JITTER_MANAGER.SetResolutonInM(JITTER_MANAGER.GetLowestPossibleResolution() + Range * Job->Settings.RelativeResolution);
	}
}

void ConsoleJobManager::SetRugosityAlgorithm(ComplexityJob* Job)
{
	if (Job->ComplexityType != "RUGOSITY")
		return;

	RUGOSITY_LAYER_PRODUCER.SetUseFindSmallestRugosity(Job->Settings.GetRugosity_Algorithm() == "MIN");
	RUGOSITY_LAYER_PRODUCER.SetUseCGALVariant(Job->Settings.GetRugosity_Algorithm() == "LSF(CGAL)");
}

void ConsoleJobManager::WaitForJitterManager()
{
	while (JITTER_MANAGER.GetJitterDoneCount() != JITTER_MANAGER.GetJitterToDoCount())
	{
		float Progress = float(JITTER_MANAGER.GetJitterDoneCount()) / float(JITTER_MANAGER.GetJitterToDoCount());
		std::cout << "\rProgress: " << std::to_string(Progress * 100.0f) << " %" << std::flush;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		THREAD_POOL.Update();
	}
}

void ConsoleJobManager::PrintHelp(std::string Command)
{
	if (Command == "")
	{
		std::cout <<
			"\nFirst command should be:\n"
			"  -console\n"
			"Then any other command:\n"
			"  -[COMMAND] [OPTIONS]=[VALUE]\n\n"

			"Commands:\n"
			"-help                                           Display this help message.\n"
			"-help command_name=[COMMAND]                    Display help for a specific command along with all its settings.\n"
			"-file_load filepath=[PATH]                      Load a file from the specified path.\n"
			"-file_save filepath=[PATH]                      Save the current state to a file at the specified path.\n"
			"-run_script_file filepath=[PATH]                Execute a sequence of commands from a specified script(text) file.Each command in the file should be on a new line.\n"
			"-complexity type=[LAYER_TYPE]                   Create a complexity job with the specified settings to create a layer.\n"
			"-evaluation type=[TYPE] subtype=[WHAT_TO_TEST]  Create an evaluation job with the specified settings to test a layer or other objects.\n\n"

			"Examples:\n"
			"-load filepath=\"C:/data/mesh.obj\"\n"
			"-save filepath=\"C:/data/processed_mesh.rug\"\n"
			"-complexity type=RUGOSITY rugosity_algorithm = MIN jitter_quality=73\n"
			"-evaluation type=COMPLEXITY subtype=MAX_LAYER_VALUE expected_value=5.02 tolerance=0.01\n\n"

			"Notes:\n"
			"For Boolean settings, use true or false.\n"
			"Paths must be enclosed in quotes.\n\n";
	}
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
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->UpdateAverageNormal();

			OutputConsoleTextWithColor("Successfully completed loading file: ", 0, 255, 0);
			OutputConsoleTextWithColor(FileJob->FilePath, 0, 255, 0);
		}
		else
		{
			std::string ErrorMessage = "Error: File not found - " + FileJob->FilePath + ". Please check the file path and try again.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
		}
	}
	else if (Job->Type == "FILE_SAVE")
	{
		COMPLEXITY_METRIC_MANAGER.SaveToRUGFile(reinterpret_cast<FileSaveJob*>(Job)->FilePath);
		OutputConsoleTextWithColor("File saved successfully.", 0, 255, 0);
	}
	else if (Job->Type == "COMPLEXITY_JOB")
	{
		if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		{
			std::string ErrorMessage = "Error: No file loaded. Please load a file before attempting to calculate complexity.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return;
		}

		ComplexityJob* CurrentComplexityJob = reinterpret_cast<ComplexityJob*>(Job);
		SetGridResolution(CurrentComplexityJob);
		JITTER_MANAGER.SetCurrentJitterVectorSetName(CurrentComplexityJob->Settings.GetJitterQuality());

		if (CurrentComplexityJob->ComplexityType == "HEIGHT")
		{
			std::cout << "Initiating Height Layer calculation." << std::endl;

			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(HEIGHT_LAYER_PRODUCER.Calculate());

			std::cout << "Height Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "AREA")
		{
			std::cout << "Initiating Area Layer calculation." << std::endl;

			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(AREA_LAYER_PRODUCER.Calculate());

			std::cout << "Area Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "TRIANGLE_EDGE")
		{
			std::cout << "Initiating Triangle Edge Layer calculation." << std::endl;

			int Mode = 0;
			if (CurrentComplexityJob->Settings.GetTriangleEdges_Mode() == "MIN_LEHGTH")
			{
				Mode = 1;
			}
			else if (CurrentComplexityJob->Settings.GetTriangleEdges_Mode() == "MEAN_LEHGTH")
			{
				Mode = 2;
			}

			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(TRIANGLE_EDGE_LAYER_PRODUCER.Calculate(Mode));

			std::cout << "Triangle Edge Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "TRIANGLE_COUNT")
		{
			std::cout << "Initiating Triangle Count Layer calculation." << std::endl;

			if (CurrentComplexityJob->Settings.IsRunOnWholeModel())
			{
				TRIANGLE_COUNT_LAYER_PRODUCER.CalculateOnWholeModel();
			}
			else
			{
				TRIANGLE_COUNT_LAYER_PRODUCER.CalculateWithJitterAsync(false);
			}

			WaitForJitterManager();

			std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
			std::cout << std::endl;
			std::cout << "Triangle Count Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "RUGOSITY")
		{
			std::cout << "Initiating Rugosity Layer calculation." << std::endl;

			SetRugosityAlgorithm(CurrentComplexityJob);
			RUGOSITY_LAYER_PRODUCER.bCalculateStandardDeviation = CurrentComplexityJob->Settings.IsStandardDeviationNeeded();
			RUGOSITY_LAYER_PRODUCER.bDeleteOutliers = CurrentComplexityJob->Settings.IsRugosity_DeleteOutliers();

			RUGOSITY_LAYER_PRODUCER.SetOrientationSetForMinRugosityName(CurrentComplexityJob->Settings.GetRugosity_MinAlgorithm_Quality());
			RUGOSITY_LAYER_PRODUCER.SetIsUsingUniqueProjectedArea(CurrentComplexityJob->Settings.GetRugosity_IsUsingUniqueProjectedArea());

			if (CurrentComplexityJob->Settings.IsRunOnWholeModel())
			{
				RUGOSITY_LAYER_PRODUCER.CalculateOnWholeModel();
			}
			else
			{
				RUGOSITY_LAYER_PRODUCER.CalculateWithJitterAsync();
			}

			WaitForJitterManager();

			std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
			std::cout << std::endl;
			std::cout << "Rugosity Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "VECTOR_DISPERSION")
		{
			std::cout << "Initiating Vector Dispersion calculation." << std::endl;
			
			VECTOR_DISPERSION_LAYER_PRODUCER.bCalculateStandardDeviation = CurrentComplexityJob->Settings.IsStandardDeviationNeeded();

			if (CurrentComplexityJob->Settings.IsRunOnWholeModel())
			{
				VECTOR_DISPERSION_LAYER_PRODUCER.CalculateOnWholeModel();
			}
			else
			{
				VECTOR_DISPERSION_LAYER_PRODUCER.CalculateWithJitterAsync(false);
			}

			WaitForJitterManager();

			std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
			std::cout << std::endl;
			std::cout << "Vector Dispersion calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "FRACTAL_DIMENSION")
		{
			std::cout << "Initiating Fractal Dimension Layer calculation." << std::endl;

			FRACTAL_DIMENSION_LAYER_PRODUCER.bCalculateStandardDeviation = CurrentComplexityJob->Settings.IsStandardDeviationNeeded();
			FRACTAL_DIMENSION_LAYER_PRODUCER.SetShouldFilterFractalDimensionValues(CurrentComplexityJob->Settings.GetFractalDimension_ShouldFilterValues());

			if (CurrentComplexityJob->Settings.IsRunOnWholeModel())
			{
				FRACTAL_DIMENSION_LAYER_PRODUCER.CalculateOnWholeModel();
			}
			else
			{
				FRACTAL_DIMENSION_LAYER_PRODUCER.CalculateWithJitterAsync(false);
			}

			WaitForJitterManager();

			std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
			std::cout << std::endl;
			std::cout << "Fractal Dimension Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "COMPARE")
		{
			std::cout << "Initiating Compare Layer calculation." << std::endl;

			int FirstLayerIndex = CurrentComplexityJob->Settings.GetCompare_FirstLayerIndex();
			if (FirstLayerIndex < 0 || FirstLayerIndex > COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
			{
				std::string ErrorMessage = "Error: First layer index is out of range. Please check the layer index and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
				return;
			}

			int SecondLayerIndex = CurrentComplexityJob->Settings.GetCompare_SecondLayerIndex();
			if (SecondLayerIndex < 0 || SecondLayerIndex > COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
			{
				std::string ErrorMessage = "Error: Second layer index is out of range. Please check the layer index and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
				return;
			}

			COMPARE_LAYER_PRODUCER.SetShouldNormalize(CurrentComplexityJob->Settings.IsCompare_Normalize());
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(COMPARE_LAYER_PRODUCER.Calculate(FirstLayerIndex, SecondLayerIndex));

			std::cout << "Compare Layer calculation completed." << std::endl;
		}
	}
 	else if (Job->Type == "EVALUATION_JOB")
	{
		EvaluationJob* CurrentEvaluationJob = reinterpret_cast<EvaluationJob*>(Job);

		if (CurrentEvaluationJob->EvaluationType == "COMPLEXITY")
		{
			ComplexityEvaluationJob* CurrentComplexityEvaluationJob = reinterpret_cast<ComplexityEvaluationJob*>(Job);

			if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.empty())
			{
				std::string ErrorMessage = "Error: No layers to evaluate. Please calculate a layer before attempting to evaluate.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
				return;
			}

			MeshLayer* LayerToEvaluate = nullptr;
			if (CurrentComplexityEvaluationJob->GetLayerIndex() != -1)
			{
				LayerToEvaluate = &COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[CurrentComplexityEvaluationJob->GetLayerIndex()];
			}
			else
			{
				LayerToEvaluate = &COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back();
			}
			
			if (LayerToEvaluate == nullptr)
			{
				std::string ErrorMessage = "Error: Layer to evaluate is null. Please check the layer index and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
				return;
			}

			CurrentEvaluationJob->bFailed = false;
			float Difference = FLT_MAX;

			if (CurrentComplexityEvaluationJob->GetEvaluationSubType() == "MEAN_LAYER_VALUE")
			{
				CurrentComplexityEvaluationJob->SetActualValue(LayerToEvaluate->GetMean());
			}
			else if (CurrentComplexityEvaluationJob->GetEvaluationSubType() == "MEDIAN_LAYER_VALUE")
			{
				CurrentComplexityEvaluationJob->SetActualValue(LayerToEvaluate->GetMedian());
			}
			else if (CurrentComplexityEvaluationJob->GetEvaluationSubType() == "MAX_LAYER_VALUE")
			{
				CurrentComplexityEvaluationJob->SetActualValue(LayerToEvaluate->GetMax());
			}
			else if (CurrentComplexityEvaluationJob->GetEvaluationSubType() == "MIN_LAYER_VALUE")
			{
				CurrentComplexityEvaluationJob->SetActualValue(LayerToEvaluate->GetMin());
			}

			Difference = CurrentComplexityEvaluationJob->GetExpectedValue() - CurrentComplexityEvaluationJob->GetActualValue();

			if (abs(Difference) > CurrentComplexityEvaluationJob->GetTolerance())
				CurrentComplexityEvaluationJob->bFailed = true;

			if (isnan(CurrentComplexityEvaluationJob->GetActualValue()))
				CurrentComplexityEvaluationJob->bFailed = true;

			EvaluationsTotalCount++;
			if (CurrentComplexityEvaluationJob->Failed())
			{
				std::string ErrorMessage = "Error: Evaluation failed. Type: " + CurrentComplexityEvaluationJob->EvaluationType + " SubType: " + CurrentComplexityEvaluationJob->GetEvaluationSubType() + " Expected: " + std::to_string(CurrentComplexityEvaluationJob->GetExpectedValue()) + " Tolerance: " + std::to_string(CurrentComplexityEvaluationJob->GetTolerance()) + " Actual: " + std::to_string(CurrentComplexityEvaluationJob->GetActualValue());
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);

				EvaluationsFailedCount++;
			}
			else
			{
				std::string Message = "Evaluation passed. Type: " + CurrentComplexityEvaluationJob->EvaluationType + " SubType: " + CurrentComplexityEvaluationJob->GetEvaluationSubType() + " Expected: " + std::to_string(CurrentComplexityEvaluationJob->GetExpectedValue()) + " Tolerance: " + std::to_string(CurrentComplexityEvaluationJob->GetTolerance()) + " Actual: " + std::to_string(CurrentComplexityEvaluationJob->GetActualValue());
				LOG.Add(Message, "CONSOLE_LOG");
				OutputConsoleTextWithColor(Message, 0, 255, 0);
			}
		}
	}
	else if (Job->Type == "HELP_JOB")
	{
		PrintHelp();
	}
	else if (Job->Type == "EXIT_JOB")
	{
		APPLICATION.Close();
	}
	else
	{
		std::string ErrorMessage = "Error: Unknown job type: " + Job->Type + ". Please check the job type and try again.";
		LOG.Add(ErrorMessage, "CONSOLE_LOG");
		OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
	}
}

void ConsoleJobManager::Update()
{
	if (JobsList.empty() && EvaluationsTotalCount > 1)
	{
		std::cout << "All jobs finished." << std::endl;

		if (EvaluationsFailedCount == 0)
		{
			OutputConsoleTextWithColor("All evaluations passed: " + std::to_string(EvaluationsTotalCount) + " out of " + std::to_string(EvaluationsTotalCount), 0, 255, 0);
		}
		else if (EvaluationsFailedCount > 0 && EvaluationsFailedCount < EvaluationsTotalCount)
		{
			OutputConsoleTextWithColor("Some evaluations failed, only: " + std::to_string(EvaluationsTotalCount - EvaluationsFailedCount) + " out of " + std::to_string(EvaluationsTotalCount) + " passed.", 255, 255, 0);
		}
		else if (EvaluationsFailedCount == EvaluationsTotalCount)
		{
			OutputConsoleTextWithColor("All evaluations failed: " + std::to_string(EvaluationsFailedCount) + " out of " + std::to_string(EvaluationsTotalCount), 255, 0, 0);

		}

		EvaluationsTotalCount = 0;
		EvaluationsFailedCount = 0;

		return;
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
			JobsList.push_back(NewJobs[i]);
		}
	}
}

std::vector<ConsoleJob*> ConsoleJobManager::ConvertCommandAction(CommandLineAction ActionToParse)
{
	std::vector<ConsoleJob*> Result;

	std::transform(ActionToParse.Action.begin(), ActionToParse.Action.end(), ActionToParse.Action.begin(), [](unsigned char c) { return std::tolower(c); });

	if (ActionToParse.Action == "load")
	{
		if (ActionToParse.Settings.find("filepath") != ActionToParse.Settings.end())
		{
			Result.push_back(new FileLoadJob(ActionToParse.Settings["filepath"]));
		}
	}
	else if (ActionToParse.Action == "save")
	{
		if (ActionToParse.Settings.find("filepath") != ActionToParse.Settings.end())
		{
			Result.push_back(new FileSaveJob(ActionToParse.Settings["filepath"]));
		}
	}
	else if (ActionToParse.Action == "complexity")
	{
		if (ActionToParse.Settings.find("type") == ActionToParse.Settings.end())
			return Result;

		std::string Type = ActionToParse.Settings["type"];
		std::transform(Type.begin(), Type.end(), Type.begin(), [](unsigned char c) { return std::toupper(c); });

		auto Iterator = ActionToParse.Settings.begin();
		while (Iterator != ActionToParse.Settings.end())
		{
			std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
			Iterator++;
		}

		ComplexityJob* NewJobToAdd = new ComplexityJob();
		NewJobToAdd->ComplexityType = Type;

		if (ActionToParse.Settings.find("resolution") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetResolutionInM(std::stof(ActionToParse.Settings["resolution"]));
		}

		if (ActionToParse.Settings.find("relative_resolution") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRelativeResolution(std::stof(ActionToParse.Settings["relative_resolution"]));
		}

		if (ActionToParse.Settings.find("jitter_quality") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetJitterQuality(ActionToParse.Settings["jitter_quality"]);
		}

		if (ActionToParse.Settings.find("run_on_whole_model") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRunOnWholeModel(ActionToParse.Settings["run_on_whole_model"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("rugosity_algorithm") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRugosity_Algorithm(ActionToParse.Settings["rugosity_algorithm"]);
		}

		if (ActionToParse.Settings.find("rugosity_min_algorithm_quality") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRugosity_MinAlgorithm_Quality(ActionToParse.Settings["rugosity_min_algorithm_quality"]);
		}

		if (ActionToParse.Settings.find("rugosity_is_using_unique_projected_area") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRugosity_IsUsingUniqueProjectedArea(ActionToParse.Settings["rugosity_is_using_unique_projected_area"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("rugosity_delete_outliers") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRugosity_DeleteOutliers(ActionToParse.Settings["rugosity_delete_outliers"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("fractal_dimension_should_filter_values") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetFractalDimension_ShouldFilterValues(ActionToParse.Settings["fractal_dimension_should_filter_values"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("triangle_edges_mode") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetTriangleEdges_Mode(ActionToParse.Settings["triangle_edges_mode"]);
		}

		if (ActionToParse.Settings.find("is_standard_deviation_needed") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetIsStandardDeviationNeeded(ActionToParse.Settings["is_standard_deviation_needed"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("compare_first_layer_index") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetCompare_FirstLayerIndex(std::stoi(ActionToParse.Settings["compare_first_layer_index"]));
		}

		if (ActionToParse.Settings.find("compare_second_layer_index") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetCompare_SecondLayerIndex(std::stoi(ActionToParse.Settings["compare_second_layer_index"]));
		}

		if (ActionToParse.Settings.find("compare_normalize") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetCompare_Normalize(ActionToParse.Settings["compare_normalize"] == "TRUE" ? true : false);
		}

		Result.push_back(NewJobToAdd);
	}
	else if (ActionToParse.Action == "evaluation")
	{
		if (ActionToParse.Settings.find("type") == ActionToParse.Settings.end())
			return Result;

		if (ActionToParse.Settings.find("subtype") == ActionToParse.Settings.end())
			return Result;

		std::string Type = ActionToParse.Settings["type"];
		std::transform(Type.begin(), Type.end(), Type.begin(), [](unsigned char c) { return std::toupper(c); });

		auto Iterator = ActionToParse.Settings.begin();
		while (Iterator != ActionToParse.Settings.end())
		{
			std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
			Iterator++;
		}

		if (ActionToParse.Settings["type"] == "COMPLEXITY")
		{
			ComplexityEvaluationJob* NewJobToAdd = new ComplexityEvaluationJob();

			NewJobToAdd->SetEvaluationSubType(ActionToParse.Settings["subtype"]);

			if (ActionToParse.Settings.find("expected_value") != ActionToParse.Settings.end())
			{
				NewJobToAdd->SetExpectedValue(std::stof(ActionToParse.Settings["expected_value"]));
			}

			if (ActionToParse.Settings.find("tolerance") != ActionToParse.Settings.end())
			{
				NewJobToAdd->SetTolerance(std::stof(ActionToParse.Settings["tolerance"]));
			}

			if (ActionToParse.Settings.find("layer_index") != ActionToParse.Settings.end())
			{
				NewJobToAdd->SetLayerIndex(std::stoi(ActionToParse.Settings["layer_index"]));
			}

			Result.push_back(NewJobToAdd);
		}
	}
	else if (ActionToParse.Action == "run_script_file")
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
				OutputConsoleTextWithColor("Script file read successfully. Jobs added to the queue: " + std::to_string(Result.size()), 0, 255, 0);
			}
			else
			{
				std::string ErrorMessage = "Error: File not found - " + FilePath + ". Please check the file path and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			}
		}
	}
	else if (ActionToParse.Action == "help")
	{
		ConsoleJob* NewJobToAdd = new ConsoleJob();
		NewJobToAdd->Type = "HELP_JOB";
		Result.push_back(NewJobToAdd);
	}
	else if (ActionToParse.Action == "exit")
	{
		ConsoleJob* NewJobToAdd = new ConsoleJob();
		NewJobToAdd->Type = "EXIT_JOB";
		Result.push_back(NewJobToAdd);
	}

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

void ConsoleJobManager::OutputConsoleTextWithColor(std::string Text, int R, int G, int B)
{
	APPLICATION.GetConsoleWindow()->SetNearestConsoleTextColor(R, G, B);
	std::cout << Text << std::endl;
	APPLICATION.GetConsoleWindow()->SetNearestConsoleTextColor(255, 255, 255);
}