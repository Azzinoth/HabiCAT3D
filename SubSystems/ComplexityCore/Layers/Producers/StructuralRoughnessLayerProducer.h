#pragma once

/*	Structural Roughness calculation implementation based on the method described in:
	Y.-F. Gu, J. Hu, K. Han, J. W. Lau and G. A. Williams, "HSC3D: A Python package 
	to quantify three-dimensional habitat structural complexity," Methods in Ecology 
	and Evolution, vol. 15, no. 4, pp. 639-646, 2024. */
#include "../LayerManager.h"
using namespace FocalEngine;

class StructuralRoughnessLayerProducer
{
public:
	SINGLETON_PUBLIC_PART(StructuralRoughnessLayerProducer)

	void CalculateWithJitterAsync(bool bSmootherResult);
	void RenderDebugInfoForSelectedNode(MeasurementGrid* Grid);

	void CalculateOnEntireObject();
private:
	SINGLETON_PRIVATE_PART(StructuralRoughnessLayerProducer)

	static void OnJitterCalculationsEnd(DataLayer* NewLayer);
	bool bWaitForJitterResult = false;

	FEEntity* DebugPlaneEntity = nullptr;

	std::pair<glm::vec3, std::vector<glm::vec3>> GetPointsInCell(GridNode* Node);
	glm::vec3 GetNormalOfBestFitPlane(const std::vector<glm::vec3>& Points);

	static void OnDebugGridSelectedCellChanged(glm::vec3 NewSelectedCell);
	static void WorkOnNode(GridNode* CurrentNode);
	//double RunOnAllInternalNodesWithData(GridNode* OuterNode, std::function<void(int BoxSizeIndex, FEAABB BoxAABB)> FunctionWithAdditionalCode = nullptr);
};

#define STRUCTURAL_ROUGHNESS_LAYER_PRODUCER StructuralRoughnessLayerProducer::GetInstance()