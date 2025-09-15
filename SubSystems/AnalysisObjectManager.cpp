#include "AnalysisObjectManager.h"
using namespace FocalEngine;
#include "ComplexityCore/Layers/LayerManager.h"

AnalysisObjectManager::AnalysisObjectManager()
{
	if (!APPLICATION.HasConsoleWindow())
	{
		CustomMeshShader = RESOURCE_MANAGER.CreateShader("MainMeshShader", CustomMesh_VS, CustomMesh_FS);
		CustomMeshShader->UpdateUniformData("lightDirection", glm::vec3(0.0, 1.0, 0.2));

		CustomMaterial = RESOURCE_MANAGER.CreateMaterial("MainMeshMaterial");
		CustomMaterial->Shader = CustomMeshShader;
	}
}

AnalysisObjectManager::~AnalysisObjectManager() {}

MeshAnalysisData* AnalysisObjectManager::ExtractAdditionalGeometryData(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals)
{
	MeshAnalysisData* Result = new MeshAnalysisData();
	Result->Vertices = Vertices;
	Result->Colors = Colors;
	Result->UVs = UVs;
	Result->Tangents = Tangents;
	Result->Indices = Indices;
	Result->Normals = Normals;

	std::vector<glm::dvec3> Triangle;
	Triangle.resize(3);
	std::vector<glm::vec3> TriangleNormal;
	TriangleNormal.resize(3);

	for (size_t i = 0; i < Indices.size(); i += 3)
	{
		int VertexPosition = Indices[i] * 3;
		Triangle[0] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 1] * 3;
		Triangle[1] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 2] * 3;
		Triangle[2] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		Result->Triangles.push_back(Triangle);
		Result->TrianglesArea.push_back(GEOMETRY.CalculateTriangleArea(Triangle[0], Triangle[1], Triangle[2]));
		Result->TotalArea += Result->TrianglesArea.back();

		Result->TrianglesCentroids.push_back((Triangle[0] + Triangle[1] + Triangle[2]) / 3.0);

		if (!Normals.empty())
		{
			VertexPosition = Indices[i] * 3;
			TriangleNormal[0] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			VertexPosition = Indices[i + 1] * 3;
			TriangleNormal[1] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			VertexPosition = Indices[i + 2] * 3;
			TriangleNormal[2] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			Result->TrianglesNormals.push_back(TriangleNormal);
		}
	}
	Result->UpdateAverageNormal();

	Result->AABB = FEAABB(Vertices.data(), static_cast<int>(Vertices.size()));
	return Result;
}

AnalysisObject* AnalysisObjectManager::ImportOBJ(const char* FilePath, bool bForceOneMesh)
{
	AnalysisObject* Result = new AnalysisObject();
	Result->Type = DATA_SOURCE_TYPE::MESH;
	Result->FilePath = FilePath;

	FEMesh* LoadedMesh = nullptr;
	FEObjLoader& OBJLoader = FEObjLoader::GetInstance();
	OBJLoader.ForceOneMesh(bForceOneMesh);
	OBJLoader.ForcePositionNormalization(true);
	OBJLoader.UseDoublePrecisionForReadingCoordinates(true);
	OBJLoader.DoubleVertexOnSeams(false);
	OBJLoader.ReadFile(FilePath);

	std::vector<FERawOBJData*>* LoadedObjects = OBJLoader.GetLoadedObjects();
	FERawOBJData* FirstObject = LoadedObjects->empty() ? nullptr : (*LoadedObjects)[0];

	if (FirstObject != nullptr)
	{
		if (!APPLICATION.HasConsoleWindow())
		{
			LoadedMesh = RESOURCE_MANAGER.RawDataToMesh(FirstObject->FVerC.data(), int(FirstObject->FVerC.size()),
														FirstObject->FTexC.data(), int(FirstObject->FTexC.size()),
														FirstObject->FNorC.data(), int(FirstObject->FNorC.size()),
														FirstObject->FTanC.data(), int(FirstObject->FTanC.size()),
														FirstObject->FInd.data(), int(FirstObject->FInd.size()),
														FirstObject->FColorsC.data(), int(FirstObject->FColorsC.size()),
														FirstObject->MaterialIDs.data(), int(FirstObject->MaterialIDs.size()), int(FirstObject->MaterialRecords.size()), "");
		}
		
		Result->EngineResource = LoadedMesh;
		Result->GeometryData = ExtractAdditionalGeometryData(FirstObject->DVerC, FirstObject->FColorsC, FirstObject->FTexC, FirstObject->FTanC, FirstObject->FInd, FirstObject->FNorC);
		
		//AnalysisObjects[Result->ID] = Result;
	}
	
	return Result;
}

