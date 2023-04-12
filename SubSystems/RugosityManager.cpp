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

void RugosityManager::CalculateOneNodeRugosity(SDFNode* CurrentNode)
{
	if (CurrentNode->TrianglesInCell.empty())
		return;

	float TotalArea = 0.0f;
	for (size_t l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
	{
		TotalArea += static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[CurrentNode->TrianglesInCell[l]]);
	}


	auto CalculateCellRugosity = [&](const glm::vec3 PointOnPlane, const glm::vec3 PlaneNormal) {
		double Result = 0.0;
		const FEPlane* ProjectionPlane = new FEPlane(PointOnPlane, PlaneNormal);

		std::vector<float> Rugosities;
		for (int l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
		{
			std::vector<glm::vec3> CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[CurrentNode->TrianglesInCell[l]];

			glm::vec3 AProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[0]);
			glm::vec3 BProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[1]);
			glm::vec3 CProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[2]);

			const double ProjectionArea = SDF::TriangleArea(AProjection, BProjection, CProjection);
			const double OriginalArea = MESH_MANAGER.ActiveMesh->TrianglesArea[CurrentNode->TrianglesInCell[l]];
			Rugosities.push_back(static_cast<float>(OriginalArea / ProjectionArea));

			if (OriginalArea == 0.0 || ProjectionArea == 0.0)
				Rugosities.back() = 1.0f;

			if (Rugosities.back() > 100.0f)
				Rugosities.back() = 100.0f;
		}

		// Weighted by triangle area rugosity.
		for (int l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
		{
			const float CurrentTriangleCoef = static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[CurrentNode->TrianglesInCell[l]] / TotalArea);

			Result += Rugosities[l] * CurrentTriangleCoef;

			if (isnan(Result))
				Result = 1.0f;
		}

		delete ProjectionPlane;
		return Result;
	};

	if (RUGOSITY_MANAGER.bUseCGALVariant)
	{
		std::vector<float> FEVerticesFinal;
		std::vector<int> FEIndicesFinal;

		for (int l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
		{
			const int TriangleIndex = CurrentNode->TrianglesInCell[l];

			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][0][0]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][0][1]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][0][2]);
			FEIndicesFinal.push_back(l * 3);

			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][1][0]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][1][1]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][1][2]);
			FEIndicesFinal.push_back(l * 3 + 1);

			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][2][0]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][2][1]);
			FEVerticesFinal.push_back(MESH_MANAGER.ActiveMesh->Triangles[TriangleIndex][2][2]);
			FEIndicesFinal.push_back(l * 3 + 2);
		}

		// Formating data to CGAL format.
		std::vector<Polygon_3> CGALFaces;
		CGALFaces.resize(FEIndicesFinal.size() / 3);
		int count = 0;
		for (size_t i = 0; i < FEIndicesFinal.size(); i += 3)
		{
			CGALFaces[count].push_back(FEIndicesFinal[i]);
			CGALFaces[count].push_back(FEIndicesFinal[i + 1]);
			CGALFaces[count].push_back(FEIndicesFinal[i + 2]);
			count++;
		}

		std::vector<Point_3> CGALPoints;
		for (size_t i = 0; i < FEVerticesFinal.size(); i += 3)
		{
			CGALPoints.push_back(Point_3(FEVerticesFinal[i], FEVerticesFinal[i + 1], FEVerticesFinal[i + 2]));
		}

		Surface_mesh result;

		if (!PMP::is_polygon_soup_a_polygon_mesh(CGALFaces))
		{
			PMP::repair_polygon_soup(CGALPoints, CGALFaces);
			PMP::orient_polygon_soup(CGALPoints, CGALFaces);
		}

		PMP::polygon_soup_to_polygon_mesh(CGALPoints, CGALFaces, result);

		Kernel::Plane_3 plane;
		Kernel::Point_3 centroid;

		Kernel::FT quality = linear_least_squares_fitting_3(result.points().begin(), result.points().end(), plane, centroid,
			CGAL::Dimension_tag<0>());


		const auto CGALNormal = plane.perpendicular_line(centroid);

		glm::vec3 Normal = glm::vec3(CGALNormal.direction().vector().x(),
			CGALNormal.direction().vector().y(),
			CGALNormal.direction().vector().z());

		Normal = glm::normalize(Normal);

		CurrentNode->Rugosity = CalculateCellRugosity(CurrentNode->CellTrianglesCentroid, Normal);

		return;
	}

	// ******* Getting average normal *******
	for (size_t l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
	{
		std::vector<glm::vec3> CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[CurrentNode->TrianglesInCell[l]];
		std::vector<glm::vec3> CurrentTriangleNormals = MESH_MANAGER.ActiveMesh->TrianglesNormals[CurrentNode->TrianglesInCell[l]];

		if (RUGOSITY_MANAGER.bWeightedNormals)
		{
			const float CurrentTriangleCoef = static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[CurrentNode->TrianglesInCell[l]] / TotalArea);

			CurrentNode->AverageCellNormal += CurrentTriangleNormals[0] * CurrentTriangleCoef;
			CurrentNode->AverageCellNormal += CurrentTriangleNormals[1] * CurrentTriangleCoef;
			CurrentNode->AverageCellNormal += CurrentTriangleNormals[2] * CurrentTriangleCoef;
		}
		else
		{
			CurrentNode->AverageCellNormal += CurrentTriangleNormals[0];
			CurrentNode->AverageCellNormal += CurrentTriangleNormals[1];
			CurrentNode->AverageCellNormal += CurrentTriangleNormals[2];
		}

		CurrentNode->CellTrianglesCentroid += CurrentTriangle[0];
		CurrentNode->CellTrianglesCentroid += CurrentTriangle[1];
		CurrentNode->CellTrianglesCentroid += CurrentTriangle[2];
	}

	if (!RUGOSITY_MANAGER.bWeightedNormals)
		CurrentNode->AverageCellNormal /= CurrentNode->TrianglesInCell.size() * 3;

	if (RUGOSITY_MANAGER.bNormalizedNormals)
		CurrentNode->AverageCellNormal = glm::normalize(CurrentNode->AverageCellNormal);
	CurrentNode->CellTrianglesCentroid /= CurrentNode->TrianglesInCell.size() * 3;
	// ******* Getting average normal END *******

	if (RUGOSITY_MANAGER.bUseFindSmallestRugosity)
	{
		std::unordered_map<int, float> TriangleNormalsToRugosity;
		TriangleNormalsToRugosity[-1] = static_cast<float>(CalculateCellRugosity(CurrentNode->CellTrianglesCentroid, CurrentNode->AverageCellNormal));

		for (int i = 0; i < SphereVectors.size(); i++)
		{
			TriangleNormalsToRugosity[i] = static_cast<float>(CalculateCellRugosity(glm::vec3(0.0f), SphereVectors[i]));
		}

		double Min = FLT_MAX;
		double Max = -FLT_MAX;
		auto MapIt = TriangleNormalsToRugosity.begin();
		while (MapIt != TriangleNormalsToRugosity.end())
		{
			if (MapIt->second < Min)
			{
				Min = MapIt->second;
				CurrentNode->Rugosity = Min;
			}

			MapIt++;
		}
	}
	else
	{
		CurrentNode->Rugosity = CalculateCellRugosity(CurrentNode->CellTrianglesCentroid, CurrentNode->AverageCellNormal);
		if (isnan(CurrentNode->Rugosity))
			CurrentNode->Rugosity = 1.0f;
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
	TIME.BeginTimeStamp("Calculate rugosity");
	result->RunOnAllNodes(CalculateOneNodeRugosity);
	result->TimeTookCalculateRugosity = TIME.EndTimeStamp("Calculate rugosity");
	//result->CalculateRugosity();

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

	FEAABB finalAABB = Input->Mesh->AABB;

	glm::mat4 transformMatrix = glm::identity<glm::mat4>();
	if (Input->UseJitterExpandedAABB)
	{
		transformMatrix = glm::scale(transformMatrix, glm::vec3(Input->GridScale));
		finalAABB = finalAABB.transform(transformMatrix);
	}

	const float cellSize = finalAABB.getSize() / Input->Dimentions;

	const glm::vec3 center = Input->Mesh->AABB.getCenter() + glm::vec3(Input->ShiftX, Input->ShiftY, Input->ShiftZ) * cellSize;
	const FEAABB SDFAABB = FEAABB(center - glm::vec3(finalAABB.getSize() / 2.0f), center + glm::vec3(finalAABB.getSize() / 2.0f));
	finalAABB = SDFAABB;

	Output->Init(0/*Input->dimentions*/, finalAABB, RUGOSITY_MANAGER.currentCamera, RUGOSITY_MANAGER.ResolutonInM);

	Output->bFindSmallestRugosity = RUGOSITY_MANAGER.bUseFindSmallestRugosity;
	Output->bCGALVariant = RUGOSITY_MANAGER.bUseCGALVariant;
	Output->bWeightedNormals = RUGOSITY_MANAGER.bWeightedNormals;
	Output->bNormalizedNormals = RUGOSITY_MANAGER.bNormalizedNormals;

	Output->FillCellsWithTriangleInfo();
	TIME.BeginTimeStamp("Calculate rugosity");
	Output->RunOnAllNodes(CalculateOneNodeRugosity);
	Output->TimeTookCalculateRugosity = TIME.EndTimeStamp("Calculate rugosity");
	//Output->CalculateRugosity();

	Output->FillMeshWithRugosityData();
	Output->bFullyLoaded = true;
}

