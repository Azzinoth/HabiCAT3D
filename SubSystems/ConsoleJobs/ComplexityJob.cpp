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