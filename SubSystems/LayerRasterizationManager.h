#pragma once

#include "ComplexityCore/Layers/LayerManager.h"
using namespace FocalEngine;

#pragma warning (disable: 4251) // for GDAL library
#include "GDAL/gdal.h"
#include "GDAL/gdal_priv.h"

class LayerRasterizationManager
{
public:
	SINGLETON_PUBLIC_PART(LayerRasterizationManager)

	void PrepareCurrentLayerForExport(MeshLayer* LayerToExport);
	void SaveToFile(const std::string FilePath);

	int GetGridRasterizationMode();
	void SetGridRasterizationMode(int NewValue);

	int GetGridResolution();
	void SetGridResolution(int NewValue);

	int GetCumulativeOutliers();
	void SetCumulativeOutliers(int NewValue);

	void SetOnCalculationsStartCallback(std::function<void()> Func);
	void SetOnCalculationsEndCallback(std::function<void()> Func);
	
	float GetProgress();

	void ClearAllData();
//private:
	SINGLETON_PRIVATE_PART(LayerRasterizationManager)

	struct GridCell
	{
		FEAABB AABB;
		std::vector<int> TrianglesInCell;
		std::vector<double> TrianglesInCellArea;
		float Value = 0.0f;
	};

	struct GridUpdateTask
	{
		int FirstIndex = -1;
		int SecondIndex = -1;
		int TriangleIndexToAdd = -1;
		double TriangleArea = -1.0;

		GridUpdateTask::GridUpdateTask(int FirstIndex, int SecondIndex, int TriangleIndexToAdd, double TriangleArea = -1.0)
			: FirstIndex(FirstIndex), SecondIndex(SecondIndex), TriangleIndexToAdd(TriangleIndexToAdd), TriangleArea(TriangleArea) {}
	};

	struct GridRasterizationThreadData
	{
		std::vector<std::vector<GridCell>>* Grid;
		glm::vec3 UpAxis;
		int Resolution;
		int FirstIndexInTriangleArray;
		int LastIndexInTriangleArray;
	};

	int THREAD_COUNT = 10;

	glm::vec3 ConvertToClosestAxis(const glm::vec3& Vector);
	std::vector<std::vector<GridCell>> GenerateGridProjection(FEAABB& OriginalAABB, const glm::vec3& Axis, int Resolution);
	static void GridRasterizationThread(void* InputData, void* OutputData);
	static void GatherGridRasterizationThreadWork(void* OutputData);

	void AfterAllGridRasterizationThreadFinished();

	std::vector<GridUpdateTask> MainThreadGridUpdateTasks;
	std::vector<std::vector<GridCell>> Grid;
	int GatherGridRasterizationThreadCount = 0;
	MeshLayer* CurrentLayer = nullptr;
	int CurrentResolution = 256;
	int GridRasterizationMode = 2;
	glm::vec3 CurrentUpAxis = glm::vec3(0.0f);
	const int GridRasterizationModeMin = 0;
	const int GridRasterizationModeMax = 1;
	const int GridRasterizationModeMean = 2;
	const int GridRasterizationModeCumulative = 3;
	float CumulativeOutliersUpper = 99.0f;
	float CumulativeOutliersLower = 0.5f;

	
	void PrepareRawImageData();
	bool b32BitsExport = false;
	bool bUsingCGAL = false;

	double GetArea(std::vector<glm::dvec3>& points);
	double GetTriangleIntersectionArea(int GridX, int GridY, int TriangleIndex);
	double Debug_ResultRawMin = 0.0;
	double Debug_ResultRawMax = 0.0;
	double Debug_ResultRawMean = 0.0;
	double Debug_ResultRawStandardDeviation = 0.0;

	double Debug_ResultRawSkewness = 0.0;
	double Debug_ResultRawKurtosis = 0.0;

	void UpdateGridDebugDistributionInfo();

	double Debug_TotalAreaUsed = 0.0;

	float Progress = 0.0f;
	static void OnCalculationsStart();
	static void OnCalculationsEnd();

	std::vector <std::function<void()>> OnCalculationsStartCallbacks;
	std::vector<std::function<void()>> OnCalculationsEndCallbacks;

	FETexture* ResultPreview = nullptr;

	void ClearDataAfterCalculation();
};

#define LAYER_RASTERIZATION_MANAGER LayerRasterizationManager::getInstance()