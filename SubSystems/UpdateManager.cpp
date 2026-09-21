#include "UpdateManager.h"
#include <winhttp.h>
#include <shellapi.h>
#pragma comment(lib, "winhttp.lib")
using namespace FocalEngine;

UpdateManager::UpdateManager() {}
UpdateManager::~UpdateManager() {}

bool UpdateManager::ParseVersion(const std::string& Text, SemanticVersion& Result)
{
	std::string Cleaned = Text;
	if (!Cleaned.empty() && (Cleaned[0] == 'v' || Cleaned[0] == 'V'))
		Cleaned.erase(0, 1);

	if (Cleaned.empty())
		return false;

	int Components[3] = { 0, 0, 0 };
	int ComponentIndex = 0;
	std::string Current;
	for (size_t i = 0; i <= Cleaned.size(); i++)
	{
		if (i == Cleaned.size() || Cleaned[i] == '.')
		{
			if (Current.empty() || ComponentIndex >= 3)
				return false;

			Components[ComponentIndex] = std::stoi(Current);
			ComponentIndex++;
			Current.clear();
			continue;
		}

		if (!std::isdigit(static_cast<unsigned char>(Cleaned[i])))
			return false;

		Current += Cleaned[i];
	}

	Result.Major = Components[0];
	Result.Minor = Components[1];
	Result.Patch = Components[2];
	return true;
}

bool UpdateManager::IsNewer(const SemanticVersion& Remote, const SemanticVersion& Local)
{
	if (Remote.Major != Local.Major)
		return Remote.Major > Local.Major;

	std::string RemoteFraction = std::to_string(Remote.Minor) + std::to_string(Remote.Patch);
	std::string LocalFraction = std::to_string(Local.Minor) + std::to_string(Local.Patch);
	size_t Length = std::max(RemoteFraction.size(), LocalFraction.size());
	RemoteFraction.append(Length - RemoteFraction.size(), '0');
	LocalFraction.append(Length - LocalFraction.size(), '0');

	return RemoteFraction > LocalFraction;
}

std::string UpdateManager::ExtractJsonString(const std::string& Json, const std::string& Key)
{
	std::string QuotedKey = "\"" + Key + "\"";
	size_t KeyPosition = Json.find(QuotedKey);
	if (KeyPosition == std::string::npos)
		return "";

	size_t ColonPosition = Json.find(':', KeyPosition + QuotedKey.size());
	if (ColonPosition == std::string::npos)
		return "";

	size_t ValueStart = Json.find('"', ColonPosition);
	if (ValueStart == std::string::npos)
		return "";

	size_t ValueEnd = Json.find('"', ValueStart + 1);
	if (ValueEnd == std::string::npos)
		return "";

	return Json.substr(ValueStart + 1, ValueEnd - ValueStart - 1);
}

std::string UpdateManager::HttpsGet(const wchar_t* Host, const wchar_t* Path)
{
	std::string Result;
	HINTERNET Session = WinHttpOpen(L"HabiCAT3D-UpdateCheck", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (Session == nullptr)
		return Result;

	WinHttpSetTimeouts(Session, 5000, 5000, 5000, 5000);
	HINTERNET Connection = WinHttpConnect(Session, Host, INTERNET_DEFAULT_HTTPS_PORT, 0);
	HINTERNET Request = nullptr;
	if (Connection != nullptr)
		Request = WinHttpOpenRequest(Connection, L"GET", Path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

	if (Request != nullptr &&
		WinHttpSendRequest(Request, L"Accept: application/vnd.github+json\r\n", static_cast<DWORD>(-1), WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
		WinHttpReceiveResponse(Request, nullptr))
	{
		DWORD StatusCode = 0;
		DWORD StatusSize = sizeof(StatusCode);
		WinHttpQueryHeaders(Request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &StatusCode, &StatusSize, WINHTTP_NO_HEADER_INDEX);

		DWORD Available = 0;
		while (StatusCode == 200 && WinHttpQueryDataAvailable(Request, &Available) && Available > 0)
		{
			std::string Chunk(Available, '\0');
			DWORD Read = 0;
			if (!WinHttpReadData(Request, Chunk.data(), Available, &Read))
				break;

			Result.append(Chunk.data(), Read);
		}
	}

	if (Request != nullptr)
		WinHttpCloseHandle(Request);

	if (Connection != nullptr)
		WinHttpCloseHandle(Connection);

	WinHttpCloseHandle(Session);
	return Result;
}

void UpdateManager::RequestJob(void* InputData, void* OutputData)
{
	JobData* Data = static_cast<JobData*>(OutputData);
	Data->Response = HttpsGet(L"api.github.com", L"/repos/Azzinoth/HabiCAT3D/releases/latest");
}

void UpdateManager::AfterRequest(void* OutputData)
{
	JobData* Data = static_cast<JobData*>(OutputData);
	std::string Tag = ExtractJsonString(Data->Response, "tag_name");
	delete Data;

	SemanticVersion Remote;
	SemanticVersion Local;
	if (!ParseVersion(Tag, Remote) || !ParseVersion(UI_CORE.GetVersion(), Local))
	{
		LOG.Add("Update check: could not determine the latest release version.");
		return;
	}

	if (!IsNewer(Remote, Local))
		return;

	UPDATE_MANAGER.LatestTag = Tag;
	UPDATE_MANAGER.bShouldOpenPopup = true;
}

void UpdateManager::CheckAsync()
{
	if (APPLICATION.HasConsoleWindow())
		return;

	JobData* Data = new JobData();
	THREAD_POOL.Execute(RequestJob, Data, Data, AfterRequest);
}

void UpdateManager::Render()
{
	if (bShouldOpenPopup && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId))
	{
		ImGui::OpenPopup("Update Available");
		bShouldOpenPopup = false;
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Update Available", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("A newer version of HabiCAT3D is available: %s", LatestTag.c_str());
		ImGui::Text("Installed version: %s", UI_CORE.GetVersion().c_str());
		ImGui::Separator();

		if (ImGui::Button("Open download page", ImVec2(170.0f, 25.0f)))
		{
			ShellExecute(NULL, "open", ReleasePageURL, NULL, NULL, SW_SHOWNORMAL);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Close", ImVec2(170.0f, 25.0f)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}