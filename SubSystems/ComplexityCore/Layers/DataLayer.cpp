#include "DataLayer.h"
#include "../../AnalysisObjectManager.h"
using namespace FocalEngine;

DataLayer::DataLayer()
{
	ID = APPLICATION.GetUniqueHexID();
}

DataLayer::DataLayer(std::vector<std::string> ParentIDs)
{
	ID = APPLICATION.GetUniqueHexID();

	for (size_t i = 0; i < ParentIDs.size(); i++)
	{
		AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(ParentIDs[i]);
		if (CurrentObject != nullptr)
			ParentObjectIDs.push_back(ParentIDs[i]);
	}
}

DataLayer::DataLayer(std::vector<std::string> ParentIDs, const std::vector<float> ElementsToData)
{
	ID = APPLICATION.GetUniqueHexID();
	for (size_t i = 0; i < ParentIDs.size(); i++)
	{
		AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(ParentIDs[i]);
		if (CurrentObject != nullptr)
			ParentObjectIDs.push_back(ParentIDs[i]);
	}

	if (ParentObjectIDs.empty())
		return;

	// FIX ME: Right now we only support one parent object.
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(ParentObjectIDs[0]);
	if (CurrentObject == nullptr)
		return;

	switch (CurrentObject->GetType())
	{
		case DATA_SOURCE_TYPE::MESH:
		{
			MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData == nullptr)
				return;

			if (ElementsToData.size() != CurrentMeshAnalysisData->Triangles.size())
				return;

			break;
		}
			
		case DATA_SOURCE_TYPE::POINT_CLOUD:
		{
			PointCloudAnalysisData* CurrentPointCloudAnalysisData = CurrentObject->GetPointCloudAnalysisData();
			if (CurrentPointCloudAnalysisData == nullptr)
				return;

			if (ElementsToData.size() != CurrentPointCloudAnalysisData->RawPointCloudData.size())
				return;

			break;
		}
	}
	
	this->ElementsToData = ElementsToData;
	ComputeStatistics();
}

DataLayer::~DataLayer() {};

std::vector<DATA_SOURCE_TYPE> DataLayer::GetDataSourceTypeForLayerType(LAYER_TYPE Type)
{
	switch (Type)
	{
		case LAYER_TYPE::HEIGHT:
		case LAYER_TYPE::TRIANGLE_EDGE:
		case LAYER_TYPE::TRIANGLE_AREA:
		case LAYER_TYPE::COMPARE:
		case LAYER_TYPE::STANDARD_DEVIATION:
		case LAYER_TYPE::RUGOSITY:
		case LAYER_TYPE::VECTOR_DISPERSION:
		case LAYER_TYPE::TRIANGLE_DENSITY:
		case LAYER_TYPE::INTERPOLATION:
			return { DATA_SOURCE_TYPE::MESH };
		case LAYER_TYPE::POINT_DENSITY:
		case LAYER_TYPE::STRUCTURAL_ROUGHNESS:
			return { DATA_SOURCE_TYPE::POINT_CLOUD };
		case LAYER_TYPE::FRACTAL_DIMENSION:
			return { DATA_SOURCE_TYPE::MESH, DATA_SOURCE_TYPE::POINT_CLOUD };
		default:
			return { DATA_SOURCE_TYPE::UNKNOWN };
	}
}

AnalysisObject* DataLayer::GetMainParentObject()
{
	if (ParentObjectIDs.empty())
		return nullptr;

	return ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(ParentObjectIDs[0]);
}

std::vector<AnalysisObject*> DataLayer::GetAllParentObjects()
{
	std::vector<AnalysisObject*> Result;
	for (size_t i = 0; i < ParentObjectIDs.size(); i++)
	{
		AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(ParentObjectIDs[i]);
		if (CurrentObject != nullptr)
			Result.push_back(CurrentObject);
	}

	return Result;
}

