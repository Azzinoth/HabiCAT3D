#include "MeshRenderer.h"
using namespace FocalEngine;

MeshRenderer* MeshRenderer::Instance = nullptr;

MeshRenderer::MeshRenderer() {}
MeshRenderer::~MeshRenderer() {}

void MeshRenderer::RenderFEMesh(FEMesh* Mesh)
{
	MESH_MANAGER.MeshShader->getParameter("AmbientFactor")->updateData(UI.GetAmbientLightFactor());
	MESH_MANAGER.MeshShader->getParameter("HaveColor")->updateData(Mesh->getColorCount() != 0);
	MESH_MANAGER.MeshShader->getParameter("HeatMapType")->updateData(Mesh->HeatMapType);
	MESH_MANAGER.MeshShader->getParameter("LayerIndex")->updateData(LAYER_MANAGER.GetActiveLayerIndex());

	MESH_MANAGER.MeshShader->getParameter("saturationFactor")->updateData(MESH_MANAGER.saturationFactor);
	MESH_MANAGER.MeshShader->getParameter("brightnessValue")->updateData(MESH_MANAGER.brightnessValue);

	if (LAYER_MANAGER.GetActiveLayer() != nullptr)
	{
		MESH_MANAGER.MeshShader->getParameter("SelectedRangeMin")->updateData(LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMin());
		MESH_MANAGER.MeshShader->getParameter("SelectedRangeMax")->updateData(LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMax());
	}
	else
	{
		MESH_MANAGER.MeshShader->getParameter("SelectedRangeMin")->updateData(0.0f);
		MESH_MANAGER.MeshShader->getParameter("SelectedRangeMax")->updateData(0.0f);
	}

	if (LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		MESH_MANAGER.MeshShader->getParameter("LayerMin")->updateData(float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].MinVisible));
		MESH_MANAGER.MeshShader->getParameter("LayerMax")->updateData(float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].MaxVisible));

		MESH_MANAGER.MeshShader->getParameter("LayerAbsoluteMin")->updateData(float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMin()));
		MESH_MANAGER.MeshShader->getParameter("LayerAbsoluteMax")->updateData(float(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMax()));
	}

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() > 1 && UI.GetLayerSelectionMode() == 2)
	{
		MESH_MANAGER.MeshShader->getParameter("MeasuredRugosityAreaRadius")->updateData(Mesh->LastMeasuredRugosityAreaRadius);
		MESH_MANAGER.MeshShader->getParameter("MeasuredRugosityAreaCenter")->updateData(Mesh->LastMeasuredRugosityAreaCenter);
	}
	else
	{
		MESH_MANAGER.MeshShader->getParameter("MeasuredRugosityAreaRadius")->updateData(-1.0f);
	}

	FE_GL_ERROR(glBindVertexArray(Mesh->GetVaoID()));
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

	glBindVertexArray(0);
}