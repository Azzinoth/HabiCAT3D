#include "FEMesh.h"
using namespace FocalEngine;

FEMesh::FEMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, std::string Name)
{
	vaoID = VaoID;
	vertexCount = VertexCount;
	vertexAttributes = VertexBuffersTypes;
}

FEMesh::~FEMesh()
{
	FE_GL_ERROR(glDeleteVertexArrays(1, &vaoID));
}

GLuint FEMesh::GetVaoID() const
{
	return vaoID;
}

GLuint FEMesh::getVertexCount() const
{
	return vertexCount;
}

GLuint FEMesh::getIndicesBufferID() const
{
	return indicesBufferID;
}

GLuint FEMesh::getIndicesCount() const
{
	return indicesCount;
}

GLuint FEMesh::getPositionsBufferID() const
{
	return positionsBufferID;
}

GLuint FEMesh::getPositionsCount() const
{
	return positionsCount;
}

GLuint FEMesh::getNormalsBufferID() const
{
	return normalsBufferID;
}

GLuint FEMesh::getNormalsCount() const
{
	return normalsCount;
}

GLuint FEMesh::getTangentsBufferID() const
{
	return tangentsBufferID;
}

GLuint FEMesh::getTangentsCount() const
{
	return tangentsCount;
}

GLuint FEMesh::getUVBufferID() const
{
	return UVBufferID;
}

GLuint FEMesh::getUVCount() const
{
	return UVCount;
}

GLuint FEMesh::getColorBufferID() const
{
	return colorBufferID;
}

GLuint FEMesh::getColorCount() const
{
	return colorCount;
}