AnalysisObject* AnalysisObjectManager::LoadRUGMesh(std::string FilePath)
{
	std::fstream File;

	File.open(FilePath, std::ios::in | std::ios::binary);
	const std::streamsize FileSize = File.tellg();
	if (FileSize < 0)
	{
		LOG.Add(std::string("Can't load file: ") + FilePath + " in function LoadRUGMesh.");
		return nullptr;
	}

	char* Buffer = new char[4];
	long long ArraySize = 0;

	// Version of FEMesh file type
	File.read(Buffer, 4);
	const float Version = *(float*)Buffer;
	if (Version > APP_VERSION && abs(Version - APP_VERSION) > 0.0001)
	{
		LOG.Add(std::string("Can't load file: ") + FilePath + " in function LoadRUGMesh. File was created in different Version of application!");
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
	
	AnalysisObject* Result = new AnalysisObject();
	Result->Type = DATA_SOURCE_TYPE::MESH;
	Result->FilePath = FilePath;
	Result->EngineResource = NewMesh;
	Result->GeometryData = ExtractAdditionalGeometryData(FEVertices, FEColors, FEUVs, FETangents, FEIndices, FENormals);

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;

	// FIX ME: That portion should not be here.
	for (size_t i = 0; i < Layers.size(); i++)
		LAYER_MANAGER.AddLayer(Layers[i]);

	return Result;
}

AnalysisObject* AnalysisObjectManager::LoadResource(std::string FilePath)
{
	AnalysisObject* Result = nullptr;
	if (!FILE_SYSTEM.DoesFileExist(FilePath.c_str()))
		return Result;

	std::string FileExtension = FILE_SYSTEM.GetFileExtension(FilePath.c_str());
	// Convert to lower case.
	std::transform(FileExtension.begin(), FileExtension.end(), FileExtension.begin(), [](const unsigned char Character) {
		return std::tolower(Character);
	});

	if (FileExtension == ".obj")
	{
		Result = ImportOBJ(FilePath.c_str(), true);
	}
	else if (FileExtension == ".rug")
	{
		Result = LoadRUGMesh(FilePath);
		Result->Name = FILE_SYSTEM.GetFileName(FilePath, false);
	}
	else if (FileExtension == ".ply")
	{
		FEObject* LoadedObject = RESOURCE_MANAGER.ImportPLYFile(FilePath);
		if (LoadedObject->GetType() == FE_POINT_CLOUD)
		{
			Result = new AnalysisObject();
			Result->Type = DATA_SOURCE_TYPE::POINT_CLOUD;
			Result->FilePath = FilePath;
			Result->Name = FILE_SYSTEM.GetFileName(FilePath, false);
			Result->EngineResource = LoadedObject;
			Result->GeometryData = ExtractAdditionalGeometryData(static_cast<FEPointCloud*>(LoadedObject));
		}
	}
	else if (FileExtension == ".las" || FileExtension == ".laz")
	{
		FEPointCloud* PointCloud = RESOURCE_MANAGER.ImportPointCloud(FilePath);
		if (PointCloud != nullptr)
		{
			Result = new AnalysisObject();
			Result->Type = DATA_SOURCE_TYPE::POINT_CLOUD;
			Result->FilePath = FilePath;
			Result->Name = FILE_SYSTEM.GetFileName(FilePath, false);
			Result->EngineResource = PointCloud;
			Result->GeometryData = ExtractAdditionalGeometryData(PointCloud);
		}
	}

	if (Result != nullptr)
	{
		InitializeSceneObjects(Result);
		AnalysisObjects[Result->ID] = Result;
		SetActiveAnalysisObject(Result->ID);

		for (size_t i = 0; i < ClientOnLoadCallbacks.size(); i++)
		{
			if (ClientOnLoadCallbacks[i] == nullptr)
				continue;

			ClientOnLoadCallbacks[i](Result);
		}
	}

	return Result;
}

void AnalysisObjectManager::AddOnLoadCallback(std::function<void(AnalysisObject*)> Callback)
{
	ClientOnLoadCallbacks.push_back(Callback);
}

void AnalysisObjectManager::ComplexityMetricDataToGPU(int LayerIndex, int GPULayerIndex)
{
	if (LayerIndex < 0 || LayerIndex >= LAYER_MANAGER.Layers.size())
		return;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	DataLayer& CurrentLayer = LAYER_MANAGER.Layers[LayerIndex];
	if (CurrentLayer.GetDataSourceType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
		if (CurrentMeshAnalysisData == nullptr)
			return;

		FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		if (LAYER_MANAGER.Layers[LayerIndex].RawData.empty())
			LAYER_MANAGER.Layers[LayerIndex].FillRawData();

		FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

		if (GPULayerIndex == 0)
		{
			CurrentMeshAnalysisData->FirstLayerBufferID = 0;
			FE_GL_ERROR(glGenBuffers(1, &CurrentMeshAnalysisData->FirstLayerBufferID));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshAnalysisData->FirstLayerBufferID));
			FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * LAYER_MANAGER.Layers[LayerIndex].RawData.size(), LAYER_MANAGER.Layers[LayerIndex].RawData.data(), GL_STATIC_DRAW));
			FE_GL_ERROR(glVertexAttribPointer(7, 3, GL_FLOAT, false, 0, nullptr));
		}
		else
		{
			CurrentMeshAnalysisData->SecondLayerBufferID = 0;
			FE_GL_ERROR(glGenBuffers(1, &CurrentMeshAnalysisData->SecondLayerBufferID));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshAnalysisData->SecondLayerBufferID));
			FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * LAYER_MANAGER.Layers[LayerIndex].RawData.size(), LAYER_MANAGER.Layers[LayerIndex].RawData.data(), GL_STATIC_DRAW));
			FE_GL_ERROR(glVertexAttribPointer(8, 3, GL_FLOAT, false, 0, nullptr));
		}

		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
	else if (CurrentLayer.GetDataSourceType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		PointCloudAnalysisData* CurrentPointCloudAnalysisData = static_cast<PointCloudAnalysisData*>(CurrentObject->GetGeometryData());
		if (CurrentPointCloudAnalysisData == nullptr)
			return;

		for (size_t i = 0; i < CurrentPointCloudAnalysisData->RawPointCloudData.size(); i++)
		{
			float NormalizedValue = (CurrentLayer.ElementsToData[i] - CurrentLayer.GetMin()) / (CurrentLayer.GetMax() - CurrentLayer.GetMin());
			glm::vec3 NewColor = GetTurboColorMap(NormalizedValue);
			CurrentPointCloudAnalysisData->RawPointCloudData[i].R = static_cast<unsigned char>(NewColor.x * 255.0f);
			CurrentPointCloudAnalysisData->RawPointCloudData[i].G = static_cast<unsigned char>(NewColor.y * 255.0f);
			CurrentPointCloudAnalysisData->RawPointCloudData[i].B = static_cast<unsigned char>(NewColor.z * 255.0f);
		}

		FEPointCloud* PointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(CurrentPointCloudAnalysisData->RawPointCloudData);

		FEEntity* PointCloudEntity = CurrentObject->GetEntity();
		FEPointCloud* OldPointCloud = static_cast<FEPointCloud*>(CurrentObject->GetEngineResource());
		if (PointCloudEntity != nullptr)
		{
			PointCloudEntity->RemoveComponent<FEPointCloudComponent>();
			RESOURCE_MANAGER.DeleteFEPointCloud(OldPointCloud);
			CurrentObject->EngineResource = PointCloud;
			PointCloudEntity->AddComponent<FEPointCloudComponent>(PointCloud);
		}
	}
}

