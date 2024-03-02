#include "ComplexityMetricInfo.h"
using namespace FocalEngine;

ComplexityMetricInfo::ComplexityMetricInfo() {}

float ComplexityMetricInfo::GetTotalArea()
{
	return TotalArea;
}

void ComplexityMetricInfo::FillTrianglesData(std::vector<float>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals)
{
	MeshData.Vertices = Vertices;
	MeshData.Colors = Colors;
	MeshData.UVs = UVs;
	MeshData.Tangents = Tangents;
	MeshData.Indices = Indices;
	MeshData.Normals = Normals;

	Triangles.clear();
	TrianglesNormals.clear();
	TrianglesArea.clear();
	TrianglesCentroids.clear();
	TotalArea = 0;

	std::vector<glm::vec3> Triangle;
	Triangle.resize(3);

	for (size_t i = 0; i < Indices.size(); i += 3)
	{
		int VertexPosition = Indices[i] * 3;
		Triangle[0] = glm::vec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 1] * 3;
		Triangle[1] = glm::vec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 2] * 3;
		Triangle[2] = glm::vec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		Triangles.push_back(Triangle);
		TrianglesArea.push_back(TriangleArea(Triangle[0], Triangle[1], Triangle[2]));
		TotalArea += static_cast<float>(TrianglesArea.back());

		TrianglesCentroids.push_back((Triangle[0] + Triangle[1] + Triangle[2]) / 3.0f);

		if (!Normals.empty())
		{
			VertexPosition = Indices[i] * 3;
			Triangle[0] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			VertexPosition = Indices[i + 1] * 3;
			Triangle[1] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			VertexPosition = Indices[i + 2] * 3;
			Triangle[2] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			TrianglesNormals.push_back(Triangle);
		}
	}

	MeshData.AABB = FEAABB(Vertices.data(), static_cast<int>(Vertices.size()));
}

void ComplexityMetricInfo::UpdateAverageNormal()
{
	AverageNormal = glm::vec3();

	std::vector<float> OriginalAreas;
	float TotalArea = 0.0f;
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		const double originalArea = TriangleArea(Triangles[i][0], Triangles[i][1], Triangles[i][2]);
		OriginalAreas.push_back(static_cast<float>(originalArea));
		TotalArea += static_cast<float>(originalArea);
	}

	// ******* Geting average normal *******
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		const float currentTriangleCoef = OriginalAreas[i] / TotalArea;

		AverageNormal += TrianglesNormals[i][0] * currentTriangleCoef;
		AverageNormal += TrianglesNormals[i][1] * currentTriangleCoef;
		AverageNormal += TrianglesNormals[i][2] * currentTriangleCoef;
	}

	AverageNormal = glm::normalize(AverageNormal);
}

glm::vec3 ComplexityMetricInfo::GetAverageNormal()
{
	return AverageNormal;
}

double ComplexityMetricInfo::TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC)
{
	const double x1 = PointA.x;
	const double x2 = PointB.x;
	const double x3 = PointC.x;

	const double y1 = PointA.y;
	const double y2 = PointB.y;
	const double y3 = PointC.y;

	const double z1 = PointA.z;
	const double z2 = PointB.z;
	const double z3 = PointC.z;

	return 0.5 * sqrt(pow(x2 * y1 - x3 * y1 - x1 * y2 + x3 * y2 + x1 * y3 - x2 * y3, 2.0) +
					  pow((x2 * z1) - (x3 * z1) - (x1 * z2) + (x3 * z2) + (x1 * z3) - (x2 * z3), 2.0) +
					  pow((y2 * z1) - (y3 * z1) - (y1 * z2) + (y3 * z2) + (y1 * z3) - (y2 * z3), 2.0));
}

void ComplexityMetricInfo::AddLayer(std::vector<float> TrianglesToData)
{
	if (TrianglesToData.size() == Triangles.size())
		Layers.push_back(MeshLayer(this, TrianglesToData));
}

void ComplexityMetricInfo::AddLayer(MeshLayer NewLayer)
{
	if (NewLayer.TrianglesToData.size() == Triangles.size())
	{
		NewLayer.SetParent(this);
		Layers.push_back(NewLayer);
	}
}

MeshLayer::MeshLayer()
{
	ID = APPLICATION.GetUniqueHexID();
};

