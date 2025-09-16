#include "AnalysisData.h"
#include "Layers/LayerManager.h"
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

bool AnalysisObject::AddLayer(DataLayer* NewLayer)
{
	if (NewLayer == nullptr)
		return false;

	if (NewLayer->GetMainParentObject() != nullptr && NewLayer->GetMainParentObject() != this)
		return false;

	if (NewLayer->GetMainParentObject() == nullptr)
		NewLayer->ParentObjectIDs.push_back(ID);
	
	for (size_t i = 0; i < Layers.size(); i++)
	{
		if (Layers[i] == NewLayer)
			return false;
	}

	Layers.push_back(NewLayer);
	Layers.back()->ComputeStatistics();
	return true;
}

DataLayer* AnalysisObject::GetLayer(std::string LayerID)
{
	DataLayer* Result = nullptr;
	if (LayerID.empty())
		return Result;

	for (size_t i = 0; i < Layers.size(); i++)
	{
		if (Layers[i]->GetID() == LayerID)
		{
			Result = Layers[i];
			return Result;
		}
	}

	return Result;
}

void AnalysisObject::ClearActiveLayer()
{
	SetActiveLayer("");
}

DataLayer* AnalysisObject::GetActiveLayer()
{
	DataLayer* Result = nullptr;
	if (ActiveLayerID.empty())
		return Result;

	Result = GetLayer(ActiveLayerID);
	if (Result == nullptr)
		ActiveLayerID = "";
	
	return Result;
}

bool AnalysisObject::SetActiveLayer(std::string LayerID)
{
	DataLayer* NewActiveLayer = GetLayer(LayerID);
	if (NewActiveLayer == nullptr && !LayerID.empty())
		return false;

	ActiveLayerID = LayerID;
	LayerEvent NewEvent = LayerEvent(LAYER_EVENT_TYPE::LAYER_ACTIVE_ID_CHANGED, ID, ActiveLayerID, { LayerID });
	LAYER_MANAGER.PropagateLayerEvent(NewEvent);
	
	return true;
}

size_t AnalysisObject::GetLayerCount()
{
	return Layers.size();
}

int AnalysisObject::GetActiveLayerIndex()
{
	if (ActiveLayerID.empty())
		return -1;

	for (size_t i = 0; i < Layers.size(); i++)
	{
		if (Layers[i]->GetID() == ActiveLayerID)
			return static_cast<int>(i);
	}

	return -1;
}

bool AnalysisObject::RemoveLayer(std::string LayerID)
{
	if (LayerID.empty())
		return false;

	for (size_t i = 0; i < Layers.size(); i++)
	{
		if (Layers[i]->GetID() == LayerID)
		{
			if (Layers[i]->GetID() == ActiveLayerID)
				ClearActiveLayer();

			delete Layers[i];
			Layers.erase(Layers.begin() + i);

			LayerEvent NewEvent = LayerEvent(LAYER_EVENT_TYPE::LAYER_REMOVED, ID, LayerID, { LayerID });
			LAYER_MANAGER.PropagateLayerEvent(NewEvent);
			return true;
		}
	}

	return false;
}