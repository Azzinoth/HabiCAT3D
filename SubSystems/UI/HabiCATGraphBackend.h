#pragma once
#include "../../SubSystems/FESceneGraphUI/BackendInterface.h"
#include "../AnalysisObjectManager.h"

#define ANALISYS_OBJECTS_DEPTH 1
#define PHOTOGRAMMETRY_ANCHOR_DEPTH (ANALISYS_OBJECTS_DEPTH + 1)
#define PHOTOGRAMMETRY_RESOURCES_DEPTH (PHOTOGRAMMETRY_ANCHOR_DEPTH + 1)

class HabiCATGraphBackend : public SceneGraphUI::BackendInterface
{
    std::string SceneID;
    FENaiveSceneGraph* Graph = nullptr;
public:
    HabiCATGraphBackend();

    bool IsReady() const override;
    bool IsAlive(SceneGraphUI::NodeHandle Node) override;

    void SetSceneID(std::string NewSceneID);
    SceneGraphUI::NodeHandle GetRoot() override;

    std::vector<SceneGraphUI::NodeHandle> GetChildren(SceneGraphUI::NodeHandle Node) override;
    SceneGraphUI::NodeHandle GetParent(SceneGraphUI::NodeHandle Node) override;

    SceneGraphUI::NodeHandle GetNodeByID(const std::string& ID) override;
    std::string GetNodeID(SceneGraphUI::NodeHandle Node)   override;

    std::string GetNodeName(SceneGraphUI::NodeHandle Node) override;
    std::string GetTag(SceneGraphUI::NodeHandle Node) override;

    bool MoveNode(SceneGraphUI::NodeHandle Node, SceneGraphUI::NodeHandle NewParent) override;
};