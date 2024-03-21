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