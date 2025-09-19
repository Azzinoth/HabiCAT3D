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

float AnalysisObjectManager::CheckRUGFileVersion(std::string FilePath)
{
	float Result = -1.0f;
	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add(std::string("Can't find file: ") + FilePath + " in function CheckRUGFileVersion.");
		return Result;
	}

	std::fstream File;
	File.open(FilePath, std::ios::in | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Can't open file: ") + FilePath + " in function CheckRUGFileVersion.");
		return Result;
	}

	File.seekg(0, std::ios::end);
	const std::streamsize FileSize = File.tellg();
	File.seekg(0, std::ios::beg);
	if (FileSize <= 0)
	{
		LOG.Add(std::string("Can't get file size: ") + FilePath + " in function CheckRUGFileVersion.");
		return Result;
	}

	char* Buffer = new char[4];
	File.read(Buffer, 4);
	Result = *(float*)Buffer;
	delete[] Buffer;

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
		Result->Type = DATA_SOURCE_TYPE::MESH;
		Result->FilePath = FilePath;
		Result->Name = FILE_SYSTEM.GetFileName(FilePath, false);
		Result->AnalysisData = ExtractAdditionalGeometryData(FirstObject->DVerC, FirstObject->FColorsC, FirstObject->FTexC, FirstObject->FTanC, FirstObject->FInd, FirstObject->FNorC);
	}
	
	return Result;
}

