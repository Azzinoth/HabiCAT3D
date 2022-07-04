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

		unsigned int vertexCount;

		int vertexAttributes = 1;
		FEAABB AABB;

		// NEW
		double minRugorsity = DBL_MAX;
		double maxRugorsity = -DBL_MAX;

		int colorMode = 0;

		int TriangleSelected = -1;
		std::vector<std::vector<glm::vec3>> Triangles;
		std::vector<std::vector<glm::vec3>> TrianglesNormals;

		std::vector<int> originalTrianglesToSegments;
		std::vector<glm::vec3> segmentsNormals;

		void fillTrianglesData();
		bool intersectWithTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& triangleVertices, float& distance);
		void SelectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera);
	};
}