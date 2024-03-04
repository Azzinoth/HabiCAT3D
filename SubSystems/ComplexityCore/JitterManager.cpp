#include "JitterManager.h"
using namespace FocalEngine;

JitterManager* JitterManager::Instance = nullptr;
JitterManager::JitterManager()
{
	if (APPLICATION.HasConsoleWindow())
	{
		COMPLEXITY_METRIC_MANAGER.AddLoadCallback(JitterManager::OnMeshUpdate);
	}
	else
	{
		MESH_MANAGER.AddLoadCallback(JitterManager::OnMeshUpdate);
	}

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
	FEAABB FinalAABB = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB.Transform(TransformMatrix);

	const float MaxMeshAABBSize = FinalAABB.GetSize();

	JITTER_MANAGER.LowestPossibleResolution = MaxMeshAABBSize / 120;
	JITTER_MANAGER.HigestPossibleResolution = MaxMeshAABBSize / 9;

	JITTER_MANAGER.ResolutonInM = JITTER_MANAGER.LowestPossibleResolution;

	delete JITTER_MANAGER.LastUsedGrid;
	JITTER_MANAGER.LastUsedGrid = nullptr;
}

float JitterManager::GetResolutionInM()
{
	return ResolutonInM;
}

void JitterManager::SetResolutionInM(float NewResolutonInM)
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

void JitterManager::CalculateWithGridJitterAsync(std::function<void(GridNode* CurrentNode)> Func, bool bSmootherResult)
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

	JitterToDoCount = static_cast<int>(TempShifts.size() / 4);
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

		RunCreationOfGridAsync();
	}
}

void JitterManager::RunCreationOfGridAsync()
{
	GridInitData_Jitter* InputData = new GridInitData_Jitter();

	InputData->ShiftX = ShiftX;
	InputData->ShiftY = ShiftY;
	InputData->ShiftZ = ShiftZ;
	InputData->GridScale = GridScale;

	MeasurementGrid* OutputData = new MeasurementGrid();
	LastUsedGrid = OutputData;

	THREAD_POOL.Execute(RunCalculationOnGridAsync, InputData, OutputData, AfterCalculationFinishGridCallback);
}

void JitterManager::RunCalculationOnGridAsync(void* InputData, void* OutputData)
{
	GridInitData_Jitter* Input = reinterpret_cast<GridInitData_Jitter*>(InputData);
	MeasurementGrid* Output = reinterpret_cast<MeasurementGrid*>(OutputData);

	FEAABB FinalAABB = JITTER_MANAGER.GetAABBForJitteredGrid(Input, JITTER_MANAGER.ResolutonInM);
	Output->Init(0, FinalAABB, JITTER_MANAGER.ResolutonInM);

	Output->FillCellsWithTriangleInfo();
	TIME.BeginTimeStamp("Calculate CurrentFunc");
	Output->RunOnAllNodes(JITTER_MANAGER.CurrentFunc);
	Output->TimeTakenToCalculate = static_cast<float>(TIME.EndTimeStamp("Calculate CurrentFunc"));

	//JITTER_MANAGER.ExtractDataFromGrid(Output);
	Output->FillMeshWithUserData();
	Output->bFullyLoaded = true;
}

void JitterManager::AfterCalculationFinishGridCallback(void* OutputData)
{
	MeasurementGrid* Input = reinterpret_cast<MeasurementGrid*>(OutputData);

	JITTER_MANAGER.LastUsedGrid = Input;
	JITTER_MANAGER.JitterDoneCount++;

	JITTER_MANAGER.MoveResultDataFromGrid(JITTER_MANAGER.LastUsedGrid);
	if (JITTER_MANAGER.JitterDoneCount != JITTER_MANAGER.JitterToDoCount)
	{
		delete JITTER_MANAGER.LastUsedGrid;
		JITTER_MANAGER.LastUsedGrid = nullptr;
	}
	else
	{
		OnCalculationsEnd();
	}
}

