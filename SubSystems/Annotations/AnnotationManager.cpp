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

std::vector<AnnotationInfo> AnnotationData::GetAllAnnotationInfos()
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
	for (size_t i = 0; i < PerTriangleID.size(); i++)
		PerTriangleID[i] = -1;

	ANNOTATION_MANAGER.UpdateBuffer(this);
}

bool AnnotationData::UpdateAnnotationForTriangle(int TriangleIndex, int AnnotationID)
{
	if (TriangleIndex < 0 || TriangleIndex >= PerTriangleID.size())
		return false;

	PerTriangleID[TriangleIndex] = AnnotationID;
	ANNOTATION_MANAGER.UpdateBuffer(this);
	return true;
}

bool AnnotationData::UpdateAnnotationForTriangles(std::vector<int>& TriangleIndexes, int AnnotationID)
{
	bool bUpdatedAtLeastOne = false;
	for (size_t i = 0; i < TriangleIndexes.size(); i++)
	{
		if (TriangleIndexes[i] < 0 || TriangleIndexes[i] >= PerTriangleID.size())
			continue;

		PerTriangleID[TriangleIndexes[i]] = AnnotationID;
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

	if (ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.find(CurrentObject->GetID()) == ANNOTATION_MANAGER.AnalisysObjectsToAnnotationData.end())
		return false;

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
		FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, CurrentAnnotationData->AnnotationSSBO));
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CurrentAnnotationData->AnnotationSSBO));

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

	if (Data->DataBufferID == GLuint(-1))
	{
		InitalizeBuffer(Data);
		// If it is still -1, it means that buffer initialization failed, so we should not try to update it.
		if (Data->DataBufferID == GLuint(-1))
		{
			LOG.Add("Failed to initialize annotation data buffer for analysis object with ID: " + Object->GetID(), "ANNOTATION_MANAGER", FE_LOG_ERROR);
			return;
		}
	}

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
	Data->UpdateColorInfoOnGPU();
	UpdateHistogramData(Data);
}

bool AnnotationManager::ReadBackBuffer(AnnotationData* Data)
{
	if (Data == nullptr)
		return false;

	if (Data->DataBufferID == GLuint(-1))
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
	FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, Data->DataBufferID));
	FE_GL_ERROR(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
		sizeof(glm::vec4) * VertexCount,
		Data->FinalPerVertexData.data()));
	FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

	Data->PerTriangleID.resize(TriangleCount);
	for (size_t i = 0; i < TriangleCount; i++)
	{
		const int VertexIndex = CurrentMeshAnalysisData->Indices[i * 3];
		Data->PerTriangleID[i] = static_cast<int>(std::lround(Data->FinalPerVertexData[VertexIndex].x));
	}

	UpdateHistogramData(Data);
	return true;
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

