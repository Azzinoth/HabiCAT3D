#pragma once
#include "HabiCATGraphBackend.h"

HabiCATGraphBackend::HabiCATGraphBackend() {}

bool HabiCATGraphBackend::IsReady() const
{
    FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
    if (Scene == nullptr)
        return false;

    return Graph != nullptr;
}

void HabiCATGraphBackend::SetSceneID(std::string NewSceneID)
{
    SceneID = NewSceneID;
    Graph = &SCENE_MANAGER.GetSceneByID(SceneID)->SceneGraph;
}

SceneGraphUI::NodeHandle HabiCATGraphBackend::GetRoot()
{
    return { Graph->GetRoot(), this };
}

std::vector<SceneGraphUI::NodeHandle> HabiCATGraphBackend::GetChildren(SceneGraphUI::NodeHandle Node)
{
    std::vector<SceneGraphUI::NodeHandle> Result;
    for (FENaiveSceneGraphNode* Child : Node.As<FENaiveSceneGraphNode>()->GetChildren())
        Result.push_back({ Child, this });

    return Result;
}

SceneGraphUI::NodeHandle HabiCATGraphBackend::GetParent(SceneGraphUI::NodeHandle Node)
{
    return { Node.As<FENaiveSceneGraphNode>()->GetParent(), this };
}

SceneGraphUI::NodeHandle HabiCATGraphBackend::GetNodeByID(const std::string& ID)
{
    FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
    if (Scene == nullptr)
        return { nullptr, this };

    return { Scene->SceneGraph.GetNodeByID(ID), this };
}

std::string HabiCATGraphBackend::GetNodeID(SceneGraphUI::NodeHandle Node)
{
    return Node.As<FENaiveSceneGraphNode>()->GetObjectID();
}

std::string HabiCATGraphBackend::GetNodeName(SceneGraphUI::NodeHandle Node)
{
	FENaiveSceneGraphNode* CurrentNode = Node.As<FENaiveSceneGraphNode>();
    size_t Depth = CurrentNode->GetDepth();

    std::string DisplayedName = CurrentNode->GetName();
    FEEntity* Entity = CurrentNode->GetEntity();
	if (Entity == nullptr)
        return "";

    std::string EntityName = Entity->GetName();
    if (Depth == ANALISYS_OBJECTS_DEPTH)
    {
        AnalysisObject* CurrentAnalysisObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(Entity->GetObjectID());
        if (CurrentAnalysisObject != nullptr)
            DisplayedName = CurrentAnalysisObject->GetName();
    }
    if (Depth == ANALISYS_OBJECTS_DEPTH + 1)
    {
        if (EntityName.find("Annotation") != std::string::npos)
        {
            DisplayedName = "Annotations";
        }
        else
        {
            DisplayedName = "Photogrammetry";
        }
    }
    if (Depth == PHOTOGRAMMETRY_RESOURCES_DEPTH)
    {
        if (EntityName.find("COLMAPImages") != std::string::npos)
        {
            DisplayedName = "Images";
        }
        else if (EntityName.find("Tie Points") != std::string::npos)
        {
            DisplayedName = "Tie Points";
        }
    }

    return DisplayedName;
}

std::string HabiCATGraphBackend::GetTag(SceneGraphUI::NodeHandle Node)
{
    FEEntity* Entity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
    if (Entity == nullptr)
        return "";

    return Entity->GetTag();
}

bool HabiCATGraphBackend::MoveNode(SceneGraphUI::NodeHandle Node, SceneGraphUI::NodeHandle NewParent)
{
    return Graph->MoveNode(Node.GetID(), NewParent.GetID());
}

bool HabiCATGraphBackend::IsAlive(SceneGraphUI::NodeHandle Node)
{
    if (!Node)
        return false;

    FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
    if (Scene == nullptr)
        return false;

    FENaiveSceneGraphNode* CurrentNode = Scene->SceneGraph.GetNodeByID(Node.GetID());
    if (CurrentNode == nullptr)
        return false;

    return true;
}