#include "JitterManager.h"

JitterManager* JitterManager::Instance = nullptr;
JitterManager::JitterManager() {}
JitterManager::~JitterManager() {}


void JitterManager::CalculateWithSDFJitterAsync(std::function<void(SDFNode* currentNode)> Func)
{
	if (Func == nullptr)
		return;

	CurrentFunc = Func;

	static std::vector<float> SphereJitter = {
		0.0000, 1.0000, -0.0000, 1.430000f,
		1.0000, 0.0000, -0.0000, 1.410000f,
		0.0000, 0.0000, -1.0000, 1.390000f,
		-1.0000, 0.0000, -0.0000, 1.330000f,
		0.0000, 0.0000, 1.0000, 1.400000f,
		0.0000, -1.0000, -0.0000, 1.340000f,
		0.5000, 0.8660, -0.0000, 1.280000f,
		0.8660, 0.5000, -0.0000, 1.290000f,
		0.0000, 0.8660, -0.5000, 1.300000f,
		0.0000, 0.5000, -0.8660, 1.410000f,
		-0.5000, 0.8660, -0.0000, 1.250000f,
		-0.8660, 0.5000, -0.0000, 1.300000f,
		0.0000, 0.8660, 0.5000, 1.270000f,
		0.0000, 0.5000, 0.8660, 1.440000f,
		0.5000, -0.8660, -0.0000, 1.280000f,
		0.8660, -0.5000, -0.0000, 1.270000f,
		0.0000, -0.8660, -0.5000, 1.250000f,
		0.0000, -0.5000, -0.8660, 1.460000f,
		-0.5000, -0.8660, -0.0000, 1.250000f,
		-0.8660, -0.5000, -0.0000, 1.260000f,
		0.0000, -0.8660, 0.5000, 1.410000f,
		0.0000, -0.5000, 0.8660, 1.440000f,
		0.8660, 0.0000, -0.5000, 1.400000f,
		0.5000, 0.0000, -0.8660, 1.390000f,
		-0.5000, 0.0000, -0.8660, 1.410000f,
		-0.8660, 0.0000, -0.5000, 1.420000f,
		-0.8660, 0.0000, 0.5000, 1.250000f,
		-0.5000, 0.0000, 0.8660, 1.280000f,
		0.5000, 0.0000, 0.8660, 1.460000f,
		0.8660, 0.0000, 0.5000, 1.400000f,
		0.5477, 0.6325, -0.5477, 1.270000f,
		-0.5477, 0.6325, -0.5477, 1.270000f,
		-0.5477, 0.6325, 0.5477, 1.390000f,
		0.5477, 0.6325, 0.5477, 1.320000f,
		0.5477, -0.6325, -0.5477, 1.270000f,
		-0.5477, -0.6325, -0.5477, 1.430000f,
		-0.5477, -0.6325, 0.5477, 1.280000f,
		0.5477, -0.6325, 0.5477, 1.300000f,
		0.0000, 0.5000, -0.0000, 1.320000f,
		0.4714, -0.1667, -0.0000, 1.260000f,
		-0.2357, -0.1667, -0.4082, 1.330000f,
		-0.2357, -0.1667, 0.4082, 1.430000f,
		0.2973, 0.4020, -0.0000, 1.470000f,
		0.4781, 0.1463, -0.0000, 1.310000f,
		-0.1487, 0.4020, -0.2575, 1.490000f,
		-0.2391, 0.1463, -0.4140, 1.250000f,
		-0.1487, 0.4020, 0.2575, 1.480000f,
		-0.2391, 0.1463, 0.4140, 1.410000f,
		0.3294, -0.2742, -0.2575, 1.320000f,
		0.0583, -0.2742, -0.4140, 1.480000f,
		0.3294, -0.2742, 0.2575, 1.450000f,
		0.0583, -0.2742, 0.4140, 1.370000f,
		-0.3877, -0.2742, -0.1565, 1.430000f,
		-0.3877, -0.2742, 0.1565, 1.390000f,
		0.2132, 0.2611, -0.3693, 1.370000f,
		-0.4264, 0.2611, -0.0000, 1.480000f,
		0.2132, 0.2611, 0.3693, 1.460000f,
		0.1040, -0.4891, -0.0000, 1.380000f,
	};

	JitterToDoCount = SphereJitter.size() / 4;
	for (size_t i = 0; i < JitterToDoCount; i++)
	{
		ShiftX = SphereJitter[i * 4];
		ShiftY = SphereJitter[i * 4 + 1];
		ShiftZ = SphereJitter[i * 4 + 2];
		GridScale = SphereJitter[i * 4 + 3];

		RunCreationOfSDFAsync();
	}
}

void JitterManager::RunCreationOfSDFAsync()
{
	SDFInitData_Jitter* InputData = new SDFInitData_Jitter();
	InputData->Dimentions = SDFDimention;
	InputData->Mesh = MESH_MANAGER.ActiveMesh;

	InputData->ShiftX = ShiftX;
	InputData->ShiftY = ShiftY;
	InputData->ShiftZ = ShiftZ;
	InputData->GridScale = GridScale;

	SDF* OutputData = new SDF();
	//OutputData->bFindSmallestRugosity = RUGOSITY_MANAGER.bUseFindSmallestRugosity;
	//OutputData->bCGALVariant = RUGOSITY_MANAGER.bUseCGALVariant;
	currentSDF = OutputData;

	THREAD_POOL.Execute(RunCalculationOnSDFAsync, InputData, OutputData, AfterCalculationFinishSDFCallback);
}

