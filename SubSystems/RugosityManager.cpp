#include "RugosityManager.h"
using namespace FocalEngine;

RugosityManager* RugosityManager::Instance = nullptr;
float RugosityManager::LastTimeTookForCalculation = 0.0f;
void(*RugosityManager::OnRugosityCalculationsStartCallbackImpl)(void) = nullptr;
void(*RugosityManager::OnRugosityCalculationsEndCallbackImpl)(MeshLayer) = nullptr;

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

	RugosityAlgorithmList.push_back("Average normal(default)");
	RugosityAlgorithmList.push_back("Min Rugosity");
	RugosityAlgorithmList.push_back("Least square fitting");

	MESH_MANAGER.AddLoadCallback(RugosityManager::OnMeshUpdate);
}

RugosityManager::~RugosityManager() {}

void RugosityManager::OnMeshUpdate()
{
	glm::mat4 TransformMatrix = glm::identity<glm::mat4>();
	TransformMatrix = glm::scale(TransformMatrix, glm::vec3(DEFAULT_GRID_SIZE + GRID_VARIANCE / 100.0f));
	FEAABB FinalAABB = MESH_MANAGER.ActiveMesh->AABB.transform(TransformMatrix);

	const float MaxMeshAABBSize = FinalAABB.getSize();

	RUGOSITY_MANAGER.LowestResolution = MaxMeshAABBSize / 120;
	RUGOSITY_MANAGER.HigestResolution = MaxMeshAABBSize / 9;

	RUGOSITY_MANAGER.ResolutonInM = RUGOSITY_MANAGER.LowestResolution;

	delete RUGOSITY_MANAGER.currentSDF;
	RUGOSITY_MANAGER.currentSDF = nullptr;
}

void RugosityManager::MoveRugosityInfoFromSDF(SDF* SDF)
{
	if (SDF == nullptr || SDF->TrianglesRugosity.empty())
		return;

	PerJitterResult.resize(PerJitterResult.size() + 1);

	if (Result.size() != SDF->TrianglesRugosity.size())
		Result.resize(SDF->TrianglesRugosity.size());

	for (size_t i = 0; i < Result.size(); i++)
	{
		PerJitterResult.back().push_back(SDF->TrianglesRugosity[i]);
		Result[i] += SDF->TrianglesRugosity[i];
		if (Result[i] <= 0.0f)
		{
			Result[i] += 0.000000001f;
			bWeightedNormals = true;
		}
	}

	if (RUGOSITY_MANAGER.JitterDoneCount == RUGOSITY_MANAGER.JitterToDoCount)
	{
		for (size_t i = 0; i < Result.size(); i++)
		{
			Result[i] /= JitterDoneCount;
		}
	}
}

SDF* RugosityManager::calculateSDF(int dimentions, FEBasicCamera* currentCamera, bool UseJitterExpandedAABB)
{
	OnRugosityCalculationsStart();

	FEAABB finalAABB = MESH_MANAGER.ActiveMesh->AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	if (UseJitterExpandedAABB)
	{
		transformMatrix = glm::scale(transformMatrix, glm::vec3(GridScale));
		finalAABB = finalAABB.transform(transformMatrix);
	}

	const float cellSize = finalAABB.getSize() / dimentions;

	//glm::vec3 center = mesh->AABB.getCenter();
	const glm::vec3 center = MESH_MANAGER.ActiveMesh->AABB.getCenter() + glm::vec3(shiftX, shiftY, shiftZ) * cellSize;
	const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;


	//transformMatrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(finalAABB.getSize() / 2.0f));
	//finalAABB = finalAABB.transform(transformMatrix);

	/*if (UseJitterExpandedAABB)
	{
		transformMatrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(cellSize * shiftX, cellSize * shiftY, cellSize * shiftZ));
		finalAABB = finalAABB.transform(transformMatrix);
	}*/

	SDF* result = new SDF(dimentions, finalAABB, currentCamera);
	result->bFindSmallestRugosity = bUseFindSmallestRugosity;
	result->bCGALVariant = bUseCGALVariant;
	result->bWeightedNormals = bWeightedNormals;
	result->bNormalizedNormals = bNormalizedNormals;

	result->FillCellsWithTriangleInfo();
	result->CalculateRugosity();

	result->FillMeshWithRugosityData();
	result->bFullyLoaded = true;

	OnRugosityCalculationsEnd();

	return result;
}

void RugosityManager::calculateSDFCallback(void* OutputData)
{
	SDF* Input = reinterpret_cast<SDF*>(OutputData);

	RUGOSITY_MANAGER.currentSDF = Input;
	RUGOSITY_MANAGER.JitterDoneCount++;

	RUGOSITY_MANAGER.MoveRugosityInfoFromSDF(RUGOSITY_MANAGER.currentSDF);

	if (RUGOSITY_MANAGER.JitterDoneCount != RUGOSITY_MANAGER.JitterToDoCount)
	{
		delete RUGOSITY_MANAGER.currentSDF;
		RUGOSITY_MANAGER.currentSDF = nullptr;
	}
	else
	{
		//LastTimeTookForCalculation = float(TIME.EndTimeStamp("CalculateRugorsityTotal"));

		//MeshLayer New
		//RUGOSITY_MANAGER.Result

		OnRugosityCalculationsEnd();
	}
}