AnalysisObject* AnalysisObjectManager::LoadRUGFile(std::string FilePath)
{
	std::fstream File;
	File.open(FilePath, std::ios::in | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Can't open file: ") + FilePath + " in function LoadRUGFile.");
		return false;
	}

	File.seekg(0, std::ios::end);
	const std::streamsize FileSize = File.tellg();
	File.seekg(0, std::ios::beg);
	if (FileSize <= 0)
	{
		LOG.Add(std::string("Can't get file size: ") + FilePath + " in function LoadRUGFile.");
		return false;
	}

	char* Buffer = new char[4];
	long long ArraySize = 0;

	// Version of FEMesh file type
	File.read(Buffer, 4);
	const float Version = *(float*)Buffer;
	if (Version > APP_VERSION && abs(Version - APP_VERSION) > 0.0001)
	{
		LOG.Add(std::string("Can't load file: ") + FilePath + " in function LoadRUGFile. File was created in different Version of application!");
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
	std::vector<DataLayer*> Layers;
	Layers.resize(LayerCount);

	for (size_t i = 0; i < Layers.size(); i++)
	{
		Layers[i] = new DataLayer();

		if (Version >= 0.55)
		{
			File.read(Buffer, 4);
			const int LayerType = *(int*)Buffer;
			Layers[i]->SetType(LAYER_TYPE(LayerType));
		}

		if (Version >= 0.62)
		{
			Layers[i]->ForceID(FILE_SYSTEM.ReadFEString(File));
		}

		Layers[i]->SetCaption(FILE_SYSTEM.ReadFEString(File));
		Layers[i]->SetNote(FILE_SYSTEM.ReadFEString(File));

		// ElementsToData
		File.read(Buffer, 4);
		const int ElementsToDataCout = *(int*)Buffer;
		std::vector<float> TrianglesData;
		Layers[i]->ElementsToData.resize(ElementsToDataCout);
		File.read((char*)Layers[i]->ElementsToData.data(), ElementsToDataCout * 4);

		// Debug info.
		File.read(Buffer, 4);
		const int DebugInfoPresent = *(int*)Buffer;
		if (DebugInfoPresent)
		{
			Layers[i]->DebugInfo = new DataLayerDebugInfo();
			Layers[i]->DebugInfo->FromFile(File);
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
	Result->AnalysisData = ExtractAdditionalGeometryData(FEVertices, FEColors, FEUVs, FETangents, FEIndices, FENormals);

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;

	for (size_t i = 0; i < Layers.size(); i++)
		Result->AddLayer(Layers[i]);

	return Result;
}

void AnalysisObjectManager::OnAnalysisObjectLoad(AnalysisObject* NewObject)
{
	if (NewObject == nullptr)
		return;
	
	InitializeSceneObjects(NewObject);
	AnalysisObjects[NewObject->ID] = NewObject;
	SetActiveAnalysisObject(NewObject->ID);

	for (size_t i = 0; i < NewObject->Layers.size(); i++)
		NewObject->Layers[i]->ComputeStatistics();

	for (size_t i = 0; i < ClientOnLoadCallbacks.size(); i++)
	{
		if (ClientOnLoadCallbacks[i] == nullptr)
			continue;

		ClientOnLoadCallbacks[i](NewObject);
	}
}

void AnalysisObjectManager::LoadResource(std::string FilePath)
{
	AnalysisObject* LoadedResource = nullptr;
	if (!FILE_SYSTEM.DoesFileExist(FilePath.c_str()))
		return;

	std::string FileExtension = FILE_SYSTEM.GetFileExtension(FilePath.c_str());
	// Convert to lower case.
	std::transform(FileExtension.begin(), FileExtension.end(), FileExtension.begin(), [](const unsigned char Character) {
		return std::tolower(Character);
	});

	if (FileExtension == ".obj")
	{
		LoadedResource = ImportOBJ(FilePath.c_str(), true);
	}
	else if (FileExtension == ".rug")
	{
		float Version = CheckRUGFileVersion(FilePath);
		if (Version >= 0.91f)
		{
			NewLoadRUGFile(FilePath);
			return;
		}
		else
		{
			LoadedResource = LoadRUGFile(FilePath);
			if (LoadedResource == nullptr)
				return;
			LoadedResource->Name = FILE_SYSTEM.GetFileName(FilePath, false);
		}
	}
	else if (FileExtension == ".ply")
	{
		FEObject* LoadedObject = RESOURCE_MANAGER.ImportPLYFile(FilePath);
		if (LoadedObject == nullptr)
			return;

		if (LoadedObject->GetType() == FE_POINT_CLOUD)
		{
			LoadedResource = new AnalysisObject();
			LoadedResource->Type = DATA_SOURCE_TYPE::POINT_CLOUD;
			LoadedResource->FilePath = FilePath;
			LoadedResource->Name = FILE_SYSTEM.GetFileName(FilePath, false);
			LoadedResource->EngineResource = LoadedObject;
			LoadedResource->AnalysisData = ExtractAdditionalGeometryData(static_cast<FEPointCloud*>(LoadedObject));
		}
	}
	else if (FileExtension == ".las" || FileExtension == ".laz")
	{
		FEPointCloud* PointCloud = RESOURCE_MANAGER.ImportPointCloud(FilePath);
		if (PointCloud == nullptr)
			return;

		if (PointCloud != nullptr)
		{
			LoadedResource = new AnalysisObject();
			LoadedResource->Type = DATA_SOURCE_TYPE::POINT_CLOUD;
			LoadedResource->FilePath = FilePath;
			LoadedResource->Name = FILE_SYSTEM.GetFileName(FilePath, false);
			LoadedResource->EngineResource = PointCloud;
			LoadedResource->AnalysisData = ExtractAdditionalGeometryData(PointCloud);
		}
	}

	OnAnalysisObjectLoad(LoadedResource);
}

void AnalysisObjectManager::AddOnLoadCallback(std::function<void(AnalysisObject*)> Callback)
{
	ClientOnLoadCallbacks.push_back(Callback);
}

void AnalysisObjectManager::ComplexityMetricDataToGPU(std::string LayerID, int GPULayerIndex)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	DataLayer* CurrentLayer = ActiveObject->GetLayer(LayerID);
	if (CurrentLayer == nullptr)
		return;

	if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		CurrentLayer->FillRawData();

		FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

		if (GPULayerIndex == 0)
		{
			CurrentMeshAnalysisData->FirstLayerBufferID = 0;
			FE_GL_ERROR(glGenBuffers(1, &CurrentMeshAnalysisData->FirstLayerBufferID));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshAnalysisData->FirstLayerBufferID));
			FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * CurrentLayer->RawData.size(), CurrentLayer->RawData.data(), GL_STATIC_DRAW));
			FE_GL_ERROR(glVertexAttribPointer(7, 3, GL_FLOAT, false, 0, nullptr));
		}
		else
		{
			CurrentMeshAnalysisData->SecondLayerBufferID = 0;
			FE_GL_ERROR(glGenBuffers(1, &CurrentMeshAnalysisData->SecondLayerBufferID));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshAnalysisData->SecondLayerBufferID));
			FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * CurrentLayer->RawData.size(), CurrentLayer->RawData.data(), GL_STATIC_DRAW));
			FE_GL_ERROR(glVertexAttribPointer(8, 3, GL_FLOAT, false, 0, nullptr));
		}

		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
	else if (ActiveObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
		if (CurrentPointCloudAnalysisData == nullptr)
			return;

		for (size_t i = 0; i < CurrentPointCloudAnalysisData->RawPointCloudData.size(); i++)
		{
			float NormalizedValue = (CurrentLayer->ElementsToData[i] - CurrentLayer->GetMin()) / (CurrentLayer->GetMax() - CurrentLayer->GetMin());
			glm::vec3 NewColor = GetTurboColorMap(NormalizedValue);
			CurrentPointCloudAnalysisData->RawPointCloudData[i].R = static_cast<unsigned char>(NewColor.x * 255.0f);
			CurrentPointCloudAnalysisData->RawPointCloudData[i].G = static_cast<unsigned char>(NewColor.y * 255.0f);
			CurrentPointCloudAnalysisData->RawPointCloudData[i].B = static_cast<unsigned char>(NewColor.z * 255.0f);
		}

		FEPointCloud* PointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(CurrentPointCloudAnalysisData->RawPointCloudData);

		FEEntity* PointCloudEntity = ActiveObject->GetEntity();
		FEPointCloud* OldPointCloud = static_cast<FEPointCloud*>(ActiveObject->GetEngineResource());
		if (PointCloudEntity != nullptr)
		{
			PointCloudEntity->RemoveComponent<FEPointCloudComponent>();
			RESOURCE_MANAGER.DeleteFEPointCloud(OldPointCloud);
			ActiveObject->EngineResource = PointCloud;
			PointCloudEntity->AddComponent<FEPointCloudComponent>(PointCloud);
		}
	}
}

