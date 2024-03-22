#pragma once

#include "ConsoleJob.h"
using namespace FocalEngine;

class GlobalSettingJob : public ConsoleJob
{
	friend ConsoleJobManager;

	std::string GlobalSettingType;

	int IntValue = 0;
	float FloatValue = 0.0f;
	bool bValue = false;

	static ConsoleJobInfo GetInfo();
	bool Execute(void* InputData = nullptr, void* OutputData = nullptr);
public:
	GlobalSettingJob();
	static GlobalSettingJob* CreateInstance(CommandLineAction ActionToParse);

	std::string GetGlobalSettingType();
	void SetGlobalSettingType(std::string NewValue);

	int GetIntValue();
	void SetIntValue(int NewValue);

	float GetFloatValue();
	void SetFloatValue(float NewValue);

	bool GetBoolValue();
	void SetBoolValue(bool NewValue);
};