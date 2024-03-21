#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class FileLoadJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string FilePath;

public:
	FileLoadJob(std::string FilePath)
	{
		this->FilePath = FilePath;
		Type = "FILE_LOAD";
	};
};