#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class FileLoadJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string FilePath;
	bool bKeepExistingData = false;

	static ConsoleJobInfo GetInfo();
	bool Execute(void* InputData = nullptr, void* OutputData = nullptr);
public:
	FileLoadJob(std::string FilePath);

	static FileLoadJob* CreateInstance(CommandLineAction ActionToParse);
};