void JitterManager::MoveResultDataFromGrid(MeasurementGrid* Grid)
{
	if (Grid == nullptr || Grid->TrianglesUserData.empty())
		return;

	PerJitterResult.resize(PerJitterResult.size() + 1);

	if (Result.size() != Grid->TrianglesUserData.size())
		Result.resize(Grid->TrianglesUserData.size());

	if (CorrectValuesCounters.empty())
	{
		CorrectValuesCounters.resize(Grid->TrianglesUserData.size());
		std::fill(CorrectValuesCounters.begin(), CorrectValuesCounters.end(), 0);
	}

	for (size_t i = 0; i < Result.size(); i++)
	{
		// We will save all results. Even if they are not correct.
		PerJitterResult.back().push_back(Grid->TrianglesUserData[i]);

		// And if user defined function is not nullptr, we will check if we should ignore this value.
		if (IgnoreValueFunc != nullptr)
		{
			if (!IgnoreValueFunc(Grid->TrianglesUserData[i]))
			{
				Result[i] += Grid->TrianglesUserData[i];
				CorrectValuesCounters[i]++;
			}
		}
		else
		{
			Result[i] += Grid->TrianglesUserData[i];
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
	if (APPLICATION.GetMainWindow() != nullptr)
		glfwMakeContextCurrent(APPLICATION.GetMainWindow()->GetGlfwWindow());

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

MeasurementGrid* JitterManager::GetLastUsedGrid()
{
	return LastUsedGrid;
}

std::vector<GridInitData_Jitter> JitterManager::GetLastUsedJitterSettings()
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

void JitterManager::CalculateOnWholeModel(std::function<void(GridNode* CurrentNode)> Func)
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

	GridInitData_Jitter* InputData = new GridInitData_Jitter();

	InputData->ShiftX = ShiftX;
	InputData->ShiftY = ShiftY;
	InputData->ShiftZ = ShiftZ;
	InputData->GridScale = GridScale;

	MeasurementGrid* OutputData = new MeasurementGrid();
	LastUsedGrid = OutputData;

	RunCalculationOnWholeModel(OutputData);
	AfterCalculationFinishGridCallback(OutputData);

	ResolutonInM = JITTER_MANAGER.GetLowestPossibleResolution();
}

void JitterManager::RunCalculationOnWholeModel(MeasurementGrid* ResultGrid)
{
	FEAABB MeshAABB = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB;

	const glm::vec3 Center = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB.GetCenter() ;
	const FEAABB GridAABB = FEAABB(Center - glm::vec3(MeshAABB.GetSize() / 2.0f), Center + glm::vec3(MeshAABB.GetSize() / 2.0f));
	MeshAABB = GridAABB;

	ResultGrid->Init(0, MeshAABB, -1);

	ResultGrid->FillCellsWithTriangleInfo();
	TIME.BeginTimeStamp("Calculate CurrentFunc");
	ResultGrid->RunOnAllNodes(JITTER_MANAGER.CurrentFunc);
	ResultGrid->TimeTakenToCalculate = static_cast<float>(TIME.EndTimeStamp("Calculate CurrentFunc"));

	ResultGrid->FillMeshWithUserData();
	ResultGrid->bFullyLoaded = true;
}

std::string JitterManager::GetCurrentJitterVectorSetName()
{
	return CurrentJitterVectorSetName;
}

void JitterManager::SetCurrentJitterVectorSetName(std::string Name)
{
	CurrentJitterVectorSetName = Name;
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
	int lowerOutlierPosition = static_cast<int>(SortedData.size() * LowerPercentile);
	int upperOutlierPosition = static_cast<int>(SortedData.size() * UpperPercentile);

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

float JitterManager::FindStandardDeviation(std::vector<float> DataPoints)
{
	float Mean = 0.0f;
	for (int i = 0; i < DataPoints.size(); i++)
	{
		Mean += DataPoints[i];
	}
	Mean /= DataPoints.size();

	float Variance = 0.0f;
	for (int i = 0; i < DataPoints.size(); i++)
	{
		DataPoints[i] -= Mean;
		DataPoints[i] = static_cast<float>(std::pow(DataPoints[i], 2.0));
		Variance += DataPoints[i];
	}
	Variance /= DataPoints.size();

	return std::sqrt(Variance);
}

std::vector<float> JitterManager::ProduceStandardDeviationData()
{
	std::vector<float> Result;

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return Result;

	if (PerJitterResult.empty())
		return Result;

	for (int i = 0; i < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); i++)
	{
		std::vector<float> CurrentTriangleResults;
		for (int j = 0; j < JitterToDoCount; j++)
		{
			CurrentTriangleResults.push_back(PerJitterResult[j][i]);
		}

		Result.push_back(FindStandardDeviation(CurrentTriangleResults));
	}

	return Result;
}

FEAABB JitterManager::GetAABBForJitteredGrid(GridInitData_Jitter* Settings, float CurrentResolutionInM)
{
	FEAABB MeshAABB = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB;
	FEAABB FinalAABB = MeshAABB;

	glm::mat4 TransformMatrix = glm::identity<glm::mat4>();
	TransformMatrix = glm::translate(TransformMatrix, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->GetPosition());
	TransformMatrix = glm::scale(TransformMatrix, glm::vec3(Settings->GridScale));
	FinalAABB = FinalAABB.Transform(TransformMatrix);

	const glm::vec3 Center = MeshAABB.GetCenter() + glm::vec3(Settings->ShiftX, Settings->ShiftY, Settings->ShiftZ) * CurrentResolutionInM;
	const FEAABB GridAABB = FEAABB(Center - glm::vec3(FinalAABB.GetSize() / 2.0f), Center + glm::vec3(FinalAABB.GetSize() / 2.0f));
	FinalAABB = GridAABB;

	return FinalAABB;
}