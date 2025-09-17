#pragma once
#include "../../EngineInclude.h"
using namespace FocalEngine;

#define APP_VERSION 0.91f

const COMDLG_FILTERSPEC RUGOSITY_LOAD_FILE_FILTER[] =
{
	{ L"Mesh files (*.obj; *.rug)", L"*.obj;*.rug" }
};

const COMDLG_FILTERSPEC RUGOSITY_SAVE_FILE_FILTER[] =
{
	{ L"Rugosity file (*.rug)", L"*.rug" }
};

enum class DATA_SOURCE_TYPE
{
	UNKNOWN = -1,
	MESH = 0,
	POINT_CLOUD = 1
};

class ResourceAnalysisData
{
	friend class AnalysisObjectManager;
	friend class AnalysisObjectManager;

	FEAABB AABB;

public:
	virtual ~ResourceAnalysisData() {}

	FEAABB GetAABB() { return AABB; }
	// Transform is saved here beacause in console mode there is no entity to hold it
	FETransformComponent* Position = new FETransformComponent();
};

class MeshAnalysisData : public ResourceAnalysisData
{
	friend class AnalysisObjectManager;
	friend class AnalysisObjectManager;

	double TotalArea = 0.0;
	glm::vec3 AverageNormal = glm::vec3();

	GLuint FirstLayerBufferID = 0;
	GLuint SecondLayerBufferID = 0;

	int HeatMapType = 5;

	float MeasuredRugosityAreaRadius = -1.0f;
	glm::vec3 MeasuredRugosityAreaCenter = glm::vec3(0.0f);

	float UnselectedAreaSaturationFactor = 0.3f;
	float UnselectedAreaBrightnessFactor = 0.2f;
public:
	double GetTotalArea();

	std::vector<double> Vertices;
	std::vector<float> Colors;
	std::vector<float> UVs;
	std::vector<float> Tangents;
	std::vector<int> Indices;
	std::vector<float> Normals;

	std::vector<int> TriangleSelected;

	std::vector<std::vector<glm::dvec3>> Triangles;
	std::vector<double> TrianglesArea;

	std::vector<std::vector<glm::vec3>> TrianglesNormals;
	std::vector<glm::dvec3> TrianglesCentroids;

	glm::vec3 GetAverageNormal();
	void UpdateAverageNormal();

	GLuint GetFirstLayerBufferID();
	GLuint GetSecondLayerBufferID();

	int GetHeatMapType();
	void SetHeatMapType(int NewValue);

	void GetMeasuredRugosityArea(float& Radius, glm::vec3& Center);
	void ClearMeasuredRugosityArea();

	float GetUnselectedAreaSaturationFactor();
	void SetUnselectedAreaSaturationFactor(float NewValue);
	float GetUnselectedAreaBrightnessFactor();
	void SetUnselectedAreaBrightnessFactor(float NewValue);
};

class PointCloudAnalysisData : public ResourceAnalysisData
{
	friend class AnalysisObjectManager;
public:
	std::vector<FEPointCloudVertexDouble> RawPointCloudData;
	std::vector<std::vector<unsigned char>> OriginalColors;
};

class DataLayer;
class AnalysisObject
{
	friend class AnalysisObjectManager;
	friend class LayerManager;

	std::string ID;
	std::string Name;
	std::string FilePath;

	bool bRenderInScene = true;

	ResourceAnalysisData* AnalysisData = nullptr;
	DATA_SOURCE_TYPE Type = DATA_SOURCE_TYPE::UNKNOWN;

	std::string ActiveLayerID = "";

	FEObject* EngineResource = nullptr;
	FEEntity* Entity = nullptr;
public:
	AnalysisObject();
	~AnalysisObject();

	std::string GetID();
	std::string GetName();
	void SetName(std::string NewName);
	std::string GetFilePath();

	bool IsRenderedInScene();
	void SetRenderInScene(bool NewValue);

	ResourceAnalysisData* GetAnalysisData();
	MeshAnalysisData* GetMeshAnalysisData();
	PointCloudAnalysisData* GetPointCloudAnalysisData();
	DATA_SOURCE_TYPE GetType();
	FEObject* GetEngineResource();
	FEEntity* GetEntity();

	std::vector<DataLayer*> Layers;

	size_t GetLayerCount();

	DataLayer* GetActiveLayer();
	int GetActiveLayerIndex();
	bool SetActiveLayer(std::string LayerID, bool bForceUpdate = false);
	void ClearActiveLayer();
	bool RemoveLayer(std::string LayerID);

	bool AddLayer(DataLayer* NewLayer);
	DataLayer* GetLayer(std::string LayerID);
};