MeshLayer::MeshLayer(ComplexityMetricInfo* Parent, const std::vector<float> TrianglesToData)
{
	if (Parent == nullptr)
		return;

	if (TrianglesToData.size() != Parent->Triangles.size())
		return;

	ID = APPLICATION.GetUniqueHexID();

	this->ParentComplexityMetricData = Parent;
	this->TrianglesToData = TrianglesToData;

	CalculateInitData();
}

MeshLayer::~MeshLayer() {};

ComplexityMetricInfo* MeshLayer::GetParent()
{
	return ParentComplexityMetricData;
}

void MeshLayer::SetParent(ComplexityMetricInfo* NewValue)
{
	if (NewValue == nullptr)
		return;

	if (TrianglesToData.size() != NewValue->Triangles.size())
		return;

	this->ParentComplexityMetricData = NewValue;
	this->TrianglesToData = TrianglesToData;

	CalculateInitData();
}

void MeshLayer::CalculateInitData()
{
	Min = FLT_MAX;
	MinVisible = FLT_MAX;
	Max = -FLT_MAX;
	MaxVisible = FLT_MAX;

	Mean = -FLT_MAX;
	Median = -FLT_MAX;

	if (ParentComplexityMetricData == nullptr)
		return;

	double TotalSum = 0.0;
	for (size_t i = 0; i < TrianglesToData.size(); i++)
	{
		Min = std::min(Min, TrianglesToData[i]);
		Max = std::max(Max, TrianglesToData[i]);

		TotalSum += TrianglesToData[i];
	}

	std::vector<float> SortedData = TrianglesToData;
	std::sort(SortedData.begin(), SortedData.end());
	int MaxVisiableIndex = static_cast<int>(SortedData.size() * 0.85);

	MinVisible = Min;
	MaxVisible = SortedData[MaxVisiableIndex];

	if (!TrianglesToData.empty())
	{
		Mean = static_cast<float>(TotalSum / TrianglesToData.size());
		Median = SortedData[SortedData.size() / 2];
	}

	ValueTriangleAreaAndIndex.clear();
	for (int i = 0; i < ParentComplexityMetricData->Triangles.size(); i++)
	{
		ValueTriangleAreaAndIndex.push_back(std::make_tuple(TrianglesToData[i], ParentComplexityMetricData->TrianglesArea[i], i));
	}

	// sort() function will sort by 1st element of tuple.
	std::sort(ValueTriangleAreaAndIndex.begin(), ValueTriangleAreaAndIndex.end());
}

void MeshLayer::FillRawData()
{
	if (ParentComplexityMetricData == nullptr || TrianglesToData.empty())
		return;

	std::vector<float> PositionsVector;
	for (size_t i = 0; i < ParentComplexityMetricData->MeshData.Vertices.size(); i++)
	{
		PositionsVector.push_back(ParentComplexityMetricData->MeshData.Vertices[i]);
	}

	std::vector<int> IndexVector;
	for (size_t i = 0; i < ParentComplexityMetricData->MeshData.Indices.size(); i++)
	{
		IndexVector.push_back(ParentComplexityMetricData->MeshData.Indices[i]);
	}

	RawData.resize(ParentComplexityMetricData->MeshData.Vertices.size());
	auto GetVertexOfFace = [&](const int FaceIndex) {
		std::vector<int> result;
		result.push_back(IndexVector[FaceIndex * 3]);
		result.push_back(IndexVector[FaceIndex * 3 + 1]);
		result.push_back(IndexVector[FaceIndex * 3 + 2]);

		return result;
	};

	auto SetRugosityOfVertex = [&](const int Index, const float Value) {
		RawData[Index * 3] = Value;
		RawData[Index * 3 + 1] = Value;
		RawData[Index * 3 + 2] = Value;
	};

	auto SetRugosityOfFace = [&](const int FaceIndex, const float Value) {
		const std::vector<int> FaceVertex = GetVertexOfFace(FaceIndex);

		for (size_t i = 0; i < FaceVertex.size(); i++)
		{
			SetRugosityOfVertex(FaceVertex[i], Value);
		}
	};

	for (size_t i = 0; i < ParentComplexityMetricData->Triangles.size(); i++)
	{
		SetRugosityOfFace(static_cast<int>(i), TrianglesToData[i]);
	}
}

