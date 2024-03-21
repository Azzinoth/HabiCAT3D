#include "ExportLayerAsImageJob.h"
using namespace FocalEngine;

ExportLayerAsImageJob::ExportLayerAsImageJob()
{
	Type = "EXPORT_LAYER_AS_IMAGE_JOB";
	ExportMode = LayerRasterizationManager::GridRasterizationMode::Min;
	SaveMode = LayerRasterizationManager::SaveMode::SaveAsPNG;
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