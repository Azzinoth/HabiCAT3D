#pragma once

#include "FETexture.h"
#include "ComplexityMetricManager.h"

#define APP_VERSION 0.55

namespace FocalEngine
{
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
		float LastMeasuredRugosityAreaRadius = -1.0f;
		glm::vec3 LastMeasuredRugosityAreaCenter = glm::vec3(0.0f);

		bool intersectWithTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& triangleVertices, float& distance, glm::vec3* HitPoint = nullptr);
		bool SelectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera);
		bool SelectTrianglesInRadius(glm::dvec3 MouseRay, FEBasicCamera* currentCamera, float Radius);
		bool SelectTrianglesInRadius(glm::vec3 CenterPoint, float Radius);
		glm::vec3 IntersectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera);

		void ComplexityMetricDataToGPU(int LayerIndex, int GPULayerIndex = 0);
	};
}