std::string MeshLayer::GetCaption()
{
	return Caption;
}

void MeshLayer::SetCaption(const std::string NewValue)
{
	Caption = NewValue;
}

std::string MeshLayer::GetNote()
{
	return UserNote;
}

void MeshLayer::SetNote(const std::string NewValue)
{
	UserNote = NewValue;
}

float MeshLayer::GetMax()
{
	return Max;
}

float MeshLayer::GetMin()
{
	return Min;
}

float MeshLayer::GetMean()
{
	return Mean;
}

float MeshLayer::GetMedian()
{
	return Median;
}

LAYER_TYPE MeshLayer::GetType()
{
	return Type;
}

void MeshLayer::SetType(LAYER_TYPE NewValue)
{
	Type = NewValue;
}

float MeshLayer::GetSelectedRangeMin()
{
	return SelectedRangeMin;
}

void MeshLayer::SetSelectedRangeMin(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	SelectedRangeMin = NewValue;
}

float MeshLayer::GetSelectedRangeMax()
{
	return SelectedRangeMax;
}

void MeshLayer::SetSelectedRangeMax(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	SelectedRangeMax = NewValue;
}

std::string MeshLayer::GetID()
{
	return ID;
}

void FocalEngine::MeshLayer::ForceID(std::string ID)
{
	this->ID = ID;
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

std::string MeshLayerDebugInfo::ToString()
{
	std::string Result;

	for (size_t i = 0; i < Entries.size(); i++)
	{
		Result += Entries[i].ToString();
		Result += "\n";
	}

	return Result;
}

void MeshLayerDebugInfo::FromFile(std::fstream& File)
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

		char* TempBuffer = new char[EntrySize];
		File.read(TempBuffer, EntrySize);

		DebugEntry NewEntry(Type, EntrySize, TempBuffer);
		NewEntry.Name = Name;
		delete[] TempBuffer;

		Entries.push_back(NewEntry);
	}

	delete[] Buffer8;
	delete[] Buffer32;
	delete[] Buffer64;
}

void MeshLayerDebugInfo::ToFile(std::fstream& File)
{
	int Count = static_cast<int>(Entries.size());
	File.write((char*)&Count, sizeof(int));

	std::string TempType;
	for (size_t i = 0; i < Entries.size(); i++)
	{
		TempType = Entries[i].Type;

		Count = static_cast<int>(TempType.size());
		File.write((char*)&Count, sizeof(int));
		File.write(TempType.data(), sizeof(char) * Count);

		Count = static_cast<int>(Entries[i].Name.size());
		File.write((char*)&Count, sizeof(int));
		File.write(Entries[i].Name.data(), sizeof(char) * Count);

		Count = Entries[i].Size;
		File.write((char*)&Count, sizeof(int));
		File.write(Entries[i].RawData, sizeof(char) * Count);
	}
}

void MeshLayerDebugInfo::AddEntry(const std::string Name, const bool Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "bool";
	Entry.RawData = new char[1];
	Entry.Size = 1;
	memcpy(Entry.RawData, &Data, 1);
	Entries.push_back(Entry);
}

void MeshLayerDebugInfo::AddEntry(const std::string Name, const int Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "int";
	Entry.RawData = new char[4];
	Entry.Size = 4;
	memcpy(Entry.RawData, &Data, 4);
	Entries.push_back(Entry);
}

void MeshLayerDebugInfo::AddEntry(const std::string Name, const float Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "float";
	Entry.RawData = new char[4];
	Entry.Size = 4;
	memcpy(Entry.RawData, &Data, 4);
	Entries.push_back(Entry);
}

void MeshLayerDebugInfo::AddEntry(const std::string Name, const double Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "double";
	Entry.RawData = new char[8];
	Entry.Size = 8;
	memcpy(Entry.RawData, &Data, 8);
	Entries.push_back(Entry);
}

void MeshLayerDebugInfo::AddEntry(const std::string Name, const uint64_t Data)
{
	DebugEntry Entry;
	Entry.Name = Name;
	Entry.Type = "uint64_t";
	Entry.RawData = new char[8];
	Entry.Size = 8;
	memcpy(Entry.RawData, &Data, 8);
	Entries.push_back(Entry);
}

void MeshLayerDebugInfo::AddEntry(const std::string Name, const std::string Data)
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