bool AnalysisObjectManager::SelectTriangle(glm::dvec3 MouseRay)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return false;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return false;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
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
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return glm::vec3(0.0f);

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return glm::vec3(0.0f);

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return glm::vec3(0.0f);

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
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

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return Result;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return Result;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
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
void AnalysisObjectManager::UpdateMeshUniforms(AnalysisObject* Object)
{
	if (Object == nullptr)
		return;

	if (Object->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		FEMesh* ActiveMesh = static_cast<FEMesh*>(Object->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		DataLayer* ActiveLayer = Object->GetActiveLayer();

		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("AmbientFactor", UI.GetAmbientLightFactor());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("HaveColor", ActiveMesh->GetColorCount() == 0 ? 0 : 1);
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("HeatMapType", CurrentMeshAnalysisData->GetHeatMapType());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerIndex", Object->GetActiveLayerIndex());

		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaSaturationFactor", CurrentMeshAnalysisData->GetUnselectedAreaSaturationFactor());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaBrightnessFactor", CurrentMeshAnalysisData->GetUnselectedAreaBrightnessFactor());

		if (ActiveLayer != nullptr)
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", ActiveLayer->GetSelectedRangeMin());
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", ActiveLayer->GetSelectedRangeMax());
		}
		else
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", 0.0f);
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", 0.0f);
		}

		if (ActiveLayer != nullptr)
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerMin", float(ActiveLayer->MinVisible));
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerMax", float(ActiveLayer->MaxVisible));

			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMin", float(ActiveLayer->GetMin()));
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMax", float(ActiveLayer->GetMax()));
		}

		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", -1.0f);
		if (ActiveObject != nullptr)
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

