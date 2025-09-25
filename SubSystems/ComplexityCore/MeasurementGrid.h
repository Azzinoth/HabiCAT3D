#pragma once

#include "../AnalysisObjectManager.h"
#include "Layers/DataLayer.h"

struct GridNode
{
    FEAABB AABB;
    std::vector<int> TrianglesInCell;
	std::vector<int> PointsInCell;
    glm::vec3 AverageCellNormal = glm::vec3(0.0f);
    glm::vec3 CellTrianglesCentroid = glm::vec3(0.0f);
    double UserData = 0.0;

    bool bWasRenderedLastFrame = false;
    bool bSelected = false;
};

struct MeasurementGrid
{
    std::vector<std::vector<std::vector<GridNode>>> Data;
    glm::vec3 SelectedCell = glm::vec3(-1.0);
    float TimeTakenToGenerateInMS = 0.0f;
    float TimeTakenFillCellsWithTriangleInfo = 0.0f;
    float TimeTakenToCalculate = 0.0f;
    float TimeTakenToFillMeasurementData = 0.0f;
    std::vector<float> PerTriangleMeasurementData;
    std::vector<float> PerPointMeasurementData;
    int DebugTotalTrianglesInCells = 0;
	int DebugTotalPointsInCells = 0;
    int RenderingMode = 0;

    bool bFullyLoaded = false;
    bool bShowTrianglesInCells = true;

    MeasurementGrid();
    ~MeasurementGrid();

    void Init(FEAABB AABB, float ResolutionInM = 0.0f);
    void FillCellsWithTriangleInfo();
	void FillCellsWithPointInfo();

    void MouseClick(double MouseX, double MouseY, glm::mat4 TransformMat = glm::identity<glm::mat4>());

    void FillMeasurementData();

    void RunOnAllNodes(std::function<void(GridNode* CurrentNode)> Func);

    bool IsInTriangleMode();
	void UpdateLineRepresentation();
private:
    void InitializeSegment(size_t BeginIndex, size_t EndIndex, size_t Dimensions, FEAABB GridAABB, float CellSize);

    bool bUsingMultiThreading = true;
	bool bTriangleMode = true;

	FEEntity* GridLinesEntity = nullptr;

    void FillPerTriangleMeasurementData();
    void FillPerPointMeasurementData();

    struct GridThreadData
    {
        int FirstIndexInArray;
        int LastIndexInArray;
    };

    struct GridUpdateTask
    {
        int FirstIndex = -1;
        int SecondIndex = -1;
        int ThirdIndex = -1;
        int GeometryElementIndexToAdd = -1;

        GridUpdateTask::GridUpdateTask(int FirstIndex, int SecondIndex, int ThirdIndex, int GeometryElementIndexToAdd)
            : FirstIndex(FirstIndex), SecondIndex(SecondIndex), ThirdIndex(ThirdIndex), GeometryElementIndexToAdd(GeometryElementIndexToAdd) {}
    };

    void FillGridWithTrianglesDataThread(void* InputData, void* OutputData);
	void FillGridWithPointsDataThread(void* InputData, void* OutputData);
};