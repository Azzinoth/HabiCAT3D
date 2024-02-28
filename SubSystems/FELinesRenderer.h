#pragma once
#include "../FECoreIncludes.h"
using namespace FocalEngine;

static const char* const LineVS = R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 lineColor;

@ViewMatrix@
@ProjectionMatrix@

void main(void)
{
	lineColor = color;
	gl_Position = FEProjectionMatrix * FEViewMatrix * vec4(position, 1.0);
}
)";

static const char* const LineFS = R"(
layout (location = 0) out vec4 out_Color;

in vec3 lineColor;

void main(void)
{
	out_Color = vec4(lineColor, 1.0f);
}
)";

//#define NEW_LINES

namespace FocalEngine
{
	struct FELinePoint
	{
		glm::vec3 Position;
		glm::vec3 Color;
	};

	struct FECustomLine
	{
		glm::vec3 A;
		glm::vec3 B;
		glm::vec3 Color;

		FECustomLine(glm::vec3 PointA, glm::vec3 PointB, glm::vec3 Color = glm::vec3(0.0f));
	};
	
	class FELinesRenderer
	{
	public:
		SINGLETON_PUBLIC_PART(FELinesRenderer)

		void Render();
		void AddLineToBuffer(FECustomLine LineToAdd);
		void SyncWithGPU();

		void RenderAABB(FEAABB AABB, glm::vec3 color);

		void clearAll();
	private:
		SINGLETON_PRIVATE_PART(FELinesRenderer)

		GLuint LineVAO = 0;
		GLenum LineBuffer = 0;
		FEShader* LineShader = nullptr;
#ifdef NEW_LINES
		std::vector<FELine> FrameLines;
		std::vector<FELinePoint> RenderLinePoints;
		bool LinesAreDifferent();
#else
		std::vector<FELinePoint> LinePointVector;
#endif
		int PointsToRender = 0;
	};

	#define LINE_RENDERER FELinesRenderer::getInstance()
}