#include "TriangleEdgeLayerProducer.h"
using namespace FocalEngine;

TriangleEdgeLayerProducer::TriangleEdgeLayerProducer() {}
TriangleEdgeLayerProducer::~TriangleEdgeLayerProducer() {}

DataLayer TriangleEdgeLayerProducer::Calculate(int Mode)
{
	DataLayer Result(DATA_SOURCE_TYPE::MESH);
	Result.SetType(LAYER_TYPE::TRIANGLE_EDGE);

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		double Edge0Length = glm::distance(CurrentMeshAnalysisData->Triangles[i][0], CurrentMeshAnalysisData->Triangles[i][1]);
		double Edge1Length = glm::distance(CurrentMeshAnalysisData->Triangles[i][1], CurrentMeshAnalysisData->Triangles[i][2]);
		double Edge2Length = glm::distance(CurrentMeshAnalysisData->Triangles[i][2], CurrentMeshAnalysisData->Triangles[i][0]);

		if (Mode == 0)
		{
			Result.ElementsToData.push_back(static_cast<float>(std::max(Edge0Length, std::max(Edge1Length, Edge2Length))));
		}
		else if (Mode == 1)
		{
			Result.ElementsToData.push_back(static_cast<float>(std::min(Edge0Length, std::min(Edge1Length, Edge2Length))));
		}
		else if (Mode == 2)
		{
			Result.ElementsToData.push_back(static_cast<float>((Edge0Length + Edge1Length + Edge2Length) / 3.0));
		}
		else
		{
			Result.ElementsToData.push_back(static_cast<float>((Edge0Length + Edge1Length + Edge2Length) / 3.0));
		}
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle edge"));
	Result.DebugInfo = new DataLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StartTime);
	Result.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

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
	Result.DebugInfo->AddEntry("Mode", ModeUsed);

	return Result;
}