#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class ExitJob : public ConsoleJob
{
	friend ConsoleJobManager;
	static ConsoleJobInfo GetInfo();

	bool Execute(void* InputData = nullptr, void* OutputData = nullptr);
public:
	ExitJob();
	static ExitJob* CreateInstance(CommandLineAction ActionToParse);
};