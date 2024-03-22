#include "ExitJob.h"
using namespace FocalEngine;

ExitJob::ExitJob()
{
	Type = "EXIT_JOB";
}

ExitJob* ExitJob::CreateInstance(CommandLineAction ActionToParse)
{
	return new ExitJob();
}

ConsoleJobInfo ExitJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "exit";
	Info.Purpose = "Prints help for all commands or for a specific command.";

	return Info;
}

bool ExitJob::Execute(void* InputData, void* OutputData)
{
	// TODO: Implement this
	return true;
}