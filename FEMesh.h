#pragma once

#include "FEShader.h"

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

		unsigned int vertexCount;

		int vertexAttributes = 1;

		// NEW
		double minRugorsity = DBL_MAX;
		double maxRugorsity = -DBL_MAX;
	};
}