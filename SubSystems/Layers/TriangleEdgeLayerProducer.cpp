#include "TriangleEdgeLayerProducer.h"
using namespace FocalEngine;

TriangleEdgeLayerProducer* TriangleEdgeLayerProducer::Instance = nullptr;

TriangleEdgeLayerProducer::TriangleEdgeLayerProducer() {}
TriangleEdgeLayerProducer::~TriangleEdgeLayerProducer() {}

MeshLayer TriangleEdgeLayerProducer::Calculate(FEMesh* Mesh, int Mode)
{
	MeshLayer Result;
	Result.SetType(TRIANGLE_EDGE);

	if (Mesh == nullptr)
		return Result;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	for (size_t i = 0; i < Mesh->Triangles.size(); i++)
	{
		float Edge0Length = glm::distance(Mesh->Triangles[i][0], Mesh->Triangles[i][1]);
		float Edge1Length = glm::distance(Mesh->Triangles[i][1], Mesh->Triangles[i][2]);
		float Edge2Length = glm::distance(Mesh->Triangles[i][2], Mesh->Triangles[i][0]);

		if (Mode == 0)
		{
			Result.TrianglesToData.push_back(std::max(Edge0Length, std::max(Edge1Length, Edge2Length)));
		}
		else if (Mode == 1)
		{
			Result.TrianglesToData.push_back(std::min(Edge0Length, std::min(Edge1Length, Edge2Length)));
		}
		else if (Mode == 2)
		{
			Result.TrianglesToData.push_back((Edge0Length + Edge1Length + Edge2Length) / 3.0f);
		}
		else
		{
			Result.TrianglesToData.push_back((Edge0Length + Edge1Length + Edge2Length) / 3.0f);
		}
	}
	
	Result.SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Triangle edge"));
	Result.DebugInfo = new MeshLayerDebugInfo();

	Result.DebugInfo->AddEntry("Start time", StarTime);
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
	Result.DebugInfo->AddEntry("Mode: ", ModeUsed);

	return Result;
}