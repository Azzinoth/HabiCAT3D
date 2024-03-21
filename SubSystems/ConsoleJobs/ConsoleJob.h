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
	static ConsoleJobInfo GetInfo();
	//virtual void Execute(void* InputData) = 0;
public:
	std::string GetID();
};