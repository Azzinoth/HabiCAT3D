#include "FELinesRenderer.h"
using namespace FocalEngine;

FELinesRenderer* FELinesRenderer::Instance = nullptr;

FELine::FELine(glm::vec3 PointA, glm::vec3 PointB, glm::vec3 Color)
{
	A = PointA;
	B = PointB;
	this->Color = Color;
}

FELinesRenderer::FELinesRenderer()
{
	LineShader = new FEShader("lineShader", LineVS, LineFS);

	glGenVertexArrays(1, &LineVAO);
	
	/*AddLineToBuffer(FELine(glm::vec3(0.0f), glm::vec3(10.0f), glm::vec3(1.0f, 1.0f, 0.0f)));
	SyncWithGPU();

	AddLineToBuffer(FELine(glm::vec3(0.0f), glm::vec3(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	SyncWithGPU();*/
}

void FELinesRenderer::Render(FEBasicCamera* camera)
{
	if (PointsToRender == 0)
		return;

	LineShader->start();

	auto iterator = LineShader->parameters.begin();
	while (iterator != LineShader->parameters.end())
	{
		if (iterator->second.nameHash == int(std::hash<std::string>{}("FEViewMatrix")))
			iterator->second.updateData(camera->getViewMatrix());

		if (iterator->second.nameHash == int(std::hash<std::string>{}("FEProjectionMatrix")))
			iterator->second.updateData(camera->getProjectionMatrix());

		iterator++;
	}

	LineShader->loadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(LineVAO));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glEnableVertexAttribArray(1));

	FE_GL_ERROR(glDrawArrays(GL_LINES, 0, PointsToRender * 2));

	FE_GL_ERROR(glBindVertexArray(0));
	LineShader->stop();
}

void FELinesRenderer::AddLineToBuffer(FELine LineToAdd)
{
	FELinePoint PointA;
	PointA.Position = LineToAdd.A;
	PointA.Color = LineToAdd.Color;
	LinePointVector.push_back(PointA);

	FELinePoint PointB;
	PointB.Position = LineToAdd.B;
	PointB.Color = LineToAdd.Color;
	LinePointVector.push_back(PointB);
}

void FELinesRenderer::SyncWithGPU()
{
	if (LinePointVector.size() == 0)
		return;

	PointsToRender = LinePointVector.size();

	glBindVertexArray(LineVAO);

	if (LineBuffer != 0)
		glDeleteBuffers(1, &LineBuffer);
	glGenBuffers(1, &LineBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, LineBuffer);
	glBufferData(GL_ARRAY_BUFFER, LinePointVector.size() * sizeof(FELinePoint), LinePointVector.data(), GL_DYNAMIC_DRAW);

	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(FELinePoint), (void*)0));
	FE_GL_ERROR(glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(FELinePoint), (void*)(3 * sizeof(float))));

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	FE_GL_ERROR(glBindVertexArray(0));
}

void FELinesRenderer::clearAll()
{
	PointsToRender = 0;
	LinePointVector.clear();
}

void FELinesRenderer::RenderAABB(FEAABB AABB, glm::vec3 color)
{
	// bottom plane
	AddLineToBuffer(FELine(glm::vec3(AABB.getMin()), glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMin()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMin()), glm::vec3(AABB.getMin()[0], AABB.getMin()[1], AABB.getMax()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMin()[2]), glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMax()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMax()[2]), glm::vec3(AABB.getMin()[0], AABB.getMin()[1], AABB.getMax()[2]), color));

	// upper plane
	AddLineToBuffer(FELine(glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMin()[2]), glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMin()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMin()[2]), glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMax()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMin()[2]), glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMax()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMax()[2]), glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMax()[2]), color));

	// conect two planes
	AddLineToBuffer(FELine(glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMin()[2]), glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMin()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMin()[0], AABB.getMin()[1], AABB.getMax()[2]), glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMax()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMax()[2]), glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMax()[2]), color));
	AddLineToBuffer(FELine(glm::vec3(AABB.getMin()[0], AABB.getMin()[1], AABB.getMin()[2]), glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMin()[2]), color));
}