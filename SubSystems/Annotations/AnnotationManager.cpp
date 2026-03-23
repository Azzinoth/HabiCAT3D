#include "AnnotationManager.h"
using namespace FocalEngine;

glm::vec4 AnnotationInfo::GetColor() const
{
	return Color;
}

AnnotationData::AnnotationData(std::string AnalysisObjectID)
{
	this->AnalysisObjectID = AnalysisObjectID;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AnalysisObjectID);
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return;

	PerTriangleID.resize(CurrentMeshAnalysisData->Triangles.size());
	for (size_t i = 0; i < PerTriangleID.size(); i++)
		PerTriangleID[i] = -1;

	// FE_FIX_ME: Only for debug.
	AnnotationInfo TestInfo;
	TestInfo.ID = 0;
	TestInfo.Name = "Coral_type_0";
	TestInfo.Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	UsedAnnotations.push_back(TestInfo);

	TestInfo.ID = 1;
	TestInfo.Name = "Coral_type_1";
	TestInfo.Color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	UsedAnnotations.push_back(TestInfo);

	TestInfo.ID = 2;
	TestInfo.Name = "Coral_type_2";
	TestInfo.Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	UsedAnnotations.push_back(TestInfo);

	UpdateColorInfoOnGPU();
}

AnnotationData::~AnnotationData()
{
	FEEntity* Entity = GetEntity();
	MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(Entity);

	if (Plane != nullptr)
		delete Plane;

	glDeleteBuffers(1, &AnnotationSSBO);
	glDeleteBuffers(1, &DataBufferID);
}

AnalysisObject* AnnotationData::GetAnalysisObject()
{
	return ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AnalysisObjectID);
}

FEEntity* AnnotationData::GetEntity()
{
	return MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(EntityID);
}

