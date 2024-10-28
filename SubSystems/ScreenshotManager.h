#pragma once

#include "MeshRenderer.h"
using namespace FocalEngine;

class ScreenshotManager
{
public:	SINGLETON_PUBLIC_PART(ScreenshotManager)

	void Init();
	void TakeScreenshot();
	void RenderTargetWasResized();
private:
	SINGLETON_PRIVATE_PART(ScreenshotManager)

	FEFramebuffer* FrameBufferObject = nullptr;

	void CreateFB();
	void UpdateFB();

	int FindHigestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List);
	std::string SuitableNewFileName(std::string Base, std::string Extension);

	bool IsFolder(const char* Path);
	std::vector<std::string> GetFileList(const char* Path);
	std::string GetCurrentWorkingDirectory();
};

#define SCREENSHOT_MANAGER ScreenshotManager::GetInstance()