size_t AnalysisObjectManager::GetAnalysisObjectCount()
{
	return AnalysisObjects.size();
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
	if (ID == ActiveAnalysisObjectID)
		return true;

	if (ID.empty())
	{
		ActiveAnalysisObjectID = "";
		for (size_t i = 0; i < ClientOnActiveObjectChangeCallbacks.size(); i++)
		{
			if (ClientOnActiveObjectChangeCallbacks[i] == nullptr)
				continue;
			ClientOnActiveObjectChangeCallbacks[i](nullptr);
		}

		return true;
	}

	if (AnalysisObjects.find(ID) != AnalysisObjects.end())
	{
		ActiveAnalysisObjectID = ID;
		AnalysisObject* NewActiveObject = GetActiveAnalysisObject();
		DataLayer* ActiveLayer = NewActiveObject->GetActiveLayer();
		if (ActiveLayer != nullptr)
			NewActiveObject->SetActiveLayer(ActiveLayer->GetID(), true);

		for (size_t i = 0; i < ClientOnActiveObjectChangeCallbacks.size(); i++)
		{
			if (ClientOnActiveObjectChangeCallbacks[i] == nullptr)
				continue;
			ClientOnActiveObjectChangeCallbacks[i](GetActiveAnalysisObject());
		}

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
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return nullptr;

	return ActiveObject->GetEntity();
}

bool AnalysisObjectManager::DeleteAnalysisObject(std::string ID)
{
	AnalysisObject* ObjectToDelete = GetAnalysisObject(ID);
	if (ObjectToDelete == nullptr)
		return false;

	for (size_t i = 0; i < ClientOnObjectDeleteCallbacks.size(); i++)
	{
		if (ClientOnObjectDeleteCallbacks[i] == nullptr)
			continue;

		ClientOnObjectDeleteCallbacks[i](ObjectToDelete);
	}

	if (GetActiveAnalysisObject() == ObjectToDelete)
		SetActiveAnalysisObject("");

	FEEntity* EntityToDelete = ObjectToDelete->GetEntity();
	if (EntityToDelete != nullptr)
	{
		RENDERER.RemoveBeforeRenderCallback(ObjectToDelete->Entity, AnalysisObjectManager::BeforeRender);
		MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(EntityToDelete);
	}

	if (ObjectToDelete->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		FEMesh* MeshToDelete = static_cast<FEMesh*>(ObjectToDelete->GetEngineResource());
		if (MeshToDelete != nullptr)
			RESOURCE_MANAGER.DeleteFEMesh(MeshToDelete);
	}
	else if (ObjectToDelete->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		FEPointCloud* PointCloudToDelete = static_cast<FEPointCloud*>(ObjectToDelete->GetEngineResource());
		if (PointCloudToDelete != nullptr)
			RESOURCE_MANAGER.DeleteFEPointCloud(PointCloudToDelete);
	}

	delete ObjectToDelete;
	AnalysisObjects.erase(ID);
	return true;
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

	RENDERER.AddBeforeRenderCallback(NewAnalysisObject->Entity, AnalysisObjectManager::BeforeRender);
}

void AnalysisObjectManager::SaveToRUGFileAskForFilePath()
{
	std::string FilePath;
	FILE_SYSTEM.ShowFileSaveDialog(FilePath, RUGOSITY_SAVE_FILE_FILTER, 1);

	SaveToRUGFile(FilePath);
}

void AnalysisObjectManager::SaveAnalysisDataToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	ResourceAnalysisData* AnalysisData = Object->GetAnalysisData();
	if (AnalysisData == nullptr)
		return;

	switch (Object->GetType())
	{
		case DATA_SOURCE_TYPE::MESH:
			SaveMeshDataToRUGFile(File, Object);
		break;

		case DATA_SOURCE_TYPE::POINT_CLOUD:
			SavePointCloudToRUGFile(File, Object);
		break;
	default:
		break;
	}
}

void AnalysisObjectManager::SaveMeshDataToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	int DebugWrittenBytes = 0;

	int Count = static_cast<int>(CurrentMeshAnalysisData->Vertices.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Vertices.data(), sizeof(double) * Count);
	DebugWrittenBytes += sizeof(double) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->Colors.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Colors.data(), sizeof(float) * Count);
	DebugWrittenBytes += sizeof(float) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->UVs.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->UVs.data(), sizeof(float) * Count);
	DebugWrittenBytes += sizeof(float) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->Normals.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Normals.data(), sizeof(float) * Count);
	DebugWrittenBytes += sizeof(float) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->Tangents.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Tangents.data(), sizeof(float) * Count);
	DebugWrittenBytes += sizeof(float) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->Indices.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Indices.data(), sizeof(int) * Count);
	DebugWrittenBytes += sizeof(int) * Count;

	FEAABB CurrentAABB = CurrentMeshAnalysisData->GetAABB();
	File.write((char*)&CurrentAABB.GetMin()[0], sizeof(float));
	File.write((char*)&CurrentAABB.GetMin()[1], sizeof(float));
	File.write((char*)&CurrentAABB.GetMin()[2], sizeof(float));

	File.write((char*)&CurrentAABB.GetMax()[0], sizeof(float));
	File.write((char*)&CurrentAABB.GetMax()[1], sizeof(float));
	File.write((char*)&CurrentAABB.GetMax()[2], sizeof(float));

	DebugWrittenBytes += sizeof(float) * 6;
}

