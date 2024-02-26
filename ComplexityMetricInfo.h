#pragma once

#include "FECoreIncludes.h"
//#include "SubSystems/FEFileSystem.h"
//#include "SubSystems/FEGeometricTools.h"
//#include "SubSystems/FETransformComponent.h"

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

	enum LAYER_TYPE
	{
		UNKNOWN = 0,
		HEIGHT = 1,
		TRIANGLE_EDGE = 2,
		TRIANGLE_AREA = 3,
		COMPARE = 4,
		STANDARD_DEVIATION = 5,
		RUGOSITY = 6,
		VECTOR_DISPERSION = 7,
		FRACTAL_DIMENSION = 8,
		TRIANGLE_DENSITY = 9
	};

	class ComplexityMetricInfo;
	class MeshLayer
	{
		ComplexityMetricInfo* ParentComplexityMetricData = nullptr;

		std::string ID;
		std::string Caption = "Layer caption";
		std::string UserNote;
		LAYER_TYPE Type = LAYER_TYPE::UNKNOWN;

		float Max = -FLT_MAX;
		float Min = FLT_MAX;
		float Mean = -FLT_MAX;
		float Median = -FLT_MAX;

		void CalculateInitData();

		float SelectedRangeMin = 0.0f;
		float SelectedRangeMax = 0.0f;
	public:
		MeshLayer();
		MeshLayer(ComplexityMetricInfo* Parent, std::vector<float> TrianglesToData);
		~MeshLayer();

		void FillRawData();

		std::string GetID();
		void ForceID(std::string ID);

		MeshLayerDebugInfo* DebugInfo = nullptr;

		std::string GetCaption();
		void SetCaption(std::string NewValue);

		std::string GetNote();
		void SetNote(std::string NewValue);

		ComplexityMetricInfo* GetParent();
		void SetParent(ComplexityMetricInfo* NewValue);

		float GetMax();
		float GetMin();
		float GetMean();
		float GetMedian();

		float MinVisible = FLT_MAX;
		float MaxVisible = FLT_MAX;

		std::vector<float> RawData;
		std::vector<float> TrianglesToData;

		//void FillDataToGPU(int LayerIndex = 0);
		std::vector<std::tuple<double, double, int>> ValueTriangleAreaAndIndex = std::vector<std::tuple<double, double, int>>();

		LAYER_TYPE GetType();
		void SetType(LAYER_TYPE NewValue);

		float GetSelectedRangeMin();
		void SetSelectedRangeMin(float NewValue);

		float GetSelectedRangeMax();
		void SetSelectedRangeMax(float NewValue);
	};

	// For purposes of complexity metric storing of all of raw data is redundant,but it is needed for saving RUG file.
	struct RawMeshData
	{
		std::vector<float> Vertices;
		std::vector<float> Colors;
		std::vector<float> UVs;
		std::vector<float> Tangents;
		std::vector<int> Indices;
		std::vector<float> Normals;

		FEAABB AABB;
	};

	class MeshManager;
	class LayerManager;

	class ComplexityMetricInfo
	{
		friend MeshManager;
		friend LayerManager;

		int CurrentLayerIndex = -1;
	public:
		ComplexityMetricInfo();

		std::vector<int> TriangleSelected;

		RawMeshData MeshData;

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

		void fillTrianglesData(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals);

		glm::vec3 AverageNormal = glm::vec3();
		glm::vec3 GetAverageNormal();
		void UpdateAverageNormal();

		static double TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC);

		std::vector<MeshLayer> Layers;

		void AddLayer(std::vector<float> TrianglesToData);
		void AddLayer(MeshLayer NewLayer);

		std::string FileName;
		FETransformComponent* Position = new FETransformComponent();
	};
}