void FEMesh::addColorToVertices(float* colors, int colorSize)
{
	if (colors == nullptr || colorSize <= 0)
		return;

	FE_GL_ERROR(glBindVertexArray(vaoID));

	colorCount = colorSize;
	colorBufferID = 0;
	vertexAttributes |= FE_COLOR;
	FE_GL_ERROR(glGenBuffers(1, &colorBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, colorBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colorCount, colors, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(1/*FE_COLOR*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void FEMesh::addSegmentsColorToVertices(float* colors, int colorSize)
{
	if (colors == nullptr || colorSize <= 0)
		return;

	FE_GL_ERROR(glBindVertexArray(vaoID));

	segmentsColorsCount = colorSize;
	segmentsColorsBufferID = 0;
	vertexAttributes |= FE_SEGMENTS_COLORS;
	FE_GL_ERROR(glGenBuffers(1, &segmentsColorsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, segmentsColorsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * segmentsColorsCount, colors, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(7/*FE_SEGMENTS_COLORS*/, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

bool FEMesh::intersectWithTriangle(glm::vec3 RayOrigin, glm::vec3 RayDirection, std::vector<glm::vec3>& triangleVertices, float& distance, glm::vec3* HitPoint)
{
	if (triangleVertices.size() != 3)
		return false;

	const float a = RayDirection[0];
	const float b = triangleVertices[0][0] - triangleVertices[1][0];
	const float c = triangleVertices[0][0] - triangleVertices[2][0];

	const float d = RayDirection[1];
	const float e = triangleVertices[0][1] - triangleVertices[1][1];
	const float f = triangleVertices[0][1] - triangleVertices[2][1];

	const float g = RayDirection[2];
	const float h = triangleVertices[0][2] - triangleVertices[1][2];
	const float j = triangleVertices[0][2] - triangleVertices[2][2];

	const float k = triangleVertices[0][0] - RayOrigin[0];
	const float l = triangleVertices[0][1] - RayOrigin[1];
	const float m = triangleVertices[0][2] - RayOrigin[2];

	const glm::mat3 temp0 = glm::mat3(a, b, c,
	                                  d, e, f,
	                                  g, h, j);

	const float determinant0 = glm::determinant(temp0);

	const glm::mat3 temp1 = glm::mat3(k, b, c,
	                                  l, e, f,
	                                  m, h, j);

	const float determinant1 = glm::determinant(temp1);

	const float t = determinant1 / determinant0;


	const glm::mat3 temp2 = glm::mat3(a, k, c,
	                                  d, l, f,
	                                  g, m, j);

	const float determinant2 = glm::determinant(temp2);
	const float u = determinant2 / determinant0;

	const float determinant3 = glm::determinant(glm::mat3(a, b, k,
	                                                      d, e, l,
	                                                      g, h, m));

	const float v = determinant3 / determinant0;

	if (t >= 0.00001 &&
		u >= 0.00001 && v >= 0.00001 &&
		u <= 1 && v <= 1 &&
		u + v >= 0.00001 &&
		u + v <= 1 && t > 0.00001)
	{
		if (HitPoint != nullptr)
			*HitPoint = triangleVertices[0] + u * (triangleVertices[1] - triangleVertices[0]) + v * (triangleVertices[2] - triangleVertices[0]);

		distance = t;
		return true;
	}

	return false;
}

void FEMesh::fillTrianglesData()
{
	Triangles.clear();
	TrianglesNormals.clear();
	TrianglesArea.clear();
	TrianglesCentroids.clear();
	TotalArea = 0;

	std::vector<float> FEVertices;
	FEVertices.resize(getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(getPositionsBufferID(), 0, sizeof(float) * FEVertices.size(), FEVertices.data()));

	std::vector<int> FEIndices;
	FEIndices.resize(getIndicesCount());
	FE_GL_ERROR(glGetNamedBufferSubData(getIndicesBufferID(), 0, sizeof(int) * FEIndices.size(), FEIndices.data()));

	std::vector<float> FENormals;
	if (getNormalsCount() > 0)
	{
		FENormals.resize(getNormalsCount());
		FE_GL_ERROR(glGetNamedBufferSubData(getNormalsBufferID(), 0, sizeof(int) * FENormals.size(), FENormals.data()));
	}

	std::vector<glm::vec3> triangle;
	triangle.resize(3);

	for (size_t i = 0; i < FEIndices.size(); i += 3)
	{
		int vertexPosition = FEIndices[i] * 3;
		triangle[0] = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		vertexPosition = FEIndices[i + 1] * 3;
		triangle[1] = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		vertexPosition = FEIndices[i + 2] * 3;
		triangle[2] = glm::vec3(FEVertices[vertexPosition], FEVertices[vertexPosition + 1], FEVertices[vertexPosition + 2]);

		Triangles.push_back(triangle);
		TrianglesArea.push_back(TriangleArea(triangle[0], triangle[1], triangle[2]));
		TotalArea += static_cast<float>(TrianglesArea.back());

		TrianglesCentroids.push_back((triangle[0] + triangle[1] + triangle[2]) / 3.0f);

		if (!FENormals.empty())
		{
			vertexPosition = FEIndices[i] * 3;
			triangle[0] = glm::vec3(FENormals[vertexPosition], FENormals[vertexPosition + 1], FENormals[vertexPosition + 2]);

			vertexPosition = FEIndices[i + 1] * 3;
			triangle[1] = glm::vec3(FENormals[vertexPosition], FENormals[vertexPosition + 1], FENormals[vertexPosition + 2]);

			vertexPosition = FEIndices[i + 2] * 3;
			triangle[2] = glm::vec3(FENormals[vertexPosition], FENormals[vertexPosition + 1], FENormals[vertexPosition + 2]);

			TrianglesNormals.push_back(triangle);
		}
	}
}

bool FEMesh::SelectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera)
{
	float currentDistance = 0.0f;
	float lastDistance = 9999.0f;

	int TriangeIndex = -1;
	TriangleSelected.clear();

	for (int i = 0; i < Triangles.size(); i++)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		const bool hit = intersectWithTriangle(currentCamera->GetPosition(), MouseRay, TranformedTrianglePoints, currentDistance);

		if (hit && currentDistance < lastDistance)
		{
			lastDistance = currentDistance;
			TriangeIndex = i;
		}
	}

	if (TriangeIndex != -1)
	{
		TriangleSelected.push_back(TriangeIndex);
		return true;
	}

	return false;
}

glm::vec3 FEMesh::IntersectTriangle(glm::dvec3 MouseRay, FEBasicCamera* currentCamera)
{
	float currentDistance = 0.0f;
	float lastDistance = 9999.0f;
	glm::vec3 HitPosition;

	for (size_t i = 0; i < Triangles.size(); i++)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		glm::vec3 HitPosition;
		//bool hit = intersectWithTriangle(currentCamera->getPosition(), MouseRay, Triangles[i], currentDistance, &HitPosition);
		const bool hit = intersectWithTriangle(currentCamera->GetPosition(), MouseRay, TranformedTrianglePoints, currentDistance, &HitPosition);

		if (hit && currentDistance < lastDistance)
		{
			lastDistance = currentDistance;

			const glm::mat4 Inverse = glm::inverse(Position->getTransformMatrix());
			return Inverse * glm::vec4(HitPosition, 1.0f);
		}
	}
}

bool FEMesh::SelectTrianglesInRadius(glm::dvec3 MouseRay, FEBasicCamera* currentCamera, float Radius)
{
	bool Result = false;
	SelectTriangle(MouseRay, currentCamera);

	if (TriangleSelected.size() == 0)
		return Result;

	LastMeasuredRugosityAreaRadius = Radius;
	LastMeasuredRugosityAreaCenter = Position->getTransformMatrix() * glm::vec4(TrianglesCentroids[TriangleSelected[0]], 1.0f);

	const glm::vec3 FirstSelectedTriangleCentroid = TrianglesCentroids[TriangleSelected[0]];

	for (size_t i = 0; i < Triangles.size(); i++)
	{
		if (i == TriangleSelected[0])
			continue;

		if (glm::distance(FirstSelectedTriangleCentroid, TrianglesCentroids[i]) <= Radius)
		{
			TriangleSelected.push_back(static_cast<int>(i));
			Result = true;
		}
	}

	return Result;
}

bool FEMesh::SelectTrianglesInRadius(glm::vec3 CenterPoint, float Radius)
{
	bool Result = false;

	TriangleSelected.clear();

	LastMeasuredRugosityAreaRadius = Radius;
	LastMeasuredRugosityAreaCenter = Position->getTransformMatrix() * glm::vec4(CenterPoint, 1.0f);

	const glm::vec3 FirstSelectedTriangleCentroid = CenterPoint;

	for (size_t i = 0; i < Triangles.size(); i++)
	{
		if (glm::distance(FirstSelectedTriangleCentroid, TrianglesCentroids[i]) <= Radius)
		{
			TriangleSelected.push_back(static_cast<int>(i));
			Result = true;
		}
	}

	return Result;
}

void FEMesh::UpdateAverageNormal()
{
	AverageNormal = glm::vec3();

	std::vector<float> originalAreas;
	float totalArea = 0.0f;
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		const double originalArea = TriangleArea(Triangles[i][0], Triangles[i][1], Triangles[i][2]);
		originalAreas.push_back(static_cast<float>(originalArea));
		totalArea += originalArea;
	}

	// ******* Geting average normal *******
	for (size_t i = 0; i < Triangles.size(); i++)
	{
		const float currentTriangleCoef = originalAreas[i] / totalArea;

		AverageNormal += TrianglesNormals[i][0] * currentTriangleCoef;
		AverageNormal += TrianglesNormals[i][1] * currentTriangleCoef;
		AverageNormal += TrianglesNormals[i][2] * currentTriangleCoef;
	}

	AverageNormal = glm::normalize(AverageNormal);
}

