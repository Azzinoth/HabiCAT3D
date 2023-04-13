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

	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

RugosityManager::~RugosityManager() {}

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

		CurrentNode->UserData = CalculateCellRugosity(CurrentNode->CellTrianglesCentroid, Normal);

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
				CurrentNode->UserData = Min;
			}

			MapIt++;
		}
	}
	else
	{
		CurrentNode->UserData = CalculateCellRugosity(CurrentNode->CellTrianglesCentroid, CurrentNode->AverageCellNormal);
		if (isnan(CurrentNode->UserData))
			CurrentNode->UserData = 1.0f;
	}
}

void RugosityManager::CalculateRugorsityWithJitterAsync(int RugosityLayerIndex)
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	RUGOSITY_MANAGER.bWaitForJitterResult = true;
	JITTER_MANAGER.CalculateWithSDFJitterAsync(CalculateOneNodeRugosity);
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

	RUGOSITY_MANAGER.bWaitForJitterResult = true;

	TIME.BeginTimeStamp("CalculateRugorsityTotal");
	RUGOSITY_MANAGER.StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	if (OnRugosityCalculationsStartCallbackImpl != nullptr)
		OnRugosityCalculationsStartCallbackImpl();
}

void RugosityManager::SetOnRugosityCalculationsEndCallback(void(*Func)(MeshLayer))
{
	OnRugosityCalculationsEndCallbackImpl = Func;
}

void RugosityManager::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	if (!RUGOSITY_MANAGER.bWaitForJitterResult)
		return;

	RUGOSITY_MANAGER.bWaitForJitterResult = false;
	NewLayer.DebugInfo->Type = "RugosityMeshLayerDebugInfo";

	std::string AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[0];
	if (RUGOSITY_MANAGER.bUseFindSmallestRugosity)
		AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[1];

	if (RUGOSITY_MANAGER.bUseCGALVariant)
		AlgorithmUsed = RUGOSITY_MANAGER.RugosityAlgorithmList[2];

	NewLayer.DebugInfo->AddEntry("Algorithm used", AlgorithmUsed);

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