void DataLayer::ComputeStatistics()
{
	Min = FLT_MAX;
	MinVisible = FLT_MAX;
	Max = -FLT_MAX;
	MaxVisible = FLT_MAX;

	Mean = -FLT_MAX;
	Median = -FLT_MAX;

	AnalysisObject* CurrentObject = GetMainParentObject();
	if (CurrentObject == nullptr)
		return;

	if (InterpolationData == nullptr && ElementsToData.empty())
		return;

	double TotalSum = 0.0;
	if (InterpolationData == nullptr)
	{
		for (size_t i = 0; i < ElementsToData.size(); i++)
		{
			Min = std::min(Min, ElementsToData[i]);
			Max = std::max(Max, ElementsToData[i]);

			TotalSum += ElementsToData[i];
		}
	}
	else
	{
		for (size_t i = 0; i < InterpolationData->RawData.size(); i++)
		{
			for (size_t j = 0; j < InterpolationData->RawData[i].size(); j++)
			{
				Min = std::min(Min, InterpolationData->RawData[i][j]);
				Max = std::max(Max, InterpolationData->RawData[i][j]);
				TotalSum += InterpolationData->RawData[i][j];
			}
		}
	}

	std::vector<float> SortedData = ElementsToData;
	if (InterpolationData != nullptr)
	{
		for (size_t i = 0; i < InterpolationData->RawData.size(); i++)
			SortedData.insert(SortedData.end(), InterpolationData->RawData[i].begin(), InterpolationData->RawData[i].end());
	}
	
	std::sort(SortedData.begin(), SortedData.end());
	int MaxVisibleIndex = static_cast<int>(SortedData.size() * 0.85);

	MinVisible = Min;
	MaxVisible = SortedData[MaxVisibleIndex];

	size_t TotalElements = SortedData.size();
	Mean = static_cast<float>(TotalSum / TotalElements);
	Median = SortedData[SortedData.size() / 2];

	if (InterpolationData != nullptr)
	{
		InterpolationData->LayerMinValues.clear();
		InterpolationData->LayerMaxValues.clear();

		for (size_t i = 0; i < InterpolationData->RawData.size(); i++)
		{
			float MinValue = FLT_MAX;
			float MaxValue = -FLT_MAX;
			for (size_t j = 0; j < InterpolationData->RawData[i].size(); j++)
			{
				if (InterpolationData->RawData[i][j] < MinValue)
					MinValue = InterpolationData->RawData[i][j];

				if (InterpolationData->RawData[i][j] > MaxValue)
					MaxValue = InterpolationData->RawData[i][j];
			}

			InterpolationData->LayerMinValues.push_back(MinValue);
			InterpolationData->LayerMaxValues.push_back(MaxValue);
		}
	}

	ValueWeightAndIndex.clear();

	switch (CurrentObject->GetType())
	{
		case DATA_SOURCE_TYPE::MESH:
		{
			MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData == nullptr)
				return;

			if (InterpolationData != nullptr)
				return;

			for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
				ValueWeightAndIndex.push_back(std::make_tuple(ElementsToData[i], CurrentMeshAnalysisData->TrianglesArea[i], i));

			// sort() function will sort by 1st element of tuple.
			std::sort(ValueWeightAndIndex.begin(), ValueWeightAndIndex.end());

			break;
		}

		case DATA_SOURCE_TYPE::POINT_CLOUD:
		{
			PointCloudAnalysisData* CurrentPointCloudAnalysisData = CurrentObject->GetPointCloudAnalysisData();
			if (CurrentPointCloudAnalysisData == nullptr)
				return;

			for (int i = 0; i < CurrentPointCloudAnalysisData->RawPointCloudData.size(); i++)
				ValueWeightAndIndex.push_back(std::make_tuple(ElementsToData[i], 1.0f, i));
			
			// sort() function will sort by 1st element of tuple.
			std::sort(ValueWeightAndIndex.begin(), ValueWeightAndIndex.end());

			break;
		}
	}
}

