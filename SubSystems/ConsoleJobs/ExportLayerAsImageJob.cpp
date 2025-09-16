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

	if (ActionToParse.Settings.find("resolution_in_pixels") != ActionToParse.Settings.end())
	{
		Result->SetResolutionInPixels(std::stoi(ActionToParse.Settings["resolution_in_pixels"]));
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
		Result->SetPercentOfAreaThatWouldBeRed(std::stof(ActionToParse.Settings["persent_of_area_that_would_be_red"]));
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
	CurrentSettingInfo.Name = "resolution_in_pixels";
	CurrentSettingInfo.Description = "Specifies the resolution in pixels for the image.";
	CurrentSettingInfo.bIsOptional = true;
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

int ExportLayerAsImageJob::GetResolutionInPixels()
{
	return ResolutionInPixels;
}

void ExportLayerAsImageJob::SetResolutionInPixels(int NewValue)
{
	ResolutionInPixels = NewValue;
}

glm::vec3 ExportLayerAsImageJob::GetForceProjectionVector()
{
	return ForceProjectionVector;
}

void ExportLayerAsImageJob::SetForceProjectionVector(glm::vec3 NewValue)
{
	ForceProjectionVector = NewValue;
}

float ExportLayerAsImageJob::GetPercentOfAreaThatWouldBeRed()
{
	return PercentOfAreaThatWouldBeRed;
}

void ExportLayerAsImageJob::SetPercentOfAreaThatWouldBeRed(float NewValue)
{
	PercentOfAreaThatWouldBeRed = NewValue;
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
	if (LAYER_MANAGER.GetLayerCount() == 0)
	{
		std::string ErrorMessage = "Error: No layers to export. Please calculate a layer before attempting to export.";
		LOG.Add(ErrorMessage, "CONSOLE_LOG");
		OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
		return false;
	}

	DataLayer* LayerToExport = LAYER_MANAGER.GetActiveLayer();
	if (LayerToExport == nullptr)
	{
		std::string ErrorMessage = "Error: Layer to export is null. Please check the layer index and try again.";
		LOG.Add(ErrorMessage, "CONSOLE_LOG");
		OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
		return false;
	}

	std::cout << "Initiating Layer export as image." << std::endl;

	LAYER_RASTERIZATION_MANAGER.SetGridRasterizationMode(GetExportMode());

	glm::vec2 MinMaxResolutionInMeters = LAYER_RASTERIZATION_MANAGER.GetMinMaxResolutionInMeters();
	float ResolutionInMeters = 0.0f;
	if (GetResolutionInPixels() > 0)
	{
		ResolutionInMeters = LAYER_RASTERIZATION_MANAGER.GetResolutionInMetersBasedOnResolutionInPixels(GetResolutionInPixels());
		std::cout << "Resolution in pixels: " + std::to_string(GetResolutionInPixels()) + " was converted from resolution in meters : " << ResolutionInMeters << " meters." << std::endl;
	}
	else
	{
		ResolutionInMeters = GetResolutionInM();
	}

	if (ResolutionInMeters > MinMaxResolutionInMeters.x)
	{
		std::string ErrorMessage = "Error: Resolution is too high. Your value: " + std::to_string(ResolutionInMeters) + " meters. Max value: " + std::to_string(MinMaxResolutionInMeters.x) + " meters. Please check the resolution and try again.";
		LOG.Add(ErrorMessage, "CONSOLE_LOG");
		OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
		return false;
	}

	if (ResolutionInMeters < MinMaxResolutionInMeters.y)
	{
		std::string ErrorMessage = "Error: Resolution is too low. Your value: " + std::to_string(ResolutionInMeters) + " meters. Min value: " + std::to_string(MinMaxResolutionInMeters.y) + " meters. Please check the resolution and try again.";
		LOG.Add(ErrorMessage, "CONSOLE_LOG");
		OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
		return false;
	}
	LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(ResolutionInMeters);

	LAYER_RASTERIZATION_MANAGER.SetCumulativeModePercentOfAreaThatWouldBeRed(GetPercentOfAreaThatWouldBeRed());

	LAYER_RASTERIZATION_MANAGER.PrepareLayerForExport(LayerToExport, GetForceProjectionVector());

	while (abs(LAYER_RASTERIZATION_MANAGER.GetProgress() - 1.0f) > FLT_EPSILON)
	{
		std::cout << "\rProgress: " << std::to_string(LAYER_RASTERIZATION_MANAGER.GetProgress() * 100.0f) << " %" << std::flush;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		THREAD_POOL.Update();
	}

	std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
	std::cout << std::endl;
	std::cout << "Layer export calculation completed." << std::endl;

	LAYER_RASTERIZATION_MANAGER.SaveToFile(GetFilePath(), SaveMode);
	return true;
}