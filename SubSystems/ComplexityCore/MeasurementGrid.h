#pragma once

#include "../MeshManager.h"
#include "../SubSystems/FELinesRenderer.h"
#include "ComplexityMetricManager.h"

struct GridNode
{
    FEAABB AABB;
    std::vector<int> TrianglesInCell;
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
    float TimeTakenToFillMeshWithUserData = 0.0f;
    std::vector<float> TrianglesUserData;
    int DebugTotalTrianglesInCells = 0;
    int RenderingMode = 0;

    bool bFullyLoaded = false;
    bool bShowTrianglesInCells = true;

    MeasurementGrid();
    ~MeasurementGrid();

    void Init(int Dimensions, FEAABB AABB, float ResolutionInM = 0.0f);
    void FillCellsWithTriangleInfo();
    void MouseClick(double MouseX, double MouseY, glm::mat4 TransformMat = glm::identity<glm::mat4>());
    void FillMeshWithUserData();
    void UpdateRenderedLines();
    void RunOnAllNodes(std::function<void(GridNode* currentNode)> Func);
    void AddLinesOfGrid();

private:
    void InitializeSegment(size_t beginIndex, size_t endIndex, size_t Dimensions, FEAABB GridAABB, float CellSize);

    bool bUsingMultiThreading = true;

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