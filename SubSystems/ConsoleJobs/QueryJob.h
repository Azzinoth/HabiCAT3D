#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class QueryJob : public ConsoleJob
{
	friend ConsoleJobManager;
	static ConsoleJobInfo GetInfo();
	std::string Request;

	bool Execute(void* InputData = nullptr, void* OutputData = nullptr);
public:
	QueryJob(std::string Request);
	static QueryJob* CreateInstance(CommandLineAction ActionToParse);
};