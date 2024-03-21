#pragma once

#include "ConsoleJob.h"
using namespace FocalEngine;

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
	friend ConsoleJobManager;

	std::string ComplexityType;

	static ConsoleJobInfo GetInfo();
public:
	ComplexityJob();
	ComplexityJob(std::string ComplexityType, ComplexityJobSettings Settings);

	std::string GetComplexityType();
	void SetComplexityType(std::string NewValue);

	ComplexityJobSettings Settings;
};