bool AnnotationManager::ReadAndAddAnnotationsFromShapeFile(std::string ShapeFilePath, AnalysisObject* Object)
{
	if (Object == nullptr)
		return false;

	AnnotationData* ExistingAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(Object->GetID());
	if (ExistingAnnotationData != nullptr)
		return false;

	if (!FILE_SYSTEM.DoesFileExist(ShapeFilePath))
	{
		LOG.Add("Shape file not found at path: " + ShapeFilePath, "ANNOTATION_MANAGER", FE_LOG_ERROR);
		return false;
	}

	if (!ANNOTATION_MANAGER.AddAnnotationToAnalysisObject(Object->GetID()))
		return false;

	ExistingAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(Object->GetID());
	if (ExistingAnnotationData == nullptr)
		return false;

	ShapeFileData NewShapeFileData;
	NewShapeFileData.Load(ShapeFilePath);

	FEAABB ShapeFileAABB = NewShapeFileData.GetBounds();
	float XRange = ShapeFileAABB.GetMax().x - ShapeFileAABB.GetMin().x;
	float YRange = ShapeFileAABB.GetMax().y - ShapeFileAABB.GetMin().y;
	float ZRange = ShapeFileAABB.GetMax().z - ShapeFileAABB.GetMin().z;

	ExistingAnnotationData->SetEditingMode(true);

	FEEntity* Entity = Object->GetEntity();
	FEAABB EntityAABB = MAIN_SCENE_MANAGER.GetMainScene()->GetEntityAABB(Entity);

	ExistingAnnotationData->ClearAllAnnotationsInfo();

	std::vector<glm::vec4> ColorFromColorBrewer = { { 127 / 255.0f, 59 / 255.0f, 8 / 255.0f, 1.0f },
													{ 179 / 255.0f, 88 / 255.0f, 6 / 255.0f, 1.0f },
													{ 224 / 255.0f, 130 / 255.0f, 20 / 255.0f, 1.0f },
													{ 253 / 255.0f, 184 / 255.0f, 99 / 255.0f, 1.0f },
													{ 254 / 255.0f, 224 / 255.0f, 182 / 255.0f, 1.0f },
													{ 216 / 255.0f, 218 / 255.0f, 235 / 255.0f, 1.0f },
													{ 178 / 255.0f, 171 / 255.0f, 210 / 255.0f, 1.0f },
													{ 128 / 255.0f, 115 / 255.0f, 172 / 255.0f, 1.0f },
													{ 84 / 255.0f, 39 / 255.0f, 136 / 255.0f, 1.0f },
													{ 45 / 255.0f, 0 / 255.0f, 75 / 255.0f, 1.0f } };
	size_t ColorIndex = 0;

	std::vector<std::vector<glm::vec3>> AllPolygonsPoints;
	std::vector<OGRPolygon> AlternativeOGRPolygons;
	PolygonPlane TemporaryPlane;
	std::vector<int> AnnotationIDToPolygonIndex;

	const std::vector<ShapeFileFeature> Features = NewShapeFileData.GetFeatures();
	for (size_t i = 0; i < Features.size(); i++)
	{
		for (size_t j = 0; j < Features[i].Polygons.size(); j++)
		{
			std::string Label = Features[i].GetField(std::string("label") , std::string(""));

			AnnotationInfo* Info = ExistingAnnotationData->GetAnnotationInfoByName(Label);
			if (Info == nullptr)
			{
				glm::vec4 AssignedColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
				if (ColorIndex < ColorFromColorBrewer.size())
					AssignedColor = ColorFromColorBrewer[ColorIndex];

				Info = ExistingAnnotationData->AddAnnotationInfo(Label, "", AssignedColor);
				ColorIndex = (ColorIndex + 1) % ColorFromColorBrewer.size();
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

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return true;

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

	return true;
}

glm::vec4 AnnotationManager::GetColor(const ShapeFileFeature& Feature)
{
	glm::vec4 Result = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	std::vector<std::string> PossibleRedColorFieldNames = { "red", "r", "Red", "R", "RED" };
	for (size_t i = 0; i < PossibleRedColorFieldNames.size(); i++)
	{
		int64_t Red = Feature.GetField(PossibleRedColorFieldNames[i], int64_t(-1));
		if (Red != -1)
		{
			Result.x = static_cast<float>(Red) / 255.0f;
			break;
		}

		double RedDouble = Feature.GetField(PossibleRedColorFieldNames[i], double(-1.0));
		if (RedDouble != -1.0)
		{
			Result.x = static_cast<float>(RedDouble) / (RedDouble > 1.0 ? 255.0f : 1.0f);
			break;
		}
	}

	std::vector<std::string> PossibleGreenColorFieldNames = { "green", "g", "Green", "G", "GREEN" };
	for (size_t i = 0; i < PossibleGreenColorFieldNames.size(); i++)
	{
		int64_t Green = Feature.GetField(PossibleGreenColorFieldNames[i], int64_t(-1));
		if (Green != -1)
		{
			Result.y = static_cast<float>(Green) / 255.0f;
			break;
		}

		double GreenDouble = Feature.GetField(PossibleGreenColorFieldNames[i], double(-1.0));
		if (GreenDouble != -1.0)
		{
			Result.y = static_cast<float>(GreenDouble) / (GreenDouble > 1.0 ? 255.0f : 1.0f);
			break;
		}
	}

	std::vector<std::string> PossibleBlueColorFieldNames = { "blue", "b", "Blue", "B", "BLUE" };
	for (size_t i = 0; i < PossibleBlueColorFieldNames.size(); i++)
	{
		int64_t Blue = Feature.GetField(PossibleBlueColorFieldNames[i], int64_t(-1));
		if (Blue != -1)
		{
			Result.z = static_cast<float>(Blue) / 255.0f;
			break;
		}

		double BlueDouble = Feature.GetField(PossibleBlueColorFieldNames[i], double(-1.0));
		if (BlueDouble != -1.0)
		{
			Result.z = static_cast<float>(BlueDouble) / (BlueDouble > 1.0 ? 255.0f : 1.0f);
			break;
		}
	}

	std::vector<std::string> PossibleAlphaColorFieldNames = { "alpha", "a", "Alpha", "A", "ALPHA" };
	for (size_t i = 0; i < PossibleAlphaColorFieldNames.size(); i++)
	{
		int64_t Alpha = Feature.GetField(PossibleAlphaColorFieldNames[i], int64_t(-1));
		if (Alpha != -1)
		{
			Result.w = static_cast<float>(Alpha) / 255.0f;
			break;
		}

		double AlphaDouble = Feature.GetField(PossibleAlphaColorFieldNames[i], double(-1.0));
		if (AlphaDouble != -1.0)
		{
			Result.w = static_cast<float>(AlphaDouble) / (AlphaDouble > 1.0 ? 255.0f : 1.0f);
			break;
		}
	}

	return Result;
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
				LocalAnnotationInfoMap[Label].ID = LocalAnnotationInfoMap.size() - 1;
				LocalAnnotationInfoMap[Label].Name = Label;

				glm::vec4 CurrentColor = GetColor(Features[i]);
				LocalAnnotationInfoMap[Label].Color = CurrentColor;

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
			PolygonIndexToAnnotationInfoMap[TargetPlane->GetPolygonIndex(Polygon)] = *CurrentInfo;
		}
	}

	return true;
}