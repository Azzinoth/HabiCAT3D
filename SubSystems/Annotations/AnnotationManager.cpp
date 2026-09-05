#include "AnnotationManager.h"
#include "../DeveloperMode.h"
#include "../UI/UICore.h"
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

	if (CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		PerElementID.resize(CurrentMeshAnalysisData->Triangles.size());
	}
	else if (CurrentObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		PointCloudAnalysisData* CurrentPointCloudAnalysisData = CurrentObject->GetPointCloudAnalysisData();
		if (CurrentPointCloudAnalysisData == nullptr)
			return;

		PerElementID.resize(CurrentPointCloudAnalysisData->RawPointCloudData.size());
	}
	else
	{
		return;
	}

	for (size_t i = 0; i < PerElementID.size(); i++)
		PerElementID[i] = -1;

	AnnotationInfo TestInfo;
	TestInfo.ID = 0;
	TestInfo.Name = "Dummy Annotation";
	TestInfo.Color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	UsedAnnotations.push_back(TestInfo);

	// FE_FIX_ME: Only for debug.
	TestInfo.ID = 1;
	TestInfo.Name = "Coral_type_0";
	TestInfo.Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	UsedAnnotations.push_back(TestInfo);

	TestInfo.ID = 2;
	TestInfo.Name = "Coral_type_1";
	TestInfo.Color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	UsedAnnotations.push_back(TestInfo);

	TestInfo.ID = 3;
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
	glDeleteBuffers(1, &MeshBufferID);

	for (size_t i = 0; i < AnnotationIDComputeShaderBuffers.size(); i++)
		glDeleteBuffers(1, &AnnotationIDComputeShaderBuffers[i]);

	for (size_t i = 0; i < OriginalColorComputeShaderBuffers.size(); i++)
		glDeleteBuffers(1, &OriginalColorComputeShaderBuffers[i]);
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
			if (UsedAnnotations[i].Color == NewColor)
				return true;

			UsedAnnotations[i].Color = NewColor;
			UpdateColorInfoOnGPU();
			ANNOTATION_MANAGER.NotifyAnnotationColorChanged(this, AnnotationID);
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

AnnotationInfo* AnnotationData::AddAnnotationInfo(std::string Name, std::string Description, glm::vec4 Color)
{
	int NewID = 1;
	for (size_t i = 0; i < UsedAnnotations.size(); i++)
	{
		if (UsedAnnotations[i].ID >= NewID)
			NewID = UsedAnnotations[i].ID + 1;
	}

	AnnotationInfo NewInfo;
	NewInfo.ID = NewID;
	NewInfo.Name = Name;
	NewInfo.Description = Description;
	NewInfo.Color = Color;
	UsedAnnotations.push_back(NewInfo);
	UpdateColorInfoOnGPU();

	return &UsedAnnotations.back();
}

const std::vector<AnnotationInfo>& AnnotationData::GetAllAnnotationInfos() const
{
	return UsedAnnotations;
}

void AnnotationData::ClearAllAnnotationsInfo()
{
	UsedAnnotations.clear();
	AnnotationInfo TestInfo;
	TestInfo.ID = 0;
	TestInfo.Name = "Dummy Annotation";
	TestInfo.Color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	UsedAnnotations.push_back(TestInfo);

	UpdateColorInfoOnGPU();
}

void AnnotationData::ClearAllAnnotation()
{
	for (size_t i = 0; i < PerElementID.size(); i++)
		PerElementID[i] = -1;

	ANNOTATION_MANAGER.UpdateBuffer(this);
}

bool AnnotationData::UpdateAnnotationForTriangle(int TriangleIndex, int AnnotationID)
{
	if (TriangleIndex < 0 || TriangleIndex >= PerElementID.size())
		return false;

	PerElementID[TriangleIndex] = AnnotationID;
	ANNOTATION_MANAGER.UpdateBuffer(this);
	return true;
}

bool AnnotationData::UpdateAnnotationForTriangles(std::vector<int>& TriangleIndexes, int AnnotationID)
{
	bool bUpdatedAtLeastOne = false;
	for (size_t i = 0; i < TriangleIndexes.size(); i++)
	{
		if (TriangleIndexes[i] < 0 || TriangleIndexes[i] >= PerElementID.size())
			continue;

		PerElementID[TriangleIndexes[i]] = AnnotationID;
		bUpdatedAtLeastOne = true;
	}

	ANNOTATION_MANAGER.UpdateBuffer(this);
	return bUpdatedAtLeastOne;
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

	// If no annotation is associated with the provided polygon index, return dummy annotation info with ID 0.
	return GetAnnotationInfoByID(0);
}

AnnotationInfo* AnnotationData::GetAnnotationInfoByName(std::string Name)
{
	for (size_t i = 0; i < UsedAnnotations.size(); i++)
	{
		if (UsedAnnotations[i].Name == Name)
			return &UsedAnnotations[i];
	}

	return nullptr;
}

bool AnnotationData::IsPolygonIndexAnnotated(int PolygonIndex)
{
	return GetAnnotationInfoByPolygonIndex(PolygonIndex)->ID != 0;
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
	ANALYSIS_OBJECT_MANAGER.AddOnObjectDeleteCallback(AnnotationManager::OnAnalysisObjectDelete);
	LAYER_MANAGER.AddActiveLayerChangedCallback(AnnotationManager::OnLayerChange);
}

