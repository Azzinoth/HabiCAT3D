#pragma once

#include "../ComplexityCore/Layers/LayerManager.h"
#include "PolygonPlane.h"
using namespace FocalEngine;

struct AnnotationInfo
{
	friend struct AnnotationData;
private:
	glm::vec4 Color;
public:
	int ID = -1;
	std::string Name;
	std::string Description;
	glm::vec4 GetColor() const;

	int StackGraphIndex = -1;
	std::vector<std::tuple<double, double, int>> HistogramData;
};

enum class ANNOTATION_SOURCE_TYPE
{
	POLYGON = 0,
	TEXTURE = 1,
};

struct AnnotationData
{
	friend class AnnotationManager;
private:
	std::vector<AnnotationInfo> UsedAnnotations;
	void UpdateColorInfoOnGPU();

	std::string AnalysisObjectID;
	std::string EntityID = "";

	bool bInEditingMode = false;

	PolygonPlane* Plane = nullptr;
	std::vector<std::pair<int, int>> AnnotationIDToPolygonIndices;
public:
	AnnotationData(std::string AnalysisObjectID);
	~AnnotationData();

	GLuint AnnotationSSBO = GLuint(-1);
	GLuint DataBufferID = GLuint(-1);
	std::vector<int> PerTriangleID;
	std::vector<glm::vec4> FinalPerVertexData;

	std::vector< AnnotationInfo> GetAllAnnotationInfos();
	AnnotationInfo* GetAnnotationInfoByID(int ID);
	AnnotationInfo* GetAnnotationInfoByPolygonIndex(int PolygonIndex);
	bool IsPolygonIndexAnnotated(int PolygonIndex);
	bool SetPolygonIndexAnnotation(int PolygonIndex, int AnnotationID);

	AnalysisObject* GetAnalysisObject();
	FEEntity* GetEntity();

	bool IsInEditingMode() const;
	void SetEditingMode(bool bEnable);
	PolygonPlane* GetPolygonPlane();

	bool UpdateAnnotationColor(int AnnotationID, const glm::vec4& NewColor);
};

class AnnotationManager
{
	friend struct AnnotationData;
public:
	SINGLETON_PUBLIC_PART(AnnotationManager)

	void Initialize();

	bool AddAnnotationToAnalysisObject(std::string AnalysisObjectID);
	AnnotationData* GetAnnotationDataByAnalysisObjectID(std::string AnalysisObjectID);
	AnnotationData* GetAnnotationDataByEntityID(std::string EntityID);
	bool RemoveAnnotationFromAnalysisObject(std::string AnalysisObjectID);

	void InitalizeBuffer(AnnotationData* Data);
	void UpdateBuffer(AnnotationData* Data);
private:
	SINGLETON_PRIVATE_PART(AnnotationManager)

	std::unordered_map<std::string, AnnotationData*> AnalisysObjectsToAnnotationData;

	static void OnAnalysisObjectLoad(AnalysisObject* NewObject);
	static void OnAnalysisObjectDelete(AnalysisObject* DeletedObject);

	static void BeforeRender(FEEntity* CurrentEntity);

	static void OnLayerChange();
	void UpdateHistogramData(AnnotationData* Data);
};

#define ANNOTATION_MANAGER AnnotationManager::GetInstance()