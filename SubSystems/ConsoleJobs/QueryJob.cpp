#include "QueryJob.h"
using namespace FocalEngine;

QueryJob::QueryJob(std::string Request)
{
	Type = "QUERY_JOB";
	this->Request = Request;
}

QueryJob* QueryJob::CreateInstance(CommandLineAction ActionToParse)
{
	QueryJob* Result = nullptr;

	if (ActionToParse.Settings.find("request") == ActionToParse.Settings.end())
		return Result;

	if (ActionToParse.Settings["request"].empty())
		return Result;

	Result = new QueryJob(ActionToParse.Settings["request"]);

	return Result;
}

ConsoleJobInfo QueryJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "query";
	Info.Purpose = "With this command, users can query information to be outputted to the console or log.";
	ConsoleJobSettingsInfo CurrentSettingInfo;
	CurrentSettingInfo.Name = "request";
	CurrentSettingInfo.Description = "Specifies what to query.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues.push_back("EVALUATION_SUMMARY");
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	return Info;
}

bool QueryJob::Execute(void* InputData, void* OutputData)
{
	return true;
}