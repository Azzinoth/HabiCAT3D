#include "EvaluationJob.h"
using namespace FocalEngine;

bool EvaluationJob::Failed()
{
	return bFailed;
}

ComplexityEvaluationJob::ComplexityEvaluationJob()
{
	Type = "EVALUATION_JOB";
	EvaluationType = "COMPLEXITY";
}

float ComplexityEvaluationJob::GetExpectedValue()
{
	return ExpectedValue;
}

void ComplexityEvaluationJob::SetExpectedValue(float NewValue)
{
	ExpectedValue = NewValue;
}

float ComplexityEvaluationJob::GetActualValue()
{
	return ActualValue;
}

void ComplexityEvaluationJob::SetActualValue(float NewValue)
{
	ActualValue = NewValue;
}

float ComplexityEvaluationJob::GetTolerance()
{
	return Tolerance;
}

void ComplexityEvaluationJob::SetTolerance(float NewValue)
{
	Tolerance = NewValue;
}

int ComplexityEvaluationJob::GetLayerIndex()
{
	return LayerIndex;
}

void ComplexityEvaluationJob::SetLayerIndex(int NewValue)
{
	LayerIndex = NewValue;
}

void ComplexityEvaluationJob::SetEvaluationSubType(std::string NewValue)
{
	EvaluationSubType = NewValue;
}

std::string ComplexityEvaluationJob::GetEvaluationSubType()
{
	return EvaluationSubType;
}