bool AnalysisObjectManager::SelectTriangle(glm::dvec3 MouseRay)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return false;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return false;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	double CurrentDistance = 0.0;
	double LastDistance = 9999.0;

	int TriangeIndex = -1;
	CurrentMeshAnalysisData->TriangleSelected.clear();

	for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		std::vector<glm::dvec3> TranformedTrianglePoints = CurrentMeshAnalysisData->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
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
		CurrentMeshAnalysisData->TriangleSelected.push_back(TriangeIndex);
		return true;
	}

	return false;
}

glm::vec3 AnalysisObjectManager::IntersectTriangle(glm::dvec3 MouseRay)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return glm::vec3(0.0f);

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return glm::vec3(0.0f);

	FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return glm::vec3(0.0f);

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return glm::vec3(0.0f);

	double CurrentDistance = 0.0;
	double LastDistance = 9999.0;

	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		std::vector<glm::dvec3> TranformedTrianglePoints = CurrentMeshAnalysisData->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		glm::dvec3 HitPosition;
		const bool bHit = GEOMETRY.IsRayIntersectingTriangle(MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MouseRay, TranformedTrianglePoints, CurrentDistance, &HitPosition);

		if (bHit && CurrentDistance < LastDistance)
		{
			LastDistance = CurrentDistance;

			const glm::mat4 Inverse = glm::inverse(ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
			return Inverse * glm::vec4(HitPosition, 1.0f);
		}
	}

	return glm::vec3(0.0f);
}

