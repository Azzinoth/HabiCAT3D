#pragma once

#include "ComplexityCore/Layers/LayerManager.h"
using namespace FocalEngine;

struct AnnotationData
{
	friend class AnnotationManager;
	AnnotationData::AnnotationData();

	std::string AnalysisObjectID;
	GLuint AnnotationSSBO = GLuint(-1);

	std::vector<glm::vec4> CurrentColors;
	void UpdateColors(const std::vector<glm::vec4>& Colors);

	GLuint DataBufferID = GLuint(-1);
	std::vector<glm::vec4> TrianglesIndexToData;
};

class AnnotationManager
{
	friend struct AnnotationData;
public:
	SINGLETON_PUBLIC_PART(AnnotationManager)

	void Initialize();

	bool IsVisualizationActive();
	void SetVisualizationActive(bool bActive);

	AnnotationData* GetAnnotationDataByAnalysisObjectID(std::string AnalysisObjectID);
	void InitalizeBuffer(AnnotationData* Data);
	void UpdateBuffer(AnnotationData* Data);
private:
	SINGLETON_PRIVATE_PART(AnnotationManager)

	std::vector<glm::vec4> DefaultAnnotationColors;
	std::unordered_map<std::string, AnnotationData*> AnalisysObjectsToAnnotationData;

	bool bIsVisualizationActive = false;

	static void OnAnalysisObjectLoad(AnalysisObject* NewObject);
	static void OnAnalysisObjectDelete(AnalysisObject* DeletedObject);

	static void BeforeRender(FEEntity* CurrentEntity);
};

#define ANNOTATION_MANAGER AnnotationManager::GetInstance()