#include "RugosityManager.h"
using namespace FocalEngine;

RugosityManager* RugosityManager::Instance = nullptr;

RugosityManager::RugosityManager()
{
	dimentionsList.push_back("4");
	dimentionsList.push_back("8");
	dimentionsList.push_back("16");
	dimentionsList.push_back("32");
	dimentionsList.push_back("64");
	dimentionsList.push_back("128");
	dimentionsList.push_back("256");
	dimentionsList.push_back("512");
	dimentionsList.push_back("1024");
	dimentionsList.push_back("2048");
	dimentionsList.push_back("4096");

	colorSchemesList.push_back("Default");
	colorSchemesList.push_back("Rainbow");
	colorSchemesList.push_back("Turbo colormap");
}

RugosityManager::~RugosityManager() {}

void RugosityManager::CheckAcceptableResolutions(FEMesh* NewMesh)
{
	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	transformMatrix = glm::scale(transformMatrix, glm::vec3(DEFAULT_GRID_SIZE + GRID_VARIANCE / 100.0f));
	FEAABB finalAABB = NewMesh->AABB.transform(transformMatrix);

	float MaxMeshAABBSize = finalAABB.getSize();

	LowestResolution = MaxMeshAABBSize / 120;
	HigestResolution = MaxMeshAABBSize / 9;

	ResolutonInM = LowestResolution;
}

void RugosityManager::MoveRugosityInfoToMesh(SDF* SDF, bool bFinalJitter)
{
	if (SDF == nullptr || SDF->TrianglesRugosity.empty())
		return;

	if (SDF->mesh->TrianglesRugosity.size() != SDF->TrianglesRugosity.size())
		SDF->mesh->TrianglesRugosity.resize(SDF->TrianglesRugosity.size());

	for (size_t i = 0; i < SDF->mesh->TrianglesRugosity.size(); i++)
	{
		SDF->mesh->TrianglesRugosity[i] += SDF->TrianglesRugosity[i];
		if (SDF->mesh->TrianglesRugosity[i] <= 0.0f)
		{
			SDF->mesh->TrianglesRugosity[i] += 0.000000001f;
			bWeightedNormals = true;
		}
	}
	JitterCounter++;

	if (bFinalJitter)
	{
		for (size_t i = 0; i < SDF->mesh->TrianglesRugosity.size(); i++)
		{
			SDF->mesh->TrianglesRugosity[i] /= JitterCounter;
		}

		SDF->mesh->fillRugosityDataToGPU(RUGOSITY_MANAGER.RugosityLayerIndex);
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
	result->bFindSmallestRugosity = bUseFindSmallestRugosity;
	result->bCGALVariant = bUseCGALVariant;
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

	Output->Init(Input->mesh, 0/*Input->dimentions*/, finalAABB, RUGOSITY_MANAGER.currentCamera, RUGOSITY_MANAGER.ResolutonInM);
	//currentSDF = new SDF(mesh, dimentions, finalAABB, currentCamera);
	Output->bFindSmallestRugosity = RUGOSITY_MANAGER.bUseFindSmallestRugosity;
	Output->bCGALVariant = RUGOSITY_MANAGER.bUseCGALVariant;
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
	OutputData->bFindSmallestRugosity = RUGOSITY_MANAGER.bUseFindSmallestRugosity;
	OutputData->bCGALVariant = RUGOSITY_MANAGER.bUseCGALVariant;
	currentSDF = OutputData;

	if (mesh->Triangles.empty())
		mesh->fillTrianglesData();

	THREAD_POOL.Execute(calculateSDFAsync, InputData, OutputData, calculateSDFCallback);
}

void RugosityManager::calculateRugorsityWithJitterAsyn(FEMesh* mesh, int RugosityLayerIndex)
{
	RUGOSITY_MANAGER.JitterCounter = 0;
	RUGOSITY_MANAGER.RugosityLayerIndex = RugosityLayerIndex;
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

		RUGOSITY_MANAGER.GridScale = DEFAULT_GRID_SIZE;
		float TempGridScale = rand() % GRID_VARIANCE;
		TempGridScale /= 100.0f;
		RUGOSITY_MANAGER.GridScale += TempGridScale;

		/*bool bFinal = false;
		if (i == SmoothingFactor - 2)
			bFinal = true;

		calculateSDF(currentMesh, SDFDimention, true);
		MoveRugosityInfoToMesh(currentSDF, bFinal);*/

		RunCreationOfSDFAsync(mesh, true);
	}
}

std::string RugosityManager::colorSchemeIndexToString(int index)
{
	switch (index)
	{
	case 3:
		return colorSchemesList[0];
	case 4:
		return colorSchemesList[1];
	case 5:
		return colorSchemesList[2];
	}

	return "Default";
}

int RugosityManager::colorSchemeIndexFromString(std::string name)
{
	if (name == colorSchemesList[0])
		return 3;

	if (name == colorSchemesList[1])
		return 4;

	if (name == colorSchemesList[2])
		return 5;

	return 3;
}

bool RugosityManager::GetUseFindSmallestRugosity()
{
	return bUseFindSmallestRugosity;
}

void RugosityManager::SetUseFindSmallestRugosity(bool NewValue)
{
	bUseFindSmallestRugosity = NewValue;
}

bool RugosityManager::GetUseCGALVariant()
{
	return bUseCGALVariant;
}

void RugosityManager::SetUseCGALVariant(bool NewValue)
{
	bUseCGALVariant = NewValue;
}

glm::vec2 RugosityManager::RugosityAreaDistribution(float RugosityValue)
{
	//if (curr)
	return glm::vec2();
}