void JitterManager::RunCalculationOnSDFAsync(void* InputData, void* OutputData)
{
	const SDFInitData_Jitter* Input = reinterpret_cast<SDFInitData_Jitter*>(InputData);
	SDF* Output = reinterpret_cast<SDF*>(OutputData);

	FEAABB finalAABB = Input->Mesh->AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	transformMatrix = glm::scale(transformMatrix, glm::vec3(Input->GridScale));
	finalAABB = finalAABB.transform(transformMatrix);

	const float cellSize = finalAABB.getSize() / Input->Dimentions;

	const glm::vec3 center = Input->Mesh->AABB.getCenter() + glm::vec3(Input->ShiftX, Input->ShiftY, Input->ShiftZ) * cellSize;
	const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;

	Output->Init(0/*Input->dimentions*/, finalAABB, nullptr/*currentCamera*/, JITTER_MANAGER.ResolutonInM);

	//Output->bFindSmallestRugosity = RUGOSITY_MANAGER.bUseFindSmallestRugosity;
	//Output->bCGALVariant = RUGOSITY_MANAGER.bUseCGALVariant;
	//Output->bWeightedNormals = RUGOSITY_MANAGER.bWeightedNormals;
	//Output->bNormalizedNormals = RUGOSITY_MANAGER.bNormalizedNormals;

	Output->FillCellsWithTriangleInfo();
	TIME.BeginTimeStamp("Calculate CurrentFunc");
	Output->RunOnAllNodes(JITTER_MANAGER.CurrentFunc);
	Output->TimeTookCalculateRugosity = TIME.EndTimeStamp("Calculate CurrentFunc");

	Output->FillMeshWithRugosityData();
	Output->bFullyLoaded = true;
}

void JitterManager::AfterCalculationFinishSDFCallback(void* OutputData)
{
	SDF* Input = reinterpret_cast<SDF*>(OutputData);

	JITTER_MANAGER.currentSDF = Input;
	JITTER_MANAGER.JitterDoneCount++;

	JITTER_MANAGER.MoveResultDataFromSDF(JITTER_MANAGER.currentSDF);

	if (JITTER_MANAGER.JitterDoneCount != JITTER_MANAGER.JitterToDoCount)
	{
		delete JITTER_MANAGER.currentSDF;
		JITTER_MANAGER.currentSDF = nullptr;
	}
	else
	{
		OnCalculationsEnd();
	}
}

void JitterManager::MoveResultDataFromSDF(SDF* SDF)
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
		/*if (Result[i] <= 0.0f)
		{
			Result[i] += 0.000000001f;
			bWeightedNormals = true;
		}*/
	}

	if (JITTER_MANAGER.JitterDoneCount == JITTER_MANAGER.JitterToDoCount)
	{
		for (size_t i = 0; i < Result.size(); i++)
		{
			Result[i] /= JitterDoneCount;
		}
	}
}

void JitterManager::OnCalculationsEnd()
{
	MeshLayer NewLayer;
	NewLayer.TrianglesToData = JITTER_MANAGER.Result;

	NewLayer.DebugInfo = new MeshLayerDebugInfo();
	NewLayer.DebugInfo->Type = "RugosityMeshLayerDebugInfo";
	NewLayer.DebugInfo->AddEntry("Start time", JITTER_MANAGER.StartTime);
	NewLayer.DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));

	/*std::string AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[0];
	if (RUGOSITY_MANAGER.bUseFindSmallestRugosity)
		AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[1];

	if (RUGOSITY_MANAGER.bUseCGALVariant)
		AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[2];

	NewLayer.DebugInfo->AddEntry("Algorithm used", AlgorithmUsed);*/
	NewLayer.DebugInfo->AddEntry("Jitter count", JITTER_MANAGER.JitterDoneCount);
	NewLayer.DebugInfo->AddEntry("Resolution used", std::to_string(JITTER_MANAGER.ResolutonInM) + " m.");

	//std::string DeleteOutliers = "No";
	//// Remove outliers.
	//if (RUGOSITY_MANAGER.bDeleteOutliers)
	//{
	//	DeleteOutliers = "Yes";
	//	float OutlierBeginValue = FLT_MAX;

	//	std::vector<float> SortedData = NewLayer.TrianglesToData;
	//	std::sort(SortedData.begin(), SortedData.end());

	//	int OutlierBeginPosition = SortedData.size() * 0.99;
	//	OutlierBeginValue = SortedData[OutlierBeginPosition];
	//	float NewMax = SortedData[OutlierBeginPosition - 1];

	//	for (int i = 0; i < NewLayer.TrianglesToData.size(); i++)
	//	{
	//		if (NewLayer.TrianglesToData[i] >= OutlierBeginValue)
	//			NewLayer.TrianglesToData[i] = NewMax;
	//	}
	//}
	//NewLayer.DebugInfo->AddEntry("Delete outliers", DeleteOutliers);

	//LastTimeTookForCalculation = float(TIME.EndTimeStamp("CalculateRugorsityTotal"));

	if (JITTER_MANAGER.OnCalculationsEndCallbackImpl != nullptr)
		JITTER_MANAGER.OnCalculationsEndCallbackImpl(NewLayer);
}

void JitterManager::SetOnCalculationsEndCallback(std::function<void(MeshLayer CurrentMeshLayer)> Func)
{
	OnCalculationsEndCallbackImpl = Func;
}