bool DataLayer::SetVertexValuesByTriangleIndex(AnalysisObject* Object, std::vector<float>& VertexData, int TriangleIndex, float Value)
{
	if (Object == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	const int BaseIndex = TriangleIndex * 3;
	for (size_t i = 0; i < 3; i++)
	{
		const int VertexIndex = CurrentMeshAnalysisData->Indices[BaseIndex + i] * 3;
		VertexData[VertexIndex] = Value;
		VertexData[VertexIndex + 1] = Value;
		VertexData[VertexIndex + 2] = Value;
	}

	return true;
}

bool DataLayer::TransfareDataFromTrianglesToVertices(AnalysisObject* Object, std::vector<float>& TriangleData, std::vector<float>& VertexData)
{
	if (Object == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	if (TriangleData.size() != CurrentMeshAnalysisData->Triangles.size())
		return false;

	if (VertexData.size() != CurrentMeshAnalysisData->Vertices.size())
		return false;

	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		const int BaseIndex = static_cast<int>(i * 3);
		for (size_t j = 0; j < 3; j++)
		{
			const int VertexIndex = CurrentMeshAnalysisData->Indices[BaseIndex + j] * 3;
			VertexData[VertexIndex] = TriangleData[i];
			VertexData[VertexIndex + 1] = TriangleData[i];
			VertexData[VertexIndex + 2] = TriangleData[i];
		}
	}

	return true;
}

bool DataLayer::TransfareDataFromTrianglesToVertices(AnalysisObject* Object, std::vector<glm::vec3>& TriangleData, std::vector<glm::vec3>& VertexData)
{
	if (Object == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	// Adjust these size checks to match how Triangles/Vertices are actually stored.
	// If Vertices is a flat float array: VertexData.size() != CurrentMeshAnalysisData->Vertices.size() / 3
	if (TriangleData.size() != CurrentMeshAnalysisData->Triangles.size())
		return false;

	if (VertexData.size() != CurrentMeshAnalysisData->Vertices.size())
		return false;

	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		const int BaseIndex = static_cast<int>(i * 3);
		for (size_t j = 0; j < 3; j++)
		{
			const int VertexIndex = CurrentMeshAnalysisData->Indices[BaseIndex + j];
			VertexData[VertexIndex] = TriangleData[i];
		}
	}

	return true;
}

void DataLayer::FillRawData()
{
	AnalysisObject* CurrentObject = GetMainParentObject();
	if (CurrentObject == nullptr)
		return;

	if (ElementsToData.empty())
		return;

	if (Type == LAYER_TYPE::INTERPOLATION)
		return;

	switch (CurrentObject->GetType())
	{
		case DATA_SOURCE_TYPE::MESH:
		{
			MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData == nullptr)
				return;

			RawData.resize(CurrentMeshAnalysisData->Vertices.size());
			TransfareDataFromTrianglesToVertices(CurrentObject, ElementsToData, RawData);

			// After all steps compact the RawData to have one value per vertex.
			std::vector<float> CompactedRawData;
			CompactedRawData.resize(CurrentMeshAnalysisData->Vertices.size() / 3);
			for (size_t i = 0; i < CurrentMeshAnalysisData->Vertices.size() / 3; i++)
				CompactedRawData[i] = RawData[i * 3];
			
			RawData = CompactedRawData;
			break;
		}
		case DATA_SOURCE_TYPE::POINT_CLOUD:
		{
			PointCloudAnalysisData* CurrentPointCloudAnalysisData = CurrentObject->GetPointCloudAnalysisData();
			if (CurrentPointCloudAnalysisData == nullptr)
				return;

			break;
		}
	}
}

std::string DataLayer::GetCaption()
{
	return Caption;
}

void DataLayer::SetCaption(const std::string NewValue)
{
	Caption = NewValue;
}

std::string DataLayer::GetNote()
{
	return UserNote;
}

void DataLayer::SetNote(const std::string NewValue)
{
	UserNote = NewValue;
}

float DataLayer::GetMax()
{
	return Max;
}

float DataLayer::GetMin()
{
	return Min;
}

float DataLayer::GetMean()
{
	return Mean;
}

float DataLayer::GetMedian()
{
	return Median;
}

LAYER_TYPE DataLayer::GetType()
{
	return Type;
}

void DataLayer::SetType(LAYER_TYPE NewValue)
{
	Type = NewValue;
}

float DataLayer::GetSelectedRangeMin()
{
	return SelectedRangeMin;
}

void DataLayer::SetSelectedRangeMin(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	SelectedRangeMin = NewValue;
}

float DataLayer::GetSelectedRangeMax()
{
	return SelectedRangeMax;
}

void DataLayer::SetSelectedRangeMax(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	SelectedRangeMax = NewValue;
}

std::string DataLayer::GetID()
{
	return ID;
}

void DataLayer::ForceID(std::string ID)
{
	this->ID = ID;
}

LayerInterpolationData* DataLayer::GetInterpolationData()
{
	return InterpolationData;
}

std::vector<float> LayerInterpolationData::GetLayersMinValues()
{
	return LayerMinValues;
}

std::vector<float> LayerInterpolationData::GetLayersMaxValues()
{
	return LayerMaxValues;
}

float LayerInterpolationData::GetInterpolationFactor()
{
	return InterpolationFactor;
}

void LayerInterpolationData::SetInterpolationFactor(float NewValue)
{
	InterpolationFactor = NewValue;
}

size_t LayerInterpolationData::GetLayerCount()
{
	return UsedLayerIDs.size();
}

bool LayerInterpolationData::IsMinMaxInterpolationEnabled()
{
	return bInterpolateMinMaxValues;
}

void LayerInterpolationData::SetMinMaxInterpolationEnabled(bool NewValue)
{
	bInterpolateMinMaxValues = NewValue;
}

std::vector<std::string> LayerInterpolationData::GetUsedLayerIDs()
{
	return UsedLayerIDs;
}

