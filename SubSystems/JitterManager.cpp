#include "JitterManager.h"

JitterManager* JitterManager::Instance = nullptr;
JitterManager::JitterManager()
{
	MESH_MANAGER.AddLoadCallback(JitterManager::OnMeshUpdate);
}

JitterManager::~JitterManager() {}

void JitterManager::OnMeshUpdate()
{
	glm::mat4 TransformMatrix = glm::identity<glm::mat4>();
	TransformMatrix = glm::scale(TransformMatrix, glm::vec3(DEFAULT_GRID_SIZE + GRID_VARIANCE / 100.0f));
	FEAABB FinalAABB = MESH_MANAGER.ActiveMesh->AABB.transform(TransformMatrix);

	const float MaxMeshAABBSize = FinalAABB.getSize();

	JITTER_MANAGER.LowestPossibleResolution = MaxMeshAABBSize / 120;
	JITTER_MANAGER.HigestPossibleResolution = MaxMeshAABBSize / 9;

	JITTER_MANAGER.ResolutonInM = JITTER_MANAGER.LowestPossibleResolution;

	delete JITTER_MANAGER.LastUsedSDF;
	JITTER_MANAGER.LastUsedSDF = nullptr;
}

float JitterManager::GetResolutonInM()
{
	return ResolutonInM;
}

void JitterManager::SetResolutonInM(float NewResolutonInM)
{
	if (NewResolutonInM < LowestPossibleResolution)
		NewResolutonInM = LowestPossibleResolution;
	if (NewResolutonInM > HigestPossibleResolution)
		NewResolutonInM = HigestPossibleResolution;

	ResolutonInM = NewResolutonInM;
}

float JitterManager::GetLowestPossibleResolution()
{
	return LowestPossibleResolution;
}

float JitterManager::GetHigestPossibleResolution()
{
	return HigestPossibleResolution;
}

int JitterManager::GetJitterDoneCount()
{
	return JitterDoneCount;
}

int JitterManager::GetJitterToDoCount()
{
	return JitterToDoCount;
}

void JitterManager::CalculateWithSDFJitterAsync(std::function<void(SDFNode* currentNode)> Func, bool bSmootherResult)
{
	if (Func == nullptr)
		return;

	OnCalculationsStart();
	CurrentFunc = Func;

	auto ShiftsToUse = &SphereJitter;
	if (bSmootherResult)
		ShiftsToUse = &PseudoRandom64;

	JitterToDoCount = ShiftsToUse->size() / 4;
	//JitterToDoCount = 1;
	if (DebugJitterToDoCount != -1)
		JitterToDoCount = DebugJitterToDoCount;

	LastUsedJitterSettings.clear();
	LastUsedJitterSettings.resize(JitterToDoCount);

	for (size_t i = 0; i < JitterToDoCount; i++)
	{
		ShiftX = ShiftsToUse->operator[](i * 4);
		ShiftY = ShiftsToUse->operator[](i * 4 + 1);
		ShiftZ = ShiftsToUse->operator[](i * 4 + 2);
		GridScale = ShiftsToUse->operator[](i * 4 + 3);

		// For debug purposes.
		LastUsedJitterSettings[i].ShiftX = ShiftX;
		LastUsedJitterSettings[i].ShiftY = ShiftY;
		LastUsedJitterSettings[i].ShiftZ = ShiftZ;
		LastUsedJitterSettings[i].GridScale = GridScale;

		RunCreationOfSDFAsync();
	}
}

void JitterManager::RunCreationOfSDFAsync()
{
	SDFInitData_Jitter* InputData = new SDFInitData_Jitter();

	InputData->ShiftX = ShiftX;
	InputData->ShiftY = ShiftY;
	InputData->ShiftZ = ShiftZ;
	InputData->GridScale = GridScale;

	SDF* OutputData = new SDF();
	LastUsedSDF = OutputData;

	THREAD_POOL.Execute(RunCalculationOnSDFAsync, InputData, OutputData, AfterCalculationFinishSDFCallback);
}