void AnalysisObjectManager::SavePointCloudToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	FEPointCloud* PointCloud = static_cast<FEPointCloud*>(Object->GetEngineResource());
	if (PointCloud == nullptr)
		return;

	std::vector<FEPointCloudVertex> RawData = PointCloud->GetRawData();

	size_t Count = PointCloud->GetPointCount() * 3;
	float* Positions = new float[Count];
	for (size_t i = 0; i < PointCloud->GetPointCount(); i++)
	{
		Positions[i * 3] = RawData[i].X;
		Positions[i * 3 + 1] = RawData[i].Y;
		Positions[i * 3 + 2] = RawData[i].Z;
	}

	File.write((char*)&Count, sizeof(size_t));
	File.write((char*)Positions, sizeof(float) * Count);

	Count = PointCloud->GetPointCount() * 4;
	unsigned char* Colors = new unsigned char[PointCloud->GetPointCount() * 4];
	for (size_t i = 0; i < PointCloud->GetPointCount(); i++)
	{
		Colors[i * 4] = RawData[i].R;
		Colors[i * 4 + 1] = RawData[i].G;
		Colors[i * 4 + 2] = RawData[i].B;
		Colors[i * 4 + 3] = RawData[i].A;
	}

	File.write((char*)&Count, sizeof(size_t));
	File.write((char*)Colors, sizeof(unsigned char) * Count);

	delete[] Positions;
	delete[] Colors;
}

void AnalysisObjectManager::SaveLayersDataToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	int Count = static_cast<int>(Object->Layers.size());
	File.write((char*)&Count, sizeof(int));
	for (size_t i = 0; i < Object->Layers.size(); i++)
	{
		DataLayer* CurrentLayer = Object->Layers[i];
		LAYER_TYPE LayerType = CurrentLayer->GetType();
		File.write((char*)&LayerType, sizeof(LAYER_TYPE));

		int LayerIDSize = static_cast<int>(CurrentLayer->GetID().size() + 1);
		File.write((char*)&LayerIDSize, sizeof(int));
		File.write((char*)CurrentLayer->GetID().c_str(), sizeof(char) * LayerIDSize);

		int ParentIDSize = (int)CurrentLayer->ParentObjectIDs.size();
		File.write((char*)&ParentIDSize, sizeof(int));
		for (size_t j = 0; j < CurrentLayer->ParentObjectIDs.size(); j++)
		{
			int SingleParentIDSize = static_cast<int>(CurrentLayer->ParentObjectIDs[j].size() + 1);
			File.write((char*)&SingleParentIDSize, sizeof(int));
			File.write((char*)CurrentLayer->ParentObjectIDs[j].c_str(), sizeof(char) * SingleParentIDSize);
		}

		Count = static_cast<int>(CurrentLayer->GetCaption().size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)CurrentLayer->GetCaption().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(CurrentLayer->GetNote().size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)CurrentLayer->GetNote().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(CurrentLayer->ElementsToData.size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)CurrentLayer->ElementsToData.data(), sizeof(float) * Count);

		Count = CurrentLayer->DebugInfo != nullptr;
		File.write((char*)&Count, sizeof(int));
		if (Count)
			CurrentLayer->DebugInfo->ToFile(File);
	}
}

void AnalysisObjectManager::SaveToRUGFile(std::string FilePath)
{
	if (FilePath.empty())
		return;

	if (FilePath.find(".rug") == std::string::npos)
		FilePath += ".rug";

	std::fstream File;
	File.open(FilePath, std::ios::out | std::ios::binary);

	float Version = APP_VERSION;
	File.write((char*)&Version, sizeof(float));

	size_t ObjectCount = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.size();
	File.write((char*)&ObjectCount, sizeof(size_t));

	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject != nullptr)
		{
			int ObjectIDSize = static_cast<int>(CurrentObject->GetID().size() + 1);
			File.write((char*)&ObjectIDSize, sizeof(int));
			File.write((char*)CurrentObject->GetID().c_str(), sizeof(char) * ObjectIDSize);

			int ObjectNameSize = static_cast<int>(CurrentObject->GetName().size() + 1);
			File.write((char*)&ObjectNameSize, sizeof(int));
			File.write((char*)CurrentObject->GetName().c_str(), sizeof(char) * ObjectNameSize);

			DATA_SOURCE_TYPE ObjectType = CurrentObject->GetType();
			File.write((char*)&ObjectType, sizeof(DATA_SOURCE_TYPE));

			int FilePathSize = static_cast<int>(CurrentObject->GetFilePath().size() + 1);
			File.write((char*)&FilePathSize, sizeof(int));
			File.write((char*)CurrentObject->GetFilePath().c_str(), sizeof(char) * FilePathSize);

			int RenderedInScene = CurrentObject->IsRenderedInScene();
			File.write((char*)&RenderedInScene, sizeof(int));

			SaveAnalysisDataToRUGFile(File, CurrentObject);
			SaveLayersDataToRUGFile(File, CurrentObject);
		}

		ObjectsMapIterator++;
	}

	File.close();
}