glm::vec3 FEMesh::GetAverageNormal()
{
	return AverageNormal;
}

double FEMesh::TriangleArea(glm::dvec3 PointA, glm::dvec3 PointB, glm::dvec3 PointC)
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

void FEMesh::AddLayer(std::vector<float> TrianglesToData)
{
	if (TrianglesToData.size() == Triangles.size())
		Layers.push_back(MeshLayer(this, TrianglesToData));
}

void FEMesh::AddLayer(MeshLayer NewLayer)
{
	if (NewLayer.TrianglesToData.size() == Triangles.size())
	{
		NewLayer.SetParentMesh(this);
		Layers.push_back(NewLayer);
	}
}

MeshLayer::MeshLayer()
{
	ID = APPLICATION.GetUniqueHexID();
};

MeshLayer::MeshLayer(FEMesh* Parent, const std::vector<float> TrianglesToData)
{
	if (Parent == nullptr)
		return;

	if (TrianglesToData.size() != Parent->Triangles.size())
		return;

	this->ParentMesh = Parent;
	this->TrianglesToData = TrianglesToData;

	CalculateInitData();
}

MeshLayer::~MeshLayer() {};

FEMesh* MeshLayer::GetParentMesh()
{
	return ParentMesh;
}

void MeshLayer::SetParentMesh(FEMesh* NewValue)
{
	if (NewValue == nullptr)
		return;

	if (TrianglesToData.size() != NewValue->Triangles.size())
		return;

	this->ParentMesh = NewValue;
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

	if (ParentMesh == nullptr)
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
	int MaxVisiableIndex = SortedData.size() * 0.85;

	MinVisible = Min;
	MaxVisible = SortedData[MaxVisiableIndex];

	if (!TrianglesToData.empty())
	{
		Mean = TotalSum / TrianglesToData.size();
		Median = SortedData[SortedData.size() / 2];
	}

	ValueTriangleAreaAndIndex.clear();
	for (int i = 0; i < ParentMesh->Triangles.size(); i++)
	{
		ValueTriangleAreaAndIndex.push_back(std::make_tuple(TrianglesToData[i], ParentMesh->TrianglesArea[i], i));
	}

	// sort() function will sort by 1st element of tuple.
	std::sort(ValueTriangleAreaAndIndex.begin(), ValueTriangleAreaAndIndex.end());
}

