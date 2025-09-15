#pragma once

#include "../AnalysisObjectManager.h"
#include "../SubSystems/FELinesRenderer.h"
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

    void Init(int Dimensions, FEAABB AABB, float ResolutionInM = 0.0f);
    void FillCellsWithTriangleInfo();
	void FillCellsWithPointInfo();

    void MouseClick(double MouseX, double MouseY, glm::mat4 TransformMat = glm::identity<glm::mat4>());

    void FillMeasurementData();

    void UpdateRenderedLines();
    void RunOnAllNodes(std::function<void(GridNode* currentNode)> Func);
    void AddLinesOfGrid();

    bool IsInTriangleMode();
private:
    void InitializeSegment(size_t BeginIndex, size_t EndIndex, size_t Dimensions, FEAABB GridAABB, float CellSize);

    bool bUsingMultiThreading = true;
	bool bTriangleMode = true;

    void FillPerTriangleMeasurementData();
    void FillPerPointMeasurementData();

    struct GridThreadData
    {
        int FirstIndexInTriangleArray;
        int LastIndexInTriangleArray;
    };

    struct GridUpdateTask
    {
        int FirstIndex = -1;
        int SecondIndex = -1;
        int ThirdIndex = -1;
        int TriangleIndexToAdd = -1;

        GridUpdateTask::GridUpdateTask(int FirstIndex, int SecondIndex, int ThirdIndex, int TriangleIndexToAdd)
            : FirstIndex(FirstIndex), SecondIndex(SecondIndex), ThirdIndex(ThirdIndex), TriangleIndexToAdd(TriangleIndexToAdd) {}
    };

    void GridFillingThread(void* InputData, void* OutputData);
};