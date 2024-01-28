#include "JitterManager.h"

JitterManager* JitterManager::Instance = nullptr;
JitterManager::JitterManager()
{
	MESH_MANAGER.AddLoadCallback(JitterManager::OnMeshUpdate);

	JitterVectorSetNames.push_back("1");
	JitterVectorSetNames.push_back("7");
	JitterVectorSetNames.push_back("13");
	JitterVectorSetNames.push_back("25");
	JitterVectorSetNames.push_back("37");
	JitterVectorSetNames.push_back("55");
	JitterVectorSetNames.push_back("73");
}

JitterManager::~JitterManager() {}

void JitterManager::OnMeshUpdate()
{
	glm::mat4 TransformMatrix = glm::identity<glm::mat4>();
	TransformMatrix = glm::scale(TransformMatrix, glm::vec3(DEFAULT_GRID_SIZE + GRID_VARIANCE / 100.0f));
	FEAABB FinalAABB = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB.transform(TransformMatrix);

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

	if (TetrahedronJitterOrientationsOptions.find(CurrentJitterVectorSetName) == TetrahedronJitterOrientationsOptions.end())
		CurrentJitterVectorSetName = "55";

	std::vector<float> TempShifts = TetrahedronJitterOrientationsOptions[CurrentJitterVectorSetName];
	if (bSmootherResult)
		TempShifts = PseudoRandom64;

	//auto ShiftsToUse = &TetrahedronJitterOrientationsOptions[CurrentJitterVectorSetName]/*Tetrahedron73Jitter*//*SphereJitter*/;
	//if (bSmootherResult)
	//	ShiftsToUse = &PseudoRandom64;

	JitterToDoCount = TempShifts.size() / 4;
	if (DebugJitterToDoCount != -1)
		JitterToDoCount = DebugJitterToDoCount;

	for (size_t i = 0; i < JitterToDoCount; i++)
	{
		TempShifts[i * 4] *= 1.3f;
		TempShifts[i * 4 + 1] *= 1.3f;
		TempShifts[i * 4 + 2] *= 1.3f;
		TempShifts[i * 4 + 3] *= 1.2f;
	}

	LastUsedJitterSettings.clear();
	LastUsedJitterSettings.resize(JitterToDoCount);

	for (size_t i = 0; i < JitterToDoCount; i++)
	{
		ShiftX = TempShifts[i * 4];
		ShiftY = TempShifts[i * 4 + 1];
		ShiftZ = TempShifts[i * 4 + 2];
		GridScale = TempShifts[i * 4 + 3];

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

void JitterManager::RunCalculationOnSDFAsync(void* InputData, void* OutputData)
{
	const SDFInitData_Jitter* Input = reinterpret_cast<SDFInitData_Jitter*>(InputData);
	SDF* Output = reinterpret_cast<SDF*>(OutputData);

	FEAABB finalAABB = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	transformMatrix = glm::scale(transformMatrix, glm::vec3(Input->GridScale));
	finalAABB = finalAABB.transform(transformMatrix);

	const glm::vec3 center = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB.getCenter() + glm::vec3(Input->ShiftX, Input->ShiftY, Input->ShiftZ) * JITTER_MANAGER.ResolutonInM;
	const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;

	Output->Init(0, finalAABB, JITTER_MANAGER.ResolutonInM);

	Output->FillCellsWithTriangleInfo();
	TIME.BeginTimeStamp("Calculate CurrentFunc");
	Output->RunOnAllNodes(JITTER_MANAGER.CurrentFunc);
	Output->TimeTookCalculate = TIME.EndTimeStamp("Calculate CurrentFunc");

	//JITTER_MANAGER.ExtractDataFromSDF(Output);
	Output->FillMeshWithUserData();
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

	if (CorrectValuesCounters.empty())
	{
		CorrectValuesCounters.resize(SDF->TrianglesUserData.size());
		std::fill(CorrectValuesCounters.begin(), CorrectValuesCounters.end(), 0);
	}

	for (size_t i = 0; i < Result.size(); i++)
	{
		// We will save all results. Even if they are not correct.
		PerJitterResult.back().push_back(SDF->TrianglesUserData[i]);

		// And if user defined function is not nullptr, we will check if we should ignore this value.
		if (IgnoreValueFunc != nullptr)
		{
			if (!IgnoreValueFunc(SDF->TrianglesUserData[i]))
			{
				Result[i] += SDF->TrianglesUserData[i];
				CorrectValuesCounters[i]++;
			}
		}
		else
		{
			Result[i] += SDF->TrianglesUserData[i];
			CorrectValuesCounters[i]++;
		}
		
	}

	if (JITTER_MANAGER.JitterDoneCount == JITTER_MANAGER.JitterToDoCount)
	{
		for (size_t i = 0; i < Result.size(); i++)
		{
			if (CorrectValuesCounters[i] == 0)
			{
				Result[i] = FallbackValue;
				continue;
			}
				
			Result[i] /= CorrectValuesCounters[i];
		}
	}
}

void JitterManager::OnCalculationsStart()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	JITTER_MANAGER.Result.clear();
	JITTER_MANAGER.PerJitterResult.clear();
	JITTER_MANAGER.JitterDoneCount = 0;
	JITTER_MANAGER.CorrectValuesCounters.resize(0);

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
	JITTER_MANAGER.IgnoreValueFunc = nullptr;
	JITTER_MANAGER.FallbackValue = 1.0f;

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

	for (size_t i = 0; i < JITTER_MANAGER.LastUsedJitterSettings.size(); i++)
	{
		NewLayer.DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftX", JITTER_MANAGER.LastUsedJitterSettings[i].ShiftX);
		NewLayer.DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftY", JITTER_MANAGER.LastUsedJitterSettings[i].ShiftY);
		NewLayer.DebugInfo->AddEntry("Jitter " + std::to_string(i) + " ShiftZ", JITTER_MANAGER.LastUsedJitterSettings[i].ShiftZ);
		NewLayer.DebugInfo->AddEntry("Jitter " + std::to_string(i) + " GridScale", JITTER_MANAGER.LastUsedJitterSettings[i].GridScale);
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

void JitterManager::SetIgnoreValueFunction(std::function<bool(float Value)> Func)
{
	IgnoreValueFunc = Func;
}

void JitterManager::SetFallbackValue(float NewValue)
{
	FallbackValue = NewValue;
}

void JitterManager::CalculateOnWholeModel(std::function<void(SDFNode* currentNode)> Func)
{
	if (Func == nullptr)
		return;

	OnCalculationsStart();
	CurrentFunc = Func;

	JitterToDoCount = 1;
	ResolutonInM = -1.0f;

	LastUsedJitterSettings.clear();
	LastUsedJitterSettings.resize(1);

	ShiftX = 0.0f;
	ShiftY = 0.0f;
	ShiftZ = 0.0f;
	GridScale = 1.0f;

	LastUsedJitterSettings[0].ShiftX = ShiftX;
	LastUsedJitterSettings[0].ShiftY = ShiftY;
	LastUsedJitterSettings[0].ShiftZ = ShiftZ;
	LastUsedJitterSettings[0].GridScale = GridScale;

	SDFInitData_Jitter* InputData = new SDFInitData_Jitter();

	InputData->ShiftX = ShiftX;
	InputData->ShiftY = ShiftY;
	InputData->ShiftZ = ShiftZ;
	InputData->GridScale = GridScale;

	SDF* OutputData = new SDF();
	LastUsedSDF = OutputData;

	RunCalculationOnWholeModel(OutputData);
	AfterCalculationFinishSDFCallback(OutputData);

	ResolutonInM = JITTER_MANAGER.GetLowestPossibleResolution();
}

void JitterManager::RunCalculationOnWholeModel(SDF* ResultSDF)
{
	FEAABB MeshAABB = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB;

	const glm::vec3 Center = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB.getCenter() ;
	const FEAABB SDFAABB = FEAABB(Center - glm::vec3(MeshAABB.getSize() / 2.0f), Center + glm::vec3(MeshAABB.getSize() / 2.0f));
	MeshAABB = SDFAABB;

	ResultSDF->Init(0, MeshAABB, -1);

	ResultSDF->FillCellsWithTriangleInfo();
	TIME.BeginTimeStamp("Calculate CurrentFunc");
	ResultSDF->RunOnAllNodes(JITTER_MANAGER.CurrentFunc);
	ResultSDF->TimeTookCalculate = TIME.EndTimeStamp("Calculate CurrentFunc");

	ResultSDF->FillMeshWithUserData();
	ResultSDF->bFullyLoaded = true;
}

std::string JitterManager::GetCurrentJitterVectorSetName()
{
	return CurrentJitterVectorSetName;
}

void JitterManager::SetCurrentJitterVectorSetName(std::string name)
{
	CurrentJitterVectorSetName = name;
}

std::vector<std::string> JitterManager::GetJitterVectorSetNames()
{
	return JitterVectorSetNames;
}

void JitterManager::AdjustOutliers(std::vector<float>& Data, float LowerPercentile, float UpperPercentile)
{
	if (Data.empty()) return;

	// Copy and sort the data
	std::vector<float> SortedData = Data;
	std::sort(SortedData.begin(), SortedData.end());

	// Calculate positions for lower and upper outliers
	int lowerOutlierPosition = SortedData.size() * LowerPercentile;
	int upperOutlierPosition = SortedData.size() * UpperPercentile;

	// Get the values for outlier thresholds
	float lowerOutlierValue = SortedData[lowerOutlierPosition];
	float upperOutlierValue = SortedData[upperOutlierPosition];

	// Get the new min and max values (just inside the outlier thresholds)
	float NewMin = SortedData[lowerOutlierPosition + 1];
	float NewMax = SortedData[upperOutlierPosition - 1];

	// Adjust the data
	for (int i = 0; i < Data.size(); i++)
	{
		if (Data[i] <= lowerOutlierValue && LowerPercentile > 0.0f)
		{
			Data[i] = NewMin;
		}
		else if (Data[i] >= upperOutlierValue)
		{
			Data[i] = NewMax;
		}
	}
}