void RugosityManager::RunCreationOfSDFAsync(bool bJitter)
{
	//OnRugosityCalculationsStart();

	SDFInitData* InputData = new SDFInitData();
	InputData->Dimentions = SDFDimention;
	InputData->Mesh = MESH_MANAGER.ActiveMesh;
	InputData->UseJitterExpandedAABB = bJitter;

	InputData->ShiftX = RUGOSITY_MANAGER.shiftX;
	InputData->ShiftY = RUGOSITY_MANAGER.shiftY;
	InputData->ShiftZ = RUGOSITY_MANAGER.shiftZ;
	InputData->GridScale = RUGOSITY_MANAGER.GridScale;

	SDF* OutputData = new SDF();
	OutputData->bFindSmallestRugosity = RUGOSITY_MANAGER.bUseFindSmallestRugosity;
	OutputData->bCGALVariant = RUGOSITY_MANAGER.bUseCGALVariant;
	currentSDF = OutputData;
#ifdef NODE_PER_THREAD
	calculateSDFAsync(InputData, OutputData);
	calculateSDFCallback(OutputData);
#else
	THREAD_POOL.Execute(calculateSDFAsync, InputData, OutputData, calculateSDFCallback);
#endif
}

void RugosityManager::CalculateRugorsityWithJitterAsync(int RugosityLayerIndex)
{
	OnRugosityCalculationsStart();

	RUGOSITY_MANAGER.RugosityLayerIndex = RugosityLayerIndex;

	//RunCreationOfSDFAsync(true);

	// In cells
	float KernelSize = 0.5;
	KernelSize *= 2.0f;
	KernelSize *= 100.0f;

	/*struct GridModifications
	{
		float ShiftX = 0.0f;
		float ShiftY = 0.0f;
		float ShiftZ = 0.0f;

		float GridScale = 2.5f;
	};
	std::vector<GridModifications> Modifications;*/

	static std::vector<float> Modifications = {
		0.300000f, 0.100000f, 0.020000f, 1.430000f,
		0.450000f, 0.130000f, 0.090000f, 1.410000f,
		0.410000f, -0.310000f, 0.300000f, 1.390000f,
		0.340000f, 0.130000f, 0.470000f, 1.330000f,
		-0.390000f, 0.060000f, -0.440000f, 1.400000f,
		-0.350000f, -0.020000f, 0.380000f, 1.340000f,
		-0.300000f, 0.480000f, -0.500000f, 1.280000f,
		-0.390000f, 0.060000f, 0.370000f, 1.290000f,
		-0.120000f, 0.270000f, 0.300000f, 1.300000f,
		0.030000f, -0.060000f, -0.290000f, 1.410000f,
		-0.240000f, -0.130000f, -0.130000f, 1.250000f,
		0.220000f, -0.320000f, 0.160000f, 1.300000f,
		-0.360000f, -0.010000f, -0.190000f, 1.270000f,
		-0.200000f, -0.220000f, -0.140000f, 1.440000f,
		0.340000f, -0.100000f, -0.060000f, 1.280000f,
		-0.210000f, 0.290000f, 0.090000f, 1.270000f,
		0.130000f, 0.010000f, 0.180000f, 1.250000f,
		0.090000f, 0.460000f, 0.350000f, 1.460000f,
		-0.500000f, -0.090000f, 0.320000f, 1.250000f,
		-0.200000f, 0.080000f, -0.320000f, 1.260000f,
		-0.390000f, -0.440000f, 0.420000f, 1.410000f,
		0.260000f, -0.040000f, 0.400000f, 1.440000f,
		-0.020000f, -0.010000f, 0.290000f, 1.400000f,
		-0.020000f, -0.370000f, 0.280000f, 1.390000f,
		0.240000f, -0.320000f, -0.290000f, 1.410000f,
		-0.100000f, 0.360000f, -0.110000f, 1.420000f,
		-0.270000f, -0.490000f, 0.470000f, 1.250000f,
		-0.450000f, 0.390000f, -0.310000f, 1.280000f,
		0.440000f, -0.290000f, -0.360000f, 1.460000f,
		0.000000f, -0.050000f, -0.370000f, 1.400000f,
		-0.060000f, 0.120000f, 0.430000f, 1.270000f,
		-0.490000f, 0.410000f, 0.150000f, 1.270000f,
		-0.500000f, 0.370000f, -0.040000f, 1.390000f,
		0.380000f, -0.210000f, 0.040000f, 1.320000f,
		0.280000f, 0.340000f, -0.430000f, 1.270000f,
		-0.410000f, -0.450000f, 0.330000f, 1.430000f,
		-0.180000f, -0.370000f, 0.070000f, 1.280000f,
		-0.110000f, -0.310000f, 0.400000f, 1.300000f,
		-0.060000f, 0.270000f, 0.240000f, 1.320000f,
		-0.390000f, 0.480000f, 0.180000f, 1.260000f,
		-0.390000f, 0.100000f, 0.070000f, 1.330000f,
		-0.260000f, -0.230000f, 0.450000f, 1.430000f,
		0.480000f, 0.080000f, -0.140000f, 1.470000f,
		0.330000f, -0.380000f, -0.350000f, 1.310000f,
		0.180000f, 0.200000f, -0.330000f, 1.490000f,
		-0.230000f, -0.110000f, 0.070000f, 1.250000f,
		0.190000f, -0.230000f, -0.330000f, 1.480000f,
		0.480000f, 0.150000f, 0.290000f, 1.410000f,
		-0.210000f, 0.240000f, 0.180000f, 1.320000f,
		0.450000f, -0.230000f, 0.240000f, 1.480000f,
		0.400000f, 0.370000f, -0.060000f, 1.450000f,
		0.080000f, -0.500000f, 0.200000f, 1.370000f,
		-0.240000f, -0.380000f, 0.440000f, 1.430000f,
		-0.230000f, -0.210000f, 0.240000f, 1.390000f,
		-0.320000f, -0.050000f, 0.330000f, 1.370000f,
		-0.030000f, -0.470000f, -0.320000f, 1.480000f,
		0.320000f, 0.160000f, 0.460000f, 1.460000f,
		0.270000f, 0.430000f, 0.410000f, 1.380000f,
		0.240000f, -0.470000f, -0.190000f, 1.430000f,
		0.480000f, -0.420000f, -0.240000f, 1.490000f,
		-0.400000f, -0.270000f, 0.060000f, 1.450000f,
		0.280000f, 0.430000f, -0.010000f, 1.490000f,
		-0.190000f, 0.110000f, -0.100000f, 1.450000f,
		-0.210000f, -0.040000f, 0.340000f, 1.440000f,
	};

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

	std::vector<float> ModificationsTest;
	float Step = 0.3f;
	float Start = -0.2f;

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				ModificationsTest.push_back(Start + Step * i);
				ModificationsTest.push_back(Start + Step * j);
				ModificationsTest.push_back(Start + Step * k);
				ModificationsTest.push_back(1.0f /*+ Step / 3 * i * j * k*/);
			}
		}
	}

	std::vector<float>* WhatToUse = &Modifications;
	/*if (bTestJitter)
		WhatToUse = &ModificationsTest;
	if (bTestSphereJitter)
		WhatToUse = &SphereJitter;*/

	// This jitter is stable and more easy to explain.
	WhatToUse = &SphereJitter;
	JitterToDoCount = WhatToUse->size() / 4;

	for (size_t i = 0; i < JitterToDoCount; i++)
	{
		/*RUGOSITY_MANAGER.shiftX = rand() % int(KernelSize);
		RUGOSITY_MANAGER.shiftX -= KernelSize / 2.0f;
		RUGOSITY_MANAGER.shiftX /= 100.0f;

		RUGOSITY_MANAGER.shiftY = rand() % int(KernelSize);
		RUGOSITY_MANAGER.shiftY -= KernelSize / 2.0f;
		RUGOSITY_MANAGER.shiftY /= 100.0f;

		RUGOSITY_MANAGER.shiftZ = rand() % int(KernelSize);
		RUGOSITY_MANAGER.shiftZ -= KernelSize / 2.0f;
		RUGOSITY_MANAGER.shiftZ /= 100.0f;

		RUGOSITY_MANAGER.GridScale = DEFAULT_GRID_SIZE;
		float TempGridScale = rand() % GRID_VARIANCE;
		TempGridScale /= 100.0f;
		RUGOSITY_MANAGER.GridScale += TempGridScale;*/

		RUGOSITY_MANAGER.shiftX = WhatToUse->operator[](i * 4);
		RUGOSITY_MANAGER.shiftY = WhatToUse->operator[](i * 4 + 1);
		RUGOSITY_MANAGER.shiftZ = WhatToUse->operator[](i * 4 + 2);
		RUGOSITY_MANAGER.GridScale = WhatToUse->operator[](i * 4 + 3);

		/*GridModifications New;
		New.ShiftX = RUGOSITY_MANAGER.shiftX;
		New.ShiftY = RUGOSITY_MANAGER.shiftY;
		New.ShiftZ = RUGOSITY_MANAGER.shiftZ;

		New.GridScale = RUGOSITY_MANAGER.GridScale;

		Modifications.push_back(New);

		Result += std::to_string(New.ShiftX) + ", " + std::to_string(New.ShiftY) + ", " + std::to_string(New.ShiftZ) + ", " + std::to_string(New.GridScale) + ",\n";*/

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
	
	std::string DeleteOutliers = "No";
	// Remove outliers.
	if (RUGOSITY_MANAGER.bDeleteOutliers)
	{
		DeleteOutliers = "Yes";
		float OutlierBeginValue = FLT_MAX;

		std::vector<float> SortedData = NewLayer.TrianglesToData;
		std::sort(SortedData.begin(), SortedData.end());

		int OutlierBeginPosition = SortedData.size() * 0.99;
		OutlierBeginValue = SortedData[OutlierBeginPosition];
		float NewMax = SortedData[OutlierBeginPosition - 1];

		for (int i = 0; i < NewLayer.TrianglesToData.size(); i++)
		{
			if (NewLayer.TrianglesToData[i] >= OutlierBeginValue)
				NewLayer.TrianglesToData[i] = NewMax;
		}
	}
	NewLayer.DebugInfo->AddEntry("Delete outliers", DeleteOutliers);

	LastTimeTookForCalculation = float(TIME.EndTimeStamp("CalculateRugorsityTotal"));

	if (OnRugosityCalculationsEndCallbackImpl != nullptr)
		OnRugosityCalculationsEndCallbackImpl(NewLayer);
}

void RugosityManager::SetOnRugosityCalculationsEndCallback(void(*Func)(MeshLayer))
{
	OnRugosityCalculationsEndCallbackImpl = Func;
}