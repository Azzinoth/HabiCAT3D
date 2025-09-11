#include "MeshManager.h"
using namespace FocalEngine;
#include "ComplexityCore/Layers/LayerManager.h"

MeshManager::MeshManager()
{
	if (!APPLICATION.HasConsoleWindow())
	{
		CustomMeshShader = RESOURCE_MANAGER.CreateShader("MainMeshShader", CustomMesh_VS, CustomMesh_FS);
		CustomMeshShader->UpdateUniformData("lightDirection", glm::vec3(0.0, 1.0, 0.2));

		CustomMaterial = RESOURCE_MANAGER.CreateMaterial("MainMeshMaterial");
		CustomMaterial->Shader = CustomMeshShader;
	}
}

MeshManager::~MeshManager() {}

FEMesh* MeshManager::ImportOBJ(const char* FileName, bool bForceOneMesh)
{
	FEMesh* Result = nullptr;
	FEObjLoader& OBJLoader = FEObjLoader::GetInstance();
	OBJLoader.ForceOneMesh(bForceOneMesh);
	OBJLoader.ForcePositionNormalization(true);
	OBJLoader.UseDoublePrecisionForReadingCoordinates(true);
	OBJLoader.DoubleVertexOnSeams(false);
	OBJLoader.ReadFile(FileName);

	std::vector<FERawOBJData*>* LoadedObjects = OBJLoader.GetLoadedObjects();
	FERawOBJData* FirstObject = LoadedObjects->size() > 0 ? LoadedObjects->at(0) : nullptr;

	if (FirstObject != nullptr)
	{
		Result = RESOURCE_MANAGER.RawDataToMesh(FirstObject->FVerC.data(), int(FirstObject->FVerC.size()),
												FirstObject->FTexC.data(), int(FirstObject->FTexC.size()),
												FirstObject->FNorC.data(), int(FirstObject->FNorC.size()),
												FirstObject->FTanC.data(), int(FirstObject->FTanC.size()),
												FirstObject->FInd.data(), int(FirstObject->FInd.size()),
												FirstObject->FColorsC.data(), int(FirstObject->FColorsC.size()),
												FirstObject->MaterialIDs.data(), int(FirstObject->MaterialIDs.size()), int(FirstObject->MaterialRecords.size()), "");
	
		ANALYSIS_OBJECT_MANAGER.InitializeMeshData(FirstObject->DVerC, FirstObject->FColorsC, FirstObject->FTexC, FirstObject->FTanC, FirstObject->FInd, FirstObject->FNorC);
	}
	
	return Result;
}

