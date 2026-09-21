#pragma once

#include "UI/UICore.h"
using namespace FocalEngine;

struct SemanticVersion
{
	int Major = 0;
	int Minor = 0;
	int Patch = 0;
};

class UpdateManager
{
public:
	SINGLETON_PUBLIC_PART(UpdateManager)

	void CheckAsync();
	void Render();

	static bool ParseVersion(const std::string& Text, SemanticVersion& Result);
	static bool IsNewer(const SemanticVersion& Remote, const SemanticVersion& Local);
private:
	SINGLETON_PRIVATE_PART(UpdateManager)

	struct JobData
	{
		std::string Response;
	};

	const char* ReleasePageURL = "https://github.com/Azzinoth/HabiCAT3D/releases/latest";

	bool bShouldOpenPopup = false;
	std::string LatestTag;

	static void RequestJob(void* InputData, void* OutputData);
	static void AfterRequest(void* OutputData);

	static std::string HttpsGet(const wchar_t* Host, const wchar_t* Path);
	static std::string ExtractJsonString(const std::string& Json, const std::string& Key);
};

#define UPDATE_MANAGER UpdateManager::GetInstance()