void AnnotationManager::AddOnAnnotationColorChangedCallback(std::function<void(AnnotationData*, int)> Callback)
{
	ClientOnAnnotationColorChangedCallbacks.push_back(Callback);
}

void AnnotationManager::NotifyAnnotationColorChanged(AnnotationData* Data, int AnnotationID)
{
	for (size_t i = 0; i < ClientOnAnnotationColorChangedCallbacks.size(); i++)
	{
		if (ClientOnAnnotationColorChangedCallbacks[i] == nullptr)
			continue;

		ClientOnAnnotationColorChangedCallbacks[i](Data, AnnotationID);
	}
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

	FEEntity* Entity = CurrentObject->GetEntity();
	if (Entity != nullptr)
	{
		FEEntity* AnnotationEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("AnnotationEntity_" + CurrentObject->GetName());
		CurrentAnnotationData->EntityID = AnnotationEntity->GetObjectID();
		AnnotationEntity->AttachTo(Entity, false);

		RENDERER.AddBeforeRenderCallback(Entity, AnnotationManager::BeforeRender);

		InitalizeBuffer(CurrentAnnotationData);
	}

	return true;
}

bool AnnotationManager::RemoveAnnotationFromAnalysisObject(std::string AnalysisObjectID)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(AnalysisObjectID);
	if (CurrentObject == nullptr)
		return false;

	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(CurrentObject->GetID());
	if (CurrentAnnotationData == nullptr)
		return false;

	CurrentAnnotationData->ClearAllAnnotation();

	// We should updated the point cloud color before deleting the annotation data.
	if (CurrentObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
		ANALYSIS_OBJECT_MANAGER.RecolorPointCloud(CurrentObject);

	FEEntity* Entity = CurrentObject->GetEntity();
	if (Entity != nullptr)
		RENDERER.RemoveBeforeRenderCallback(Entity, AnnotationManager::BeforeRender);

	delete ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData[CurrentObject->GetID()];
	ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.erase(CurrentObject->GetID());

	return true;
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

	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(CurrentObject->GetID());
	if (CurrentAnnotationData == nullptr)
		return;

	bool bShouldBeVisualized = false;
	FEEntity* AnnotationEntity = CurrentAnnotationData->GetEntity();
	if (AnnotationEntity != nullptr)
		bShouldBeVisualized = AnnotationEntity->IsVisible();

	if (CurrentAnnotationData->AnnotationSSBO == GLuint(-1))
		bShouldBeVisualized = false;

	if (CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			bShouldBeVisualized = false;

		if (CurrentAnnotationData->MeshBufferID == GLuint(-1))
			bShouldBeVisualized = false;

		if (bShouldBeVisualized)
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("AnnotationVisualizationActive", 1);
			FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, CurrentAnnotationData->AnnotationSSBO));
			FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CurrentAnnotationData->AnnotationSSBO));

			FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

			FE_GL_ERROR(glEnableVertexAttribArray(15));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentAnnotationData->MeshBufferID));

			FE_GL_ERROR(glBindVertexArray(0));
		}
		else
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("AnnotationVisualizationActive", 0);
		}
	}
	else if (CurrentObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		// It is done in AnalysisObjectManager::BeforeRender.
	}
}

