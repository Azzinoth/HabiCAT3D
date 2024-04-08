#include "ComplexityJob.h"
using namespace FocalEngine;

ComplexityJob::ComplexityJob()
{
	Type = "COMPLEXITY_JOB";
	ComplexityType = "NONE";
}

ComplexityJob::ComplexityJob(std::string ComplexityType, ComplexityJobSettings Settings)
{
	this->ComplexityType = ComplexityType;
	this->Settings = Settings;

	Type = "COMPLEXITY_JOB";
}

ComplexityJob* ComplexityJob::CreateInstance(CommandLineAction ActionToParse)
{
	ComplexityJob* Result = nullptr;

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

	Result = new ComplexityJob();
	Result->ComplexityType = Type;

	if (ActionToParse.Settings.find("resolution") != ActionToParse.Settings.end())
	{
		Result->Settings.SetResolutionInM(std::stof(ActionToParse.Settings["resolution"]));
	}

	if (ActionToParse.Settings.find("relative_resolution") != ActionToParse.Settings.end())
	{
		Result->Settings.SetRelativeResolution(std::stof(ActionToParse.Settings["relative_resolution"]));
	}

	if (ActionToParse.Settings.find("jitter_quality") != ActionToParse.Settings.end())
	{
		Result->Settings.SetJitterQuality(ActionToParse.Settings["jitter_quality"]);
	}

	if (ActionToParse.Settings.find("run_on_whole_model") != ActionToParse.Settings.end())
	{
		Result->Settings.SetRunOnWholeModel(ActionToParse.Settings["run_on_whole_model"] == "TRUE" ? true : false);
	}

	if (ActionToParse.Settings.find("rugosity_algorithm") != ActionToParse.Settings.end())
	{
		Result->Settings.SetRugosity_Algorithm(ActionToParse.Settings["rugosity_algorithm"]);
	}

	if (ActionToParse.Settings.find("rugosity_min_algorithm_quality") != ActionToParse.Settings.end())
	{
		Result->Settings.SetRugosity_MinAlgorithm_Quality(ActionToParse.Settings["rugosity_min_algorithm_quality"]);
	}

	if (ActionToParse.Settings.find("rugosity_is_using_unique_projected_area") != ActionToParse.Settings.end())
	{
		Result->Settings.SetRugosity_IsUsingUniqueProjectedArea(ActionToParse.Settings["rugosity_is_using_unique_projected_area"] == "TRUE" ? true : false);
	}

	if (ActionToParse.Settings.find("rugosity_delete_outliers") != ActionToParse.Settings.end())
	{
		Result->Settings.SetRugosity_DeleteOutliers(ActionToParse.Settings["rugosity_delete_outliers"] == "TRUE" ? true : false);
	}

	if (ActionToParse.Settings.find("fractal_dimension_should_filter_values") != ActionToParse.Settings.end())
	{
		Result->Settings.SetFractalDimension_ShouldFilterValues(ActionToParse.Settings["fractal_dimension_should_filter_values"] == "TRUE" ? true : false);
	}

	if (ActionToParse.Settings.find("triangle_edges_mode") != ActionToParse.Settings.end())
	{
		Result->Settings.SetTriangleEdges_Mode(ActionToParse.Settings["triangle_edges_mode"]);
	}

	if (ActionToParse.Settings.find("is_standard_deviation_needed") != ActionToParse.Settings.end())
	{
		Result->Settings.SetIsStandardDeviationNeeded(ActionToParse.Settings["is_standard_deviation_needed"] == "TRUE" ? true : false);
	}

	if (ActionToParse.Settings.find("compare_first_layer_index") != ActionToParse.Settings.end())
	{
		Result->Settings.SetCompare_FirstLayerIndex(std::stoi(ActionToParse.Settings["compare_first_layer_index"]));
	}

	if (ActionToParse.Settings.find("compare_second_layer_index") != ActionToParse.Settings.end())
	{
		Result->Settings.SetCompare_SecondLayerIndex(std::stoi(ActionToParse.Settings["compare_second_layer_index"]));
	}

	if (ActionToParse.Settings.find("compare_normalize") != ActionToParse.Settings.end())
	{
		Result->Settings.SetCompare_Normalize(ActionToParse.Settings["compare_normalize"] == "TRUE" ? true : false);
	}

	return Result;
}

