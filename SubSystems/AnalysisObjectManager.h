#pragma once
#include "ComplexityCore/Layers/DataLayer.h"
using namespace FocalEngine;

class FECGALWrapper;
class AnalysisObjectManager
{
	friend FECGALWrapper;
public:
	SINGLETON_PUBLIC_PART(AnalysisObjectManager)

	FEShader* CustomMeshShader = nullptr;
	FEMaterial* CustomMaterial = nullptr;

	FEShader* PointCloudRecoloringShader = nullptr;
	GLuint TurboColorBuffer = -1;

	void ClearAll();

	AnalysisObject* ImportOBJ(const char* FilePath, bool bForceOneMesh);
	void LoadResource(std::string FilePath);

	void SaveToRUGFile(std::string FilePath);
	void SaveToRUGFileAskForFilePath();

	size_t GetAnalysisObjectCount();
	AnalysisObject* GetAnalysisObject(std::string ID);
	std::vector<std::string> AnalysisObjectManager::GetAnalysisObjectsIDList();

	bool SetActiveAnalysisObject(std::string ID);
	AnalysisObject* GetActiveAnalysisObject();
	FEEntity* GetActiveEntity();

	bool DeleteAnalysisObject(std::string ID);

	void AddOnActiveObjectChangeCallback(std::function<void(AnalysisObject*)> Callback);
	void AddOnObjectDeleteCallback(std::function<void(AnalysisObject*)> Callback);
	void AddOnLoadCallback(std::function<void(AnalysisObject*)> Callback);

	void ComplexityMetricDataToGPU(std::string LayerID, int GPULayerIndex = 0);

	bool SelectTriangle(glm::dvec3 MouseRay);
	bool SelectTrianglesInRadius(glm::dvec3 MouseRay, float Radius);
	glm::vec3 IntersectTriangle(glm::dvec3 MouseRay);

	FEAABB GetAllObjectsAABB();
	double GetAllMeshObjectsTotalArea();
	glm::vec3 GetAllMeshObjectsAverageNormal();
private:
	SINGLETON_PRIVATE_PART(AnalysisObjectManager)

	float CheckRUGFileVersion(std::string FilePath);

	void OnAnalysisObjectLoad(AnalysisObject* NewObject);
	AnalysisObject* LoadRUGFile(std::string FilePath);
	bool NewLoadRUGFile(std::string FilePath);
	void SaveAnalysisDataToRUGFile(std::fstream& File, AnalysisObject* Object);

	void LoadMeshDataFromRUGFile(std::fstream& File, AnalysisObject* Object);
	void SaveMeshDataToRUGFile(std::fstream& File, AnalysisObject* Object);

	void LoadPointCloudDataFromRUGFile(std::fstream& File, AnalysisObject* Object);
	void SavePointCloudToRUGFile(std::fstream& File, AnalysisObject* Object);

	void LoadLayersDataFromRUGFile(std::fstream& File, AnalysisObject* Object);
	void SaveLayersDataToRUGFile(std::fstream& File, AnalysisObject* Object);

	std::vector<std::function<void(AnalysisObject*)>> ClientOnLoadCallbacks;
	std::vector<std::function<void(AnalysisObject*)>> ClientOnActiveObjectChangeCallbacks;
	std::vector<std::function<void(AnalysisObject*)>> ClientOnObjectDeleteCallbacks;

	std::unordered_map<std::string, AnalysisObject*> AnalysisObjects;
	std::string ActiveAnalysisObjectID = "";
	MeshAnalysisData* ExtractAdditionalGeometryData(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals);
	PointCloudAnalysisData* ExtractAdditionalGeometryData(FEPointCloud* PointCloud);

	void InitializeSceneObjects(AnalysisObject* NewAnalysisObject);

	void UpdateMeshUniforms(AnalysisObject* Object);
	static void BeforeRender(FEEntity* CurrentEntity);

	static std::vector<int> GetVertexAttributeIndexes(int InterpolationLayerCount);
};

#define ANALYSIS_OBJECT_MANAGER AnalysisObjectManager::GetInstance()