void AnnotationManager::InitalizeBuffer(AnnotationData* Data)
{
	if (Data == nullptr)
		return;

	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(Data->AnalysisObjectID);
	if (Object == nullptr)
		return;

	if (Object->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		FEMesh* ActiveMesh = static_cast<FEMesh*>(Object->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

		FE_GL_ERROR(glGenBuffers(1, &Data->MeshBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, Data->MeshBufferID));

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
	else if (Object->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		UpdatePointCloudBuffers(Data);
	}
}

void AnnotationManager::UpdateBuffer(AnnotationData* Data)
{
	if (Data == nullptr)
		return;

	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(Data->AnalysisObjectID);
	if (Object == nullptr)
		return;

	if (Object->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		FEMesh* ActiveMesh = static_cast<FEMesh*>(Object->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		if (Data->MeshBufferID == GLuint(-1))
		{
			InitalizeBuffer(Data);
			// If it is still -1, it means that buffer initialization failed, so we should not try to update it.
			if (Data->MeshBufferID == GLuint(-1))
			{
				LOG.Add("Failed to initialize annotation data buffer for analysis object with ID: " + Object->GetID(), "ANNOTATION_MANAGER", FE_LOG_ERROR);
				return;
			}
		}

		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, Data->MeshBufferID));
		Data->FinalPerVertexData.resize(CurrentMeshAnalysisData->Vertices.size() / 3);

		std::vector<float> PerVertexData;
		PerVertexData.resize(CurrentMeshAnalysisData->Vertices.size());

		std::vector<float> PerElementIDFloat;
		PerElementIDFloat.resize(Data->PerElementID.size());
		for (size_t i = 0; i < Data->PerElementID.size(); i++)
			PerElementIDFloat[i] = static_cast<float>(Data->PerElementID[i]);

		DataLayer::TransfareDataFromTrianglesToVertices(Object, PerElementIDFloat, PerVertexData);
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
	}
	else if (Object->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		UpdatePointCloudBuffers(Data);
	}
	Data->UpdateColorInfoOnGPU();
	UpdateHistogramData(Data);
}

void AnnotationManager::UpdatePointCloudBuffers(AnnotationData* Data)
{
	if (Data == nullptr)
		return;

	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(Data->AnalysisObjectID);
	if (Object == nullptr)
		return;

	PointCloudAnalysisData* CurrentPointCloudAnalysisData = Object->GetPointCloudAnalysisData();
	if (CurrentPointCloudAnalysisData == nullptr)
		return;

	const size_t PointCount = Data->PerElementID.size();
	if (PointCount == 0 || CurrentPointCloudAnalysisData->OriginalColors.size() < PointCount)
		return;

	for (size_t i = 0; i < Data->AnnotationIDComputeShaderBuffers.size(); i++)
		FE_GL_ERROR(glDeleteBuffers(1, &Data->AnnotationIDComputeShaderBuffers[i]));
	Data->AnnotationIDComputeShaderBuffers.clear();

	for (size_t i = 0; i < Data->OriginalColorComputeShaderBuffers.size(); i++)
		FE_GL_ERROR(glDeleteBuffers(1, &Data->OriginalColorComputeShaderBuffers[i]));
	Data->OriginalColorComputeShaderBuffers.clear();

	std::vector<unsigned int> PackedOriginalColors;
	for (size_t i = 0; i < PointCount; i += FEPointCloud::MaxPointsPerBuffer)
	{
		size_t ElementCount = std::min(FEPointCloud::MaxPointsPerBuffer, PointCount - i);

		Data->AnnotationIDComputeShaderBuffers.resize(Data->AnnotationIDComputeShaderBuffers.size() + 1);
		FE_GL_ERROR(glGenBuffers(1, &Data->AnnotationIDComputeShaderBuffers.back()));
		FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, Data->AnnotationIDComputeShaderBuffers.back()));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * ElementCount, Data->PerElementID.data() + i, GL_DYNAMIC_DRAW));

		PackedOriginalColors.resize(ElementCount);
		for (size_t j = 0; j < ElementCount; j++)
		{
			const std::vector<unsigned char>& OriginalColor = CurrentPointCloudAnalysisData->OriginalColors[i + j];
			PackedOriginalColors[j] = (static_cast<unsigned int>(OriginalColor[0]) << 0) |
				(static_cast<unsigned int>(OriginalColor[1]) << 8) |
				(static_cast<unsigned int>(OriginalColor[2]) << 16) |
				(static_cast<unsigned int>(OriginalColor[3]) << 24);
		}

		Data->OriginalColorComputeShaderBuffers.resize(Data->OriginalColorComputeShaderBuffers.size() + 1);
		FE_GL_ERROR(glGenBuffers(1, &Data->OriginalColorComputeShaderBuffers.back()));
		FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, Data->OriginalColorComputeShaderBuffers.back()));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * ElementCount, PackedOriginalColors.data(), GL_STATIC_DRAW));
	}

	FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

bool AnnotationManager::ReadBackBuffer(AnnotationData* Data)
{
	if (Data == nullptr)
		return false;

	if (Data->MeshBufferID == GLuint(-1))
		return false;

	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(Data->AnalysisObjectID);
	if (Object == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	const size_t VertexCount = CurrentMeshAnalysisData->Vertices.size() / 3;
	const size_t TriangleCount = CurrentMeshAnalysisData->Triangles.size();

	if (VertexCount == 0 || TriangleCount == 0)
		return false;

	if (CurrentMeshAnalysisData->Indices.size() < TriangleCount * 3)
		return false;

	Data->FinalPerVertexData.resize(VertexCount);

	FE_GL_ERROR(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT));
	FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, Data->MeshBufferID));
	FE_GL_ERROR(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
		sizeof(glm::vec4) * VertexCount,
		Data->FinalPerVertexData.data()));
	FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

	Data->PerElementID.resize(TriangleCount);
	for (size_t i = 0; i < TriangleCount; i++)
	{
		const int VertexIndex = CurrentMeshAnalysisData->Indices[i * 3];
		Data->PerElementID[i] = static_cast<int>(std::lround(Data->FinalPerVertexData[VertexIndex].x));
	}

	UpdateHistogramData(Data);
	return true;
}

