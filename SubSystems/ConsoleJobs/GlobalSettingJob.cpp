#include "GlobalSettingJob.h"
using namespace FocalEngine;

GlobalSettingJob::GlobalSettingJob()
{
	Type = "GLOBAL_SETTINGS_JOB";
	GlobalSettingType = "EVALUATION_JOB_TO_SCRIPT";
}

std::string GlobalSettingJob::GetGlobalSettingType()
{
	return GlobalSettingType;
}

void GlobalSettingJob::SetGlobalSettingType(std::string NewValue)
{
	GlobalSettingType = NewValue;
}

int GlobalSettingJob::GetIntValue()
{
	return IntValue;
}

void GlobalSettingJob::SetIntValue(int NewValue)
{
	IntValue = NewValue;
}

float GlobalSettingJob::GetFloatValue()
{
	return FloatValue;
}

void GlobalSettingJob::SetFloatValue(float NewValue)
{
	FloatValue = NewValue;
}

bool GlobalSettingJob::GetBoolValue()
{
	return bValue;
}

void GlobalSettingJob::SetBoolValue(bool NewValue)
{
	bValue = NewValue;
}