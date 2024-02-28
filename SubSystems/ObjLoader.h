#pragma once

#include "../FECoreIncludes.h"

using namespace FocalEngine;

struct materialRecord
{
	std::string name = "";

	std::string albedoMapFile = "";
	std::string specularMapFile = "";
	std::string specularHighlightMapFile = "";
	std::string alphaMapFile = "";
	std::string normalMapFile = "";
	std::string bumpMapFile = "";
	std::string displacementMapFile = "";
	std::string stencilDecalMapFile = "";

	unsigned int minVertexIndex = INT_MAX;
	unsigned int maxVertexIndex = 0;

	unsigned int minTextureIndex = INT_MAX;
	unsigned int maxTextureIndex = 0;

	unsigned int minNormalIndex = INT_MAX;
	unsigned int maxNormalIndex = 0;

	unsigned int facesSeenBefore = 0;
	unsigned int faceCount = 0;
};

struct RawOBJData
{
	std::vector<glm::dvec3> rawVertexCoordinates;
	std::vector<glm::vec3> rawVertexColors;
	std::vector<glm::vec2> rawTextureCoordinates;
	std::vector<glm::vec3> rawNormalCoordinates;
	std::vector<int> rawIndices;

	// final vertex coordinates
	std::vector<double> fVerC;
	// final colors
	std::vector<float> fColorsC;
	// final texture coordinates
	std::vector<float> fTexC;
	// final normal coordinates
	std::vector<float> fNorC;
	// final tangent coordinates
	std::vector<float> fTanC;
	// final indices
	std::vector<int> fInd;
	// material records
	std::vector<materialRecord> materialRecords;
	std::vector<float> matIDs;
};

class ObjLoader
{
	friend FEResourceManager;
public:
	SINGLETON_PUBLIC_PART(ObjLoader)
//private:
	SINGLETON_PRIVATE_PART(ObjLoader)
		
	std::vector<RawOBJData*> loadedObjects;
	bool forceOneMesh = false;
	std::string currentFilePath = "";

	void readFile(const char* fileName);

	void readLine(std::stringstream& lineStream, RawOBJData* data);
	void processRawData(RawOBJData* data);

	glm::vec3 calculateNormal(glm::dvec3 v0, glm::dvec3 v1, glm::dvec3 v2);
	void calculateNormals(RawOBJData* data);

	glm::vec3 calculateTangent(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, std::vector<glm::vec2>&& textures);
	void calculateTangents(RawOBJData* data);

	std::string materialFileName = "";
	void readMaterialFile(const char* originalOBJFile);
	void readMaterialLine(std::stringstream& lineStream);
	RawOBJData* currentMaterialObject = nullptr;
	bool checkCurrentMaterialObject();

	bool haveColors = false;
	bool haveTextureCoord = false;
	bool haveNormalCoord = false;

	void NormilizeVertexPositions(RawOBJData* data);

#ifdef DOUBLE_VERTEX_ON_SEAMS
	struct vertexThatNeedDoubling
	{
		vertexThatNeedDoubling(int IndexInArray, int AcctualIndex, int TexIndex, int NormIndex) : indexInArray(IndexInArray),
			acctualIndex(AcctualIndex), texIndex(TexIndex), normIndex(NormIndex), wasDone(false) {};

		int indexInArray;
		int acctualIndex;
		int texIndex;
		int normIndex;
		bool wasDone;

		friend bool operator==(const vertexThatNeedDoubling& lhs, const vertexThatNeedDoubling& rhs)
		{
			return lhs.acctualIndex == rhs.acctualIndex && lhs.indexInArray == rhs.indexInArray && lhs.texIndex == rhs.texIndex && lhs.normIndex == rhs.normIndex;
		}
	};
#endif // DOUBLE_VERTEX_ON_SEAMS
};