void AnnotationManager::UpdateHistogramData(AnnotationData* Data)
{
	if (Data == nullptr)
		return;

	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(Data->AnalysisObjectID);
	if (Object == nullptr)
		return;

	DataLayer* ActiveLayerData = Object->GetActiveLayer();
	if (ActiveLayerData == nullptr)
		return;

	for (int i = 0; i < Data->UsedAnnotations.size(); i++)
		Data->UsedAnnotations[i].HistogramData.clear();

	if (Object->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
		{
			for (int j = 0; j < Data->UsedAnnotations.size(); j++)
			{
				if (Data->PerElementID[i] == Data->UsedAnnotations[j].ID)
				{
					double CurrentLayerTriangleValue = ActiveLayerData->ElementsToData[i];
					Data->UsedAnnotations[j].HistogramData.push_back(std::make_tuple(CurrentLayerTriangleValue, CurrentMeshAnalysisData->TrianglesArea[i], i));
				}
			}
		}
	}
	else if (Object->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		size_t ElementCount = std::min(Data->PerElementID.size(), ActiveLayerData->ElementsToData.size());
		for (int i = 0; i < ElementCount; i++)
		{
			for (int j = 0; j < Data->UsedAnnotations.size(); j++)
			{
				if (Data->PerElementID[i] == Data->UsedAnnotations[j].ID)
				{
					double CurrentLayerPointValue = ActiveLayerData->ElementsToData[i];
					// For point clouds each point has weight of 1.0.
					Data->UsedAnnotations[j].HistogramData.push_back(std::make_tuple(CurrentLayerPointValue, 1.0, i));
				}
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

// ColorBrewer palette used when a label has no color from the file and none chosen by the user.
static const std::vector<glm::vec4> DefaultAnnotationColors = { { 127 / 255.0f, 59 / 255.0f, 8 / 255.0f, 1.0f },
																{ 179 / 255.0f, 88 / 255.0f, 6 / 255.0f, 1.0f },
																{ 224 / 255.0f, 130 / 255.0f, 20 / 255.0f, 1.0f },
																{ 253 / 255.0f, 184 / 255.0f, 99 / 255.0f, 1.0f },
																{ 254 / 255.0f, 224 / 255.0f, 182 / 255.0f, 1.0f },
																{ 216 / 255.0f, 218 / 255.0f, 235 / 255.0f, 1.0f },
																{ 178 / 255.0f, 171 / 255.0f, 210 / 255.0f, 1.0f },
																{ 128 / 255.0f, 115 / 255.0f, 172 / 255.0f, 1.0f },
																{ 84 / 255.0f, 39 / 255.0f, 136 / 255.0f, 1.0f },
																{ 45 / 255.0f, 0 / 255.0f, 75 / 255.0f, 1.0f } };

void AnnotationManager::ClearTemporaryShapeFileData()
{
	delete TemporaryShapeFileData;
	TemporaryShapeFileData = nullptr;
	TemporaryFields.clear();
	FieldLabelToConsiderAnnotation = "";
	TemporaryLabelToAnnotationInfo.clear();
}

// Same rule for the popup preview and for the import, so the prepared annotations line up with the created ones.
std::string AnnotationManager::GetFeatureLabel(const ShapeFileFeature& Feature, const std::string& LabelFieldName)
{
	auto FieldIterator = Feature.Fields.find(LabelFieldName);
	if (FieldIterator == Feature.Fields.end())
		return "Unlabeled";

	if (const std::string* Text = std::get_if<std::string>(&FieldIterator->second))
		return Text->empty() ? "Unlabeled" : *Text;

	if (const int64_t* Integer = std::get_if<int64_t>(&FieldIterator->second))
		return std::to_string(*Integer);

	if (const double* Real = std::get_if<double>(&FieldIterator->second))
		return std::to_string(*Real);

	return "Unlabeled";
}

bool AnnotationManager::InitializeReadAnnotationDataFromShapeFile(std::string ShapeFilePath, AnalysisObject* Object)
{
	ClearTemporaryShapeFileData();

	if (Object == nullptr)
		return false;

	if (!FILE_SYSTEM.DoesFileExist(ShapeFilePath))
	{
		LOG.Add("Shape file not found at path: " + ShapeFilePath, "ANNOTATION_MANAGER", FE_LOG_ERROR);
		return false;
	}

	TemporaryShapeFileData = new ShapeFileData();
	TemporaryShapeFileData->Load(ShapeFilePath);
	TemporaryFields = TemporaryShapeFileData->GetFieldDefinitions();

	if (TemporaryFields.empty())
		ClearTemporaryShapeFileData();

	return TemporaryShapeFileData != nullptr;
}

bool AnnotationManager::AddAnnotationsFromShapeFileData(ShapeFileData* CurrentShapeFile, AnalysisObject* Object, const std::string& LabelFieldName)
{
	AnnotationData* ExistingAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(Object->GetID());
	if (ExistingAnnotationData != nullptr)
	{
		ClearTemporaryShapeFileData();
		return false;
	}

	if (!ANNOTATION_MANAGER.AddAnnotationToAnalysisObject(Object->GetID()))
	{
		ClearTemporaryShapeFileData();
		return false;
	}

	ExistingAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(Object->GetID());
	if (ExistingAnnotationData == nullptr)
	{
		ClearTemporaryShapeFileData();
		return false;
	}

	if (DEVELOPER_MODE.IsOn())
		ExistingAnnotationData->SetEditingMode(true);

	FEEntity* Entity = Object->GetEntity();
	FEAABB EntityAABB = MAIN_SCENE_MANAGER.GetMainScene()->GetEntityAABB(Entity);

	ExistingAnnotationData->ClearAllAnnotationsInfo();

	size_t ColorIndex = 0;

	std::vector<std::vector<glm::vec3>> AllPolygonsPoints;
	std::vector<OGRPolygon> AlternativeOGRPolygons;
	PolygonPlane TemporaryPlane;
	std::vector<int> AnnotationIDToPolygonIndex;

	const std::vector<ShapeFileFeature> Features = TemporaryShapeFileData->GetFeatures();
	for (size_t i = 0; i < Features.size(); i++)
	{
		for (size_t j = 0; j < Features[i].Polygons.size(); j++)
		{
			std::string Label = GetFeatureLabel(Features[i], LabelFieldName);

			AnnotationInfo* Info = ExistingAnnotationData->GetAnnotationInfoByName(Label);
			if (Info == nullptr)
			{
				// Prefer the annotation prepared in the import popup, otherwise cycle through the default palette.
				auto PreparedInfo = TemporaryLabelToAnnotationInfo.find(Label);
				if (PreparedInfo != TemporaryLabelToAnnotationInfo.end())
				{
					Info = ExistingAnnotationData->AddAnnotationInfo(Label, PreparedInfo->second.Description, PreparedInfo->second.Color);
				}
				else
				{
					Info = ExistingAnnotationData->AddAnnotationInfo(Label, "", DefaultAnnotationColors[ColorIndex % DefaultAnnotationColors.size()]);
					ColorIndex++;
				}
			}

			std::vector<glm::vec2> PolygonPointsInUV;
			for (size_t k = 0; k < Features[i].Polygons[j].size(); k++)
			{
				glm::vec2 PolygonPoint = Features[i].Polygons[j][k];

				glm::vec3 EntityPosition = Entity->GetComponent<FETransformComponent>().GetPosition();
				glm::vec3 ObjectAppliedShift = Object->GetAppliedShift();

				glm::vec2 PointInEntitySpace = PolygonPoint;
				PointInEntitySpace -= glm::vec2(ObjectAppliedShift.x, ObjectAppliedShift.y);
				PolygonPointsInUV.push_back(PointInEntitySpace);
			}

			std::vector<glm::vec3> CurrentPolygonPoints;
			for (size_t k = 0; k < PolygonPointsInUV.size(); k++)
			{
				glm::vec3 PointIn3DSpace = glm::vec3(PolygonPointsInUV[k], 0.0f);
				CurrentPolygonPoints.push_back(PointIn3DSpace);
			}
			AllPolygonsPoints.push_back(CurrentPolygonPoints);

			AlternativeOGRPolygons.push_back(FEPolygon::CreatedOGRPolygonFromPoints(AllPolygonsPoints.back()));
			AnnotationIDToPolygonIndex.push_back(Info->ID);
		}
	}

	if (Object->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
		{
			ClearTemporaryShapeFileData();
			return false;
		}

		std::vector<glm::vec3> ProjectedTriangleCentroids;
		std::vector<OGRPoint> OGRProjectedTriangleCentroids;
		ProjectedTriangleCentroids.resize(CurrentMeshAnalysisData->TrianglesCentroids.size());
		OGRProjectedTriangleCentroids.resize(CurrentMeshAnalysisData->TrianglesCentroids.size());
		for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
		{
			glm::vec3 TriangleCentroid = CurrentMeshAnalysisData->TrianglesCentroids[i];
			glm::vec3 TransformedTriangleCentroid = Object->GetEntity()->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TriangleCentroid, 1.0f);
		
			ProjectedTriangleCentroids[i] = glm::vec3(CurrentMeshAnalysisData->TrianglesCentroids[i].x, CurrentMeshAnalysisData->TrianglesCentroids[i].y, 0.0f);
			OGRProjectedTriangleCentroids[i] = OGRPoint(ProjectedTriangleCentroids[i].x, ProjectedTriangleCentroids[i].y, ProjectedTriangleCentroids[i].z);
		}

		for (size_t i = 0; i < AllPolygonsPoints.size(); i++)
		{
			std::vector<int> CurrentPolygonResult = FEPolygon::GetTriangleIndicesInPolygon(AllPolygonsPoints[i], AlternativeOGRPolygons[i], ProjectedTriangleCentroids, OGRProjectedTriangleCentroids);
			ExistingAnnotationData->UpdateAnnotationForTriangles(CurrentPolygonResult, AnnotationIDToPolygonIndex[i]);
		}
	}
	else if (Object->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		PointCloudAnalysisData* CurrentPointCloudAnalysisData = Object->GetPointCloudAnalysisData();
		if (CurrentPointCloudAnalysisData == nullptr)
		{
			ClearTemporaryShapeFileData();
			return false;
		}

		const std::vector<FEPointCloudVertexDouble>& Points = CurrentPointCloudAnalysisData->RawPointCloudData;

		std::vector<FEAABB> PolygonAABBs;
		PolygonAABBs.resize(AllPolygonsPoints.size());
		for (size_t i = 0; i < AllPolygonsPoints.size(); i++)
			PolygonAABBs[i] = FEAABB(AllPolygonsPoints[i]);

		size_t ElementCount = std::min(Points.size(), ExistingAnnotationData->PerElementID.size());
		for (size_t i = 0; i < ElementCount; i++)
		{
			glm::vec3 ProjectedPoint = glm::vec3(Points[i].X, Points[i].Y, 0.0f);
			for (size_t j = 0; j < AllPolygonsPoints.size(); j++)
			{
				if (!PolygonAABBs[j].ContainsPoint(ProjectedPoint))
					continue;

				if (FEPolygon::IsPointInsidePolygonXY(AllPolygonsPoints[j], Points[i].X, Points[i].Y))
					ExistingAnnotationData->PerElementID[i] = AnnotationIDToPolygonIndex[j];
			}
		}

		ANNOTATION_MANAGER.UpdateBuffer(ExistingAnnotationData);
	}

	ClearTemporaryShapeFileData();
	return true;
}

bool AnnotationManager::GetColor(const ShapeFileFeature& Feature, glm::vec4& OutColor)
{
	OutColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// Integer fields hold 0 to 255, real fields hold either 0 to 1 or 0 to 255.
	auto ReadChannel = [&](const std::vector<std::string>& PossibleFieldNames, float& OutChannel) -> bool
	{
		for (size_t i = 0; i < PossibleFieldNames.size(); i++)
		{
			int64_t IntegerValue = Feature.GetField(PossibleFieldNames[i], int64_t(-1));
			if (IntegerValue != -1)
			{
				OutChannel = static_cast<float>(IntegerValue) / 255.0f;
				return true;
			}

			double RealValue = Feature.GetField(PossibleFieldNames[i], double(-1.0));
			if (RealValue != -1.0)
			{
				OutChannel = static_cast<float>(RealValue) / (RealValue > 1.0 ? 255.0f : 1.0f);
				return true;
			}
		}

		return false;
	};

	bool bRedFound = ReadChannel({ "red", "r", "Red", "R", "RED" }, OutColor.x);
	bool bGreenFound = ReadChannel({ "green", "g", "Green", "G", "GREEN" }, OutColor.y);
	bool bBlueFound = ReadChannel({ "blue", "b", "Blue", "B", "BLUE" }, OutColor.z);
	bool bAlphaFound = ReadChannel({ "alpha", "a", "Alpha", "A", "ALPHA" }, OutColor.w);

	return bRedFound || bGreenFound || bBlueFound || bAlphaFound;
}

bool AnnotationManager::ReadAnnotationsToPolygonPlane(std::string ShapeFilePath, PolygonPlane* TargetPlane, std::unordered_map<int, AnnotationInfo>& PolygonIndexToAnnotationInfoMap)
{
	if (TargetPlane == nullptr)
		return false;

	ShapeFileData NewShapeFileData;
	if (!NewShapeFileData.Load(ShapeFilePath))
		return false;

	std::unordered_map<std::string, AnnotationInfo> LocalAnnotationInfoMap;
	PolygonIndexToAnnotationInfoMap.clear();

	FEAABB ShapeFileAABB = NewShapeFileData.GetBounds();
	float XRange = ShapeFileAABB.GetMax().x - ShapeFileAABB.GetMin().x;
	float YRange = ShapeFileAABB.GetMax().y - ShapeFileAABB.GetMin().y;

	const std::vector<ShapeFileFeature> Features = NewShapeFileData.GetFeatures();
	for (size_t i = 0; i < Features.size(); i++)
	{
		for (size_t j = 0; j < Features[i].Polygons.size(); j++)
		{
			std::string Label = Features[i].GetField(std::string("label"), std::string(""));
			if (Label.empty())
				Label = "Annotation_" + std::to_string(i) + "_" + std::to_string(j);

			AnnotationInfo* CurrentInfo = nullptr;
			if (LocalAnnotationInfoMap.find(Label) != LocalAnnotationInfoMap.end())
			{
				CurrentInfo = &LocalAnnotationInfoMap[Label];
			}
			else
			{
				LocalAnnotationInfoMap[Label] = AnnotationInfo();
				LocalAnnotationInfoMap[Label].ID = static_cast<int>(LocalAnnotationInfoMap.size() - 1);
				LocalAnnotationInfoMap[Label].Name = Label;

				GetColor(Features[i], LocalAnnotationInfoMap[Label].Color);

				CurrentInfo = &LocalAnnotationInfoMap[Label];
			}

			std::vector<glm::vec2> PolygonPointsInUV;
			for (size_t k = 0; k < Features[i].Polygons[j].size(); k++)
			{
				glm::vec2 PolygonPoint = Features[i].Polygons[j][k];

				glm::vec2 PointInUV;
				PointInUV.x = (PolygonPoint.x - ShapeFileAABB.GetMin().x) / XRange;
				PointInUV.y = (PolygonPoint.y - ShapeFileAABB.GetMin().y) / YRange;

				PolygonPointsInUV.push_back(PointInUV);
			}

			FEPolygon* Polygon = TargetPlane->AddPolygon(PolygonPointsInUV);
			PolygonIndexToAnnotationInfoMap[static_cast<int>(TargetPlane->GetPolygonIndex(Polygon))] = *CurrentInfo;
		}
	}

	return true;
}

void AnnotationManager::Render()
{
	static const char* PopupName = "Import annotations";

	static std::string PreviewField;
	static std::map<std::string, int> LabelPreview;

	// If TemporaryShapeFileData is not nullptr, open the popup.
	if (TemporaryShapeFileData != nullptr && !ImGui::IsPopupOpen(PopupName))
		ImGui::OpenPopup(PopupName);

	ImGui::SetNextWindowSize(ImVec2(480, 0));
	if (!ImGui::BeginPopupModal(PopupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	int WindowW = 0;
	int WindowH = 0;
	APPLICATION.GetMainWindow()->GetSize(&WindowW, &WindowH);
	ImGui::SetWindowPos(ImVec2(WindowW / 2.0f - ImGui::GetWindowWidth() / 2.0f, WindowH / 2.0f - ImGui::GetWindowHeight() / 2.0f));

	auto CloseImport = [&]() {
		ClearTemporaryShapeFileData();
		LabelPreview.clear();
		// Forces the preview to be rebuilt for the next file even if the same field name gets selected.
		PreviewField.clear();
		ImGui::CloseCurrentPopup();
	};

	AnalysisObject* TargetObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (TemporaryShapeFileData == nullptr || TargetObject == nullptr)
	{
		CloseImport();
		ImGui::EndPopup();
		return;
	}

	if (PreviewField != FieldLabelToConsiderAnnotation)
	{
		PreviewField = FieldLabelToConsiderAnnotation;
		LabelPreview.clear();
		TemporaryLabelToAnnotationInfo.clear();

		const std::vector<ShapeFileFeature>& Features = TemporaryShapeFileData->GetFeatures();
		for (size_t i = 0; i < Features.size(); i++)
		{
			std::string Label = GetFeatureLabel(Features[i], FieldLabelToConsiderAnnotation);
			LabelPreview[Label]++;

			// The first feature of a label decides its color: color fields in the file win over the default palette.
			if (TemporaryLabelToAnnotationInfo.find(Label) == TemporaryLabelToAnnotationInfo.end())
			{
				AnnotationInfo NewInfo;
				NewInfo.Name = Label;
				if (!GetColor(Features[i], NewInfo.Color))
					NewInfo.Color = DefaultAnnotationColors[TemporaryLabelToAnnotationInfo.size() % DefaultAnnotationColors.size()];

				TemporaryLabelToAnnotationInfo[Label] = NewInfo;
			}
		}
	}

	ImGui::Text("Object: %s", TargetObject->GetName().c_str());
	ImGui::Text("Features: %d", static_cast<int>(TemporaryShapeFileData->GetFeatures().size()));
	ImGui::Separator();

	ImGui::Text("Field with annotation labels:");
	ImGui::SetNextItemWidth(-FLT_MIN);
	const char* ComboPreview = FieldLabelToConsiderAnnotation.empty() ? "Select field..." : FieldLabelToConsiderAnnotation.c_str();
	if (ImGui::BeginCombo("##AnnotationLabelField", ComboPreview))
	{
		for (size_t i = 0; i < TemporaryFields.size(); i++)
		{
			std::string ItemText = TemporaryFields[i].Name + " (" + OGRFieldDefn::GetFieldTypeName(TemporaryFields[i].Type) + ")";
			bool bIsSelected = TemporaryFields[i].Name == FieldLabelToConsiderAnnotation;
			if (ImGui::Selectable(ItemText.c_str(), bIsSelected))
				FieldLabelToConsiderAnnotation = TemporaryFields[i].Name;

			if (bIsSelected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	ImGui::Text("Distinct labels: %d", static_cast<int>(LabelPreview.size()));

	// Rows follow LabelPreview order, so the changed row index maps back to its label.
	std::vector<std::string> RowLabels;
	std::vector<LabeledColor> Rows;
	for (auto& Entry : LabelPreview)
	{
		RowLabels.push_back(Entry.first);
		Rows.push_back({ Entry.first + ": " + std::to_string(Entry.second), TemporaryLabelToAnnotationInfo[Entry.first].Color });
	}

	int ChangedRowIndex = UI_CORE.ShowLabeledColorTable("##AnnotationLabelPreview", Rows);
	if (ChangedRowIndex != -1)
		TemporaryLabelToAnnotationInfo[RowLabels[ChangedRowIndex]].Color = Rows[ChangedRowIndex].Color;

	// A file in a different coordinate system does not overlap the object at all, warn before anything is assigned.
	ResourceAnalysisData* ObjectAnalysisData = nullptr;
	if (TargetObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		ObjectAnalysisData = TargetObject->GetMeshAnalysisData();
	}
	else if (TargetObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		ObjectAnalysisData = TargetObject->GetPointCloudAnalysisData();
	}

	if (ObjectAnalysisData != nullptr)
	{
		FEAABB ObjectAABB = ObjectAnalysisData->GetAABB();
		FEAABB ShapeFileAABB = TemporaryShapeFileData->GetBounds();
		glm::dvec3 AppliedShift = TargetObject->GetAppliedShift();

		bool bOverlapX = ShapeFileAABB.GetMin().x - AppliedShift.x <= ObjectAABB.GetMax().x && ShapeFileAABB.GetMax().x - AppliedShift.x >= ObjectAABB.GetMin().x;
		bool bOverlapY = ShapeFileAABB.GetMin().y - AppliedShift.y <= ObjectAABB.GetMax().y && ShapeFileAABB.GetMax().y - AppliedShift.y >= ObjectAABB.GetMin().y;
		if (!bOverlapX || !bOverlapY)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
			ImGui::TextWrapped("The file extent does not overlap the object. Check that it uses the same coordinate system as the object.");
			ImGui::PopStyleColor();
		}
	}

	ImGui::Separator();

	// Each button is centered in its own half of the popup.
	const float ButtonWidth = 120.0f;
	const float ContentWidth = ImGui::GetContentRegionAvail().x;
	const float ContentStartX = ImGui::GetCursorPosX();
	const float ImportButtonX = ContentStartX + ContentWidth * 0.25f - ButtonWidth / 2.0f;
	const float CancelButtonX = ContentStartX + ContentWidth * 0.75f - ButtonWidth / 2.0f;

	bool bCanImport = !FieldLabelToConsiderAnnotation.empty();
	if (!bCanImport)
		ImGui::BeginDisabled();

	ImGui::SetCursorPosX(ImportButtonX);
	if (ImGui::Button("Import", ImVec2(ButtonWidth, 0)))
	{
		AddAnnotationsFromShapeFileData(TemporaryShapeFileData, TargetObject, FieldLabelToConsiderAnnotation);
		CloseImport();
	}

	if (!bCanImport)
		ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::SetCursorPosX(CancelButtonX);
	if (ImGui::Button("Cancel", ImVec2(ButtonWidth, 0)))
		CloseImport();

	ImGui::EndPopup();
}

#include "../UI/UIManager.h"
bool AnnotationManager::UpdateHistogramWithAnnotationData()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return false;

	if (ActiveObject->GetType() != DATA_SOURCE_TYPE::MESH && ActiveObject->GetType() != DATA_SOURCE_TYPE::POINT_CLOUD)
		return false;

	DataLayer* ActiveLayerData = ActiveObject->GetActiveLayer();
	if (ActiveLayerData == nullptr)
		return false;

	AnnotationData* AnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(ActiveObject->GetID());
	if (AnnotationData == nullptr)
		return false;

	// Stacks are assigned again below, a stale index would let a color change recolor the wrong stack.
	for (size_t i = 0; i < AnnotationData->UsedAnnotations.size(); i++)
		AnnotationData->UsedAnnotations[i].StackGraphIndex = -1;

	FEWeightedHistogram* Histogram = UI.GetHistogramPointer();
	Histogram->Clear();

	std::vector<std::tuple<double, double, int>> NoAnnotationHistogramData;
	if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return false;

		FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			return false;

		for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
		{
			if (AnnotationData->PerElementID[i] == -1)
			{
				double CurrentLayerTriangleValue = ActiveLayerData->ElementsToData[i];
				NoAnnotationHistogramData.push_back(std::make_tuple(CurrentLayerTriangleValue, CurrentMeshAnalysisData->TrianglesArea[i], i));
			}
		}
	}
	else if (ActiveObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		size_t ElementCount = std::min(AnnotationData->PerElementID.size(), ActiveLayerData->ElementsToData.size());
		for (size_t i = 0; i < ElementCount; i++)
		{
			if (AnnotationData->PerElementID[i] == -1)
			{
				double CurrentLayerPointValue = ActiveLayerData->ElementsToData[i];
				// For point clouds, each point has weight equal to 1.0.
				NoAnnotationHistogramData.push_back(std::make_tuple(CurrentLayerPointValue, 1.0, static_cast<int>(i)));
			}
		}
	}

	std::vector<double> Values;
	std::vector<double> Weights;
	for (const auto& Tuple : NoAnnotationHistogramData)
	{
		Values.push_back(std::get<0>(Tuple));
		Weights.push_back(std::get<1>(Tuple));
	}

	std::vector<FEGraphDataPoint> NoAnnotationGraphDataPoints = Histogram->ConvertToDataPoints(Values, Weights,
																							   Histogram->GetBinCount(), ActiveLayerData->GetMin(),
																							   ActiveLayerData->GetMax());
	Histogram->GetGraphPointer()->AddDataPoints(NoAnnotationGraphDataPoints);

	FEGraphStackInfo* NoAnnotationStackInfo = Histogram->GetGraphPointer()->GetStackInfoByID(0);
	if (NoAnnotationStackInfo != nullptr)
		NoAnnotationStackInfo->Name = "No annotation";

	std::vector<AnnotationInfo>& AllAnnotationInfo = AnnotationData->UsedAnnotations;
	std::vector<std::vector<FEGraphDataPoint>> AnnotationGraphDataPoints;
	int GraphStackIndex = 1;
	for (size_t i = 0; i < AllAnnotationInfo.size(); i++)
	{
		if (AllAnnotationInfo[i].HistogramData.empty())
			continue;

		Values.clear();
		Weights.clear();
		for (const auto& Tuple : AllAnnotationInfo[i].HistogramData)
		{
			Values.push_back(std::get<0>(Tuple));
			Weights.push_back(std::get<1>(Tuple));
		}

		std::vector<FEGraphDataPoint> CurrentAnnotationGraphDataPoints = Histogram->ConvertToDataPoints(Values, Weights,
																										Histogram->GetBinCount(), ActiveLayerData->GetMin(),
																										ActiveLayerData->GetMax());
		for (size_t j = 0; j < CurrentAnnotationGraphDataPoints.size(); j++)
			CurrentAnnotationGraphDataPoints[j].StackID = GraphStackIndex;

		if (!CurrentAnnotationGraphDataPoints.empty())
		{
			AllAnnotationInfo[i].StackGraphIndex = GraphStackIndex;
			GraphStackIndex++;
		}
		else
		{
			AllAnnotationInfo[i].StackGraphIndex = -1;
		}

		AnnotationGraphDataPoints.push_back(CurrentAnnotationGraphDataPoints);
		UI.GetHistogramPointer()->GetGraphPointer()->AddDataPoints(AnnotationGraphDataPoints.back());

		if (AllAnnotationInfo[i].StackGraphIndex != -1)
		{
			FEGraphStackInfo* CurrentStackInfo = UI.GetHistogramPointer()->GetGraphPointer()->GetStackInfoByID(AllAnnotationInfo[i].StackGraphIndex);
			if (CurrentStackInfo != nullptr)
			{
				CurrentStackInfo->Name = "Annotation: " + AllAnnotationInfo[i].Name;

				glm::vec4 Color = AllAnnotationInfo[i].GetColor();
				CurrentStackInfo->StartGradientColor = ImColor(Color.x, Color.y, Color.z);
				CurrentStackInfo->EndGradientColor = ImColor(Color.x, Color.y, Color.z);
			}
		}
	}

	return true;
}