ConsoleJobInfo ComplexityJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "complexity";
	Info.Purpose = "Creates a job to add complexity layer of a model based on the specified type.";
	ConsoleJobSettingsInfo CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "type";
	CurrentSettingInfo.Description = "Specifies the type of complexity calculation.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "HEIGHT", "AREA", "RUGOSITY", "TRIANGLE_EDGE", "TRIANGLE_COUNT", "VECTOR_DISPERSION", "FRACTAL_DIMENSION", "COMPARE" };
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "resolution";
	CurrentSettingInfo.Description = "Specifies the resolution in meters for the complexity calculation. Alternative to relative_resolution.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "Minimal possible";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "relative_resolution";
	CurrentSettingInfo.Description = "Specifies the resolution as a float between 0.0 and 1.0, where 0.0 represents the lowest possible resolution and 1.0 represents the highest. Alternative to resolution.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "0.0";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "jitter_quality";
	CurrentSettingInfo.Description = "Specifies the quality of jitter applied to the model. Higher values mean more jitters and potentially smoother results but slower processing.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "55";
	CurrentSettingInfo.PossibleValues = JITTER_MANAGER.GetJitterVectorSetNames();
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "run_on_whole_model";
	CurrentSettingInfo.Description = "Specifies if the calculation should be run on the whole model without jitter.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "triangle_edges_mode";
	CurrentSettingInfo.Description = "Specifies the mode of triangle edges calculation. Relevant only for 'TRIANGLE_EDGE' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "MAX_LEHGTH";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "rugosity_algorithm";
	CurrentSettingInfo.Description = "Specifies the algorithm for rugosity calculation. Relevant only for 'RUGOSITY' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "AVERAGE";
	CurrentSettingInfo.PossibleValues = { "AVERAGE", "MIN", "LSF(CGAL)" };
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "rugosity_is_using_unique_projected_area";
	CurrentSettingInfo.Description = "Specifies if the unique projected area should be used for rugosity calculation. Relevant only for 'RUGOSITY' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "rugosity_delete_outliers";
	CurrentSettingInfo.Description = "Specifies if the outliers should be deleted from the rugosity calculation. Relevant only for 'RUGOSITY' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "true";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "rugosity_min_algorithm_quality";
	CurrentSettingInfo.Description = "Specifies the quality of the rugosity calculation. Relevant only for 'RUGOSITY' complexity type and when the rugosity_algorithm is set to 'MIN'.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "91";
	CurrentSettingInfo.PossibleValues = RUGOSITY_LAYER_PRODUCER.GetOrientationSetNamesForMinRugosityList();
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "fractal_dimension_should_filter_values";
	CurrentSettingInfo.Description = "Specifies if the app should filter values that are less that 2.0. Relevant only for 'FRACTAL_DIMENSION' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "true";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "is_standard_deviation_needed";
	CurrentSettingInfo.Description = "Specifies if the app should also add layer with standard deviation.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "compare_first_layer_index";
	CurrentSettingInfo.Description = "Specifies the index of the first layer to compare. Relevant only for 'COMPARE' complexity type.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "compare_second_layer_index";
	CurrentSettingInfo.Description = "Specifies the index of the second layer to compare. Relevant only for 'COMPARE' complexity type.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "compare_normalize";
	CurrentSettingInfo.Description = "Specifies if the app should normalize the layers before comparing. Relevant only for 'COMPARE' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "true";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	return Info;
}

std::string ComplexityJob::GetComplexityType()
{
	return ComplexityType;
}

void ComplexityJob::SetComplexityType(std::string NewValue)
{
	ComplexityType = NewValue;
}

void ComplexityJob::WaitForJitterManager()
{
	while (JITTER_MANAGER.GetJitterDoneCount() != JITTER_MANAGER.GetJitterToDoCount())
	{
		float Progress = float(JITTER_MANAGER.GetJitterDoneCount()) / float(JITTER_MANAGER.GetJitterToDoCount());
		std::cout << "\rProgress: " << std::to_string(Progress * 100.0f) << " %" << std::flush;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		THREAD_POOL.Update();
	}
}

bool ComplexityJob::SetGridResolution()
{
	if (Settings.ResolutionInM == 0.0f && Settings.RelativeResolution == 0.0f)
	{
		JITTER_MANAGER.SetResolutionInM(JITTER_MANAGER.GetLowestPossibleResolution());
		return true;
	}

	if (Settings.ResolutionInM != 0.0f &&
		Settings.ResolutionInM >= JITTER_MANAGER.GetLowestPossibleResolution() &&
		Settings.ResolutionInM <= JITTER_MANAGER.GetHigestPossibleResolution())
	{
		JITTER_MANAGER.SetResolutionInM(Settings.ResolutionInM);
		return true;
	}

	if (Settings.ResolutionInM == 0.0f && Settings.RelativeResolution != 0.0f)
	{
		float Range = JITTER_MANAGER.GetHigestPossibleResolution() - JITTER_MANAGER.GetLowestPossibleResolution();
		JITTER_MANAGER.SetResolutionInM(JITTER_MANAGER.GetLowestPossibleResolution() + Range * Settings.RelativeResolution);
		return true;
	}

	return false;
}

void ComplexityJob::SetRugosityAlgorithm()
{
	if (ComplexityType != "RUGOSITY")
		return;

	RUGOSITY_LAYER_PRODUCER.SetUseFindSmallestRugosity(Settings.GetRugosity_Algorithm() == "MIN");
	RUGOSITY_LAYER_PRODUCER.SetUseCGALVariant(Settings.GetRugosity_Algorithm() == "LSF(CGAL)");
}

