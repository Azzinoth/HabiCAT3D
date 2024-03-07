#pragma once

#include "ComplexityCore/Layers/LayerManager.h"
using namespace FocalEngine;

class LayerRasterizationManager
{
public:
	SINGLETON_PUBLIC_PART(LayerRasterizationManager)

	void ExportCurrentLayerAsMap(MeshLayer* LayerToExport);

	int GetGridRasterizationMode();
	void SetGridRasterizationMode(int NewValue);

	int GetGridResolution();
	void SetGridResolution(int NewValue);

	int GetCumulativeOutliers();
	void SetCumulativeOutliers(int NewValue);
	
private:
	SINGLETON_PRIVATE_PART(LayerRasterizationManager)

	struct GridCell
	{
		FEAABB AABB;
		std::vector<int> TrianglesInCell;
		float Value = 0.0f;
	};

	struct GridUpdateTask
	{
		int FirstIndex = -1;
		int SecondIndex = -1;
		int TriangleIndexToAdd = -1;

		GridUpdateTask::GridUpdateTask(int FirstIndex, int SecondIndex, int TriangleIndexToAdd)
			: FirstIndex(FirstIndex), SecondIndex(SecondIndex), TriangleIndexToAdd(TriangleIndexToAdd)
		{
		}
	};

	struct GridRasterizationThreadData
	{
		std::vector<std::vector<GridCell>>* Grid;
		glm::vec3 UpAxis;
		int Resolution;
		int FirstIndexInTriangleArray;
		int LastIndexInTriangleArray;
	};

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
	int CumulativeOutliers = 1;
};

#define LAYER_RASTERIZATION_MANAGER LayerRasterizationManager::getInstance()