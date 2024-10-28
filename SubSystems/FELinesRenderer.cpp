#include "FELinesRenderer.h"
using namespace FocalEngine;

FECustomLine::FECustomLine(glm::vec3 PointA, glm::vec3 PointB, glm::vec3 Color)
{
	A = PointA;
	B = PointB;
	this->Color = Color;
}

FELinesRenderer::FELinesRenderer()
{
	LineShader = RESOURCE_MANAGER.CreateShader("Custom:ineShader", LineVS, LineFS);
	glGenVertexArrays(1, &LineVAO);
}

void FELinesRenderer::Render()
{
#ifdef NEW_LINES
	SyncWithGPU();  // Ensure GPU buffer is up-to-date
#endif

	if (PointsToRender == 0)
		return;

	LineShader->Start();

	LineShader->UpdateParameterData("FEViewMatrix", MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().GetViewMatrix());
	LineShader->UpdateParameterData("FEProjectionMatrix", MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FECameraComponent>().GetProjectionMatrix());
	LineShader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(LineVAO));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glEnableVertexAttribArray(1));

	FE_GL_ERROR(glDrawArrays(GL_LINES, 0, PointsToRender * 2));

	FE_GL_ERROR(glBindVertexArray(0));
	LineShader->Stop();
}

void FELinesRenderer::AddLineToBuffer(FECustomLine LineToAdd)
{
#ifdef NEW_LINES
	FrameLines.push_back(LineToAdd);
#else
	FELinePoint PointA;
	PointA.Position = LineToAdd.A;
	PointA.Color = LineToAdd.Color;
	LinePointVector.push_back(PointA);

	FELinePoint PointB;
	PointB.Position = LineToAdd.B;
	PointB.Color = LineToAdd.Color;
	LinePointVector.push_back(PointB);
#endif
}

#ifdef NEW_LINES
bool FELinesRenderer::LinesAreDifferent() {
	if (FrameLines.size() * 2 != RenderLinePoints.size()) return true;

	for (size_t i = 0; i < FrameLines.size(); i++) {
		if (FrameLines[i].A != RenderLinePoints[i * 2].Position ||
			FrameLines[i].B != RenderLinePoints[i * 2 + 1].Position ||
			FrameLines[i].Color != RenderLinePoints[i * 2].Color) {  // Assuming FELinePoint has the same color for both A and B
			return true;
		}
	}
	return false;
}
#endif

void FELinesRenderer::SyncWithGPU()
{
#ifdef NEW_LINES
	if(!LinesAreDifferent()) return;

	// Convert FrameLines to RenderLinePoints
	RenderLinePoints.clear();
	for (const auto& line : FrameLines) {
		RenderLinePoints.push_back({ line.A, line.Color });
		RenderLinePoints.push_back({ line.B, line.Color });
	}

	PointsToRender = RenderLinePoints.size();

	glBindVertexArray(LineVAO);

	if (LineBuffer != 0)
		glDeleteBuffers(1, &LineBuffer);
	glGenBuffers(1, &LineBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, LineBuffer);
	glBufferData(GL_ARRAY_BUFFER, RenderLinePoints.size() * sizeof(FELinePoint), RenderLinePoints.data(), GL_DYNAMIC_DRAW);

	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(FELinePoint), (void*)0));
	FE_GL_ERROR(glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(FELinePoint), (void*)(3 * sizeof(float))));

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	FE_GL_ERROR(glBindVertexArray(0));

	FrameLines.clear();
#else
	if (LinePointVector.size() == 0)
		return;

	PointsToRender = static_cast<int>(LinePointVector.size());

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
#endif
}

void FELinesRenderer::ClearAll()
{
#ifdef NEW_LINES
	PointsToRender = 0;
	FrameLines.clear();
#else
	PointsToRender = 0;
	LinePointVector.clear();
#endif
}

void FELinesRenderer::RenderAABB(FEAABB AABB, glm::vec3 Color)
{
	// bottom plane
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMin()), glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMin()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMin()), glm::vec3(AABB.GetMin()[0], AABB.GetMin()[1], AABB.GetMax()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMax()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMax()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMin()[1], AABB.GetMax()[2]), Color));

	// upper plane
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMin()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMax()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color));

	// conect two planes
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMin()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMin()[0], AABB.GetMin()[1], AABB.GetMax()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMax()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color));
	AddLineToBuffer(FECustomLine(glm::vec3(AABB.GetMin()[0], AABB.GetMin()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMin()[2]), Color));
}