bool AnalysisObjectManager::SelectTrianglesInRadius(glm::dvec3 MouseRay, float Radius)
{
	bool Result = false;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return Result;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;
	
	SelectTriangle(MouseRay);

	if (CurrentMeshAnalysisData->TriangleSelected.size() == 0)
		return Result;

	CurrentMeshAnalysisData->MeasuredRugosityAreaRadius = Radius;
	CurrentMeshAnalysisData->MeasuredRugosityAreaCenter = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(CurrentMeshAnalysisData->TrianglesCentroids[CurrentMeshAnalysisData->TriangleSelected[0]], 1.0f);

	const glm::dvec3 FirstSelectedTriangleCentroid = CurrentMeshAnalysisData->TrianglesCentroids[CurrentMeshAnalysisData->TriangleSelected[0]];

	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		if (i == CurrentMeshAnalysisData->TriangleSelected[0])
			continue;

		if (glm::distance(FirstSelectedTriangleCentroid, CurrentMeshAnalysisData->TrianglesCentroids[i]) <= Radius)
		{
			CurrentMeshAnalysisData->TriangleSelected.push_back(static_cast<int>(i));
			Result = true;
		}
	}

	return Result;
}

#include "UI/UIManager.h"
void AnalysisObjectManager::UpdateUniforms()
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	if (CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
		if (CurrentMeshAnalysisData == nullptr)
			return;

		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("AmbientFactor", UI.GetAmbientLightFactor());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("HaveColor", ActiveMesh->GetColorCount() == 0 ? 0 : 1);
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("HeatMapType", CurrentMeshAnalysisData->GetHeatMapType());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerIndex", LAYER_MANAGER.GetActiveLayerIndex());

		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaSaturationFactor", CurrentMeshAnalysisData->GetUnselectedAreaSaturationFactor());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaBrightnessFactor", CurrentMeshAnalysisData->GetUnselectedAreaBrightnessFactor());

		if (LAYER_MANAGER.GetActiveLayer() != nullptr)
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMin());
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", LAYER_MANAGER.GetActiveLayer()->GetSelectedRangeMax());
		}
		else
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", 0.0f);
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", 0.0f);
		}

		if (LAYER_MANAGER.GetActiveLayerIndex() != -1)
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerMin", float(LAYER_MANAGER.Layers[LAYER_MANAGER.GetActiveLayerIndex()].MinVisible));
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerMax", float(LAYER_MANAGER.Layers[LAYER_MANAGER.GetActiveLayerIndex()].MaxVisible));

			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMin", float(LAYER_MANAGER.Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMin()));
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMax", float(LAYER_MANAGER.Layers[LAYER_MANAGER.GetActiveLayerIndex()].GetMax()));
		}

		AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", -1.0f);
		if (CurrentObject != nullptr)
		{
			if (CurrentMeshAnalysisData->TriangleSelected.size() > 1 && UI.GetLayerSelectionMode() == 2)
			{
				float TempMeasuredRugosityAreaRadius = 0.0f;
				glm::vec3 TempMeasuredRugosityAreaCenter = glm::vec3(0.0f);
				CurrentMeshAnalysisData->GetMeasuredRugosityArea(TempMeasuredRugosityAreaRadius, TempMeasuredRugosityAreaCenter);
				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", TempMeasuredRugosityAreaRadius);
				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaCenter", TempMeasuredRugosityAreaCenter);
			}
		}
	}
}

