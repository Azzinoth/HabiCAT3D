#include "MeshRenderer.h"
using namespace FocalEngine;

MeshRenderer::MeshRenderer() {}
MeshRenderer::~MeshRenderer() {}

void MeshRenderer::RenderFEMesh(FEMesh* Mesh)
{
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("AmbientFactor", UI.GetAmbientLightFactor());
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("HaveColor", Mesh->GetColorCount() == 0 ? 0 : 1);
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("HeatMapType", MESH_MANAGER.GetHeatMapType());
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerIndex", LAYER_MANAGER.GetActiveLayerIndex());

	MESH_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaSaturationFactor", MESH_MANAGER.GetUnselectedAreaSaturationFactor());
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaBrightnessFactor", MESH_MANAGER.GetUnselectedAreaBrightnessFactor());

	if (LAYER_MANAGER.GetActiveLayer() != nullptr)
	{
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMin());
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMax());
	}
	else
	{
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", 0.0f);
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", 0.0f);
	}

	if (LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerMin", float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].MinVisible));
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerMax", float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].MaxVisible));

		MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMin", float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMin()));
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMax", float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMax()));
	}

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() > 1 && UI.GetLayerSelectionMode() == 2)
	{
		float TempMeasuredRugosityAreaRadius = 0.0f;
		glm::vec3 TempMeasuredRugosityAreaCenter = glm::vec3(0.0f);
		MESH_MANAGER.GetMeasuredRugosityArea(TempMeasuredRugosityAreaRadius, TempMeasuredRugosityAreaCenter);
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", TempMeasuredRugosityAreaRadius);
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaCenter", TempMeasuredRugosityAreaCenter);
	}
	else
	{
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", -1.0f);
	}

	MESH_MANAGER.ActiveEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
	FE_GL_ERROR(glBindVertexArray(MESH_MANAGER.ActiveMesh->GetVaoID()));

	if (MESH_MANAGER.ActiveMesh->GetColorCount() > 0) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if (MESH_MANAGER.GetFirstLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(7));
	if (MESH_MANAGER.GetSecondLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(8));
	RENDERER.RenderGameModelComponentForward(MESH_MANAGER.ActiveEntity, MAIN_SCENE_MANAGER.GetMainCamera(), false);
	MESH_MANAGER.ActiveEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
}