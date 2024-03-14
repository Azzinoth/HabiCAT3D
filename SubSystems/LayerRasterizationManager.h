#pragma once

#include "ComplexityCore/Layers/LayerManager.h"
using namespace FocalEngine;

#include "CGALDeclarations.h"

#pragma warning (disable: 4251) // for GDAL library
#include "GDAL/gdal.h"
#include "GDAL/gdal_priv.h"

class LayerRasterizationManager
{
public:
	SINGLETON_PUBLIC_PART(LayerRasterizationManager)

	void PrepareCurrentLayerForExport(MeshLayer* LayerToExport, glm::vec3 ForceProjectionVector = glm::vec3(0.0f));

	enum GridRasterizationMode
	{
		Min = 0,
		Max = 1,
		Mean = 2,
		Cumulative = 3
	};

	enum SaveMode
	{
		SaveAsPNG = 0,
		SaveAsTIF = 1,
		SaveAs32bitTIF = 2
	};
	
	bool SaveToFile(std::string FilePath, SaveMode SaveMode = SaveAsPNG);
	bool PromptUserForSaveLocation();

	GridRasterizationMode GetGridRasterizationMode();
	void SetGridRasterizationMode(GridRasterizationMode NewValue);

	int GetGridResolution();
	void SetGridResolution(int NewValue);

	glm::vec3 GetProjectionVector();

	float GetCumulativeModeLowerOutlierPercentile();
	void SetCumulativeModeLowerOutlierPercentile(float NewValue);

	float GetCumulativeModeUpperOutlierPercentile();
	void SetCumulativeModeUpperOutlierPercentile(float NewValue);

	void SetOnCalculationsStartCallback(std::function<void()> Func);
	void SetOnCalculationsEndCallback(std::function<void()> Func);
	
	float GetProgress();

	int GetTexturePreviewID();

	void ClearAllData();
private:
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
	GridRasterizationMode Mode = GridRasterizationMode::Max;
	glm::vec3 CurrentProjectionVector = glm::vec3(0.0f);
	float CumulativeModeUpperOutlierPercentile = 99.0f;
	float CumulativeModeLowerOutlierPercentile = 0.5f;
	
	void PrepareRawImageData();
	bool bUsingCGAL = true;

	glm::dvec3 CalculateCentroid(const std::vector<glm::dvec3>& points);
	bool CompareAngles(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& centroid);
	void SortPointsByAngle(std::vector<glm::dvec3>& points);
	double CalculatePolygonArea(const std::vector<glm::dvec2>& glmPoints);
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

	std::vector<std::function<void()>> OnCalculationsStartCallbacks;
	std::vector<std::function<void()>> OnCalculationsEndCallbacks;

	FETexture* ResultPreview = nullptr;

	void ClearDataAfterCalculation();
};

#define LAYER_RASTERIZATION_MANAGER LayerRasterizationManager::getInstance()