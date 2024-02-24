#include "VectorDispersionLayerProducer.h"
using namespace FocalEngine;

VectorDispersionLayerProducer* VectorDispersionLayerProducer::Instance = nullptr;
void(*VectorDispersionLayerProducer::OnCalculationsEndCallbackImpl)(MeshLayer) = nullptr;

VectorDispersionLayerProducer::VectorDispersionLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

VectorDispersionLayerProducer::~VectorDispersionLayerProducer() {}

void VectorDispersionLayerProducer::WorkOnNode(SDFNode* CurrentNode)
{
	if (CurrentNode->TrianglesInCell.empty())
		return;

	std::vector<double> NormalX;
	std::vector<double> NormalY;
	std::vector<double> NormalZ;

	for (size_t p = 0; p < CurrentNode->TrianglesInCell.size(); p++)
	{
		std::vector<glm::vec3> CurrentTriangleNormals = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals[CurrentNode->TrianglesInCell[p]];

		for (size_t l = 0; l < CurrentTriangleNormals.size(); l++)
		{
			NormalX.push_back(CurrentTriangleNormals[l][0]);
			NormalY.push_back(CurrentTriangleNormals[l][1]);
			NormalZ.push_back(CurrentTriangleNormals[l][2]);
		}
	}

	double meanX = std::accumulate(NormalX.begin(), NormalX.end(), 0.0) / NormalX.size();
	double meanY = std::accumulate(NormalY.begin(), NormalY.end(), 0.0) / NormalY.size();
	double meanZ = std::accumulate(NormalZ.begin(), NormalZ.end(), 0.0) / NormalZ.size();

	double sumX = std::inner_product(NormalX.begin(), NormalX.end(), NormalX.begin(), 0.0);
	double sumY = std::inner_product(NormalY.begin(), NormalY.end(), NormalY.begin(), 0.0);
	double sumZ = std::inner_product(NormalZ.begin(), NormalZ.end(), NormalZ.begin(), 0.0);

	double XPortionOfResult = sumX / NormalX.size() - meanX * meanX;
	if (isnan(XPortionOfResult))
		XPortionOfResult = 0.0;
	double YPortionOfResult = sumY / NormalY.size() - meanY * meanY;
	if (isnan(YPortionOfResult))
		YPortionOfResult = 0.0;
	double ZPortionOfResult = sumZ / NormalZ.size() - meanZ * meanZ;
	if (isnan(ZPortionOfResult))
		ZPortionOfResult = 0.0;

	double DoubleResult = sqrt(XPortionOfResult + YPortionOfResult + ZPortionOfResult);

	if (isnan(DoubleResult))
		DoubleResult = 0.0;

	CurrentNode->UserData = DoubleResult;
}

void VectorDispersionLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	JITTER_MANAGER.CalculateWithSDFJitterAsync(WorkOnNode, bSmootherResult);
}

void VectorDispersionLayerProducer::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	if (!VECTOR_DISPERSION_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	NewLayer.SetType(VECTOR_DISPERSION);

	VECTOR_DISPERSION_LAYER_PRODUCER.bWaitForJitterResult = false;
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(NewLayer);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Vector dispersion"));
	LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 1));

	if (VECTOR_DISPERSION_LAYER_PRODUCER.bCalculateStandardDeviation)
	{
		uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
		std::vector<float> TrianglesToStandardDeviation = JITTER_MANAGER.ProduceStandardDeviationData();
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(TrianglesToStandardDeviation);
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));

		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo = new MeshLayerDebugInfo();
		MeshLayerDebugInfo* DebugInfo = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo;
		DebugInfo->Type = "VectorDispersionDeviationLayerDebugInfo";
		DebugInfo->AddEntry("Start time", StarTime);
		DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
		DebugInfo->AddEntry("Source layer ID", COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 2].GetID());
		DebugInfo->AddEntry("Source layer caption", COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 2].GetCaption());
	}

	if (OnCalculationsEndCallbackImpl != nullptr)
		OnCalculationsEndCallbackImpl(NewLayer);
}

void VectorDispersionLayerProducer::RenderDebugInfoForSelectedNode(SDF* Grid)
{
	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	Grid->UpdateRenderedLines();

	SDFNode* CurrentlySelectedCell = &Grid->Data[int(Grid->SelectedCell.x)][int(Grid->SelectedCell.y)][int(Grid->SelectedCell.z)];
	for (size_t i = 0; i < CurrentlySelectedCell->TrianglesInCell.size(); i++)
	{
		const auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[CurrentlySelectedCell->TrianglesInCell[i]];

		std::vector<glm::vec3> TranformedTrianglePoints = CurrentTriangle;
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = MESH_MANAGER.ActiveMesh->Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
	}

	LINE_RENDERER.SyncWithGPU();
}

void VectorDispersionLayerProducer::CalculateOnWholeModel()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	JITTER_MANAGER.CalculateOnWholeModel(WorkOnNode);
}

void VectorDispersionLayerProducer::SetOnCalculationsEndCallback(void(*Func)(MeshLayer))
{
	OnCalculationsEndCallbackImpl = Func;
}