#pragma once

#include "FEShader.h"
#include "SubSystems/FEGeometricTools.h"

#define APP_VERSION 0.21

namespace FocalEngine
{
	class FEMesh;
	class MeshLayer
	{
		FEMesh* ParentMesh = nullptr;
		std::string Caption = "Layer caption";
		std::string UserNote;

		void FillRawData();
	public:
		MeshLayer();
		MeshLayer(FEMesh* Parent, std::vector<float> TrianglesToData);
		~MeshLayer();

		std::string GetCaption();
		void SetCaption(std::string NewValue);

		std::string GetNote();
		void SetNote(std::string NewValue);

		FEMesh* GetParentMesh();
		void SetParentMesh(FEMesh* NewValue);

		float Min = FLT_MAX;
		float MinVisible = FLT_MAX;
		float Max = -FLT_MAX;
		float MaxVisible = FLT_MAX;

		std::vector<float> RawData;
		std::vector<float> TrianglesToData;

		void FillDataToGPU(int LayerIndex = 0);
	};

	class FEMesh
	{
	public:
		FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, std::string Name);
		~FEMesh();

		GLuint GetVaoID() const;
		GLuint getVertexCount() const;
		GLuint getIndicesBufferID() const;
		GLuint getIndicesCount() const;
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

		double minRugorsity = DBL_MAX;
		double maxRugorsity = -DBL_MAX;
		double minVisibleRugorsity = 1.0;
		double maxVisibleRugorsity = -DBL_MAX;
		double MinHeight = DBL_MAX;
		double MaxHeight = -DBL_MAX;
		bool bShowRugosity = false;

		int ColorMode = 5;

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

		std::vector<float> rugosityData;
		std::vector<float> TrianglesRugosity;
		std::vector<float> rugosityDataAdditional;
		std::vector<float> TrianglesRugosityAdditional;
		void fillRugosityDataToGPU(int RugosityLayerIndex = 0);

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
		int CurrentLayerIndex = -1;

		void AddLayer(std::vector<float> TrianglesToData);
	};
}