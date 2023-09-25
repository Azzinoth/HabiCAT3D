#pragma once

#include "FETexture.h"
#include "SubSystems/FEGeometricTools.h"

#define APP_VERSION 0.47

namespace FocalEngine
{
	struct DebugEntry
	{
		DebugEntry();
		DebugEntry(std::string Type, int Size, char* RawData);

		std::string Name;
		char* RawData = nullptr;
		std::string Type;
		int Size;

		std::string ToString();
	};

	struct MeshLayerDebugInfo
	{
		uint64_t StartCalculationsTime;
		uint64_t EndCalculationsTime;

		std::string Type = "MeshLayerDebugInfo";

		virtual std::string ToString();

		virtual void FromFile(std::fstream& File);
		virtual void ToFile(std::fstream& File);

		std::vector<DebugEntry> Entries;

		void AddEntry(std::string Name, bool Data);
		void AddEntry(std::string Name, int Data);
		void AddEntry(std::string Name, float Data);
		void AddEntry(std::string Name, double Data);
		void AddEntry(std::string Name, uint64_t Data);
		void AddEntry(std::string Name, std::string Data);
	};

	class FEMesh;
	enum LAYER_TYPE
	{
		UNKNOWN = 0,
		HEIGHT = 1,
		TRIANGLE_EDGE = 2,
		AREA = 3,
		COMPARE = 4,
		STANDARD_DEVIATION = 5,
		RUGOSITY = 6,
		VECTOR_DISPERSION = 7,
		FRACTAL_DIMENSION = 8
	};

	class MeshLayer
	{
		FEMesh* ParentMesh = nullptr;
		std::string ID;
		std::string Caption = "Layer caption";
		std::string UserNote;
		LAYER_TYPE Type = LAYER_TYPE::UNKNOWN;

		float Mean = -FLT_MAX;
		float Median = -FLT_MAX;

		void FillRawData();
		void CalculateInitData();

		float SelectedRangeMin = 0.0f;
		float SelectedRangeMax = 0.0f;
	public:
		MeshLayer();
		MeshLayer(FEMesh* Parent, std::vector<float> TrianglesToData);
		~MeshLayer();

		void ForceID(std::string ID);

		MeshLayerDebugInfo* DebugInfo = nullptr;

		std::string GetCaption();
		void SetCaption(std::string NewValue);

		std::string GetNote();
		void SetNote(std::string NewValue);

		FEMesh* GetParentMesh();
		void SetParentMesh(FEMesh* NewValue);

		float GetMean();
		float GetMedian();

		float Min = FLT_MAX;
		float MinVisible = FLT_MAX;
		float Max = -FLT_MAX;
		float MaxVisible = FLT_MAX;

		std::vector<float> RawData;
		std::vector<float> TrianglesToData;

		void FillDataToGPU(int LayerIndex = 0);
		std::vector<std::tuple<double, double, int>> ValueTriangleAreaAndIndex = std::vector<std::tuple<double, double, int>>();

		LAYER_TYPE GetType();
		void SetType(LAYER_TYPE NewValue);

		float GetSelectedRangeMin();
		void SetSelectedRangeMin(float NewValue);

		float GetSelectedRangeMax();
		void SetSelectedRangeMax(float NewValue);
	};

	class MeshManager;
	class LayerManager;
	class FEMesh
	{
		friend MeshManager;
		friend LayerManager;
	public:
		FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, std::string Name);
		~FEMesh();

		GLuint GetVaoID() const;
		GLuint getVertexCount() const;
		GLuint getIndicesBufferID() const;
		GLuint getIndicesCount() const;
		GLuint getTriangleCount() const;
		GLuint getPositionsBufferID() const;
		GLuint getPositionsCount() const;
		GLuint getNormalsBufferID() const;
		GLuint getNormalsCount() const;
		GLuint getTangentsBufferID() const;
		GLuint getTangentsCount() const;
		GLuint getUVBufferID() const;
		GLuint getUVCount() const;
		GLuint getColorBufferID() const;
		GLuint getColorCount() const;

		void addColorToVertices(float* colors, int colorSize);
		void addSegmentsColorToVertices(float* colors, int colorSize);
	//private:
		GLuint vaoID = -1;
		GLuint indicesBufferID = -1;
		unsigned int indicesCount = -1;
		GLuint positionsBufferID = -1;
		unsigned int positionsCount = -1;
		GLuint normalsBufferID = -1;
		unsigned int normalsCount = -1;
		GLuint tangentsBufferID = -1;
		unsigned int tangentsCount = -1;
		GLuint UVBufferID = -1;
		unsigned int UVCount = -1;
		GLuint colorBufferID = -1;
		unsigned int colorCount = -1;
		GLuint segmentsColorsBufferID = -1;
		unsigned int segmentsColorsCount = -1;
		GLuint FirstLayerBufferID = -1;
		GLuint SecondLayerBufferID = -1;

		unsigned int vertexCount;

		int vertexAttributes = 1;
		FEAABB AABB;

		// NEW
		FETransformComponent* Position = new FETransformComponent();

		int HeatMapType = 5;

		std::vector<int> TriangleSelected;
		float LastMeasuredRugosityAreaRadius = -1.0f;
		glm::vec3 LastMeasuredRugosityAreaCenter = glm::vec3(0.0f);
		std::vector<std::vector<glm::vec3>> Triangles;
		std::vector<double> TrianglesArea;
		float TotalArea = 0.0f;
		std::vector<std::vector<glm::vec3>> TrianglesNormals;
		std::vector<glm::vec3> TrianglesCentroids;

		std::vector<int> originalTrianglesToSegments;
		std::vector<glm::vec3> segmentsNormals;

		//std::vector<float> TrianglesRugosity;
		std::vector<float> rugosityDataAdditional;
		std::vector<float> TrianglesRugosityAdditional;

		void fillTrianglesData();
		bool intersectWithTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& triangleVertices, float& distance, glm::vec3* HitPoint = nullptr);
		bool SelectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera);
		glm::vec3 IntersectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera);
		bool SelectTrianglesInRadius(glm::dvec3 MouseRay, FEBasicCamera* currentCamera, float Radius);
		bool SelectTrianglesInRadius(glm::vec3 CenterPoint, float Radius);

		glm::vec3 AverageNormal = glm::vec3();
		glm::vec3 GetAverageNormal();
		void UpdateAverageNormal();

		static double TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC);

		std::vector<MeshLayer> Layers;

		void AddLayer(std::vector<float> TrianglesToData);
		void AddLayer(MeshLayer NewLayer);

		std::string FileName;
	private:
		int CurrentLayerIndex = -1;
	};
}