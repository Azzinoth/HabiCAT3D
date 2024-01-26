#include "MeshManager.h"
using namespace FocalEngine;

MeshManager* MeshManager::Instance = nullptr;

MeshManager::MeshManager()
{
	MeshShader = new FEShader("mainShader", sTestVS, sTestFS);
	MeshShader->getParameter("lightDirection")->updateData(glm::vec3(0.0, 1.0, 0.2));
}

MeshManager::~MeshManager() {}

FEMesh* MeshManager::RawDataToMesh(float* positions, int posSize,
	float* colors, int colorSize,
	float* UV, int UVSize,
	float* normals, int normSize,
	float* tangents, int tanSize,
	int* indices, int indexSize,
	float* matIndexs, int matIndexsSize, int matCount,
	std::string Name)
{
	int vertexType = FE_POSITION | FE_INDEX;

	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint indicesBufferID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &indicesBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBufferID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexSize, indices, GL_STATIC_DRAW));

	GLuint positionsBufferID;
	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &positionsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, positionsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * posSize, positions, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint colorsBufferID = 0;
	if (colors != nullptr)
	{
		vertexType |= FE_COLOR;
		// colors
		FE_GL_ERROR(glGenBuffers(1, &colorsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, colorsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colorSize, colors, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(1/*FE_COLOR*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint normalsBufferID = 0;
	if (normals != nullptr)
	{
		vertexType |= FE_NORMAL;
		// normals
		FE_GL_ERROR(glGenBuffers(1, &normalsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, normalsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normSize, normals, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint tangentsBufferID = 0;
	if (tangents != nullptr)
	{
		//vertexType |= FE_TANGENTS;
		//// tangents
		//FE_GL_ERROR(glGenBuffers(1, &tangentsBufferID));
		//FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, tangentsBufferID));
		//FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tanSize, tangents, GL_STATIC_DRAW));
		//FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
		//FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint UVBufferID = 0;
	if (UV != nullptr)
	{
		// UV
		//FE_GL_ERROR(glGenBuffers(1, &UVBufferID));
		//FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, UVBufferID));
		//FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVSize, UV, GL_STATIC_DRAW));
		//FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
		//FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	FEMesh* newMesh = new FEMesh(vaoID, indexSize, vertexType, Name);
	newMesh->indicesCount = indexSize;
	newMesh->indicesBufferID = indicesBufferID;

	newMesh->positionsCount = posSize;
	newMesh->positionsBufferID = positionsBufferID;

	newMesh->colorCount = colorSize;
	newMesh->colorBufferID = colorsBufferID;

	newMesh->normalsCount = normSize;
	newMesh->normalsBufferID = normalsBufferID;

	newMesh->tangentsCount = tanSize;
	newMesh->tangentsBufferID = tangentsBufferID;

	newMesh->UVCount = UVSize;
	newMesh->UVBufferID = UVBufferID;

	newMesh->AABB = FEAABB(positions, posSize);

	return newMesh;
}

FEMesh* MeshManager::RawDataToMesh(double* positions, int posSize,
	float* colors, int colorSize,
	float* UV, int UVSize,
	float* normals, int normSize,
	float* tangents, int tanSize,
	int* indices, int indexSize,
	float* matIndexs, int matIndexsSize, int matCount,
	std::string Name)
{
	float* FloatPositions = new float[posSize];

	for (size_t i = 0; i < posSize; i++)
	{
		FloatPositions[i] = float(positions[i]);
	}

	FEMesh* result = RawDataToMesh(FloatPositions, posSize,
		colors, colorSize,
		UV, UVSize,
		normals, normSize,
		tangents, tanSize,
		indices, indexSize,
		matIndexs, matIndexsSize, matCount,
		Name);

	delete[] FloatPositions;

	return result;
}

FEMesh* MeshManager::ImportOBJ(const char* FileName, bool bForceOneMesh)
{
	FEMesh* result = nullptr;
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.forceOneMesh = bForceOneMesh;
	objLoader.readFile(FileName);

	if (objLoader.loadedObjects.size() > 0)
	{
		result = RawDataToMesh(objLoader.loadedObjects[0]->fVerC.data(), int(objLoader.loadedObjects[0]->fVerC.size()),
			objLoader.loadedObjects[0]->fColorsC.data(), int(objLoader.loadedObjects[0]->fColorsC.size()),
			objLoader.loadedObjects[0]->fTexC.data(), int(objLoader.loadedObjects[0]->fTexC.size()),
			objLoader.loadedObjects[0]->fNorC.data(), int(objLoader.loadedObjects[0]->fNorC.size()),
			objLoader.loadedObjects[0]->fTanC.data(), int(objLoader.loadedObjects[0]->fTanC.size()),
			objLoader.loadedObjects[0]->fInd.data(), int(objLoader.loadedObjects[0]->fInd.size()),
			objLoader.loadedObjects[0]->matIDs.data(), int(objLoader.loadedObjects[0]->matIDs.size()), int(objLoader.loadedObjects[0]->materialRecords.size()), "");
	}

	COMPLEXITY_METRIC_MANAGER.Init(objLoader.loadedObjects[0]->fVerC, objLoader.loadedObjects[0]->fInd, objLoader.loadedObjects[0]->fNorC);

	return result;
}

FEMesh* MeshManager::LoadRUGMesh(std::string FileName)
{
	std::fstream File;

	File.open(FileName, std::ios::in | std::ios::binary);
	const std::streamsize FileSize = File.tellg();
	if (FileSize < 0)
	{
		LOG.Add(std::string("Can't load file: ") + FileName + " in function LoadRUGMesh.");
		return nullptr;
	}

	char* Buffer = new char[4];
	long long ArraySize = 0;

	// version of FEMesh file type
	File.read(Buffer, 4);
	const float Version = *(float*)Buffer;
	if (Version > APP_VERSION && abs(Version - APP_VERSION) > 0.0001)
	{
		LOG.Add(std::string("Can't load file: ") + FileName + " in function LoadRUGMesh. File was created in different version of application!");
		return nullptr;
	}

	File.read(Buffer, 4);
	const int VertexCount = *(int*)Buffer;
	ArraySize = long long(VertexCount) * long long(4);
	char* VertexBuffer = new char[ArraySize];
	File.read(VertexBuffer, ArraySize);

	File.read(Buffer, 4);
	const int ColorCount = *(int*)Buffer;
	char* ColorBuffer = nullptr;
	if (ColorCount != 0)
	{
		ArraySize = long long(ColorCount) * long long(4);
		ColorBuffer = new char[ArraySize];
		File.read(ColorBuffer, ArraySize);
	}

	File.read(Buffer, 4);
	const int TexCout = *(int*)Buffer;
	ArraySize = long long(TexCout) * long long(4);
	char* TexBuffer = new char[ArraySize];
	File.read(TexBuffer, ArraySize);

	File.read(Buffer, 4);
	const int NormCout = *(int*)Buffer;
	ArraySize = long long(NormCout) * long long(4);
	char* NormBuffer = new char[ArraySize];
	File.read(NormBuffer, ArraySize);

	File.read(Buffer, 4);
	const int TangCout = *(int*)Buffer;
	ArraySize = long long(TangCout) * long long(4);
	char* TangBuffer = new char[ArraySize];
	File.read(TangBuffer, ArraySize);

	File.read(Buffer, 4);
	const int IndexCout = *(int*)Buffer;
	ArraySize = long long(IndexCout) * long long(4);
	char* IndexBuffer = new char[ArraySize];
	File.read(IndexBuffer, ArraySize);

	File.read(Buffer, 4);
	const int LayerCount = *(int*)Buffer;
	std::vector<MeshLayer> Layers;
	Layers.resize(LayerCount);

	for (size_t i = 0; i < Layers.size(); i++)
	{
		if (Version >= 0.55)
		{
			File.read(Buffer, 4);
			const int LayerType = *(int*)Buffer;
			Layers[i].SetType(LAYER_TYPE(LayerType));
		}

		Layers[i].SetCaption(FILE_SYSTEM.ReadFEString(File));
		Layers[i].SetNote(FILE_SYSTEM.ReadFEString(File));

		// TrianglesToData
		File.read(Buffer, 4);
		const int TrianglesToDataCout = *(int*)Buffer;
		std::vector<float> TrianglesData;
		Layers[i].TrianglesToData.resize(TrianglesToDataCout);
		File.read((char*)Layers[i].TrianglesToData.data(), TrianglesToDataCout * 4);

		// Debug info.
		File.read(Buffer, 4);
		const int DebugInfoPresent = *(int*)Buffer;
		if (DebugInfoPresent)
		{
			Layers[i].DebugInfo = new MeshLayerDebugInfo();
			Layers[i].DebugInfo->FromFile(File);
		}
	}

	FEAABB MeshAABB;

	glm::vec3 Min;
	File.read(Buffer, 4);
	Min.x = *(float*)Buffer;
	File.read(Buffer, 4);
	Min.y = *(float*)Buffer;
	File.read(Buffer, 4);
	Min.z = *(float*)Buffer;

	glm::vec3 Max;
	File.read(Buffer, 4);
	Max.x = *(float*)Buffer;
	File.read(Buffer, 4);
	Max.y = *(float*)Buffer;
	File.read(Buffer, 4);
	Max.z = *(float*)Buffer;

	MeshAABB = FEAABB(Min, Max);

	File.close();

	FEMesh* NewMesh = RawDataToMesh((float*)VertexBuffer, VertexCount,
		(float*)ColorBuffer, ColorCount,
		(float*)TexBuffer, TexCout,
		(float*)NormBuffer, NormCout,
		(float*)TangBuffer, TangCout,
		(int*)IndexBuffer, IndexCout,
		nullptr, 0, 0, "");

	std::vector<double> FEVertices;
	FEVertices.resize(VertexCount);
	for (size_t i = 0; i < VertexCount; i++)
	{
		FEVertices[i] = ((float*)VertexBuffer)[i];
	}

	std::vector<int> FEIndices;
	FEIndices.resize(IndexCout);
	for (size_t i = 0; i < IndexCout; i++)
	{
		FEIndices[i] = ((int*)IndexBuffer)[i];
	}

	std::vector<float> FENormals;
	FENormals.resize(NormCout);
	for (size_t i = 0; i < NormCout; i++)
	{
		FENormals[i] = ((float*)NormBuffer)[i];
	}

	COMPLEXITY_METRIC_MANAGER.Init(FEVertices, FEIndices, FENormals);

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;

	NewMesh->AABB = MeshAABB;

	for (size_t i = 0; i < Layers.size(); i++)
	{
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(Layers[i]);
	}

	return NewMesh;
}

FEMesh* MeshManager::LoadMesh(std::string FileName)
{
	FEMesh* Result = nullptr;

	if (!FILE_SYSTEM.CheckFile(FileName.c_str()))
		return Result;

	std::string FileExtention = FILE_SYSTEM.GetFileExtension(FileName.c_str());
	// Convert to lower case.
	std::transform(FileExtention.begin(), FileExtention.end(), FileExtention.begin(), [](const unsigned char C) { return std::tolower(C); });

	if (FileExtention == ".obj")
	{
		Result = ImportOBJ(FileName.c_str(), true);
	}
	else if (FileExtention == ".rug")
	{
		Result = LoadRUGMesh(FileName);
	}

	if (Result == nullptr)
	{
		LOG.Add("Failed to load mesh with path: " + FileName);
		return Result;
	}

	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->FileName = FILE_SYSTEM.GetFileName(FileName.c_str());
	ActiveMesh = Result;

	for (size_t i = 0; i < ClientLoadCallbacks.size(); i++)
	{
		if (ClientLoadCallbacks[i] == nullptr)
			continue;

		ClientLoadCallbacks[i]();
	}

	return Result;
}

void MeshManager::AddLoadCallback(std::function<void()> Func)
{
	ClientLoadCallbacks.push_back(Func);
}

void MeshManager::SaveRUGMesh(FEMesh* Mesh)
{
	if (Mesh == nullptr)
		return;

	std::string FilePath;
	FILE_SYSTEM.ShowFileSaveDialog(FilePath, RUGOSITY_SAVE_FILE_FILTER, 1);

	if (FilePath.empty())
		return;

	if (FilePath.find(".rug") == std::string::npos)
		FilePath += ".rug";

	std::fstream file;
	file.open(FilePath, std::ios::out | std::ios::binary);

	// Version of FEMesh file type.
	float version = APP_VERSION;
	file.write((char*)&version, sizeof(float));

	int Count = Mesh->getPositionsCount();
	float* Positions = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getPositionsBufferID(), 0, sizeof(float) * Count, Positions));
	file.write((char*)&Count, sizeof(int));
	file.write((char*)Positions, sizeof(float) * Count);

	Count = Mesh->getColorCount();
	float* Colors = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getColorBufferID(), 0, sizeof(float) * Count, Colors));
	file.write((char*)&Count, sizeof(int));
	file.write((char*)Colors, sizeof(float) * Count);

	Count = Mesh->getUVCount();
	float* UV = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getUVBufferID(), 0, sizeof(float) * Count, UV));
	file.write((char*)&Count, sizeof(int));
	file.write((char*)UV, sizeof(float) * Count);

	Count = Mesh->getNormalsCount();
	float* Normals = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getNormalsBufferID(), 0, sizeof(float) * Count, Normals));
	file.write((char*)&Count, sizeof(int));
	file.write((char*)Normals, sizeof(float) * Count);

	Count = Mesh->getTangentsCount();
	float* Tangents = new float[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getTangentsBufferID(), 0, sizeof(float) * Count, Tangents));
	file.write((char*)&Count, sizeof(int));
	file.write((char*)Tangents, sizeof(float) * Count);

	Count = Mesh->getIndicesCount();
	int* Indices = new int[Count];
	FE_GL_ERROR(glGetNamedBufferSubData(Mesh->getIndicesBufferID(), 0, sizeof(int) * Count, Indices));
	file.write((char*)&Count, sizeof(int));
	file.write((char*)Indices, sizeof(int) * Count);

	Count = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size();
	file.write((char*)&Count, sizeof(int));

	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size(); i++)
	{
		int LayerType = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].GetType();
		file.write((char*)&LayerType, sizeof(int));

		Count = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].GetCaption().size());
		file.write((char*)&Count, sizeof(int));
		file.write((char*)COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].GetCaption().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].GetNote().size());
		file.write((char*)&Count, sizeof(int));
		file.write((char*)COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].GetNote().c_str(), sizeof(char) * Count);

		Count = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].TrianglesToData.size();
		file.write((char*)&Count, sizeof(int));
		file.write((char*)COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].TrianglesToData.data(), sizeof(float) * Count);

		Count = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].DebugInfo != nullptr;
		file.write((char*)&Count, sizeof(int));
		if (Count)
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[i].DebugInfo->ToFile(file);
	}

	FEAABB TempAABB(Positions, Mesh->getPositionsCount());
	file.write((char*)&TempAABB.getMin()[0], sizeof(float));
	file.write((char*)&TempAABB.getMin()[1], sizeof(float));
	file.write((char*)&TempAABB.getMin()[2], sizeof(float));

	file.write((char*)&TempAABB.getMax()[0], sizeof(float));
	file.write((char*)&TempAABB.getMax()[1], sizeof(float));
	file.write((char*)&TempAABB.getMax()[2], sizeof(float));

	file.close();

	delete[] Positions;
	delete[] UV;
	delete[] Normals;
	delete[] Tangents;
	delete[] Indices;
}