#pragma once

#include "MeshRenderer.h"
using namespace FocalEngine;

class ScreenshotManager
{
public:
	SINGLETON_PUBLIC_PART(ScreenshotManager)

	void Init();
	void TakeScreenshot(FEBasicCamera* CurrentCamera);
	void RenderTargetWasResized();
private:
	SINGLETON_PRIVATE_PART(ScreenshotManager)

	GLuint ColorBufferTexture = -1;
	GLuint DepthBufferTexture = -1;
	GLuint FrameBuffer = -1;

	void CreateFB();
	void UpdateFB();

	void FlipImageVertically(unsigned char* Data, size_t Width, size_t Height);
	bool ExportRawDataToPNG(const char* FileName, const unsigned char* TextureData, const int Width, const int Height, const GLint Internalformat);

	int FindHigestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List);
	std::string SuitableNewFileName(std::string Base, std::string Extension);

	bool IsFolder(const char* Path);
	std::vector<std::string> GetFileList(const char* Path);
	std::string GetCurrentWorkingDirectory();
};

#define SCREENSHOT_MANAGER ScreenshotManager::getInstance()