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

	enum GridRasterizationMode { Min = 0, Max = 1, Mean = 2, Cumulative = 3 };
	enum SaveMode { SaveAsPNG = 0, SaveAsTIF = 1, SaveAs32bitTIF = 2 };
	
	bool SaveToFile(std::string FilePath, SaveMode SaveMode = SaveAsPNG);
	bool PromptUserForSaveLocation();

	GridRasterizationMode GetGridRasterizationMode();
	void SetGridRasterizationMode(GridRasterizationMode NewValue);

	int GetGridResolution();
	float GetResolutionInMeters();
	void SetResolutionInMeters(float NewValue);
	float GetResolutionInMetersThatWouldGiveSuchResolutionInPixels(int Pixels);
	int GetResolutionInPixelsThatWouldGiveSuchResolutionInMeters(float Meters);
	glm::vec2 GetMinMaxResolutionInMeters(glm::vec3 ProjectionVector = glm::vec3(0.0f));
	glm::vec3 GetProjectionVector();
	float GetProgress();

	float GetCumulativeModePersentOfAreaThatWouldBeRed();
	void SetCumulativeModePersentOfAreaThatWouldBeRed(float NewValue);
	void ActivateAutomaticOutliersSuppression();

	// Callback setters
	void SetOnCalculationsStartCallback(std::function<void()> Func);
	void SetOnCalculationsEndCallback(std::function<void()> Func);
	
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

	const int RASTERIZATION_MIN_RESOLUTION = 16;
	const int RASTERIZATION_MAX_RESOLUTION = 4096;

	glm::vec3 ConvertToClosestAxis(const glm::vec3& Vector);
	std::vector<std::vector<GridCell>> GenerateGridProjection(const glm::vec3& Axis);
	static void GridRasterizationThread(void* InputData, void* OutputData);
	static void GatherGridRasterizationThreadWork(void* OutputData);

	void AfterAllGridRasterizationThreadFinished();

	std::vector<GridUpdateTask> MainThreadGridUpdateTasks;
	std::vector<std::vector<GridCell>> Grid;
	int GatherGridRasterizationThreadCount = 0;
	MeshLayer* CurrentLayer = nullptr;
	int CurrentResolution = 256;
	float CurrentResolutionInMeters = 1.0f;
	GridRasterizationMode Mode = GridRasterizationMode::Max;
	glm::vec3 CurrentProjectionVector = glm::vec3(0.0f);
	float PersentOfAreaThatWouldBeRed = 5.0f;
	
	std::vector<float> ImageRawData32Bits;
	void PrepareRawImageData();
	bool bUsingCGAL = true;

	Point_2 ProjectPointOntoPlane(const Point_3& Point, const Plane_3& Plane);
	glm::dvec3 CalculateCentroid(const std::vector<glm::dvec3>& Points);
	bool CompareAngles(const glm::dvec3& A, const glm::dvec3& B, const glm::dvec3& Centroid);
	void SortPointsByAngle(std::vector<glm::dvec3>& Points);
	double CalculatePolygonArea(const std::vector<glm::dvec2>& GlmPoints);
	double GetArea(std::vector<glm::dvec3>& Points);
	double GetTriangleIntersectionArea(size_t GridX, size_t GridY, int TriangleIndex);

	// Debug related
	double Debug_ResultRawMin = 0.0;
	double Debug_ResultRawMax = 0.0;
	double Debug_ResultRawMean = 0.0;
	double Debug_ResultRawStandardDeviation = 0.0;
	double Debug_ResultRawSkewness = 0.0;
	double Debug_ResultRawKurtosis = 0.0;
	double Debug_TotalAreaUsed = 0.0;
	glm::vec2 DebugSelectedCell = glm::vec2(-1.0);

	bool bDebugShowOnlyCellsWithTriangles = true;
	bool bDebugShowOnlySelectedCells = false;

	void ShowDebugWindow();
	void DebugMouseClick();
	void UpdateGridDebugDistributionInfo();
	void DebugSelectCell(int X, int Y);
	void DebugRenderGrid();

	float Progress = 0.0f;
	static void OnCalculationsStart();
	static void OnCalculationsEnd();

	std::vector<std::function<void()>> OnCalculationsStartCallbacks;
	std::vector<std::function<void()>> OnCalculationsEndCallbacks;

	FETexture* ResultPreview = nullptr;

	void ClearDataAfterCalculation();
	void UpdateProjectionVector();

	glm::uvec2 GetResolutionInPixelsBasedOnResolutionInMeters(glm::vec3 ProjectionVector, float ResolutionInMeters);

	bool GLMVec3Equal(const glm::vec3& A, const glm::vec3& B);
};

#define LAYER_RASTERIZATION_MANAGER LayerRasterizationManager::getInstance()