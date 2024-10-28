#include "ScreenshotManager.h"
using namespace FocalEngine;

ScreenshotManager::ScreenshotManager() {}
ScreenshotManager::~ScreenshotManager() {}

void ScreenshotManager::CreateFB()
{
	FrameBufferObject = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, APPLICATION.GetMainWindow()->GetWidth(), APPLICATION.GetMainWindow()->GetHeight());
}

void ScreenshotManager::UpdateFB()
{
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
	std::vector<std::string> Result;
	std::string Pattern(Path);
	Pattern.append("\\*");
	WIN32_FIND_DATAA Data;
	HANDLE HFind;
	if ((HFind = FindFirstFileA(Pattern.c_str(), &Data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!IsFolder((Path + std::string("/") + std::string(Data.cFileName)).c_str()) && std::string(Data.cFileName) != std::string(".") && std::string(Data.cFileName) != std::string(".."))
				Result.push_back(Data.cFileName);
		} while (FindNextFileA(HFind, &Data) != 0);
		FindClose(HFind);
	}

	return Result;
}

std::string ScreenshotManager::GetCurrentWorkingDirectory()
{
	DWORD Size = GetCurrentDirectory(0, NULL);
	if (Size == 0) {
		// Handle error
		return "";
	}

	char* Buffer = new char[Size];
	if (GetCurrentDirectory(Size, Buffer) == 0)
	{
		delete[] Buffer;
		// Handle error
		return "";
	}

	std::string CurrentDirectory(Buffer);
	delete[] Buffer;

	return CurrentDirectory;
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

	FrameBufferObject->Bind();
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// FXI ME!
	//ENGINE.GetCamera()->Move(10);

	if (MESH_MANAGER.ActiveEntity != nullptr)
	{
		RENDERER.RenderGameModelComponentForward(MESH_MANAGER.ActiveEntity, MAIN_SCENE_MANAGER.GetMainCamera());
	}

	UI.Render(true);

	auto TempRenderFunction = APPLICATION.GetMainWindow()->GetRenderFunction();
	APPLICATION.GetMainWindow()->ClearRenderFunction();
	APPLICATION.GetMainWindow()->Render();
	APPLICATION.GetMainWindow()->EndFrame();
	APPLICATION.GetMainWindow()->SetRenderFunction(TempRenderFunction);

	FrameBufferObject->UnBind();

	RESOURCE_MANAGER.ExportFETextureToPNG(FrameBufferObject->GetColorAttachment(), SuitableNewFileName(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->FileName, ".png").c_str());
}

void ScreenshotManager::RenderTargetWasResized()
{
	UpdateFB();
}