void AnalysisObjectManager::LoadMeshDataFromRUGFile(std::fstream& File, AnalysisObject* Object)
{
	char* Buffer = new char[4];
	long long ArraySize = 0;

	File.read(Buffer, 4);
	const int VertexCount = *(int*)Buffer;

	int BytesPerVertex = 8;
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

	std::vector<double> FEVertices;
	FEMesh* NewMesh = nullptr;

	std::vector<float> FEFloatVertices;
	FEFloatVertices.resize(VertexCount);
	for (size_t i = 0; i < VertexCount; i++)
		FEFloatVertices[i] = static_cast<float>(((double*)VertexBuffer)[i]);
	
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

	std::vector<float> FEColors;
	FEColors.resize(ColorCount);
	for (size_t i = 0; i < ColorCount; i++)
		FEColors[i] = ((float*)ColorBuffer)[i];
	
	std::vector<float> FEUVs;
	FEUVs.resize(TexCout);
	for (size_t i = 0; i < TexCout; i++)
		FEUVs[i] = ((float*)TexBuffer)[i];
	
	std::vector<float> FETangents;
	FETangents.resize(TangCout);
	for (size_t i = 0; i < TangCout; i++)
		FETangents[i] = ((float*)TangBuffer)[i];
	
	std::vector<int> FEIndices;
	FEIndices.resize(IndexCout);
	for (size_t i = 0; i < IndexCout; i++)
		FEIndices[i] = ((int*)IndexBuffer)[i];
	
	std::vector<float> FENormals;
	FENormals.resize(NormCout);
	for (size_t i = 0; i < NormCout; i++)
		FENormals[i] = ((float*)NormBuffer)[i];
	
	Object->EngineResource = NewMesh;
	Object->AnalysisData = ExtractAdditionalGeometryData(FEVertices, FEColors, FEUVs, FETangents, FEIndices, FENormals);

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;
	if (ColorBuffer != nullptr)
		delete[] ColorBuffer;
}

void AnalysisObjectManager::LoadPointCloudDataFromRUGFile(std::fstream& File, AnalysisObject* Object)
{
	char* Buffer_8Byte = new char[8];
	File.read(Buffer_8Byte, sizeof(size_t));
	const size_t VertexCout = *(size_t*)Buffer_8Byte;
	char* VertexBuffer = new char[VertexCout * sizeof(float)];
	File.read(VertexBuffer, VertexCout * sizeof(float));

	File.read(Buffer_8Byte, sizeof(size_t));
	const size_t ColorCout = *(size_t*)Buffer_8Byte;
	char* ColorBuffer = new char[ColorCout * sizeof(unsigned char)];
	File.read(ColorBuffer, ColorCout * sizeof(unsigned char));

	std::vector<FEPointCloudVertex> PointCloudData;
	for (size_t i = 0; i < VertexCout / 3; i++)
	{
		PointCloudData.push_back(FEPointCloudVertex());
		PointCloudData[i].X = *(float*)(VertexBuffer + i * 3 * sizeof(float));
		PointCloudData[i].Y = *(float*)(VertexBuffer + i * 3 * sizeof(float) + sizeof(float));
		PointCloudData[i].Z = *(float*)(VertexBuffer + i * 3 * sizeof(float) + sizeof(float) * 2);

		PointCloudData[i].R = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char));
		PointCloudData[i].G = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char));
		PointCloudData[i].B = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char) * 2);
		PointCloudData[i].A = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char) * 3);
	}

	FEPointCloud* NewPointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(PointCloudData, "", "", false);
	Object->EngineResource = NewPointCloud;
	Object->AnalysisData = ExtractAdditionalGeometryData(NewPointCloud);

	delete[] Buffer_8Byte;
	delete[] VertexBuffer;
	delete[] ColorBuffer;
}

