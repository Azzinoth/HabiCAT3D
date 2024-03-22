#include "ExportLayerAsImageJob.h"
using namespace FocalEngine;

ExportLayerAsImageJob::ExportLayerAsImageJob()
{
	Type = "EXPORT_LAYER_AS_IMAGE_JOB";
	ExportMode = LayerRasterizationManager::GridRasterizationMode::Min;
	SaveMode = LayerRasterizationManager::SaveMode::SaveAsPNG;
}

ExportLayerAsImageJob* ExportLayerAsImageJob::CreateInstance(CommandLineAction ActionToParse)
{
	ExportLayerAsImageJob* Result = new ExportLayerAsImageJob();

	auto Iterator = ActionToParse.Settings.begin();
	while (Iterator != ActionToParse.Settings.end())
	{
		if (Iterator->first == "filepath")
		{
			Iterator++;
			continue;
		}

		std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
		Iterator++;
	}

	if (ActionToParse.Settings.find("export_mode") != ActionToParse.Settings.end())
	{
		std::string ExportMode = ActionToParse.Settings["export_mode"];
		if (ExportMode == "MIN") Result->SetExportMode(LayerRasterizationManager::GridRasterizationMode::Min);
		if (ExportMode == "MAX") Result->SetExportMode(LayerRasterizationManager::GridRasterizationMode::Max);
		if (ExportMode == "MEAN") Result->SetExportMode(LayerRasterizationManager::GridRasterizationMode::Mean);
		if (ExportMode == "CUMULATIVE") Result->SetExportMode(LayerRasterizationManager::GridRasterizationMode::Cumulative);
	}

	if (ActionToParse.Settings.find("save_mode") != ActionToParse.Settings.end())
	{
		std::string SaveMode = ActionToParse.Settings["save_mode"];
		if (SaveMode == "PNG") Result->SetSaveMode(LayerRasterizationManager::SaveMode::SaveAsPNG);
		if (SaveMode == "GEOTIF") Result->SetSaveMode(LayerRasterizationManager::SaveMode::SaveAsTIF);
		if (SaveMode == "GEOTIF_32_BITS") Result->SetSaveMode(LayerRasterizationManager::SaveMode::SaveAs32bitTIF);
	}

	if (ActionToParse.Settings.find("filepath") != ActionToParse.Settings.end())
	{
		Result->SetFilePath(ActionToParse.Settings["filepath"]);
	}

	if (ActionToParse.Settings.find("resolution") != ActionToParse.Settings.end())
	{
		Result->SetResolutionInM(std::stof(ActionToParse.Settings["resolution"]));
	}

	if (ActionToParse.Settings.find("layer_index") != ActionToParse.Settings.end())
	{
		Result->SetLayerIndex(std::stoi(ActionToParse.Settings["layer_index"]));
	}

	if (ActionToParse.Settings.find("force_projection_vector") != ActionToParse.Settings.end())
	{
		std::string ForceProjectionVector = ActionToParse.Settings["force_projection_vector"];
		if (ForceProjectionVector == "X") Result->SetForceProjectionVector(glm::vec3(1.0f, 0.0f, 0.0f));
		if (ForceProjectionVector == "Y") Result->SetForceProjectionVector(glm::vec3(0.0f, 1.0f, 0.0f));
		if (ForceProjectionVector == "Z") Result->SetForceProjectionVector(glm::vec3(0.0f, 0.0f, 1.0f));
	}

	if (ActionToParse.Settings.find("persent_of_area_that_would_be_red") != ActionToParse.Settings.end())
	{
		Result->SetPersentOfAreaThatWouldBeRed(std::stof(ActionToParse.Settings["persent_of_area_that_would_be_red"]));
	}

	return Result;
}

ConsoleJobInfo ExportLayerAsImageJob::GetInfo()
{
	ConsoleJobInfo Info;
	Info.CommandName = "export_layer_as_image";
	Info.Purpose = "Exports a layer as an image.";

	ConsoleJobSettingsInfo CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "export_mode";
	CurrentSettingInfo.Description = "Specifies the mode of the export.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "MIN", "MAX", "MEAN", "CUMULATIVE" };
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "save_mode";
	CurrentSettingInfo.Description = "Specifies the type of image file.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "PNG", "GEOTIF", "GEOTIF_32_BITS" };
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "Specifies the path of the file to save.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "resolution";
	CurrentSettingInfo.Description = "Specifies the resolution in meters for the image.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "layer_index";
	CurrentSettingInfo.Description = "Specifies the index of the layer to export.";
	CurrentSettingInfo.bIsOptional = false;
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "force_projection_vector";
	CurrentSettingInfo.Description = "Specifies the projection vector for the image.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.PossibleValues = { "X", "Y", "Z" };
	CurrentSettingInfo.DefaultValue = "Calculated on fly";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "persent_of_area_that_would_be_red";
	CurrentSettingInfo.Description = "Specifies the persent of area that would be considered outliers and would be red.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "5.0";
	Info.SettingsInfo.push_back(CurrentSettingInfo);

	return Info;
}

LayerRasterizationManager::GridRasterizationMode ExportLayerAsImageJob::GetExportMode()
{
	return ExportMode;
}

void ExportLayerAsImageJob::SetExportMode(LayerRasterizationManager::GridRasterizationMode NewValue)
{
	ExportMode = NewValue;
}

LayerRasterizationManager::SaveMode ExportLayerAsImageJob::GetSaveMode()
{
	return SaveMode;
}

void ExportLayerAsImageJob::SetSaveMode(LayerRasterizationManager::SaveMode NewValue)
{
	SaveMode = NewValue;
}

std::string ExportLayerAsImageJob::GetFilePath()
{
	return FilePath;
}

void ExportLayerAsImageJob::SetFilePath(std::string NewValue)
{
	FilePath = NewValue;
}

float ExportLayerAsImageJob::GetResolutionInM()
{
	return ResolutionInM;
}

void ExportLayerAsImageJob::SetResolutionInM(float NewValue)
{
	ResolutionInM = NewValue;
}

glm::vec3 ExportLayerAsImageJob::GetForceProjectionVector()
{
	return ForceProjectionVector;
}

void ExportLayerAsImageJob::SetForceProjectionVector(glm::vec3 NewValue)
{
	ForceProjectionVector = NewValue;
}

float ExportLayerAsImageJob::GetPersentOfAreaThatWouldBeRed()
{
	return PersentOfAreaThatWouldBeRed;
}

void ExportLayerAsImageJob::SetPersentOfAreaThatWouldBeRed(float NewValue)
{
	PersentOfAreaThatWouldBeRed = NewValue;
}

int ExportLayerAsImageJob::GetLayerIndex()
{
	return LayerIndex;
}

void ExportLayerAsImageJob::SetLayerIndex(int NewValue)
{
	LayerIndex = NewValue;
}

bool ExportLayerAsImageJob::Execute(void* InputData, void* OutputData)
{
	// TODO: Implement this
	return true;
}