void MeshLayer::FillRawData()
{
	if (ParentMesh == nullptr || TrianglesToData.empty())
		return;

	const int PosSize = ParentMesh->getPositionsCount();
	float* positions = new float[PosSize];
	FE_GL_ERROR(glGetNamedBufferSubData(ParentMesh->getPositionsBufferID(), 0, sizeof(float) * PosSize, positions));

	const int IndexSize = ParentMesh->getIndicesCount();
	int* indices = new int[IndexSize];
	FE_GL_ERROR(glGetNamedBufferSubData(ParentMesh->getIndicesBufferID(), 0, sizeof(int) * IndexSize, indices));

	std::vector<float> PositionsVector;
	for (size_t i = 0; i < PosSize; i++)
	{
		PositionsVector.push_back(positions[i]);
	}

	std::vector<int> IndexVector;
	for (size_t i = 0; i < IndexSize; i++)
	{
		IndexVector.push_back(indices[i]);
	}

	delete positions;
	delete indices;

	RawData.resize(PosSize);
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

	for (size_t i = 0; i < ParentMesh->Triangles.size(); i++)
	{
		SetRugosityOfFace(static_cast<int>(i), TrianglesToData[i]);
	}
}

void MeshLayer::FillDataToGPU(const int LayerIndex)
{
	if (ParentMesh == nullptr)
		return;

	if (RawData.empty())
		FillRawData();

	FE_GL_ERROR(glBindVertexArray(ParentMesh->vaoID));

	if (LayerIndex == 0)
	{
		ParentMesh->FirstLayerBufferID = 0;
		ParentMesh->vertexAttributes |= FE_RUGOSITY_FIRST;
		FE_GL_ERROR(glGenBuffers(1, &ParentMesh->FirstLayerBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, ParentMesh->FirstLayerBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * RawData.size(), RawData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(7, 3, GL_FLOAT, false, 0, nullptr));
	}
	else
	{
		ParentMesh->SecondLayerBufferID = 0;
		ParentMesh->vertexAttributes |= FE_RUGOSITY_SECOND;
		FE_GL_ERROR(glGenBuffers(1, &ParentMesh->SecondLayerBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, ParentMesh->SecondLayerBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * RawData.size(), RawData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(8, 3, GL_FLOAT, false, 0, nullptr));
	}

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
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

float MeshLayer::GetMean()
{
	return Mean;
}

float MeshLayer::GetMedian()
{
	return Median;
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
	int Count = Entries.size();
	File.write((char*)&Count, sizeof(int));

	std::string TempType;
	for (size_t i = 0; i < Entries.size(); i++)
	{
		TempType = Entries[i].Type;

		Count = TempType.size();
		File.write((char*)&Count, sizeof(int));
		File.write(TempType.data(), sizeof(char) * Count);

		Count = Entries[i].Name.size();
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
	Entry.Size = Data.size() + 1;
	memcpy(Entry.RawData, Data.data(), Data.size());
	Entry.RawData[Data.size()] = '\0';
	Entries.push_back(Entry);
}