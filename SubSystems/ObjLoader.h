#pragma once

#include "../FECoreIncludes.h"

using namespace FocalEngine;

struct materialRecord
{
	std::string Name = "";

	std::string AlbedoMapFile = "";
	std::string SpecularMapFile = "";
	std::string SpecularHighlightMapFile = "";
	std::string AlphaMapFile = "";
	std::string NormalMapFile = "";
	std::string BumpMapFile = "";
	std::string DisplacementMapFile = "";
	std::string StencilDecalMapFile = "";

	unsigned int MinVertexIndex = INT_MAX;
	unsigned int MaxVertexIndex = 0;

	unsigned int MinTextureIndex = INT_MAX;
	unsigned int MaxTextureIndex = 0;

	unsigned int MinNormalIndex = INT_MAX;
	unsigned int MaxNormalIndex = 0;

	unsigned int FacesSeenBefore = 0;
	unsigned int FaceCount = 0;
};

#define DOUBLE_VERTEX_ON_SEAMS

struct RawOBJData
{
	std::vector<glm::dvec3> RawVertexCoordinates;
	std::vector<glm::vec3> RawVertexColors;
	std::vector<glm::vec2> RawTextureCoordinates;
	std::vector<glm::vec3> RawNormalCoordinates;
	std::vector<int> RawIndices;

	// final vertex coordinates
	std::vector<double> FVerC;
	// final colors
	std::vector<float> fColorsC;
	// final texture coordinates
	std::vector<float> FTexC;
	// final normal coordinates
	std::vector<float> FNorC;
	// final tangent coordinates
	std::vector<float> FTanC;
	// final indices
	std::vector<int> FInd;
	// material records
	std::vector<materialRecord> MaterialRecords;
	std::vector<float> MatIDs;
};

class ObjLoader
{
	friend FEResourceManager;
public:
	SINGLETON_PUBLIC_PART(ObjLoader)

	void ForceOneMesh(bool bForce);
	bool IsForcingOneMesh();

	void ForcePositionNormalization(bool bForce);
	bool IsForcingPositionNormalization();

	void ReadFile(const char* FileName);

	// Use to get raw data from the loaded file.
	// Recommenede only if you know what you are doing.
	std::vector<RawOBJData*>* GetLoadedObjects();
private:
	SINGLETON_PRIVATE_PART(ObjLoader)
		
	std::vector<RawOBJData*> LoadedObjects;

	bool bForceOneMesh = false;
	bool bForcePositionNormalization = false;
	bool bHaveColors = false;
	bool bHaveTextureCoord = false;
	bool bHaveNormalCoord = false;

	std::string CurrentFilePath = "";
	std::string MaterialFileName = "";
	RawOBJData* CurrentMaterialObject = nullptr;
	void ReadMaterialFile(const char* OriginalOBJFile);
	void ReadMaterialLine(std::stringstream& lineStream);
	bool CheckCurrentMaterialObject();

	void ReadLine(std::stringstream& lineStream, RawOBJData* data);
	void ProcessRawData(RawOBJData* Data);

	glm::vec3 CalculateNormal(glm::dvec3 V0, glm::dvec3 V1, glm::dvec3 V2);
	void CalculateNormals(RawOBJData* Data);

	glm::vec3 CalculateTangent(const glm::vec3 V0, const glm::vec3 V1, const glm::vec3 V2, std::vector<glm::vec2>&& Textures);
	void CalculateTangents(RawOBJData* Data);

	void NormalizeVertexPositions(RawOBJData* data);

#ifdef DOUBLE_VERTEX_ON_SEAMS
	struct VertexThatNeedDoubling
	{
		VertexThatNeedDoubling(int IndexInArray, int AcctualIndex, int TexIndex, int NormIndex) : IndexInArray(IndexInArray),
			AcctualIndex(AcctualIndex), TexIndex(TexIndex), NormIndex(NormIndex), bWasDone(false) {};

		int IndexInArray;
		int AcctualIndex;
		int TexIndex;
		int NormIndex;
		bool bWasDone;

		friend bool operator==(const VertexThatNeedDoubling& lhs, const VertexThatNeedDoubling& rhs)
		{
			return lhs.AcctualIndex == rhs.AcctualIndex && lhs.IndexInArray == rhs.IndexInArray && lhs.TexIndex == rhs.TexIndex && lhs.NormIndex == rhs.NormIndex;
		}
	};
#endif // DOUBLE_VERTEX_ON_SEAMS
};
