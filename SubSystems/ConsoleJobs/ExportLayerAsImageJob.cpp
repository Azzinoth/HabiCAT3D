#include "ExportLayerAsImageJob.h"
using namespace FocalEngine;

ExportLayerAsImageJob::ExportLayerAsImageJob()
{
	Type = "EXPORT_LAYER_AS_IMAGE_JOB";
	ExportMode = LayerRasterizationManager::GridRasterizationMode::Min;
	SaveMode = LayerRasterizationManager::SaveMode::SaveAsPNG;
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