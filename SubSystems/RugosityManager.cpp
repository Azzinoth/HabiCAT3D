#include "RugosityManager.h"
using namespace FocalEngine;

RugosityManager* RugosityManager::Instance = nullptr;

RugosityManager::RugosityManager() {}
RugosityManager::~RugosityManager() {}

void RugosityManager::MoveRugosityInfoToMesh(SDF* SDF, bool bFinalJitter)
{
	if (SDF == nullptr || SDF->TrianglesRugosity.empty())
		return;

	if (SDF->mesh->TrianglesRugosity.size() != SDF->TrianglesRugosity.size())
		SDF->mesh->TrianglesRugosity.resize(SDF->TrianglesRugosity.size());

	for (size_t i = 0; i < SDF->mesh->TrianglesRugosity.size(); i++)
	{
		SDF->mesh->TrianglesRugosity[i] += SDF->TrianglesRugosity[i];
	}
	JitterCounter++;

	if (bFinalJitter)
	{
		for (size_t i = 0; i < SDF->mesh->TrianglesRugosity.size(); i++)
		{
			SDF->mesh->TrianglesRugosity[i] /= JitterCounter;
		}

		SDF->mesh->fillRugosityDataToGPU();
	}
}

SDF* RugosityManager::calculateSDF(FEMesh* mesh, int dimentions, FEFreeCamera* currentCamera, bool UseJitterExpandedAABB)
{
	FEAABB finalAABB = mesh->AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	if (UseJitterExpandedAABB)
	{
		transformMatrix = glm::scale(transformMatrix, glm::vec3(GridScale));
		finalAABB = finalAABB.transform(transformMatrix);
	}

	float cellSize = finalAABB.getSize() / dimentions;

	//glm::vec3 center = mesh->AABB.getCenter();
	glm::vec3 center = mesh->AABB.getCenter() + glm::vec3(shiftX, shiftY, shiftZ) * cellSize;
	FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;


	//transformMatrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(finalAABB.getSize() / 2.0f));
	//finalAABB = finalAABB.transform(transformMatrix);

	/*if (UseJitterExpandedAABB)
	{
		transformMatrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(cellSize * shiftX, cellSize * shiftY, cellSize * shiftZ));
		finalAABB = finalAABB.transform(transformMatrix);
	}*/

	SDF* result = new SDF(mesh, dimentions, finalAABB, currentCamera);
	result->bWeightedNormals = bWeightedNormals;
	result->bNormalizedNormals = bNormalizedNormals;

	result->fillCellsWithTriangleInfo();
	result->calculateRugosity();

	result->fillMeshWithRugosityData();

	result->bFullyLoaded = true;

	return result;
}

void RugosityManager::calculateSDFCallback(void* OutputData)
{
	SDF* Input = reinterpret_cast<SDF*>(OutputData);

	RUGOSITY_MANAGER.currentSDF = Input;

	RUGOSITY_MANAGER.newSDFSeen++;
	if (RUGOSITY_MANAGER.newSDFSeen == RUGOSITY_MANAGER.SmoothingFactor)
		RUGOSITY_MANAGER.bLastJitter = true;

	RUGOSITY_MANAGER.MoveRugosityInfoToMesh(RUGOSITY_MANAGER.currentSDF, RUGOSITY_MANAGER.bLastJitter);

	if (!RUGOSITY_MANAGER.bLastJitter)
	{
		delete RUGOSITY_MANAGER.currentSDF;
		RUGOSITY_MANAGER.currentSDF = nullptr;
	}
}

