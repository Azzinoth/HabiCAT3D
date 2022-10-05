#pragma once

#include "FEShader.h"
#include "SubSystems/FEGeometricTools.h"

namespace FocalEngine
{
	class FEEntity;
	class FEEntityInstanced;
	class FERenderer;
	class FEResourceManager;

	class FEMesh
	{	
		friend FEEntity;
		friend FEEntityInstanced;
		friend FERenderer;
		friend FEResourceManager;
	public:
		FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, std::string Name);
		~FEMesh();

		GLuint getVaoID() const;
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
		GLuint rugosityBufferID = -1;
		unsigned int rugosityColorsCount = -1;
		GLuint rugositySecondBufferID = -1;
		unsigned int rugositySecondColorsCount = -1;

		unsigned int vertexCount;

		int vertexAttributes = 1;
		FEAABB AABB;

		// NEW
		FETransformComponent* Position = new FETransformComponent();

		double minRugorsity = DBL_MAX;
		double maxRugorsity = -DBL_MAX;
		bool showRugosity = false;

		int colorMode = 0;

		std::vector<int> TriangleSelected;
		float LastMeasuredRugosityAreaRadius = -1.0f;
		glm::vec3 LastMeasuredRugosityAreaCenter = glm::vec3(0.0f);
		std::vector<std::vector<glm::vec3>> Triangles;
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
		void SelectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera);
		glm::vec3 IntersectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera);
		void SelectTrianglesInRadius(glm::dvec3 MouseRay, FEBasicCamera* currentCamera, float Radius);
		void SelectTrianglesInRadius(glm::vec3 CenterPoint, float Radius);
	};
}