AnalysisObject* AnalysisObjectManager::GetAnalysisObject(std::string ID)
{
	if (AnalysisObjects.find(ID) != AnalysisObjects.end())
		return AnalysisObjects[ID];

	return nullptr;
}

AnalysisObject* AnalysisObjectManager::GetActiveAnalysisObject()
{
	return GetAnalysisObject(ActiveAnalysisObjectID);
}

bool AnalysisObjectManager::SetActiveAnalysisObject(std::string ID)
{
	if (AnalysisObjects.find(ID) != AnalysisObjects.end())
	{
		ActiveAnalysisObjectID = ID;
		return true;
	}

	return false;
}

std::vector<std::string> AnalysisObjectManager::GetAnalysisObjectsIDList()
{
	FE_MAP_TO_STR_VECTOR(AnalysisObjects)
}

PointCloudAnalysisData* AnalysisObjectManager::ExtractAdditionalGeometryData(FEPointCloud* PointCloud)
{
	PointCloudAnalysisData* Result = new PointCloudAnalysisData();

	std::vector<FEPointCloudVertex> TemporaryRawData = PointCloud->GetRawData();
	Result->RawPointCloudData.resize(TemporaryRawData.size());
	Result->OriginalColors.resize(TemporaryRawData.size());
	for (size_t i = 0; i < TemporaryRawData.size(); i++)
	{
		Result->RawPointCloudData[i].X = static_cast<double>(TemporaryRawData[i].X);
		Result->RawPointCloudData[i].Y = static_cast<double>(TemporaryRawData[i].Y);
		Result->RawPointCloudData[i].Z = static_cast<double>(TemporaryRawData[i].Z);

		Result->RawPointCloudData[i].R = TemporaryRawData[i].R;
		Result->RawPointCloudData[i].G = TemporaryRawData[i].G;
		Result->RawPointCloudData[i].B = TemporaryRawData[i].B;
		Result->RawPointCloudData[i].A = TemporaryRawData[i].A;

		Result->OriginalColors[i] = { TemporaryRawData[i].R, TemporaryRawData[i].G, TemporaryRawData[i].B, TemporaryRawData[i].A };
	}

	Result->AABB = PointCloud->GetAABB();
	return Result;
}

FEEntity* AnalysisObjectManager::GetActiveEntity()
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return nullptr;

	return CurrentObject->GetEntity();
}