void RugosityManager::calculateSDFAsync(void* InputData, void* OutputData)
{
	const SDFInitData* Input = reinterpret_cast<SDFInitData*>(InputData);
	SDF* Output = reinterpret_cast<SDF*>(OutputData);

	FEAABB finalAABB = Input->mesh->AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	if (Input->UseJitterExpandedAABB)
	{
		transformMatrix = glm::scale(transformMatrix, glm::vec3(Input->GridScale));
		finalAABB = finalAABB.transform(transformMatrix);
	}

	const float cellSize = finalAABB.getSize() / Input->dimentions;

	const glm::vec3 center = Input->mesh->AABB.getCenter() + glm::vec3(Input->shiftX, Input->shiftY, Input->shiftZ) * cellSize;
	const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;

	Output->Init(0/*Input->dimentions*/, finalAABB, RUGOSITY_MANAGER.currentCamera, RUGOSITY_MANAGER.ResolutonInM);

	Output->bFindSmallestRugosity = RUGOSITY_MANAGER.bUseFindSmallestRugosity;
	Output->bCGALVariant = RUGOSITY_MANAGER.bUseCGALVariant;
	Output->bWeightedNormals = RUGOSITY_MANAGER.bWeightedNormals;
	Output->bNormalizedNormals = RUGOSITY_MANAGER.bNormalizedNormals;

	Output->FillCellsWithTriangleInfo();
	Output->CalculateRugosity();

	Output->FillMeshWithRugosityData();
	Output->bFullyLoaded = true;
}

void RugosityManager::RunCreationOfSDFAsync(bool bJitter)
{
	OnRugosityCalculationsStart();

	SDFInitData* InputData = new SDFInitData();
	InputData->dimentions = SDFDimention;
	InputData->mesh = MESH_MANAGER.ActiveMesh;
	InputData->UseJitterExpandedAABB = bJitter;

	InputData->shiftX = RUGOSITY_MANAGER.shiftX;
	InputData->shiftY = RUGOSITY_MANAGER.shiftY;
	InputData->shiftZ = RUGOSITY_MANAGER.shiftZ;
	InputData->GridScale = RUGOSITY_MANAGER.GridScale;

	SDF* OutputData = new SDF();
	OutputData->bFindSmallestRugosity = RUGOSITY_MANAGER.bUseFindSmallestRugosity;
	OutputData->bCGALVariant = RUGOSITY_MANAGER.bUseCGALVariant;
	currentSDF = OutputData;

	THREAD_POOL.Execute(calculateSDFAsync, InputData, OutputData, calculateSDFCallback);
}

void RugosityManager::calculateRugorsityWithJitterAsyn(int RugosityLayerIndex)
{
	OnRugosityCalculationsStart();

	RUGOSITY_MANAGER.RugosityLayerIndex = RugosityLayerIndex;

	RunCreationOfSDFAsync(true);

	// In cells
	float kernelSize = 0.5;
	kernelSize *= 2.0f;
	kernelSize *= 100.0f;

	for (size_t i = 0; i < JitterToDoCount - 1; i++)
	{
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

		RunCreationOfSDFAsync(true);
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

std::string RugosityManager::GetUsedRugosityAlgorithmName()
{
	if (bUseFindSmallestRugosity)
		return RugosityAlgorithmList[1];

	if (bUseCGALVariant)
		return RugosityAlgorithmList[2];

	return RugosityAlgorithmList[0];
}

void RugosityManager::SetUsedRugosityAlgorithmName(std::string name)
{
	if (name == RugosityAlgorithmList[1])
	{
		bUseFindSmallestRugosity = true;
		bUseCGALVariant = false;
	} 
	else if (name == RugosityAlgorithmList[2])
	{
		bUseFindSmallestRugosity = false;
		bUseCGALVariant = true;
	}
	else
	{
		bUseFindSmallestRugosity = false;
		bUseCGALVariant = false;
	}
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

float RugosityManager::GetLastTimeTookForCalculation()
{
	return LastTimeTookForCalculation;
}

void RugosityManager::SetOnRugosityCalculationsStartCallback(void(*Func)(void))
{
	OnRugosityCalculationsStartCallbackImpl = Func;
}

void RugosityManager::OnRugosityCalculationsStart()
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	RUGOSITY_MANAGER.Result.clear();
	RUGOSITY_MANAGER.PerJitterResult.clear();
	RUGOSITY_MANAGER.JitterDoneCount = 0;

	TIME.BeginTimeStamp("CalculateRugorsityTotal");
	RUGOSITY_MANAGER.StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	if (OnRugosityCalculationsStartCallbackImpl != nullptr)
		OnRugosityCalculationsStartCallbackImpl();
}

void RugosityManager::OnRugosityCalculationsEnd()
{
	MeshLayer NewLayer;
	NewLayer.TrianglesToData = RUGOSITY_MANAGER.Result;

	NewLayer.DebugInfo = new MeshLayerDebugInfo();
	NewLayer.DebugInfo->Type = "RugosityMeshLayerDebugInfo";
	NewLayer.DebugInfo->AddEntry("Start time", RUGOSITY_MANAGER.StartTime);
	NewLayer.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	std::string AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[0];
	if (RUGOSITY_MANAGER.bUseFindSmallestRugosity)
		AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[1];

	if (RUGOSITY_MANAGER.bUseCGALVariant)
		AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[2];

	NewLayer.DebugInfo->AddEntry("Algorithm used", AlgorithmUsed);
	NewLayer.DebugInfo->AddEntry("Jitter count", RUGOSITY_MANAGER.JitterDoneCount);
	NewLayer.DebugInfo->AddEntry("Resolution used", std::to_string(RUGOSITY_MANAGER.ResolutonInM) + " m.");

	LastTimeTookForCalculation = float(TIME.EndTimeStamp("CalculateRugorsityTotal"));

	if (OnRugosityCalculationsEndCallbackImpl != nullptr)
		OnRugosityCalculationsEndCallbackImpl(NewLayer);
}

void RugosityManager::SetOnRugosityCalculationsEndCallback(void(*Func)(MeshLayer))
{
	OnRugosityCalculationsEndCallbackImpl = Func;
}