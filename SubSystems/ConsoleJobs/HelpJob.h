#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class HelpJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string CommandName = "";

	static ConsoleJobInfo GetInfo();
	bool Execute(void* InputData = nullptr, void* OutputData = nullptr);
	void PrintCommandHelp(std::string CommandName);

	static std::map<std::string, ConsoleJobInfo>* ConsoleJobsInfo;
public:
	HelpJob(std::string CommandName = "");
	static HelpJob* CreateInstance(CommandLineAction ActionToParse);

	std::string GetCommandName();
};