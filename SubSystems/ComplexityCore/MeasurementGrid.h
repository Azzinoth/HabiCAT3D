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
};