void RugosityManager::calculateSDFAsync(void* InputData, void* OutputData)
{
	SDFInitData* Input = reinterpret_cast<SDFInitData*>(InputData);
	SDF* Output = reinterpret_cast<SDF*>(OutputData);

	FEAABB finalAABB = Input->mesh->AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	if (Input->UseJitterExpandedAABB)
	{
		transformMatrix = glm::scale(transformMatrix, glm::vec3(Input->GridScale));
		finalAABB = finalAABB.transform(transformMatrix);
	}

	float cellSize = finalAABB.getSize() / Input->dimentions;

	//glm::vec3 center = mesh->AABB.getCenter();
	glm::vec3 center = Input->mesh->AABB.getCenter() + glm::vec3(Input->shiftX, Input->shiftY, Input->shiftZ) * cellSize;
	FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;

	Output->Init(Input->mesh, Input->dimentions, finalAABB, RUGOSITY_MANAGER.currentCamera);
	//currentSDF = new SDF(mesh, dimentions, finalAABB, currentCamera);
	Output->bWeightedNormals = RUGOSITY_MANAGER.bWeightedNormals;
	Output->bNormalizedNormals = RUGOSITY_MANAGER.bNormalizedNormals;

	Output->fillCellsWithTriangleInfo();
	Output->calculateRugosity();

	Output->fillMeshWithRugosityData();

	Output->bFullyLoaded = true;
}

void RugosityManager::RunCreationOfSDFAsync(FEMesh* mesh, bool bJitter)
{
	SDFInitData* InputData = new SDFInitData();
	InputData->dimentions = SDFDimention;
	InputData->mesh = mesh;
	InputData->UseJitterExpandedAABB = bJitter;

	InputData->shiftX = RUGOSITY_MANAGER.shiftX;
	InputData->shiftY = RUGOSITY_MANAGER.shiftY;
	InputData->shiftZ = RUGOSITY_MANAGER.shiftZ;
	InputData->GridScale = RUGOSITY_MANAGER.GridScale;

	SDF* OutputData = new SDF();
	currentSDF = OutputData;

	if (mesh->Triangles.empty())
		mesh->fillTrianglesData();

	THREAD_POOL.Execute(calculateSDFAsync, InputData, OutputData, calculateSDFCallback);
}

void RugosityManager::calculateRugorsityWithJitterAsyn(FEMesh* mesh)
{
	RUGOSITY_MANAGER.JitterCounter = 0;
	newSDFSeen = 0;
	mesh->TrianglesRugosity.clear();
	mesh->rugosityData.clear();

	bLastJitter = false;
	RunCreationOfSDFAsync(mesh, true);
	//calculateSDF(currentMesh, SDFDimention, true);
	//MoveRugosityInfoToMesh(currentSDF, false);

	// In cells
	float kernelSize = 0.5;
	kernelSize *= 2.0f;
	kernelSize *= 100.0f;

	for (size_t i = 0; i < SmoothingFactor - 1; i++)
	{
		//delete currentSDF;
		//currentSDF = nullptr;

		RUGOSITY_MANAGER.shiftX = rand() % int(kernelSize);
		RUGOSITY_MANAGER.shiftX -= kernelSize / 2.0f;
		RUGOSITY_MANAGER.shiftX /= 100.0f;

		RUGOSITY_MANAGER.shiftY = rand() % int(kernelSize);
		RUGOSITY_MANAGER.shiftY -= kernelSize / 2.0f;
		RUGOSITY_MANAGER.shiftY /= 100.0f;

		RUGOSITY_MANAGER.shiftZ = rand() % int(kernelSize);
		RUGOSITY_MANAGER.shiftZ -= kernelSize / 2.0f;
		RUGOSITY_MANAGER.shiftZ /= 100.0f;

		/*GridScale = 2.5f;
		float TempGridScale = rand() % 100;
		TempGridScale -= 50;
		TempGridScale /= 100.0f;
		GridScale += TempGridScale;*/

		RUGOSITY_MANAGER.GridScale = 1.0f;
		float TempGridScale = rand() % 200;
		TempGridScale /= 100.0f;
		RUGOSITY_MANAGER.GridScale += TempGridScale;

		/*bool bFinal = false;
		if (i == SmoothingFactor - 2)
			bFinal = true;

		calculateSDF(currentMesh, SDFDimention, true);
		MoveRugosityInfoToMesh(currentSDF, bFinal);*/

		RunCreationOfSDFAsync(mesh);
	}
}