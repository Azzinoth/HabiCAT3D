#pragma once

#include "../ComplexityCore/Layers/LayerManager.h"
#include "ShapeFileData.h"
#include "PolygonPlane.h"
using namespace FocalEngine;

struct AnnotationInfo
{
	friend struct AnnotationData;
	friend class AnnotationManager;
	friend class AnalysisObjectManager;
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
	friend class AnalysisObjectManager;
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

	void ClearAllAnnotation();
	bool UpdateAnnotationForTriangle(int TriangleIndex, int AnnotationID);
	bool UpdateAnnotationForTriangles(std::vector<int>& TriangleIndexes, int AnnotationID);

	std::vector< AnnotationInfo> GetAllAnnotationInfos();
	void ClearAllAnnotationsInfo();

	AnnotationInfo* AddAnnotationInfo(std::string Name, std::string Description, glm::vec4 Color);
	AnnotationInfo* GetAnnotationInfoByID(int ID);
	AnnotationInfo* GetAnnotationInfoByPolygonIndex(int PolygonIndex);
	AnnotationInfo* GetAnnotationInfoByName(std::string Name);

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

	bool ReadAndAddAnnotationsFromShapeFile(std::string ShapeFilePath, AnalysisObject* Object);
	bool ReadAnnotationsToPolygonPlane(std::string ShapeFilePath, PolygonPlane* TargetPlane, std::unordered_map<int, AnnotationInfo>& PolygonIndexToAnnotationInfoMap);

	void InitalizeBuffer(AnnotationData* Data);
	void UpdateBuffer(AnnotationData* Data);
	bool ReadBackBuffer(AnnotationData* Data);
private:
	SINGLETON_PRIVATE_PART(AnnotationManager)

	std::unordered_map<std::string, AnnotationData*> AnalisysObjectsToAnnotationData;

	static void OnAnalysisObjectDelete(AnalysisObject* DeletedObject);

	static void BeforeRender(FEEntity* CurrentEntity);

	static void OnLayerChange();
	void UpdateHistogramData(AnnotationData* Data);

	glm::vec4 GetColor(const ShapeFileFeature& Feature);
};

#define ANNOTATION_MANAGER AnnotationManager::GetInstance()