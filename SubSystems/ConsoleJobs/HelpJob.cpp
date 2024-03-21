#include "HelpJob.h"
using namespace FocalEngine;

HelpJob::HelpJob(std::string CommandName)
{
	this->CommandName = CommandName;
	Type = "HELP_JOB";
}

std::string HelpJob::GetCommandName()
{
	return CommandName;
}