//void JitterManager::ExtractDataFromSDF(SDF* SDF)
//{
//	if (MESH_MANAGER.ActiveMesh == nullptr)
//		return;
//
//	int CurrentIndex = PerSDFTrianglesResult.size();
//	PerSDFTrianglesResult.resize(PerSDFTrianglesResult.size() + 1);
//	PerSDFTrianglesResult[CurrentIndex].resize(MESH_MANAGER.ActiveMesh->Triangles.size());
//	std::vector<int> TrianglesRugosityCount;
//	TrianglesRugosityCount.resize(MESH_MANAGER.ActiveMesh->Triangles.size());
//
//	auto WorkOnNode = [&](SDFNode* CurrentNode) {
//		for (size_t i = 0; i < CurrentNode->TrianglesInCell.size(); i++)
//		{
//			const int TriangleIndex = CurrentNode->TrianglesInCell[i];
//			TrianglesRugosityCount[TriangleIndex]++;
//			PerSDFTrianglesResult[CurrentIndex][TriangleIndex] += static_cast<float>(CurrentNode->UserData);
//		}
//	};
//	SDF->RunOnAllNodes(WorkOnNode);
//
//	for (size_t i = 0; i < PerSDFTrianglesResult[CurrentIndex].size(); i++)
//	{
//		PerSDFTrianglesResult[CurrentIndex][i] /= TrianglesRugosityCount[i];
//	}
//
//	//TimeTookFillMeshWithRugosityData = TIME.EndTimeStamp("FillMeshWithRugosityData");
//}

void JitterManager::RunCalculationOnSDFAsync(void* InputData, void* OutputData)
{
	const SDFInitData_Jitter* Input = reinterpret_cast<SDFInitData_Jitter*>(InputData);
	SDF* Output = reinterpret_cast<SDF*>(OutputData);

	FEAABB finalAABB = MESH_MANAGER.ActiveMesh->AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	transformMatrix = glm::scale(transformMatrix, glm::vec3(Input->GridScale));
	finalAABB = finalAABB.transform(transformMatrix);

	const glm::vec3 center = MESH_MANAGER.ActiveMesh->AABB.getCenter() + glm::vec3(Input->ShiftX, Input->ShiftY, Input->ShiftZ) * JITTER_MANAGER.ResolutonInM;
	const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;

	Output->Init(0, finalAABB, JITTER_MANAGER.ResolutonInM);

	Output->FillCellsWithTriangleInfo();
	TIME.BeginTimeStamp("Calculate CurrentFunc");
	Output->RunOnAllNodes(JITTER_MANAGER.CurrentFunc);
	Output->TimeTookCalculateRugosity = TIME.EndTimeStamp("Calculate CurrentFunc");

	//JITTER_MANAGER.ExtractDataFromSDF(Output);
	Output->FillMeshWithRugosityData();
	Output->bFullyLoaded = true;
}

void JitterManager::AfterCalculationFinishSDFCallback(void* OutputData)
{
	SDF* Input = reinterpret_cast<SDF*>(OutputData);

	JITTER_MANAGER.LastUsedSDF = Input;
	JITTER_MANAGER.JitterDoneCount++;

	JITTER_MANAGER.MoveResultDataFromSDF(JITTER_MANAGER.LastUsedSDF);

	if (JITTER_MANAGER.JitterDoneCount != JITTER_MANAGER.JitterToDoCount)
	{
		delete JITTER_MANAGER.LastUsedSDF;
		JITTER_MANAGER.LastUsedSDF = nullptr;
	}
	else
	{
		OnCalculationsEnd();
	}
}

