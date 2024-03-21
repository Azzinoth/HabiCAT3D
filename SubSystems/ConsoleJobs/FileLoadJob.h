#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class FileLoadJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string FilePath;

	static ConsoleJobInfo GetInfo();
public:
	FileLoadJob(std::string FilePath);

	static FileLoadJob* CreateFileLoadJob(CommandLineAction ActionToParse);
};