void AnalysisObjectManager::LoadLayersDataFromRUGFile(std::fstream& File, AnalysisObject* Object)
{
	char* Buffer = new char[4];
	File.read(Buffer, 4);
	const int LayerCount = *(int*)Buffer;
	Object->Layers.resize(LayerCount);

	for (size_t i = 0; i < Object->Layers.size(); i++)
	{
		Object->Layers[i] = new DataLayer();

		File.read(Buffer, 4);
		const int LayerType = *(int*)Buffer;
		Object->Layers[i]->SetType(LAYER_TYPE(LayerType));

		Object->Layers[i]->ForceID(FILE_SYSTEM.ReadFEString(File));

		File.read(Buffer, 4);
		const int ParentsIDsCount = *(int*)Buffer;
		Object->Layers[i]->ParentObjectIDs.resize(ParentsIDsCount);
		for (size_t j = 0; j < ParentsIDsCount; j++)
			Object->Layers[i]->ParentObjectIDs[j] = FILE_SYSTEM.ReadFEString(File);
		
		Object->Layers[i]->SetCaption(FILE_SYSTEM.ReadFEString(File));
		Object->Layers[i]->SetNote(FILE_SYSTEM.ReadFEString(File));

		// ElementsToData
		File.read(Buffer, 4);
		const int ElementsToDataCout = *(int*)Buffer;
		std::vector<float> TrianglesData;
		Object->Layers[i]->ElementsToData.resize(ElementsToDataCout);
		File.read((char*)Object->Layers[i]->ElementsToData.data(), ElementsToDataCout * 4);

		// Debug info.
		File.read(Buffer, 4);
		const int DebugInfoPresent = *(int*)Buffer;
		if (DebugInfoPresent)
		{
			Object->Layers[i]->DebugInfo = new DataLayerDebugInfo();
			Object->Layers[i]->DebugInfo->FromFile(File);
		}
	}

	delete[] Buffer;
}

bool AnalysisObjectManager::NewLoadRUGFile(std::string FilePath)
{
	std::fstream File;
	File.open(FilePath, std::ios::in | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Can't open file: ") + FilePath + " in function NewLoadRUGFile.");
		return false;
	}

	File.seekg(0, std::ios::end);
	const std::streamsize FileSize = File.tellg();
	File.seekg(0, std::ios::beg);
	if (FileSize <= 0)
	{
		LOG.Add(std::string("Can't get file size: ") + FilePath + " in function NewLoadRUGFile.");
		return false;
	}

	char* Buffer32 = new char[4];
	char* Buffer64 = new char[8];
	long long ArraySize = 0;

	File.read(Buffer32, 4);
	const float Version = *(float*)Buffer32;
	if (Version > APP_VERSION && abs(Version - APP_VERSION) > 0.0001f || APP_VERSION < 0.91f)
	{
		LOG.Add(std::string("Can't load file: ") + FilePath + " in function NewLoadRUGFile. File was created in different Version of application!");
		return false;
	}

	File.read(Buffer64, 8);
	const size_t AnalysisObjectCount = *(size_t*)Buffer64;

	for (size_t i = 0; i < AnalysisObjectCount; i++)
	{
		AnalysisObject* NewAnalysisObject = new AnalysisObject();

		File.read(Buffer32, 4);
		const int ObjectIDSize = *(int*)Buffer32;
		char* ObjectIDBuffer = new char[ObjectIDSize];
		File.read(ObjectIDBuffer, ObjectIDSize);
		const std::string ObjectID = std::string(ObjectIDBuffer);

		// FIX ME: It is not good solution, it would not delete all previously loaded objects.
		// Better solution would to have header in the file with all object IDs and check it before loading.
		if (AnalysisObjects.find(ObjectID) != AnalysisObjects.end())
		{
			LOG.Add(std::string("Can't load file: ") + FilePath + " in function NewLoadRUGFile. Object with ID " + ObjectID + " already exists in the scene!");
			delete NewAnalysisObject;
			delete[] ObjectIDBuffer;
			delete[] Buffer32;
			delete[] Buffer64;
			File.close();
			return false;
		}
		NewAnalysisObject->ID = ObjectID;
		delete[] ObjectIDBuffer;

		File.read(Buffer32, 4);
		const int ObjectNameSize = *(int*)Buffer32;
		char* ObjectNameBuffer = new char[ObjectNameSize];
		File.read(ObjectNameBuffer, ObjectNameSize);
		const std::string ObjectName = std::string(ObjectNameBuffer);
		NewAnalysisObject->Name = ObjectName;
		delete[] ObjectNameBuffer;

		File.read(Buffer32, 4);
		const DATA_SOURCE_TYPE ObjectType = *(DATA_SOURCE_TYPE*)Buffer32;
		NewAnalysisObject->Type = ObjectType;
		File.read(Buffer32, 4);

		const int FilePathSize = *(int*)Buffer32;
		char* FilePathBuffer = new char[FilePathSize];
		File.read(FilePathBuffer, FilePathSize);
		const std::string ObjectFilePath = std::string(FilePathBuffer);
		NewAnalysisObject->FilePath = ObjectFilePath;
		delete[] FilePathBuffer;

		File.read(Buffer32, 4);
		const int RenderedInScene = *(int*)Buffer32;
		NewAnalysisObject->SetRenderInScene(RenderedInScene);

		switch (ObjectType)
		{
			case DATA_SOURCE_TYPE::MESH:
			{
				LoadMeshDataFromRUGFile(File, NewAnalysisObject);
				break;
			}

			case DATA_SOURCE_TYPE::POINT_CLOUD:
			{
				LoadPointCloudDataFromRUGFile(File, NewAnalysisObject);
				break;
			}
			
			default:
			{
				LOG.Add(std::string("Can't load file: ") + FilePath + " in function NewLoadRUGFile. Unknown data source type!");
				delete NewAnalysisObject;
				continue;
				break;
			}
		}

		LoadLayersDataFromRUGFile(File, NewAnalysisObject);
		AnalysisObjects[NewAnalysisObject->GetID()] = NewAnalysisObject;
		OnAnalysisObjectLoad(NewAnalysisObject);
	}

	delete[] Buffer32;
	delete[] Buffer64;
	File.close();

	return true;
}