void AnalysisObjectManager::InitializeSceneObjects(AnalysisObject* NewAnalysisObject)
{
	if (APPLICATION.HasConsoleWindow())
		return;

	if (NewAnalysisObject == nullptr)
		return;

	if (NewAnalysisObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		FEMesh* ActiveMesh = static_cast<FEMesh*>(NewAnalysisObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(ActiveMesh, ANALYSIS_OBJECT_MANAGER.CustomMaterial);
		NewAnalysisObject->Entity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Mesh entity");
		NewAnalysisObject->Entity->AddComponent<FEGameModelComponent>(NewGameModel);
	}
	else if (NewAnalysisObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		FEPointCloud* PointCloud = static_cast<FEPointCloud*>(NewAnalysisObject->GetEngineResource());
		NewAnalysisObject->Entity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Point cloud entity");
		NewAnalysisObject->Entity->AddComponent<FEPointCloudComponent>(PointCloud);
		PointCloud->SetAdvancedRenderingEnabled(true);
	}
}

void AnalysisObjectManager::SaveToRUGFileAskForFilePath(std::string AnalysisObjectID)
{
	std::string FilePath;
	FILE_SYSTEM.ShowFileSaveDialog(FilePath, RUGOSITY_SAVE_FILE_FILTER, 1);

	SaveToRUGFile(FilePath, AnalysisObjectID);
}

void AnalysisObjectManager::SaveToRUGFile(std::string FilePath, std::string AnalysisObjectID)
{
	AnalysisObject* CurrentObject = GetAnalysisObject(AnalysisObjectID);
	if (CurrentObject == nullptr)
		return;

	if (FilePath.empty())
		return;

	if (FilePath.find(".rug") == std::string::npos)
		FilePath += ".rug";

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	std::fstream File;
	File.open(FilePath, std::ios::out | std::ios::binary);

	// Version of file.
	float Version = APP_VERSION;
	File.write((char*)&Version, sizeof(float));

	int Count = static_cast<int>(CurrentMeshAnalysisData->Vertices.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshAnalysisData->Vertices.data(), sizeof(double) * Count);

	Count = static_cast<int>(CurrentMeshAnalysisData->Colors.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshAnalysisData->Colors.data(), sizeof(float) * Count);

	Count = static_cast<int>(CurrentMeshAnalysisData->UVs.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshAnalysisData->UVs.data(), sizeof(float) * Count);

	Count = static_cast<int>(CurrentMeshAnalysisData->Normals.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshAnalysisData->Normals.data(), sizeof(float) * Count);

	Count = static_cast<int>(CurrentMeshAnalysisData->Tangents.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshAnalysisData->Tangents.data(), sizeof(float) * Count);

	Count = static_cast<int>(CurrentMeshAnalysisData->Indices.size());
	File.write((char*)&Count, sizeof(int));
	File.write((char*)CurrentMeshAnalysisData->Indices.data(), sizeof(int) * Count);

	// FIX ME: That portion should not be here.
	/*Count = static_cast<int>(ActiveComplexityMetricInfo->Layers.size());
	File.write((char*)&Count, sizeof(int));

	for (size_t i = 0; i < ActiveComplexityMetricInfo->Layers.size(); i++)
	{
		int LayerType = ActiveComplexityMetricInfo->Layers[i].GetType();
		File.write((char*)&LayerType, sizeof(int));

		int LayerIDSize = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetID().size() + 1);
		File.write((char*)&LayerIDSize, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].GetID().c_str(), sizeof(char) * LayerIDSize);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetCaption().size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].GetCaption().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].GetNote().size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].GetNote().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(ActiveComplexityMetricInfo->Layers[i].ElementsToData.size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)ActiveComplexityMetricInfo->Layers[i].ElementsToData.data(), sizeof(float) * Count);

		Count = ActiveComplexityMetricInfo->Layers[i].DebugInfo != nullptr;
		File.write((char*)&Count, sizeof(int));
		if (Count)
			ActiveComplexityMetricInfo->Layers[i].DebugInfo->ToFile(File);
	}*/

	FEAABB TempAABB(CurrentMeshAnalysisData->Vertices.data(), static_cast<int>(CurrentMeshAnalysisData->Vertices.size()));
	File.write((char*)&TempAABB.GetMin()[0], sizeof(float));
	File.write((char*)&TempAABB.GetMin()[1], sizeof(float));
	File.write((char*)&TempAABB.GetMin()[2], sizeof(float));

	File.write((char*)&TempAABB.GetMax()[0], sizeof(float));
	File.write((char*)&TempAABB.GetMax()[1], sizeof(float));
	File.write((char*)&TempAABB.GetMax()[2], sizeof(float));

	File.close();
}