void AnnotationData::UpdateColorInfoOnGPU()
{
	std::vector<glm::vec4> Colors;
	for (size_t i = 0; i < UsedAnnotations.size(); i++)
		Colors.push_back(UsedAnnotations[i].Color);

	glDeleteBuffers(1, &AnnotationSSBO);
	glGenBuffers(1, &AnnotationSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, AnnotationSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, Colors.size() * sizeof(glm::vec4), Colors.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, AnnotationSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

bool AnnotationData::UpdateAnnotationColor(int AnnotationID, const glm::vec4& NewColor)
{
	for (size_t i = 0; i < UsedAnnotations.size(); i++)
	{
		if (UsedAnnotations[i].ID == AnnotationID)
		{
			UsedAnnotations[i].Color = NewColor;
			UpdateColorInfoOnGPU();
			return true;
		}
	}

	return false;
}

bool AnnotationData::IsInEditingMode() const
{
	return bInEditingMode;
}

void AnnotationData::SetEditingMode(bool bEnable)
{
	if (bInEditingMode == bEnable)
		return;

	if (!bInEditingMode && bEnable)
	{
		Plane = new PolygonPlane();
		Plane->Initialize();

		// FE_TO_DO: Rotate plane to match entity forward direction
		FEEntity* MainEntity = GetEntity();
		FEAABB EntityAABB = MAIN_SCENE_MANAGER.GetMainScene()->GetEntityAABB(MainEntity);
		EntityAABB.GetApproximateForwardDirection();
	}
	else if (bInEditingMode && !bEnable)
	{

	}

	bInEditingMode = bEnable;
}

PolygonPlane* AnnotationData::GetPolygonPlane()
{
	return Plane;
}

std::vector<AnnotationInfo> AnnotationData::GetAllAnnotationInfos()
{
	return UsedAnnotations;
}

AnnotationInfo* AnnotationData::GetAnnotationInfoByID(int ID)
{
	for (size_t i = 0; i < UsedAnnotations.size(); i++)
	{
		if (UsedAnnotations[i].ID == ID)
			return &UsedAnnotations[i];
	}

	return nullptr;
}

AnnotationInfo* AnnotationData::GetAnnotationInfoByPolygonIndex(int PolygonIndex)
{
	for (size_t i = 0; i < AnnotationIDToPolygonIndices.size(); i++)
	{
		if (AnnotationIDToPolygonIndices[i].second == PolygonIndex)
			return GetAnnotationInfoByID(AnnotationIDToPolygonIndices[i].first);
	}

	return nullptr;
}

bool AnnotationData::IsPolygonIndexAnnotated(int PolygonIndex)
{
	return GetAnnotationInfoByPolygonIndex(PolygonIndex) != nullptr;
}

bool AnnotationData::SetPolygonIndexAnnotation(int PolygonIndex, int AnnotationID)
{
	for (size_t i = 0; i < AnnotationIDToPolygonIndices.size(); i++)
	{
		if (AnnotationIDToPolygonIndices[i].second == PolygonIndex)
		{
			if (AnnotationIDToPolygonIndices[i].first == AnnotationID)
				return true;

			AnnotationIDToPolygonIndices[i].first = AnnotationID;
			return true;
		}
	}

	AnnotationIDToPolygonIndices.push_back(std::make_pair(AnnotationID, PolygonIndex));
	return true;
}

AnnotationManager::AnnotationManager() {}
AnnotationManager::~AnnotationManager() {}

void AnnotationManager::Initialize()
{
	ANALYSIS_OBJECT_MANAGER.AddOnObjectLoadCallback(AnnotationManager::OnAnalysisObjectLoad);
	ANALYSIS_OBJECT_MANAGER.AddOnObjectDeleteCallback(AnnotationManager::OnAnalysisObjectDelete);
	LAYER_MANAGER.AddActiveLayerChangedCallback(AnnotationManager::OnLayerChange);
}

bool AnnotationManager::AddAnnotationToAnalysisObject(std::string AnalysisObjectID)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AnalysisObjectID);
	if (CurrentObject == nullptr)
		return false;

	if (ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.find(CurrentObject->GetID()) != ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.end())
		return false;

	ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData[CurrentObject->GetID()] = new AnnotationData(AnalysisObjectID);
	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData[CurrentObject->GetID()];
	//CurrentAnnotationData->AnalysisObjectID = CurrentObject->GetID();

	FEEntity* Entity = CurrentObject->GetEntity();
	if (Entity != nullptr)
	{
		FEEntity* AnnotationEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("AnnotationEntity_" + CurrentObject->GetName());
		CurrentAnnotationData->EntityID = AnnotationEntity->GetObjectID();
		AnnotationEntity->AttachTo(Entity, false);

		RENDERER.AddBeforeRenderCallback(Entity, AnnotationManager::BeforeRender);
	}

	return true;
}

bool AnnotationManager::RemoveAnnotationFromAnalysisObject(std::string AnalysisObjectID)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AnalysisObjectID);
	if (CurrentObject == nullptr)
		return false;

	if (ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.find(CurrentObject->GetID()) == ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.end())
		return false;

	FEEntity* Entity = CurrentObject->GetEntity();
	if (Entity != nullptr)
		RENDERER.RemoveBeforeRenderCallback(Entity, AnnotationManager::BeforeRender);

	delete ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData[CurrentObject->GetID()];
	ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.erase(CurrentObject->GetID());

	return true;
}

void AnnotationManager::OnAnalysisObjectLoad(AnalysisObject* NewObject)
{
	if (NewObject == nullptr)
		return;

	if (NewObject->GetType() != DATA_SOURCE_TYPE::MESH)
		return;

	//ANNOTATION_MANAGER.AddAnnotationToAnalysisObject(NewObject->GetID());
}

