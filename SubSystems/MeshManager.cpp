#include "MeshManager.h"
using namespace FocalEngine;

MeshManager* MeshManager::Instance = nullptr;

MeshManager::MeshManager()
{
	if (!APPLICATION.HasConsoleWindow())
	{
		CustomMeshShader = RESOURCE_MANAGER.CreateShader("MainMeshShader", CustomMesh_VS, CustomMesh_FS);
		CustomMeshShader->UpdateParameterData("lightDirection", glm::vec3(0.0, 1.0, 0.2));

		CustomMaterial = RESOURCE_MANAGER.CreateMaterial("MainMeshMaterial");
		CustomMaterial->Shader = CustomMeshShader;
	}
}

MeshManager::~MeshManager() {}

FEMesh* MeshManager::ImportOBJ(const char* FileName, bool bForceOneMesh)
{
	FEMesh* result = nullptr;
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.ForceOneMesh(bForceOneMesh);
	objLoader.ForcePositionNormalization(true);
	objLoader.UseDoublePrecisionForReadingCoordinates(true);
	objLoader.DoubleVertexOnSeams(false);
	objLoader.ReadFile(FileName);

	std::vector<FERawOBJData*>* LoadedObjects = objLoader.GetLoadedObjects();
	FERawOBJData* FirstObject = LoadedObjects->size() > 0 ? LoadedObjects->at(0) : nullptr;

	if (FirstObject != nullptr)
	{
		result = RESOURCE_MANAGER.RawDataToMesh(FirstObject->FVerC.data(), int(FirstObject->FVerC.size()),
												FirstObject->FTexC.data(), int(FirstObject->FTexC.size()),
												FirstObject->FNorC.data(), int(FirstObject->FNorC.size()),
												FirstObject->FTanC.data(), int(FirstObject->FTanC.size()),
												FirstObject->FInd.data(), int(FirstObject->FInd.size()),
												FirstObject->fColorsC.data(), int(FirstObject->fColorsC.size()),
												FirstObject->MatIDs.data(), int(FirstObject->MatIDs.size()), int(FirstObject->MaterialRecords.size()), "");
	
		COMPLEXITY_METRIC_MANAGER.Init(FirstObject->FVerC, FirstObject->fColorsC, FirstObject->FTexC, FirstObject->FTanC, FirstObject->FInd, FirstObject->FNorC);
	}
	
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

		if (Version >= 0.62)
		{
			Layers[i].ForceID(FILE_SYSTEM.ReadFEString(File));
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

	FEMesh* NewMesh = RESOURCE_MANAGER.RawDataToMesh((float*)VertexBuffer, VertexCount,
													 (float*)TexBuffer, TexCout,
													 (float*)NormBuffer, NormCout,
													 (float*)TangBuffer, TangCout,
													 (int*)IndexBuffer, IndexCout,
													 (float*)ColorBuffer, ColorCount,
													 nullptr, 0, 0, "");

	std::vector<float> FEVertices;
	FEVertices.resize(VertexCount);
	for (size_t i = 0; i < VertexCount; i++)
	{
		FEVertices[i] = ((float*)VertexBuffer)[i];
	}

	std::vector<float> FEColors;
	FEColors.resize(ColorCount);
	for (size_t i = 0; i < ColorCount; i++)
	{
		FEColors[i] = ((float*)ColorBuffer)[i];
	}

	std::vector<float> FEUVs;
	FEUVs.resize(TexCout);
	for (size_t i = 0; i < TexCout; i++)
	{
		FEUVs[i] = ((float*)TexBuffer)[i];
	}

	std::vector<float> FETangents;
	FETangents.resize(TangCout);
	for (size_t i = 0; i < TangCout; i++)
	{
		FETangents[i] = ((float*)TangBuffer)[i];
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
	
	COMPLEXITY_METRIC_MANAGER.Init(FEVertices, FEColors, FEUVs, FETangents, FEIndices, FENormals);

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;

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

	COMPLEXITY_METRIC_MANAGER.SaveToRUGFileAskForFilePath();
}

int MeshManager::GetHeatMapType()
{
	return HeatMapType;
}

void MeshManager::SetHeatMapType(int NewValue)
{
	HeatMapType = NewValue;
}

void MeshManager::ComplexityMetricDataToGPU(int LayerIndex, int GPULayerIndex)
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	if (ActiveMesh == nullptr)
		return;

	if (LayerIndex < 0 || LayerIndex >= COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
		return;

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.empty())
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].FillRawData();

	FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

	if (GPULayerIndex == 0)
	{
		FirstLayerBufferID = 0;
		FE_GL_ERROR(glGenBuffers(1, &FirstLayerBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, FirstLayerBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.size(), COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(7, 3, GL_FLOAT, false, 0, nullptr));
	}
	else
	{
		SecondLayerBufferID = 0;
		FE_GL_ERROR(glGenBuffers(1, &SecondLayerBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, SecondLayerBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.size(), COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[LayerIndex].RawData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(8, 3, GL_FLOAT, false, 0, nullptr));
	}

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

GLuint MeshManager::GetFirstLayerBufferID()
{
	return FirstLayerBufferID;
}

GLuint MeshManager::GetSecondLayerBufferID()
{
	return SecondLayerBufferID;
}

void MeshManager::GetMeasuredRugosityArea(float& Radius, glm::vec3& Center)
{
	Radius = MeasuredRugosityAreaRadius;
	Center = MeasuredRugosityAreaCenter;
}

void MeshManager::ClearMeasuredRugosityArea()
{
	MeasuredRugosityAreaRadius = -1.0f;
	MeasuredRugosityAreaCenter = glm::vec3(0.0f);
}

bool MeshManager::SelectTriangle(glm::dvec3 MouseRay)
{
	if (ActiveMesh == nullptr || ActiveEntity == nullptr)
		return false;

	float CurrentDistance = 0.0f;
	float LastDistance = 9999.0f;

	int TriangeIndex = -1;
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.clear();

	for (int i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		const bool bHit = GEOMETRY.IsRayIntersectingTriangle(ENGINE.GetCamera()->GetPosition(), MouseRay, TranformedTrianglePoints, CurrentDistance);

		if (bHit && CurrentDistance < LastDistance)
		{
			LastDistance = CurrentDistance;
			TriangeIndex = i;
		}
	}

	if (TriangeIndex != -1)
	{
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.push_back(TriangeIndex);
		return true;
	}

	return false;
}

glm::vec3 MeshManager::IntersectTriangle(glm::dvec3 MouseRay)
{
	if (ActiveMesh == nullptr || ActiveEntity == nullptr)
		return glm::vec3(0.0f);

	float CurrentDistance = 0.0f;
	float LastDistance = 9999.0f;

	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		std::vector<glm::vec3> TranformedTrianglePoints = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		glm::vec3 HitPosition;
		const bool bHit = GEOMETRY.IsRayIntersectingTriangle(ENGINE.GetCamera()->GetPosition(), MouseRay, TranformedTrianglePoints, CurrentDistance, &HitPosition);

		if (bHit && CurrentDistance < LastDistance)
		{
			LastDistance = CurrentDistance;

			const glm::mat4 Inverse = glm::inverse(ActiveEntity->Transform.GetTransformMatrix());
			return Inverse * glm::vec4(HitPosition, 1.0f);
		}
	}

	return glm::vec3(0.0f);
}

bool MeshManager::SelectTrianglesInRadius(glm::dvec3 MouseRay, float Radius)
{
	bool Result = false;

	if (ActiveMesh == nullptr || ActiveEntity == nullptr)
		return Result;
	
	SelectTriangle(MouseRay);

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.size() == 0)
		return Result;

	MeasuredRugosityAreaRadius = Radius;
	MeasuredRugosityAreaCenter = ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesCentroids[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]], 1.0f);

	const glm::vec3 FirstSelectedTriangleCentroid = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesCentroids[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]];

	for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		if (i == COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0])
			continue;

		if (glm::distance(FirstSelectedTriangleCentroid, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesCentroids[i]) <= Radius)
		{
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected.push_back(static_cast<int>(i));
			Result = true;
		}
	}

	return Result;
}

float MeshManager::GetUnselectedAreaSaturationFactor()
{
	return UnselectedAreaSaturationFactor;
}

void MeshManager::SetUnselectedAreaSaturationFactor(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	UnselectedAreaSaturationFactor = NewValue;
}

float MeshManager::GetUnselectedAreaBrightnessFactor()
{
	return UnselectedAreaBrightnessFactor;
}

void MeshManager::SetUnselectedAreaBrightnessFactor(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	UnselectedAreaBrightnessFactor = NewValue;
}

void MeshManager::ClearBuffers()
{
	if (FirstLayerBufferID > 0)
		glDeleteBuffers(1, &FirstLayerBufferID);

	if (SecondLayerBufferID > 0)
		glDeleteBuffers(1, &SecondLayerBufferID);

	FirstLayerBufferID = 0;
	SecondLayerBufferID = 0;
}