#pragma once

#include "ConsoleJob.h"
using namespace FocalEngine;

class EvaluationJob : public ConsoleJob
{
	friend ConsoleJobManager;

protected:
	bool bFailed = true;
	std::string EvaluationType;
public:

	bool Failed();
};

class ComplexityEvaluationJob : public EvaluationJob
{
	friend ConsoleJobManager;

	std::string EvaluationSubType;

	float ExpectedValue = 0.0f;
	float ActualValue = 0.0f;
	float Tolerance = 0.0f;

	int LayerIndex = -1;

	static ConsoleJobInfo GetInfo();
	bool Execute(void* InputData = nullptr, void* OutputData = nullptr);
public:
	ComplexityEvaluationJob();
	static ComplexityEvaluationJob* CreateInstance(CommandLineAction ActionToParse);

	float GetExpectedValue();
	void SetExpectedValue(float NewValue);

	float GetActualValue();
	void SetActualValue(float NewValue);

	float GetTolerance();
	void SetTolerance(float NewValue);

	int GetLayerIndex();
	void SetLayerIndex(int NewValue);

	std::string GetEvaluationSubType();
	void SetEvaluationSubType(std::string NewValue);
};