void AnnotationManager::OnAnalysisObjectDelete(AnalysisObject* DeletedObject)
{
	if (DeletedObject == nullptr)
		return;

	ANNOTATION_MANAGER.RemoveAnnotationFromAnalysisObject(DeletedObject->GetID());
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

AnnotationData* AnnotationManager::GetAnnotationDataByEntityID(std::string EntityID)
{
	for (auto& MapRecord : ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData)
	{
		if (MapRecord.second->EntityID == EntityID)
			return MapRecord.second;
	}

	return nullptr;
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

	bool bShouldBeVisualized = false;
	FEEntity* AnnotationEntity = CurrentAnnotationData->GetEntity();
	if (AnnotationEntity != nullptr)
		bShouldBeVisualized = AnnotationEntity->IsVisible();

	if (CurrentAnnotationData->AnnotationSSBO == GLuint(-1))
		bShouldBeVisualized = false;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		bShouldBeVisualized = false;

	if (CurrentAnnotationData->DataBufferID == GLuint(-1))
		bShouldBeVisualized = false;

	if (bShouldBeVisualized)
	{
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("AnnotationVisualizationActive", 1);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, CurrentAnnotationData->AnnotationSSBO);

		FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

		FE_GL_ERROR(glEnableVertexAttribArray(15));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentAnnotationData->DataBufferID));

		FE_GL_ERROR(glBindVertexArray(0));
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
	Data->FinalPerVertexData.resize(CurrentMeshAnalysisData->Vertices.size() / 3);

	std::vector<float> PerVertexData;
	PerVertexData.resize(CurrentMeshAnalysisData->Vertices.size());

	std::vector<float> PerTriangleIDFloat;
	PerTriangleIDFloat.resize(Data->PerTriangleID.size());
	for (size_t i = 0; i < Data->PerTriangleID.size(); i++)
		PerTriangleIDFloat[i] = static_cast<float>(Data->PerTriangleID[i]);
	
	DataLayer::TransfareDataFromTrianglesToVertices(Object, PerTriangleIDFloat, PerVertexData);
	std::vector<float> CompactedVertexData;
	CompactedVertexData.resize(CurrentMeshAnalysisData->Vertices.size() / 3);
	for (size_t i = 0; i < CurrentMeshAnalysisData->Vertices.size() / 3; i++)
		CompactedVertexData[i] = PerVertexData[i * 3];

	for (size_t i = 0; i < Data->FinalPerVertexData.size(); i++)
	{
		Data->FinalPerVertexData[i].x = CompactedVertexData[i];
		Data->FinalPerVertexData[i].y = 2.0f;
		Data->FinalPerVertexData[i].z = 3.0f;
		Data->FinalPerVertexData[i].w = 4.0f;
	}

	FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * Data->FinalPerVertexData.size() * 4, Data->FinalPerVertexData.data()));
	UpdateHistogramData(Data);
}

void AnnotationManager::UpdateHistogramData(AnnotationData* Data)
{
	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(Data->AnalysisObjectID);
	if (Object == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	DataLayer* ActiveLayerData = Object->GetActiveLayer();
	if (ActiveLayerData == nullptr)
		return;

	for (int i = 0; i < Data->UsedAnnotations.size(); i++)
		Data->UsedAnnotations[i].HistogramData.clear();

	for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		for (int j = 0; j < Data->UsedAnnotations.size(); j++)
		{
			if (Data->PerTriangleID[i] == Data->UsedAnnotations[j].ID)
			{
				double CurrentLayerTriangleValue = ActiveLayerData->ElementsToData[i];
				Data->UsedAnnotations[j].HistogramData.push_back(std::make_tuple(CurrentLayerTriangleValue, CurrentMeshAnalysisData->TrianglesArea[i], i));
			}
		}
	}

	for (int i = 0; i < Data->UsedAnnotations.size(); i++)
	{
		if (Data->UsedAnnotations[i].HistogramData.empty())
			continue;

		std::sort(Data->UsedAnnotations[i].HistogramData.begin(), Data->UsedAnnotations[i].HistogramData.end());
	}
}

void AnnotationManager::OnLayerChange()
{
	DataLayer* ActiveLayer = LAYER_MANAGER.GetActiveLayer();
	if (ActiveLayer == nullptr)
		return;

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(ActiveObject->GetID());
	if (CurrentAnnotationData == nullptr)
		return;

	ANNOTATION_MANAGER.UpdateHistogramData(CurrentAnnotationData);
}