std::string DebugEntry::ToString()
{
	std::string Result;

	if (Type == "bool")
	{
		Result += Name + ": " + std::to_string(*reinterpret_cast<bool*>(RawData));
	}
	else if (Type == "int")
	{
		Result += Name + ": " + std::to_string(*reinterpret_cast<int*>(RawData));
	}
	else if (Type == "float")
	{
		Result += Name + ": " + std::to_string(*reinterpret_cast<float*>(RawData));
	}
	else if (Type == "double")
	{
		Result += Name + ": " + std::to_string(*reinterpret_cast<double*>(RawData));
	}
	else if (Type == "uint64_t")
	{
		Result += Name + ": " + TIME.NanosecondTimeStampToDate(*reinterpret_cast<uint64_t*>(RawData));
	}
	else if (Type == "std::string")
	{
		Result += Name + ": " + std::string(RawData);
	}

	return Result;
}

DebugEntry::DebugEntry() {};

DebugEntry::DebugEntry(const std::string Type, const int Size, char* RawData)
{
	this->Type = Type;
	this->Size = Size;
	this->RawData = new char[Size];
	memcpy(this->RawData, RawData, Size);
	if (Type == "std::string")
		this->RawData[this->Size - 1] = '\0';
}

std::string DataLayerDebugInfo::ToString()
{
	std::string Result;

	for (size_t i = 0; i < Entries.size(); i++)
	{
		Result += Entries[i].ToString();
		Result += "\n";
	}

	return Result;
}

void DataLayerDebugInfo::FromFile(std::fstream& File)
{
	char* Buffer8 = new char[1];
	char* Buffer32 = new char[4];
	char* Buffer64 = new char[8];

	File.read(Buffer32, 4);
	const int EntriesCount = *(int*)Buffer32;

	std::string Type;
	std::string Name;
	int EntrySize;
	for (size_t i = 0; i < EntriesCount; i++)
	{
		Type = FILE_SYSTEM.ReadFEString(File);
		Name = FILE_SYSTEM.ReadFEString(File);

		File.read(Buffer32, 4);
		EntrySize = *(int*)Buffer32;

		char* TemporaryBuffer = new char[EntrySize];
		File.read(TemporaryBuffer, EntrySize);

		DebugEntry NewEntry(Type, EntrySize, TemporaryBuffer);
		NewEntry.Name = Name;
		delete[] TemporaryBuffer;

		Entries.push_back(NewEntry);
	}

	delete[] Buffer8;
	delete[] Buffer32;
	delete[] Buffer64;
}

void DataLayerDebugInfo::ToFile(std::fstream& File)
{
	int Count = static_cast<int>(Entries.size());
	File.write((char*)&Count, sizeof(int));

	std::string TemporaryType;
	for (size_t i = 0; i < Entries.size(); i++)
	{
		TemporaryType = Entries[i].Type;

		Count = static_cast<int>(TemporaryType.size());
		File.write((char*)&Count, sizeof(int));
		File.write(TemporaryType.data(), sizeof(char) * Count);

		Count = static_cast<int>(Entries[i].Name.size());
		File.write((char*)&Count, sizeof(int));
		File.write(Entries[i].Name.data(), sizeof(char) * Count);

		Count = Entries[i].Size;
		File.write((char*)&Count, sizeof(int));
		File.write(Entries[i].RawData, sizeof(char) * Count);
	}
}

void DataLayerDebugInfo::AddEntry(const std::string Name, const bool Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "bool";
	Entry.RawData = new char[1];
	Entry.Size = 1;
	memcpy(Entry.RawData, &Data, 1);
	Entries.push_back(Entry);
}

void DataLayerDebugInfo::AddEntry(const std::string Name, const int Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "int";
	Entry.RawData = new char[4];
	Entry.Size = 4;
	memcpy(Entry.RawData, &Data, 4);
	Entries.push_back(Entry);
}

void DataLayerDebugInfo::AddEntry(const std::string Name, const float Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "float";
	Entry.RawData = new char[4];
	Entry.Size = 4;
	memcpy(Entry.RawData, &Data, 4);
	Entries.push_back(Entry);
}

void DataLayerDebugInfo::AddEntry(const std::string Name, const double Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "double";
	Entry.RawData = new char[8];
	Entry.Size = 8;
	memcpy(Entry.RawData, &Data, 8);
	Entries.push_back(Entry);
}

void DataLayerDebugInfo::AddEntry(const std::string Name, const uint64_t Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "uint64_t";
	Entry.RawData = new char[8];
	Entry.Size = 8;
	memcpy(Entry.RawData, &Data, 8);
	Entries.push_back(Entry);
}

void DataLayerDebugInfo::AddEntry(const std::string Name, const std::string Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "std::string";
	Entry.RawData = new char[Data.size() + 1];
	Entry.Size = static_cast<int>(Data.size() + 1);
	memcpy(Entry.RawData, Data.data(), Data.size());
	Entry.RawData[Data.size()] = '\0';
	Entries.push_back(Entry);
}