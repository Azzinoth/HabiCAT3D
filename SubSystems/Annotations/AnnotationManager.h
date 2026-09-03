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
	GLuint MeshBufferID = GLuint(-1);
	std::vector<int> PerElementID;
	std::vector<glm::vec4> FinalPerVertexData;

	// Point cloud only.
	std::vector<GLuint> AnnotationIDComputeShaderBuffers;
	std::vector<GLuint> OriginalColorComputeShaderBuffers;

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

	bool InitializeReadAnnotationDataFromShapeFile(std::string ShapeFilePath, AnalysisObject* Object);
	bool AddAnnotationsFromShapeFileData(ShapeFileData* CurrentShapeFile, AnalysisObject* Object, const std::string& LabelFieldName);

	bool ReadAnnotationsToPolygonPlane(std::string ShapeFilePath, PolygonPlane* TargetPlane, std::unordered_map<int, AnnotationInfo>& PolygonIndexToAnnotationInfoMap);

	void InitalizeBuffer(AnnotationData* Data);
	void UpdateBuffer(AnnotationData* Data);
	bool ReadBackBuffer(AnnotationData* Data);

	void Render();
private:
	SINGLETON_PRIVATE_PART(AnnotationManager)

	std::unordered_map<std::string, AnnotationData*> AnalisysObjectsToAnnotationData;

	ShapeFileData* TemporaryShapeFileData = nullptr;
	std::vector<ShapeFileFieldInfo> TemporaryFields;
	std::string FieldLabelToConsiderAnnotation = "";
	std::map<std::string, AnnotationInfo> TemporaryLabelToAnnotationInfo;
	void ClearTemporaryShapeFileData();
	std::string GetFeatureLabel(const ShapeFileFeature& Feature, const std::string& LabelFieldName);

	static void OnAnalysisObjectDelete(AnalysisObject* DeletedObject);

	static void BeforeRender(FEEntity* CurrentEntity);

	static void OnLayerChange();
	void UpdateHistogramData(AnnotationData* Data);

	void UpdatePointCloudBuffers(AnnotationData* Data);

	// Reads red, green, blue and alpha fields of a feature. Returns false when the feature has none of them, OutColor is white then.
	bool GetColor(const ShapeFileFeature& Feature, glm::vec4& OutColor);
};

#define ANNOTATION_MANAGER AnnotationManager::GetInstance()