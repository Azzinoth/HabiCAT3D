#pragma once

#include "ConsoleJob.h"

using namespace FocalEngine;

class ConsoleJobManager;
class HelpJob : public ConsoleJob
{
	friend ConsoleJobManager;
	std::string CommandName = "";
	static ConsoleJobInfo GetInfo();
public:
	HelpJob(std::string CommandName = "");
	std::string GetCommandName();
};