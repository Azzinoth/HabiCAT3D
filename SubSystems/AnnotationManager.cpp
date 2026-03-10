#include "AnnotationManager.h"
using namespace FocalEngine;

AnnotationData::AnnotationData()
{
    UpdateColors(ANNOTATION_MANAGER.DefaultAnnotationColors);
}

void AnnotationData::UpdateColors(const std::vector<glm::vec4>& Colors)
{
	CurrentColors = Colors;

	glDeleteBuffers(1, &AnnotationSSBO);
	glGenBuffers(1, &AnnotationSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, AnnotationSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, Colors.size() * sizeof(glm::vec4), Colors.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, AnnotationSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

AnnotationManager::AnnotationManager() {}
AnnotationManager::~AnnotationManager() {}

void AnnotationManager::Initialize()
{
	DefaultAnnotationColors.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	DefaultAnnotationColors.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	DefaultAnnotationColors.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	ANALYSIS_OBJECT_MANAGER.AddOnObjectLoadCallback(AnnotationManager::OnAnalysisObjectLoad);
	ANALYSIS_OBJECT_MANAGER.AddOnObjectDeleteCallback(AnnotationManager::OnAnalysisObjectDelete);
}

bool AnnotationManager::IsVisualizationActive()
{
	return bIsVisualizationActive;
}

void AnnotationManager::SetVisualizationActive(bool bActive)
{
	bIsVisualizationActive = bActive;
}

void AnnotationManager::OnAnalysisObjectLoad(AnalysisObject* NewObject)
{
	if (NewObject == nullptr)
		return;

	if (NewObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return;

	ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData[NewObject->GetID()] = new AnnotationData();
	ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData[NewObject->GetID()]->AnalysisObjectID = NewObject->GetID();
	FEEntity* Entity = NewObject->GetEntity();
	if (Entity != nullptr)
		RENDERER.AddBeforeRenderCallback(Entity, AnnotationManager::BeforeRender);
}

void AnnotationManager::OnAnalysisObjectDelete(AnalysisObject* DeletedObject)
{
	if (DeletedObject == nullptr)
		return;

	FEEntity* Entity = DeletedObject->GetEntity();
	if (Entity != nullptr)
		RENDERER.RemoveBeforeRenderCallback(Entity, AnnotationManager::BeforeRender);
	
	delete ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData[DeletedObject->GetID()];
	ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.erase(DeletedObject->GetID());
}

AnnotationData* AnnotationManager::GetAnnotationDataByAnalysisObjectID(std::string AnalysisObjectID)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AnalysisObjectID);
	if (CurrentObject == nullptr)
		return nullptr;

	if (ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.find(CurrentObject->GetID()) == ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.end())
		return nullptr;

	return ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData[CurrentObject->GetID()];
}

void AnnotationManager::BeforeRender(FEEntity* CurrentEntity)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByEntityID(CurrentEntity->GetObjectID());
	if (CurrentObject == nullptr)
		return;

	if (CurrentObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return;

	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(CurrentObject->GetID());
	if (CurrentAnnotationData == nullptr)
		return;

	if (ANNOTATION_MANAGER.IsVisualizationActive())
	{
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("AnnotationVisualizationActive", 1);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, CurrentAnnotationData->AnnotationSSBO);

		FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
		if (ActiveMesh != nullptr)
		{
			if (CurrentAnnotationData->DataBufferID != GLuint(-1))
			{
				FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

				FE_GL_ERROR(glEnableVertexAttribArray(15));
				FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentAnnotationData->DataBufferID));

				FE_GL_ERROR(glBindVertexArray(0));
			}
		}
	}
	else
	{
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("AnnotationVisualizationActive", 0);
	}
}

void AnnotationManager::InitalizeBuffer(AnnotationData* Data)
{
	if (Data == nullptr)
		return;

	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(Data->AnalysisObjectID);
	if (Object == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(Object->GetEngineResource());
	if (ActiveMesh == nullptr)
		return;

	FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

	FE_GL_ERROR(glGenBuffers(1, &Data->DataBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, Data->DataBufferID));

	std::vector<glm::vec4> EmptyData;
	EmptyData.resize(CurrentMeshAnalysisData->Vertices.size() / 3);
	for (size_t i = 0; i < EmptyData.size(); i++)
	{
		EmptyData[i].x = 0.0f;
		EmptyData[i].y = 0.0f;
		EmptyData[i].z = 0.0f;
		EmptyData[i].w = 0.0f;
	}

	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * EmptyData.size() * 4, EmptyData.data(), GL_DYNAMIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(15, 4, GL_FLOAT, false, 0, nullptr));

	glBindVertexArray(0);
}

void AnnotationManager::UpdateBuffer(AnnotationData* Data)
{
	if (Data == nullptr)
		return;

	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(Data->AnalysisObjectID);
	if (Object == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(Object->GetEngineResource());
	if (ActiveMesh == nullptr)
		return;

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, Data->DataBufferID));
	Data->TrianglesIndexToData.resize(CurrentMeshAnalysisData->Vertices.size() / 3);

	std::vector<float> PerVertexData;
	std::vector<float> PerTriangleData;
	PerVertexData.resize(CurrentMeshAnalysisData->Vertices.size());
	PerTriangleData.resize(CurrentMeshAnalysisData->Triangles.size());
	for (size_t i = 0; i < PerTriangleData.size(); i++)
	{
		PerTriangleData[i] = 0.0f;
	}

	for (size_t i = 0; i < CurrentMeshAnalysisData->TriangleSelected.size(); i++)
	{
		int TriangleIndex = CurrentMeshAnalysisData->TriangleSelected[i];
		PerTriangleData[TriangleIndex] = 1.0f;
	}

	DataLayer::TransfareDataFromTrianglesToVertices(Object, PerTriangleData, PerVertexData);
	std::vector<float> CompactedVertexData;
	CompactedVertexData.resize(CurrentMeshAnalysisData->Vertices.size() / 3);
	for (size_t i = 0; i < CurrentMeshAnalysisData->Vertices.size() / 3; i++)
		CompactedVertexData[i] = PerVertexData[i * 3];

	for (size_t i = 0; i < Data->TrianglesIndexToData.size(); i++)
	{
		Data->TrianglesIndexToData[i].x = CompactedVertexData[i];
		Data->TrianglesIndexToData[i].y = 2.0f;
		Data->TrianglesIndexToData[i].z = 3.0f;
		Data->TrianglesIndexToData[i].w = 4.0f;
	}

	FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * Data->TrianglesIndexToData.size() * 4, Data->TrianglesIndexToData.data()));
}