void AnalysisObjectManager::BeforeRender(FEEntity* CurrentEntity)
{
	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject->GetEntity() == nullptr)
		{
			ObjectsMapIterator++;
			continue;
		}

		CurrentObject->GetEntity()->SetComponentVisible(ComponentVisibilityType::ALL, CurrentObject->IsRenderedInScene());
		if (!CurrentObject->IsRenderedInScene())
		{
			ObjectsMapIterator++;
			continue;
		}

		if (CurrentObject != nullptr &&
			CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH &&
			CurrentObject->GetEntity() == CurrentEntity)
		{
			FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
			if (ActiveMesh != nullptr)
			{
				if (UI.GetWireFrameMode())
				{
					CurrentEntity->GetComponent<FEGameModelComponent>().SetWireframeMode(true);
				}
				else
				{
					CurrentEntity->GetComponent<FEGameModelComponent>().SetWireframeMode(false);
				}

				ANALYSIS_OBJECT_MANAGER.UpdateMeshUniforms(CurrentObject);

				FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

				if (ActiveMesh->GetColorCount() > 0) FE_GL_ERROR(glEnableVertexAttribArray(1));
				MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
				if (CurrentMeshAnalysisData->GetFirstLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(7));
				if (CurrentMeshAnalysisData->GetSecondLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(8));
			}
		}

		ObjectsMapIterator++;
	}
}

void AnalysisObjectManager::AddOnActiveObjectChangeCallback(std::function<void(AnalysisObject*)> Callback)
{
	ClientOnActiveObjectChangeCallbacks.push_back(Callback);
}

FEAABB AnalysisObjectManager::GetAllObjectsAABB()
{
	FEAABB Result;
	bool bFirst = true;
	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject != nullptr)
		{
			if (bFirst)
			{
				Result = CurrentObject->AnalysisData->GetAABB();
				bFirst = false;
			}
			else
			{
				Result = Result.Merge(CurrentObject->AnalysisData->GetAABB());
			}
		}

		ObjectsMapIterator++;
	}

	return Result;
}

double AnalysisObjectManager::GetAllMeshObjectsTotalArea()
{
	double Result = 0.0;
	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject != nullptr && CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData != nullptr)
				Result += CurrentMeshAnalysisData->GetTotalArea();
		}

		ObjectsMapIterator++;
	}

	return Result;
}

glm::vec3 AnalysisObjectManager::GetAllMeshObjectsAverageNormal()
{
	glm::vec3 Result = glm::vec3(0.0f);
	double TotalArea = GetAllMeshObjectsTotalArea();
	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject != nullptr && CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData != nullptr)
			{
				glm::vec3 CurrentNormal = CurrentMeshAnalysisData->GetAverageNormal();
				double AreaFactor = CurrentMeshAnalysisData->GetTotalArea() / TotalArea;
				Result += CurrentNormal * float(AreaFactor);
			}
		}
		ObjectsMapIterator++;
	}

	return Result;
}

void AnalysisObjectManager::AddOnObjectDeleteCallback(std::function<void(AnalysisObject*)> Callback)
{
	ClientOnObjectDeleteCallbacks.push_back(Callback);
}