void JitterManager::MoveResultDataFromSDF(SDF* SDF)
{
	if (SDF == nullptr || SDF->TrianglesUserData.empty())
		return;

	PerJitterResult.resize(PerJitterResult.size() + 1);

	if (Result.size() != SDF->TrianglesUserData.size())
		Result.resize(SDF->TrianglesUserData.size());

	for (size_t i = 0; i < Result.size(); i++)
	{
		PerJitterResult.back().push_back(SDF->TrianglesUserData[i]);
		Result[i] += SDF->TrianglesUserData[i];
	}

	if (JITTER_MANAGER.JitterDoneCount == JITTER_MANAGER.JitterToDoCount)
	{
		for (size_t i = 0; i < Result.size(); i++)
		{
			Result[i] /= JitterDoneCount;
		}
	}

	/*if (SDF == nullptr || PerSDFTrianglesResult.empty())
		return;

	PerJitterResult.resize(PerJitterResult.size() + 1);

	if (Result.size() != PerSDFTrianglesResult.back().size())
		Result.resize(PerSDFTrianglesResult.back().size());

	if (Result.size() != SDF->TrianglesUserData.size())
		Result.resize(SDF->TrianglesUserData.size());

	for (size_t i = 0; i < Result.size(); i++)
	{
		PerJitterResult.back().push_back(PerSDFTrianglesResult.back()[i]);
		Result[i] += PerSDFTrianglesResult.back()[i];
	}

	if (JITTER_MANAGER.JitterDoneCount == JITTER_MANAGER.JitterToDoCount)
	{
		for (size_t i = 0; i < Result.size(); i++)
		{
			Result[i] /= JitterDoneCount;
		}
	}*/
}

void JitterManager::OnCalculationsStart()
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	JITTER_MANAGER.Result.clear();
	JITTER_MANAGER.PerJitterResult.clear();
	JITTER_MANAGER.JitterDoneCount = 0;

	TIME.BeginTimeStamp("JitterCalculateTotal");
	JITTER_MANAGER.StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	for (size_t i = 0; i < JITTER_MANAGER.OnCalculationsStartCallbacks.size(); i++)
	{
		if (JITTER_MANAGER.OnCalculationsStartCallbacks[i] != nullptr)
			JITTER_MANAGER.OnCalculationsStartCallbacks[i]();
	}
}

void JitterManager::OnCalculationsEnd()
{
	MeshLayer NewLayer;
	NewLayer.TrianglesToData = JITTER_MANAGER.Result;

	NewLayer.DebugInfo = new MeshLayerDebugInfo();
	NewLayer.DebugInfo->Type = "JitterMeshLayerDebugInfo";
	NewLayer.DebugInfo->AddEntry("Start time", JITTER_MANAGER.StartTime);
	NewLayer.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
	NewLayer.DebugInfo->AddEntry("Jitter count", JITTER_MANAGER.JitterDoneCount);
	NewLayer.DebugInfo->AddEntry("Resolution used", std::to_string(JITTER_MANAGER.ResolutonInM) + " m.");

	JITTER_MANAGER.LastTimeTookForCalculation = float(TIME.EndTimeStamp("JitterCalculateTotal"));

	for (size_t i = 0; i < JITTER_MANAGER.OnCalculationsEndCallbacks.size(); i++)
	{
		if (JITTER_MANAGER.OnCalculationsEndCallbacks[i] != nullptr)
			JITTER_MANAGER.OnCalculationsEndCallbacks[i](NewLayer);
	}
}

void JitterManager::SetOnCalculationsStartCallback(std::function<void()> Func)
{
	OnCalculationsStartCallbacks.push_back(Func);
}

void JitterManager::SetOnCalculationsEndCallback(std::function<void(MeshLayer CurrentMeshLayer)> Func)
{
	OnCalculationsEndCallbacks.push_back(Func);
}

std::vector<std::vector<float>> JitterManager::GetPerJitterResult()
{
	return JITTER_MANAGER.PerJitterResult;
}

SDF* JitterManager::GetLastUsedSDF()
{
	return LastUsedSDF;
}

std::vector<SDFInitData_Jitter> JitterManager::GetLastUsedJitterSettings()
{
	return LastUsedJitterSettings;
}

int JitterManager::GetDebugJitterToDoCount()
{
	return DebugJitterToDoCount;
}

void JitterManager::SetDebugJitterToDoCount(int NewValue)
{
	// Deactivate feature.
	if (NewValue == -1)
	{
		DebugJitterToDoCount = -1;
		return;
	}

	if (NewValue > 0 && NewValue < SphereJitter.size())
		DebugJitterToDoCount = NewValue;
}