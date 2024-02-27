#include "ScreenshotManager.h"
using namespace FocalEngine;

ScreenshotManager* ScreenshotManager::Instance = nullptr;

ScreenshotManager::ScreenshotManager() {}
ScreenshotManager::~ScreenshotManager() {}

void ScreenshotManager::CreateFB()
{
	/*glGenFramebuffers(1, &FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenTextures(1, &ColorBufferTexture);
	glBindTexture(GL_TEXTURE_2D, ColorBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, APPLICATION.GetMainWindow()->GetWidth(), APPLICATION.GetMainWindow()->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBufferTexture, 0);

	glGenRenderbuffers(1, &DepthBufferTexture);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthBufferTexture);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, APPLICATION.GetMainWindow()->GetWidth(), APPLICATION.GetMainWindow()->GetHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBufferTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

	FrameBufferObject = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, APPLICATION.GetMainWindow()->GetWidth(), APPLICATION.GetMainWindow()->GetHeight());
}

void ScreenshotManager::UpdateFB()
{
	/*glDeleteFramebuffers(1, &FrameBuffer);
	glDeleteTextures(1, &ColorBufferTexture);
	glDeleteRenderbuffers(1, &DepthBufferTexture);*/
	delete FrameBufferObject;

	CreateFB();
}

void ScreenshotManager::Init()
{
	CreateFB();
}

int ScreenshotManager::FindHigestIntPostfix(std::string Prefix, std::string Delimiter, std::vector<std::string> List)
{
	int Result = 0;
	std::transform(Prefix.begin(), Prefix.end(), Prefix.begin(), [](const unsigned char C) { return std::tolower(C); });
	std::transform(Delimiter.begin(), Delimiter.end(), Delimiter.begin(), [](const unsigned char C) { return std::tolower(C); });

	for (size_t i = 0; i < List.size(); i++)
	{
		std::transform(List[i].begin(), List[i].end(), List[i].begin(), [](const unsigned char C) { return std::tolower(C); });

		int PrefixPos = static_cast<int>(List[i].find(Prefix));
		if (PrefixPos != std::string::npos)
		{
			int DelimiterPos = static_cast<int>(List[i].find(Delimiter));
			if (DelimiterPos != std::string::npos && List[i].size() > Prefix.size() + Delimiter.size())
			{
				std::string PostfixPart = List[i].substr(DelimiterPos + 1, List[i].size() - (DelimiterPos + 1));
				Result = std::max(Result, atoi(PostfixPart.c_str()));
			}
		}
	}

	return Result;
}

bool ScreenshotManager::IsFolder(const char* Path)
{
	const DWORD DwAttrib = GetFileAttributesA(Path);
	return (DwAttrib != INVALID_FILE_ATTRIBUTES &&
		(DwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::vector<std::string> ScreenshotManager::GetFileList(const char* Path)
{
	std::vector<std::string> result;
	std::string pattern(Path);
	pattern.append("\\*");
	WIN32_FIND_DATAA data;
	HANDLE HFind;
	if ((HFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!IsFolder((Path + std::string("/") + std::string(data.cFileName)).c_str()) && std::string(data.cFileName) != std::string(".") && std::string(data.cFileName) != std::string(".."))
				result.push_back(data.cFileName);
		} while (FindNextFileA(HFind, &data) != 0);
		FindClose(HFind);
	}

	return result;
}

std::string ScreenshotManager::GetCurrentWorkingDirectory()
{
	DWORD dwSize = GetCurrentDirectory(0, NULL);
	if (dwSize == 0) {
		// Handle error
		return "";
	}

	char* buffer = new char[dwSize];
	if (GetCurrentDirectory(dwSize, buffer) == 0) {
		delete[] buffer;
		// Handle error
		return "";
	}

	std::string currentDir(buffer);
	delete[] buffer;

	return currentDir;
}

std::string ScreenshotManager::SuitableNewFileName(std::string Base, std::string Extension)
{
	std::string Result = Base;

	std::vector<std::string> FileNameList = GetFileList(GetCurrentWorkingDirectory().c_str());
	int IndexToAdd = FindHigestIntPostfix(Base, "_", FileNameList);

	if (IndexToAdd == 0)
	{
		for (size_t i = 0; i < FileNameList.size(); i++)
		{
			if (FileNameList[i] == Base + ".png")
			{
				IndexToAdd = 1;
				break;
			}
		}
	}

	if (IndexToAdd != 0)
		IndexToAdd++;

	if (IndexToAdd > 1)
		Result += "_" + std::to_string(IndexToAdd);

	return Result + Extension;
}

void ScreenshotManager::TakeScreenshot()
{
	if (MESH_MANAGER.ActiveEntity == nullptr)
	{
		APPLICATION.EndFrame();
		return;
	}

	/*static int FEWorldMatrix_hash = int(std::hash<std::string>{}("FEWorldMatrix"));
	static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));*/

	FrameBufferObject->Bind();
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	ENGINE.GetCamera()->Move(10);

	if (MESH_MANAGER.ActiveEntity != nullptr)
	{
		RENDERER.RenderEntityForward(MESH_MANAGER.ActiveEntity, ENGINE.GetCamera());
		
		// FIX ME ?
		/*MESH_MANAGER.MeshShader->start();

		auto iterator = MESH_MANAGER.MeshShader->parameters.begin();
		while (iterator != MESH_MANAGER.MeshShader->parameters.end())
		{
			if (iterator->second.nameHash == FEWorldMatrix_hash)
				iterator->second.updateData(MESH_MANAGER.ActiveMesh->Position->getTransformMatrix());

			if (iterator->second.nameHash == FEViewMatrix_hash)
				iterator->second.updateData(CurrentCamera->GetViewMatrix());

			if (iterator->second.nameHash == FEProjectionMatrix_hash)
				iterator->second.updateData(CurrentCamera->GetProjectionMatrix());

			iterator++;
		}

		MESH_MANAGER.MeshShader->loadDataToGPU();

		if (UI.GetWireFrameMode())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		MESH_RENDERER.RenderFEMesh(MESH_MANAGER.ActiveMesh);

		MESH_MANAGER.MeshShader->stop();*/
	}

	UI.Render(true);

	auto TempRenderFunction = APPLICATION.GetMainWindow()->GetRenderFunction();
	APPLICATION.GetMainWindow()->ClearRenderFunction();
	APPLICATION.GetMainWindow()->Render();
	APPLICATION.GetMainWindow()->EndFrame();
	APPLICATION.GetMainWindow()->SetRenderFunction(TempRenderFunction);

	FrameBufferObject->UnBind();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	RESOURCE_MANAGER.ExportFETextureToPNG(FrameBufferObject->GetColorAttachment(), SuitableNewFileName(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->FileName, ".png").c_str());
}

void ScreenshotManager::RenderTargetWasResized()
{
	UpdateFB();
}