#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class FileSaveJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string FilePath;

public:
	FileSaveJob(std::string FilePath)
	{
		this->FilePath = FilePath;
		Type = "FILE_SAVE";
	}
};