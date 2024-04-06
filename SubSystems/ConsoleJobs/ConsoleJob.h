#pragma once

#include "../UI/UIManager.h"
#include "../ScreenshotManager.h"

using namespace FocalEngine;

struct ConsoleJobSettingsInfo
{
	std::string Name;
	std::string Description;
	bool bIsOptional;
	std::string DefaultValue;
	std::vector<std::string> PossibleValues;
};

struct ConsoleJobInfo
{
	std::string CommandName;
	std::string Purpose;
	std::vector<ConsoleJobSettingsInfo> SettingsInfo;
};

class ConsoleJobManager;
class ConsoleJob
{
	friend ConsoleJobManager;
	std::string ID;
protected:
	std::string Type;

	ConsoleJob();

	// Returns true if the job was successfully executed.
	// Returns false if the job failed to execute.
	virtual bool Execute(void* InputData = nullptr, void* OutputData = nullptr) = 0;

	static void OutputConsoleTextWithColor(std::string Text, int R, int G, int B);
public:
	std::string GetID();
};