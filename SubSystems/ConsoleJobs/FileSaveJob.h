#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class FileSaveJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string FilePath;

	static ConsoleJobInfo GetInfo();
public:
	FileSaveJob(std::string FilePath);
};