#include "TriangleEdgeLayerProducer.h"
using namespace FocalEngine;

TriangleEdgeLayerProducer::TriangleEdgeLayerProducer() {}
TriangleEdgeLayerProducer::~TriangleEdgeLayerProducer() {}

DataLayer* TriangleEdgeLayerProducer::Calculate(int Mode)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return nullptr;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(ActiveObject->GetAnalysisData());
	if (CurrentMeshAnalysisData == nullptr)
		return nullptr;

	DataLayer* NewLayer = new DataLayer({ ActiveObject->GetID() });
	NewLayer->SetType(LAYER_TYPE::TRIANGLE_EDGE);

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		double Edge0Length = glm::distance(CurrentMeshAnalysisData->Triangles[i][0], CurrentMeshAnalysisData->Triangles[i][1]);
		double Edge1Length = glm::distance(CurrentMeshAnalysisData->Triangles[i][1], CurrentMeshAnalysisData->Triangles[i][2]);
		double Edge2Length = glm::distance(CurrentMeshAnalysisData->Triangles[i][2], CurrentMeshAnalysisData->Triangles[i][0]);

		if (Mode == 0)
		{
			NewLayer->ElementsToData.push_back(static_cast<float>(std::max(Edge0Length, std::max(Edge1Length, Edge2Length))));
		}
		else if (Mode == 1)
		{
			NewLayer->ElementsToData.push_back(static_cast<float>(std::min(Edge0Length, std::min(Edge1Length, Edge2Length))));
		}
		else if (Mode == 2)
		{
			NewLayer->ElementsToData.push_back(static_cast<float>((Edge0Length + Edge1Length + Edge2Length) / 3.0));
		}
		else
		{
			NewLayer->ElementsToData.push_back(static_cast<float>((Edge0Length + Edge1Length + Edge2Length) / 3.0));
		}
	}
	
	NewLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle edge"));
	NewLayer->DebugInfo = new DataLayerDebugInfo();

	NewLayer->DebugInfo->AddEntry("Start time", StartTime);
	NewLayer->DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	std::string ModeUsed = "Unknown.";
	if (Mode == 0)
	{
		ModeUsed = "Max edge length.";
	}
	else if (Mode == 1)
	{
		ModeUsed = "Min edge length.";
	}
	else if (Mode == 2)
	{
		ModeUsed = "Mean edge length.";
	}
	NewLayer->DebugInfo->AddEntry("Mode", ModeUsed);

	return NewLayer;
}