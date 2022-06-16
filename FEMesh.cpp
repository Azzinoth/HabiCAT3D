#include "FEMesh.h"
using namespace FocalEngine;

FEMesh::FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, std::string Name)
{
	vaoID = VaoID;
	vertexCount = VertexCount;
	vertexAttributes = VertexBuffersTypes;
}

FEMesh::~FEMesh()
{
	FE_GL_ERROR(glDeleteVertexArrays(1, &vaoID));
}

GLuint FEMesh::getVaoID() const
{
	return vaoID;
}

GLuint FEMesh::getVertexCount() const
{
	return vertexCount;
}

GLuint FEMesh::getIndicesBufferID() const
{
	return indicesBufferID;
}

GLuint FEMesh::getIndicesCount() const
{
	return indicesCount;
}

GLuint FEMesh::getPositionsBufferID() const
{
	return positionsBufferID;
}

GLuint FEMesh::getPositionsCount() const
{
	return positionsCount;
}

GLuint FEMesh::getNormalsBufferID() const
{
	return normalsBufferID;
}

GLuint FEMesh::getNormalsCount() const
{
	return normalsCount;
}

GLuint FEMesh::getTangentsBufferID() const
{
	return tangentsBufferID;
}

GLuint FEMesh::getTangentsCount() const
{
	return tangentsCount;
}

GLuint FEMesh::getUVBufferID() const
{
	return UVBufferID;
}

GLuint FEMesh::getUVCount() const
{
	return UVCount;
}

GLuint FEMesh::getColorBufferID() const
{
	return colorBufferID;
}

GLuint FEMesh::getColorCount() const
{
	return colorCount;
}

void FEMesh::addColorToVertices(float* colors, int colorSize)
{
	if (colors == nullptr || colorSize <= 0)
		return;

	FE_GL_ERROR(glBindVertexArray(vaoID));

	colorCount = colorSize;
	colorBufferID = 0;
	vertexAttributes |= FE_COLOR;
	FE_GL_ERROR(glGenBuffers(1, &colorBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, colorBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colorCount, colors, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(1/*FE_COLOR*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void FEMesh::addSegmentsColorToVertices(float* colors, int colorSize)
{
	if (colors == nullptr || colorSize <= 0)
		return;

	FE_GL_ERROR(glBindVertexArray(vaoID));

	segmentsColorsCount = colorSize;
	segmentsColorsBufferID = 0;
	vertexAttributes |= FE_SEGMENTS_COLORS;
	FE_GL_ERROR(glGenBuffers(1, &segmentsColorsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, segmentsColorsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * segmentsColorsCount, colors, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(7/*FE_SEGMENTS_COLORS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}