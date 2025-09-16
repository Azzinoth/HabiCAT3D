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

ComplexityEvaluationJob* ComplexityEvaluationJob::CreateInstance(CommandLineAction ActionToParse)
{
	ComplexityEvaluationJob* Result = nullptr;

	if (ActionToParse.Settings.find("type") == ActionToParse.Settings.end())
		return Result;

	if (ActionToParse.Settings.find("subtype") == ActionToParse.Settings.end())
		return Result;

	std::string Type = ActionToParse.Settings["type"];
	std::transform(Type.begin(), Type.end(), Type.begin(), [](unsigned char c) { return std::toupper(c); });

	auto Iterator = ActionToParse.Settings.begin();
	while (Iterator != ActionToParse.Settings.end())
	{
		std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
		Iterator++;
	}

	if (ActionToParse.Settings["type"] != "COMPLEXITY")
		return Result;

	Result = new ComplexityEvaluationJob();
	Result->SetEvaluationSubType(ActionToParse.Settings["subtype"]);

	if (ActionToParse.Settings.find("expected_value") != ActionToParse.Settings.end())
	{
		Result->SetExpectedValue(std::stof(ActionToParse.Settings["expected_value"]));
	}

	if (ActionToParse.Settings.find("tolerance") != ActionToParse.Settings.end())
	{
		Result->SetTolerance(std::stof(ActionToParse.Settings["tolerance"]));
	}

	if (ActionToParse.Settings.find("layer_index") != ActionToParse.Settings.end())
	{
		Result->SetLayerIndex(std::stoi(ActionToParse.Settings["layer_index"]));
	}

	return Result;
}

ConsoleJobInfo ComplexityEvaluationJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "evaluation";
	Info.Purpose = "Creates an evaluation job with the specified settings to test a layer or other objects.";
	ConsoleJobSettingsInfo CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "type";
	CurrentSettingInfo.Description = "Specifies the type of evaluation.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "COMPLEXITY" };
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "subtype";
	CurrentSettingInfo.Description = "Specifies the subtype of evaluation.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "MEAN_LAYER_VALUE", "MEDIAN_LAYER_VALUE", "MAX_LAYER_VALUE", "MIN_LAYER_VALUE" };
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "expected_value";
	CurrentSettingInfo.Description = "Specifies the expected value for the evaluation.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "tolerance";
	CurrentSettingInfo.Description = "Specifies the tolerance for the evaluation.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "layer_index";
	CurrentSettingInfo.Description = "Specifies the index of the layer to evaluate. Relevant only for 'COMPLEXITY' evaluation type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "'-1' Which means the last layer.";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "convert_to_script";
	CurrentSettingInfo.Description = "Specifies if the job should be converted to a script that later can be used to run the same job but with actual values.(Mostly used to make it easier to create a script file for new models)";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	return Info;
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

bool ComplexityEvaluationJob::Execute(void* InputData, void* OutputData)
{
	if (EvaluationType == "COMPLEXITY")
	{
		if (LAYER_MANAGER.GetLayerCount() == 0)
		{
			std::string ErrorMessage = "Error: No layers to evaluate. Please calculate a layer before attempting to evaluate.";
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return false;
		}

		DataLayer* LayerToEvaluate = LAYER_MANAGER.GetActiveLayer();
		if (LayerToEvaluate == nullptr)
		{
			std::string ErrorMessage = "Error: Layer to evaluate is null. Please check the layer index and try again.";
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return false;
		}

		bFailed = false;
		float Difference = FLT_MAX;

		if (GetEvaluationSubType() == "MEAN_LAYER_VALUE")
		{
			SetActualValue(LayerToEvaluate->GetMean());
		}
		else if (GetEvaluationSubType() == "MEDIAN_LAYER_VALUE")
		{
			SetActualValue(LayerToEvaluate->GetMedian());
		}
		else if (GetEvaluationSubType() == "MAX_LAYER_VALUE")
		{
			SetActualValue(LayerToEvaluate->GetMax());
		}
		else if (GetEvaluationSubType() == "MIN_LAYER_VALUE")
		{
			SetActualValue(LayerToEvaluate->GetMin());
		}

		Difference = GetExpectedValue() - GetActualValue();

		if (abs(Difference) > GetTolerance())
			bFailed = true;

		if (isnan(GetActualValue()))
			bFailed = true;

		if (Failed())
		{
			std::string ErrorMessage = "Error: Evaluation failed. Type: " + EvaluationType + " SubType: " + GetEvaluationSubType() + " Expected: " + std::to_string(GetExpectedValue()) + " Tolerance: " + std::to_string(GetTolerance()) + " Actual: " + std::to_string(GetActualValue());
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
		}
		else
		{
			std::string Message = "Evaluation passed. Type: " + EvaluationType + " SubType: " + GetEvaluationSubType() + " Expected: " + std::to_string(GetExpectedValue()) + " Tolerance: " + std::to_string(GetTolerance()) + " Actual: " + std::to_string(GetActualValue());
			OutputConsoleTextWithColor(Message, 0, 255, 0);
		}

		if (InputData != nullptr)
		{
			std::string Script = "-evaluation type=" + EvaluationType + " subtype=" + GetEvaluationSubType() + " expected_value=" + std::to_string(GetActualValue()) + " tolerance=" + std::to_string(GetTolerance());
			if (GetLayerIndex() != -1)
				Script += " layer_index=" + std::to_string(GetLayerIndex());

			reinterpret_cast<std::vector<std::string>*>(InputData)->push_back(Script);
		}
	}

	return true;
}