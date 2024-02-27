#include "MeshRenderer.h"
using namespace FocalEngine;

MeshRenderer* MeshRenderer::Instance = nullptr;

MeshRenderer::MeshRenderer() {}
MeshRenderer::~MeshRenderer() {}

void MeshRenderer::RenderFEMesh(FEMesh* Mesh)
{
	// FIX ME
	MESH_MANAGER.CustomMeshShader->UpdateParameterData("AmbientFactor", UI.GetAmbientLightFactor());
	MESH_MANAGER.CustomMeshShader->UpdateParameterData("HaveColor", Mesh->GetColorCount() != 0);
	MESH_MANAGER.CustomMeshShader->UpdateParameterData("HeatMapType", MESH_MANAGER.GetHeatMapType());
	MESH_MANAGER.CustomMeshShader->UpdateParameterData("LayerIndex", LAYER_MANAGER.GetActiveLayerIndex());

	MESH_MANAGER.CustomMeshShader->UpdateParameterData("saturationFactor", MESH_MANAGER.saturationFactor);
	MESH_MANAGER.CustomMeshShader->UpdateParameterData("brightnessValue", MESH_MANAGER.brightnessValue);

	if (LAYER_MANAGER.GetActiveLayer() != nullptr)
	{
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("SelectedRangeMin", LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMin());
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("SelectedRangeMax", LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMax());
	}
	else
	{
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("SelectedRangeMin", 0.0f);
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("SelectedRangeMax", 0.0f);
	}

	if (LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("LayerMin", float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].MinVisible));
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("LayerMax", float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].MaxVisible));

		MESH_MANAGER.CustomMeshShader->UpdateParameterData("LayerAbsoluteMin", float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMin()));
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("LayerAbsoluteMax", float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMax()));
	}

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() > 1 && UI.GetLayerSelectionMode() == 2)
	{
		//MESH_MANAGER.CustomMeshShader->UpdateParameterData("MeasuredRugosityAreaRadius", Mesh->LastMeasuredRugosityAreaRadius);
		//MESH_MANAGER.CustomMeshShader->UpdateParameterData("MeasuredRugosityAreaCenter", Mesh->LastMeasuredRugosityAreaCenter);
	}
	else
	{
		MESH_MANAGER.CustomMeshShader->UpdateParameterData("MeasuredRugosityAreaRadius", -1.0f);
	}

	MESH_MANAGER.ActiveEntity->SetVisibility(true);
	FE_GL_ERROR(glBindVertexArray(MESH_MANAGER.ActiveMesh->GetVaoID()));
	if (MESH_MANAGER.GetFirstLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(7));
	if (MESH_MANAGER.GetSecondLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(8));
	RENDERER.RenderEntityForward(MESH_MANAGER.ActiveEntity, ENGINE.GetCamera(), false);
	MESH_MANAGER.ActiveEntity->SetVisibility(false);

	/*FE_GL_ERROR(glBindVertexArray(Mesh->GetVaoID()));
	if ((Mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((Mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((Mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((Mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((Mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));

	if ((Mesh->vertexAttributes & FE_RUGOSITY_FIRST) == FE_RUGOSITY_FIRST) FE_GL_ERROR(glEnableVertexAttribArray(7));
	if ((Mesh->vertexAttributes & FE_RUGOSITY_SECOND) == FE_RUGOSITY_SECOND) FE_GL_ERROR(glEnableVertexAttribArray(8));

	if ((Mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, Mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	if ((Mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, Mesh->getVertexCount()));

	glBindVertexArray(0);*/
}