// FIX ME: That function should not be here.
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

	int BytesPerVertex = 8;
	if (Version < 0.87)
		BytesPerVertex = 4;
	ArraySize = long long(VertexCount) * long long(BytesPerVertex);
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
	std::vector<DataLayer> Layers;
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

		// ElementsToData
		File.read(Buffer, 4);
		const int ElementsToDataCout = *(int*)Buffer;
		std::vector<float> TrianglesData;
		Layers[i].ElementsToData.resize(ElementsToDataCout);
		File.read((char*)Layers[i].ElementsToData.data(), ElementsToDataCout * 4);

		// Debug info.
		File.read(Buffer, 4);
		const int DebugInfoPresent = *(int*)Buffer;
		if (DebugInfoPresent)
		{
			Layers[i].DebugInfo = new DataLayerDebugInfo();
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

	std::vector<double> FEVertices;
	FEMesh* NewMesh = nullptr;
	if (Version < 0.87)
	{
		NewMesh = RESOURCE_MANAGER.RawDataToMesh((float*)VertexBuffer, VertexCount,
												 (float*)TexBuffer, TexCout,
												 (float*)NormBuffer, NormCout,
												 (float*)TangBuffer, TangCout,
												 (int*)IndexBuffer, IndexCout,
												 (float*)ColorBuffer, ColorCount,
												 nullptr, 0, 0, "");

		FEVertices.resize(VertexCount);
		for (size_t i = 0; i < VertexCount; i++)
		{
			FEVertices[i] = ((float*)VertexBuffer)[i];
		}
	}
	else
	{
		std::vector<float> FEFloatVertices;
		FEFloatVertices.resize(VertexCount);
		for (size_t i = 0; i < VertexCount; i++)
		{
			FEFloatVertices[i] = static_cast<float>(((double*)VertexBuffer)[i]);
		}


		NewMesh = RESOURCE_MANAGER.RawDataToMesh((float*)FEFloatVertices.data(), VertexCount,
												 (float*)TexBuffer, TexCout,
												 (float*)NormBuffer, NormCout,
												 (float*)TangBuffer, TangCout,
												 (int*)IndexBuffer, IndexCout,
												 (float*)ColorBuffer, ColorCount,
												 nullptr, 0, 0, "");

		FEVertices.resize(VertexCount);
		for (size_t i = 0; i < VertexCount; i++)
		{
			FEVertices[i] = ((double*)VertexBuffer)[i];
		}
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
	
	ANALYSIS_OBJECT_MANAGER.InitializeMeshData(FEVertices, FEColors, FEUVs, FETangents, FEIndices, FENormals);

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;

	// FIX ME: That portion should not be here.
	for (size_t i = 0; i < Layers.size(); i++)
		LAYER_MANAGER.AddLayer(Layers[i]);

	return NewMesh;
}

FEMesh* MeshManager::LoadResource(std::string FileName)
{
	FEMesh* Result = nullptr;
	if (!FILE_SYSTEM.DoesFileExist(FileName.c_str()))
		return Result;

	std::string FileExtension = FILE_SYSTEM.GetFileExtension(FileName.c_str());
	// Convert to lower case.
	std::transform(FileExtension.begin(), FileExtension.end(), FileExtension.begin(), [](const unsigned char Character) {
		return std::tolower(Character);
	});

	if (FileExtension == ".obj")
	{
		Result = ImportOBJ(FileName.c_str(), true);
	}
	else if (FileExtension == ".rug")
	{
		Result = LoadRUGMesh(FileName);
	}
	else if (FileExtension == ".ply")
	{
		FEObject* LoadedObject = RESOURCE_MANAGER.ImportPLYFile(FileName);
		if (LoadedObject->GetType() == FE_POINT_CLOUD)
		{
			if (CurrentPointCloudEntity != nullptr)
			{
				MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(CurrentPointCloudEntity);
				CurrentPointCloudEntity = nullptr;
			}
				
			if (CurrentPointCloud != nullptr)
			{
				RESOURCE_MANAGER.DeleteFEPointCloud(CurrentPointCloud);
				CurrentPointCloud = nullptr;
			}

			CurrentPointCloud = static_cast<FEPointCloud*>(LoadedObject);
			CurrentPointCloudEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Point cloud entity");
			CurrentPointCloudEntity->AddComponent<FEPointCloudComponent>(CurrentPointCloud);
			CurrentPointCloud->SetAdvancedRenderingEnabled(true);

			ANALYSIS_OBJECT_MANAGER.InitializePointCloudData(CurrentPointCloud);
		}
	}
	else if (FileExtension == ".las" || FileExtension == ".laz")
	{
		FEPointCloud* PointCloud = RESOURCE_MANAGER.ImportPointCloud(FileName);
		if (PointCloud != nullptr)
		{
			if (CurrentPointCloudEntity != nullptr)
			{
				MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(CurrentPointCloudEntity);
				CurrentPointCloudEntity = nullptr;
			}

			if (CurrentPointCloud != nullptr)
			{
				RESOURCE_MANAGER.DeleteFEPointCloud(CurrentPointCloud);
				CurrentPointCloud = nullptr;
			}

			CurrentPointCloud = PointCloud;
			CurrentPointCloudEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Point cloud entity");
			CurrentPointCloudEntity->AddComponent<FEPointCloudComponent>(CurrentPointCloud);
			CurrentPointCloud->SetAdvancedRenderingEnabled(true);

			ANALYSIS_OBJECT_MANAGER.InitializePointCloudData(CurrentPointCloud);
		}
	}

	if (Result == nullptr)
	{
		LOG.Add("Failed to load resource with path: " + FileName);
		return Result;
	}

	ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->FileName = FILE_SYSTEM.GetFileName(FileName.c_str());
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

	ANALYSIS_OBJECT_MANAGER.SaveToRUGFileAskForFilePath();
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
	if (ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData == nullptr)
		return;

	if (ActiveMesh == nullptr)
		return;

	if (LayerIndex < 0 || LayerIndex >= LAYER_MANAGER.Layers.size())
		return;

	if (LAYER_MANAGER.Layers[LayerIndex].RawData.empty())
		LAYER_MANAGER.Layers[LayerIndex].FillRawData();

	FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

	if (GPULayerIndex == 0)
	{
		FirstLayerBufferID = 0;
		FE_GL_ERROR(glGenBuffers(1, &FirstLayerBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, FirstLayerBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * LAYER_MANAGER.Layers[LayerIndex].RawData.size(), LAYER_MANAGER.Layers[LayerIndex].RawData.data(), GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(7, 3, GL_FLOAT, false, 0, nullptr));
	}
	else
	{
		SecondLayerBufferID = 0;
		FE_GL_ERROR(glGenBuffers(1, &SecondLayerBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, SecondLayerBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * LAYER_MANAGER.Layers[LayerIndex].RawData.size(), LAYER_MANAGER.Layers[LayerIndex].RawData.data(), GL_STATIC_DRAW));
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

	double CurrentDistance = 0.0;
	double LastDistance = 9999.0;

	int TriangeIndex = -1;
	ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->TriangleSelected.clear();

	for (int i = 0; i < ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size(); i++)
	{
		std::vector<glm::dvec3> TranformedTrianglePoints = ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			//TranformedTrianglePoints[j] = ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
			TranformedTrianglePoints[j] = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		const bool bHit = GEOMETRY.IsRayIntersectingTriangle(MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MouseRay, TranformedTrianglePoints, CurrentDistance);

		if (bHit && CurrentDistance < LastDistance)
		{
			LastDistance = CurrentDistance;
			TriangeIndex = i;
		}
	}

	if (TriangeIndex != -1)
	{
		ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->TriangleSelected.push_back(TriangeIndex);
		return true;
	}

	return false;
}

glm::vec3 MeshManager::IntersectTriangle(glm::dvec3 MouseRay)
{
	if (ActiveMesh == nullptr || ActiveEntity == nullptr)
		return glm::vec3(0.0f);

	double CurrentDistance = 0.0;
	double LastDistance = 9999.0;

	for (size_t i = 0; i < ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size(); i++)
	{
		std::vector<glm::dvec3> TranformedTrianglePoints = ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			//TranformedTrianglePoints[j] = ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
			TranformedTrianglePoints[j] = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		glm::dvec3 HitPosition;
		const bool bHit = GEOMETRY.IsRayIntersectingTriangle(MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MouseRay, TranformedTrianglePoints, CurrentDistance, &HitPosition);

		if (bHit && CurrentDistance < LastDistance)
		{
			LastDistance = CurrentDistance;

			//const glm::mat4 Inverse = glm::inverse(ActiveEntity->Transform.GetTransformMatrix());
			const glm::mat4 Inverse = glm::inverse(ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
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

	MeshGeometryData* CurrentMeshData = ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData;
	if (ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData == nullptr)
		return Result;

	if (CurrentMeshData->TriangleSelected.size() == 0)
		return Result;

	MeasuredRugosityAreaRadius = Radius;
	//MeasuredRugosityAreaCenter = ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(CurrentMeshData->TrianglesCentroids[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TriangleSelected[0]], 1.0f);
	MeasuredRugosityAreaCenter = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(CurrentMeshData->TrianglesCentroids[CurrentMeshData->TriangleSelected[0]], 1.0f);

	const glm::dvec3 FirstSelectedTriangleCentroid = CurrentMeshData->TrianglesCentroids[CurrentMeshData->TriangleSelected[0]];

	for (size_t i = 0; i < CurrentMeshData->Triangles.size(); i++)
	{
		if (i == CurrentMeshData->TriangleSelected[0])
			continue;

		if (glm::distance(FirstSelectedTriangleCentroid, CurrentMeshData->TrianglesCentroids[i]) <= Radius)
		{
			CurrentMeshData->TriangleSelected.push_back(static_cast<int>(i));
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

#include "UI/UIManager.h"
void MeshManager::UpdateUniforms()
{
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("AmbientFactor", UI.GetAmbientLightFactor());
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("HaveColor", ActiveMesh->GetColorCount() == 0 ? 0 : 1);
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("HeatMapType", MESH_MANAGER.GetHeatMapType());
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerIndex", LAYER_MANAGER.GetActiveLayerIndex());

	MESH_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaSaturationFactor", MESH_MANAGER.GetUnselectedAreaSaturationFactor());
	MESH_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaBrightnessFactor", MESH_MANAGER.GetUnselectedAreaBrightnessFactor());

	if (LAYER_MANAGER.GetActiveLayer() != nullptr)
	{
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMin());
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMax());
	}
	else
	{
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", 0.0f);
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", 0.0f);
	}

	if (LAYER_MANAGER.GetActiveLayerIndex() != -1)
	{
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerMin", float(LAYER_MANAGER.Layers[LAYER_MANAGER.GetActiveLayerIndex()].MinVisible));
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerMax", float(LAYER_MANAGER.Layers[LAYER_MANAGER.GetActiveLayerIndex()].MaxVisible));

		MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMin", float(LAYER_MANAGER.Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMin()));
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMax", float(LAYER_MANAGER.Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMax()));
	}

	if (ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->TriangleSelected.size() > 1 && UI.GetLayerSelectionMode() == 2)
	{
		float TempMeasuredRugosityAreaRadius = 0.0f;
		glm::vec3 TempMeasuredRugosityAreaCenter = glm::vec3(0.0f);
		MESH_MANAGER.GetMeasuredRugosityArea(TempMeasuredRugosityAreaRadius, TempMeasuredRugosityAreaCenter);
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", TempMeasuredRugosityAreaRadius);
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaCenter", TempMeasuredRugosityAreaCenter);
	}
	else
	{
		MESH_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", -1.0f);
	}
}