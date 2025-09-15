#include "AnalysisData.h"
using namespace FocalEngine;

double MeshAnalysisData::GetTotalArea()
{
	return TotalArea;
}

glm::vec3 MeshAnalysisData::GetAverageNormal()
{
	return AverageNormal;
}

void MeshAnalysisData::UpdateAverageNormal()
{
	AverageNormal = glm::vec3();

	std::vector<double> OriginalAreas;
	double TotalArea = 0.0;
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		const double OriginalArea = GEOMETRY.CalculateTriangleArea(Triangles[i][0], Triangles[i][1], Triangles[i][2]);
		OriginalAreas.push_back(OriginalArea);
		TotalArea += OriginalArea;
	}

	for (size_t i = 0; i < Triangles.size(); i++)
	{
		double CurrentTriangleCoef = OriginalAreas[i] / TotalArea;

		AverageNormal += TrianglesNormals[i][0] * static_cast<float>(CurrentTriangleCoef);
		AverageNormal += TrianglesNormals[i][1] * static_cast<float>(CurrentTriangleCoef);
		AverageNormal += TrianglesNormals[i][2] * static_cast<float>(CurrentTriangleCoef);
	}

	AverageNormal = glm::normalize(AverageNormal);
}

GLuint MeshAnalysisData::GetFirstLayerBufferID()
{
	return FirstLayerBufferID;
}

GLuint MeshAnalysisData::GetSecondLayerBufferID()
{
	return SecondLayerBufferID;
}

int MeshAnalysisData::GetHeatMapType()
{
	return HeatMapType;
}

void MeshAnalysisData::SetHeatMapType(int NewValue)
{
	HeatMapType = NewValue;
}

void MeshAnalysisData::GetMeasuredRugosityArea(float& Radius, glm::vec3& Center)
{
	Radius = MeasuredRugosityAreaRadius;
	Center = MeasuredRugosityAreaCenter;
}

void MeshAnalysisData::ClearMeasuredRugosityArea()
{
	MeasuredRugosityAreaRadius = -1.0f;
	MeasuredRugosityAreaCenter = glm::vec3(0.0f);
}

float MeshAnalysisData::GetUnselectedAreaSaturationFactor()
{
	return UnselectedAreaSaturationFactor;
}

void MeshAnalysisData::SetUnselectedAreaSaturationFactor(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	UnselectedAreaSaturationFactor = NewValue;
}

float MeshAnalysisData::GetUnselectedAreaBrightnessFactor()
{
	return UnselectedAreaBrightnessFactor;
}

void MeshAnalysisData::SetUnselectedAreaBrightnessFactor(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	UnselectedAreaBrightnessFactor = NewValue;
}

AnalysisObject::AnalysisObject()
{
	ID = APPLICATION.GetUniqueHexID();
}

std::string AnalysisObject::GetID()
{ 
	return ID;
}

std::string AnalysisObject::GetName()
{ 
	return Name;
}

std::string AnalysisObject::GetFilePath() 
{ 
	return FilePath;
}

ResourceAnalysisData* AnalysisObject::GetAnalysisData()
{ 
	return AnalysisData;
}

DATA_SOURCE_TYPE AnalysisObject::GetType()
{ 
	return Type;
}

FEObject* AnalysisObject::GetEngineResource()
{ 
	return EngineResource;
}

FEEntity* AnalysisObject::GetEntity()
{ 
	return Entity;
}
