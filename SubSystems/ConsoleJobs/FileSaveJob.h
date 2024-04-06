#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class FileSaveJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string FilePath;

	static ConsoleJobInfo GetInfo();
	bool Execute(void* InputData = nullptr, void* OutputData = nullptr);
public:
	FileSaveJob(std::string FilePath);

	static FileSaveJob* CreateInstance(CommandLineAction ActionToParse);
};