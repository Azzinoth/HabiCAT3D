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

ComplexityJob::ComplexityJob(std::string ComplexityType, ComplexityJobSettings Settings, std::vector<ComplexityJobEvaluation> Evaluations)
{
	this->ComplexityType = ComplexityType;
	this->Settings = Settings;
	this->Evaluations = Evaluations;
	Type = "COMPLEXITY_JOB";
};

bool ComplexityJobEvaluation::Failed()
{
	return bFailed;
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

void ConsoleJobManager::RunEvaluations(ComplexityJob* Job)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.empty())
		return;

	MeshLayer& LastLayer = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back();

	for (size_t i = 0; i < Job->Evaluations.size(); i++)
	{
		Job->Evaluations[i].bFailed = false;
		float Difference = 0.0f;

		if (Job->Evaluations[i].Type == "MEAN_LAYER_VALUE")
		{
			Job->Evaluations[i].ActualValue = LastLayer.GetMean();
			Difference = Job->Evaluations[i].ExpectedValue - Job->Evaluations[i].ActualValue;
		}
		else if (Job->Evaluations[i].Type == "MEDIAN_LAYER_VALUE")
		{
			Job->Evaluations[i].ActualValue = LastLayer.GetMedian();
			Difference = Job->Evaluations[i].ExpectedValue - Job->Evaluations[i].ActualValue;
		}
		else if (Job->Evaluations[i].Type == "MAX_LAYER_VALUE")
		{
			Job->Evaluations[i].ActualValue = LastLayer.GetMax();
			Difference = Job->Evaluations[i].ExpectedValue - Job->Evaluations[i].ActualValue;
		}
		else if (Job->Evaluations[i].Type == "MIN_LAYER_VALUE")
		{
			Job->Evaluations[i].ActualValue = LastLayer.GetMin();
			Difference = Job->Evaluations[i].ExpectedValue - Job->Evaluations[i].ActualValue;
		}

		if (abs(Difference) > Job->Evaluations[i].Tolerance)
			Job->Evaluations[i].bFailed = true;
		
		if (Job->Evaluations[i].Failed())
		{
			std::string ErrorMessage = "Error: Evaluation failed. Type: " + Job->Evaluations[i].Type + " Expected: " + std::to_string(Job->Evaluations[i].ExpectedValue) + " Tolerance: " + std::to_string(Job->Evaluations[i].Tolerance) + " Actual: " + std::to_string(Job->Evaluations[i].ActualValue);
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			std::cout << ErrorMessage << std::endl;

			JobsWithFailedEvaluations.push_back(Job);
		}
		else
		{
			std::string ErrorMessage = "Evaluation passed. Type: " + Job->Evaluations[i].Type + " Expected: " + std::to_string(Job->Evaluations[i].ExpectedValue) + " Tolerance: " + std::to_string(Job->Evaluations[i].Tolerance) + " Actual: " + std::to_string(Job->Evaluations[i].ActualValue);
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			std::cout << ErrorMessage << std::endl;
		}
	}
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
				std::cout << ErrorMessage << std::endl;
				return;
			}

			int SecondLayerIndex = CurrentComplexityJob->Settings.GetCompare_SecondLayerIndex();
			if (SecondLayerIndex < 0 || SecondLayerIndex > COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
			{
				std::string ErrorMessage = "Error: Second layer index is out of range. Please check the layer index and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				std::cout << ErrorMessage << std::endl;
				return;
			}

			COMPARE_LAYER_PRODUCER.SetShouldNormalize(CurrentComplexityJob->Settings.IsCompare_Normalize());
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(COMPARE_LAYER_PRODUCER.Calculate(FirstLayerIndex, SecondLayerIndex));

			std::cout << "Compare Layer calculation completed." << std::endl;
		}

		RunEvaluations(CurrentComplexityJob);
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