bool ComplexityJob::Execute(void* InputData, void* OutputData)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
	{
		std::string ErrorMessage = "Error: No file loaded. Please load a file before attempting to calculate complexity.";
		OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
		return false;
	}

	if (!SetGridResolution())
	{
		std::string ErrorMessage = "Error: Invalid resolution value. Given value is - " + std::to_string(Settings.ResolutionInM) + ". Bur value should be between " + std::to_string(JITTER_MANAGER.GetLowestPossibleResolution()) + " and " + std::to_string(JITTER_MANAGER.GetHigestPossibleResolution()) + ".";
		OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
		return false;
	}
	JITTER_MANAGER.SetCurrentJitterVectorSetName(Settings.GetJitterQuality());

	if (ComplexityType == "HEIGHT")
	{
		std::cout << "Initiating Height Layer calculation." << std::endl;

		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(HEIGHT_LAYER_PRODUCER.Calculate());

		std::cout << "Height Layer calculation completed." << std::endl;
	}
	else if (ComplexityType == "AREA")
	{
		std::cout << "Initiating Area Layer calculation." << std::endl;

		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(AREA_LAYER_PRODUCER.Calculate());

		std::cout << "Area Layer calculation completed." << std::endl;
	}
	else if (ComplexityType == "TRIANGLE_EDGE")
	{
		std::cout << "Initiating Triangle Edge Layer calculation." << std::endl;

		int Mode = 0;
		if (Settings.GetTriangleEdges_Mode() == "MIN_LEHGTH")
		{
			Mode = 1;
		}
		else if (Settings.GetTriangleEdges_Mode() == "MEAN_LEHGTH")
		{
			Mode = 2;
		}

		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(TRIANGLE_EDGE_LAYER_PRODUCER.Calculate(Mode));

		std::cout << "Triangle Edge Layer calculation completed." << std::endl;
	}
	else if (ComplexityType == "TRIANGLE_COUNT")
	{
		std::cout << "Initiating Triangle Count Layer calculation." << std::endl;

		if (Settings.IsRunOnWholeModel())
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
	else if (ComplexityType == "RUGOSITY")
	{
		std::cout << "Initiating Rugosity Layer calculation." << std::endl;

		SetRugosityAlgorithm();
		RUGOSITY_LAYER_PRODUCER.SetCalculateStandardDeviation(Settings.IsStandardDeviationNeeded());
		RUGOSITY_LAYER_PRODUCER.SetDeleteOutliers(Settings.IsRugosity_DeleteOutliers());

		RUGOSITY_LAYER_PRODUCER.SetOrientationSetForMinRugosityName(Settings.GetRugosity_MinAlgorithm_Quality());
		RUGOSITY_LAYER_PRODUCER.SetIsUsingUniqueProjectedArea(Settings.GetRugosity_IsUsingUniqueProjectedArea());

		if (Settings.IsRunOnWholeModel())
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
	else if (ComplexityType == "VECTOR_DISPERSION")
	{
		std::cout << "Initiating Vector Dispersion calculation." << std::endl;

		VECTOR_DISPERSION_LAYER_PRODUCER.SetShouldCalculateStandardDeviation(Settings.IsStandardDeviationNeeded());

		if (Settings.IsRunOnWholeModel())
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
	else if (ComplexityType == "FRACTAL_DIMENSION")
	{
		std::cout << "Initiating Fractal Dimension Layer calculation." << std::endl;

		FRACTAL_DIMENSION_LAYER_PRODUCER.SetShouldCalculateStandardDeviation(Settings.IsStandardDeviationNeeded());
		FRACTAL_DIMENSION_LAYER_PRODUCER.SetShouldFilterFractalDimensionValues(Settings.GetFractalDimension_ShouldFilterValues());

		if (Settings.IsRunOnWholeModel())
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
	else if (ComplexityType == "COMPARE")
	{
		std::cout << "Initiating Compare Layer calculation." << std::endl;

		int FirstLayerIndex = Settings.GetCompare_FirstLayerIndex();
		if (FirstLayerIndex < 0 || FirstLayerIndex > COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
		{
			std::string ErrorMessage = "Error: First layer index is out of range. Please check the layer index and try again.";
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return false;
		}

		int SecondLayerIndex = Settings.GetCompare_SecondLayerIndex();
		if (SecondLayerIndex < 0 || SecondLayerIndex > COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
		{
			std::string ErrorMessage = "Error: Second layer index is out of range. Please check the layer index and try again.";
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return false;
		}

		COMPARE_LAYER_PRODUCER.SetShouldNormalize(Settings.IsCompare_Normalize());
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(COMPARE_LAYER_PRODUCER.Calculate(FirstLayerIndex, SecondLayerIndex));

		std::cout << "Compare Layer calculation completed." << std::endl;
	}

	return true;
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