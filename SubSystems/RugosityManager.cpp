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

	OrientationSetNamesForMinRugosityList.push_back("9");
	OrientationSetNamesForMinRugosityList.push_back("19");
	OrientationSetNamesForMinRugosityList.push_back("33");
	OrientationSetNamesForMinRugosityList.push_back("51");
	OrientationSetNamesForMinRugosityList.push_back("73");
	OrientationSetNamesForMinRugosityList.push_back("91");
	OrientationSetNamesForMinRugosityList.push_back("99");
	OrientationSetNamesForMinRugosityList.push_back("129");
	OrientationSetNamesForMinRugosityList.push_back("163");
	OrientationSetNamesForMinRugosityList.push_back("201");
	OrientationSetNamesForMinRugosityList.push_back("289");
	OrientationSetNamesForMinRugosityList.push_back("339");
	OrientationSetNamesForMinRugosityList.push_back("393");
	OrientationSetNamesForMinRugosityList.push_back("441");

	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);
}

RugosityManager::~RugosityManager() {}

typedef CGAL::Exact_predicates_exact_constructions_kernel  Kernel2;
typedef Kernel2::Point_2                                   Point_2;
typedef CGAL::Polygon_2<Kernel2>                           Polygon_2;
typedef std::vector<Polygon_2>                             Polygon_vector;
typedef CGAL::Polygon_set_2<Kernel2>                       Polygon_set_2;

double calculate_area(const Polygon_set_2& polygon_set)
{
	//TIME.BeginTimeStamp("calculate_area time");

	typedef CGAL::Polygon_with_holes_2<Kernel2> Polygon_with_holes_2;
	typedef std::vector<Polygon_with_holes_2>   Pwh_vector;

	double area = 0;
	Pwh_vector result_polygons;
	polygon_set.polygons_with_holes(std::back_inserter(result_polygons));

	for (const Polygon_with_holes_2& polygon : result_polygons)
	{
		area += CGAL::to_double(polygon.outer_boundary().area());
		for (auto it = polygon.holes_begin(); it != polygon.holes_end(); ++it)
		{
			area -= CGAL::to_double(it->area());
		}
	}

	/*double calculate_area_time = TIME.EndTimeStamp("calculate_area time");
	LOG.Add(std::to_string(calculate_area_time), "calculate_area");
	LOG.Add("========================================================", "calculate_area");*/

	return area;
}

void create_local_coordinate_system(const glm::dvec3& normal, glm::dvec3& u, glm::dvec3& v)
{
	glm::dvec3 temp(1, 0, 0);
	if (glm::length(glm::cross(normal, temp)) < 0.01) {
		temp = glm::dvec3(0, 1, 0);
	}
	u = glm::normalize(glm::cross(normal, temp));
	v = glm::cross(normal, u);
}

Point_2 project_to_local_coordinates(const glm::dvec3& point, const glm::dvec3& u, const glm::dvec3& v)
{
	double x = glm::dot(point, u);
	double y = glm::dot(point, v);
	return Point_2(x, y);
}

Polygon_2 create_2d_triangle(const glm::dvec3& AProjection, const glm::dvec3& BProjection, const glm::dvec3& CProjection, const glm::dvec3& normal)
{
	glm::dvec3 u, v;
	create_local_coordinate_system(normal, u, v);

	Polygon_2 triangle;
	triangle.push_back(project_to_local_coordinates(AProjection, u, v));
	triangle.push_back(project_to_local_coordinates(BProjection, u, v));
	triangle.push_back(project_to_local_coordinates(CProjection, u, v));
	return triangle;
}

#define CGAL_FOR_PROJECTION

#ifdef CGAL_FOR_PROJECTION
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Plane_3.h>

typedef Kernel::Point_3 Point_3;
typedef Kernel::Vector_3 Vector_3;
typedef Kernel::Plane_3 Plane_3;
#endif

void RugosityManager::CalculateOneNodeRugosity(SDFNode* CurrentNode)
{
	if (CurrentNode->TrianglesInCell.empty())
		return;

	TIME.BeginTimeStamp("CalculateOneNodeRugosity time");

	float TotalArea = 0.0f;
	for (size_t l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
	{
		TotalArea += static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[CurrentNode->TrianglesInCell[l]]);
	}

	float CGALCorrectTotalArea = TotalArea;

	auto CalculateCellRugosity = [&](const glm::vec3 PointOnPlane, const glm::vec3 PlaneNormal) {
		double Result = 0.0;
		const FEPlane* ProjectionPlane = new FEPlane(PointOnPlane, PlaneNormal);
#ifdef CGAL_FOR_PROJECTION
		Point_3 point_on_plane(0.0, 0.0, 0.0);
		Vector_3 plane_normal(PlaneNormal.x, PlaneNormal.y, PlaneNormal.z);
		Plane_3 plane(point_on_plane, plane_normal);

		glm::dvec3 base1 = glm::dvec3(plane.base1().x(), plane.base1().y(), plane.base1().z());
		glm::dvec3 base2 = glm::dvec3(plane.base2().x(), plane.base2().y(), plane.base2().z());
#endif

		if (RUGOSITY_MANAGER.bOverlapAware)
		{
			CGALCorrectTotalArea = TotalArea;

			//TIME.BeginTimeStamp("CalculateCellRugosity time");
			glm::dvec3 u, v;
			create_local_coordinate_system(PlaneNormal, u, v);

			Polygon_vector Triangles;
			for (int i = 0; i < CurrentNode->TrianglesInCell.size(); i++)
			{
				if (MESH_MANAGER.ActiveMesh->TrianglesArea[CurrentNode->TrianglesInCell[i]] == 0.0)
					continue;

				std::vector<glm::vec3> CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[CurrentNode->TrianglesInCell[i]];
				Polygon_2 TempTriangle;

#ifndef CGAL_FOR_PROJECTION
				glm::dvec3 AProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[0]);
				glm::dvec3 BProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[1]);
				glm::dvec3 CProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[2]);

				TempTriangle.push_back(project_to_local_coordinates(AProjection, u, v));
				TempTriangle.push_back(project_to_local_coordinates(BProjection, u, v));
				TempTriangle.push_back(project_to_local_coordinates(CProjection, u, v));

#else CGAL_FOR_PROJECTION
				for (size_t j = 0; j < CurrentTriangle.size(); j++)
				{
					Point_3 Projection_CGAL_3D = plane.projection(Point_3(CurrentTriangle[j].x, CurrentTriangle[j].y, CurrentTriangle[j].z));

					double X = glm::dot(glm::dvec3(Projection_CGAL_3D.x(), Projection_CGAL_3D.y(), Projection_CGAL_3D.z()), base1);
					double Y = glm::dot(glm::dvec3(Projection_CGAL_3D.x(), Projection_CGAL_3D.y(), Projection_CGAL_3D.z()), base2);

					Point_2 AProjection_CGAL_2D(X, Y);
					TempTriangle.push_back(AProjection_CGAL_2D);
				}
#endif

				if (!CGAL::is_ccw_strongly_convex_2(TempTriangle.vertices_begin(), TempTriangle.vertices_end()))
					TempTriangle.reverse_orientation();

				if (TempTriangle.area() == 0.0)
				{
					CGALCorrectTotalArea -= static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[CurrentNode->TrianglesInCell[i]]);
				}
				else
				{
					Triangles.push_back(TempTriangle);
				}
			}

			/*double TimeTookCalculateRugosity = TIME.EndTimeStamp("CalculateCellRugosity time");
			LOG.Add(std::to_string(TimeTookCalculateRugosity), "CalculateCellRugosity");
			LOG.Add("triangle count: " + std::to_string(CurrentNode->TrianglesInCell.size()), "CalculateCellRugosity");
			LOG.Add("========================================================", "CalculateCellRugosity");*/


			//TIME.BeginTimeStamp("TriangleGroups time");
			Polygon_set_2 TriangleGroup;
			std::vector<int> CGALFailedIndexes;

			std::vector<int> Ranges;
			try
			{
				TriangleGroup.join(Triangles.begin(), Triangles.end());
			}
			catch (...)
			{
				for (size_t i = 0; i < Triangles.size(); i++)
				{
					try
					{
						TriangleGroup.join(Triangles[i]);
					}
					catch (...)
					{
						CGALFailedIndexes.push_back(i);
					}
				}

				for (size_t i = 0; i < CGALFailedIndexes.size(); i++)
				{
					CGALCorrectTotalArea -= static_cast<float>(MESH_MANAGER.ActiveMesh->TrianglesArea[CurrentNode->TrianglesInCell[CGALFailedIndexes[i]]]);
				}
			}

			/*double TimeTookCalculateRugosity2 = TIME.EndTimeStamp("TriangleGroups time");
			LOG.Add(std::to_string(TimeTookCalculateRugosity2), "TriangleGroups");
			LOG.Add("triangle count: " + std::to_string(CurrentNode->TrianglesInCell.size()), "TriangleGroups");
			LOG.Add("========================================================", "TriangleGroups");*/

			double TotalProjectedArea = 0.0;
			TotalProjectedArea = calculate_area(TriangleGroup);

			if (TotalProjectedArea == 0)
			{
				Result = 1.0f;
			}
			else
			{
				Result = CGALCorrectTotalArea / TotalProjectedArea;
			}
			
			if (isnan(Result))
				Result = 1.0f;
		}
		else
		{
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

		if (OrientationSetOptions.find(RUGOSITY_MANAGER.OrientationSetForMinRugosity) == OrientationSetOptions.end())
			RUGOSITY_MANAGER.OrientationSetForMinRugosity = "91";

		std::vector<glm::vec3> OrientationSet = OrientationSetOptions[RUGOSITY_MANAGER.OrientationSetForMinRugosity];
		for (int i = 0; i < OrientationSet.size(); i++)
		{
			TriangleNormalsToRugosity[i] = static_cast<float>(CalculateCellRugosity(glm::vec3(0.0f), OrientationSet[i]));
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

	/*double TimeTookCalculateRugosity = TIME.EndTimeStamp("CalculateOneNodeRugosity time");
	LOG.Add(std::to_string(TimeTookCalculateRugosity), "CalculateOneNodeRugosity");
	LOG.Add("triangle count: " + std::to_string(CurrentNode->TrianglesInCell.size()), "CalculateOneNodeRugosity");
	LOG.Add("========================================================", "CalculateOneNodeRugosity");*/
}

void RugosityManager::CalculateRugorsityWithJitterAsync()
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	RUGOSITY_MANAGER.bWaitForJitterResult = true;
	JITTER_MANAGER.CalculateWithSDFJitterAsync(CalculateOneNodeRugosity);
}

void RugosityManager::CalculateOnWholeModel()
{
	if (MESH_MANAGER.ActiveMesh == nullptr)
		return;

	uint64_t StarTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	RUGOSITY_MANAGER.bWaitForJitterResult = true;
	JITTER_MANAGER.CalculateOnWholeModel(CalculateOneNodeRugosity);
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

	if (AlgorithmUsed == "Min Rugosity")
		NewLayer.DebugInfo->AddEntry("Orientation set name", RUGOSITY_MANAGER.GetOrientationSetForMinRugosityName());
	
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


	std::string OverlapAware = "No";
	if (RUGOSITY_MANAGER.bOverlapAware)
		OverlapAware = "Yes";
	NewLayer.DebugInfo->AddEntry("Unique projected area (very slow)", OverlapAware);

	LastTimeTookForCalculation = float(TIME.EndTimeStamp("CalculateRugorsityTotal"));

	if (OnRugosityCalculationsEndCallbackImpl != nullptr)
		OnRugosityCalculationsEndCallbackImpl(NewLayer);
}

void RugosityManager::RenderDebugInfoForSelectedNode(SDF* Grid)
{
	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	Grid->UpdateRenderedLines();

	SDFNode* CurrentlySelectedCell = &Grid->Data[int(Grid->SelectedCell.x)][int(Grid->SelectedCell.y)][int(Grid->SelectedCell.z)];
	for (size_t i = 0; i < 3/*CurrentlySelectedCell->TrianglesInCell.size()*/; i++)
	{
		const auto CurrentTriangle = MESH_MANAGER.ActiveMesh->Triangles[CurrentlySelectedCell->TrianglesInCell[i]];

		std::vector<glm::vec3> TranformedTrianglePoints = CurrentTriangle;
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = MESH_MANAGER.ActiveMesh->Position->getTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FELine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
	}

	LINE_RENDERER.SyncWithGPU();
}

std::string RugosityManager::GetOrientationSetForMinRugosityName()
{
	return OrientationSetForMinRugosity;
}

void RugosityManager::SetOrientationSetForMinRugosityName(std::string name)
{
	if (OrientationSetOptions.find(RUGOSITY_MANAGER.OrientationSetForMinRugosity) != OrientationSetOptions.end())
		OrientationSetForMinRugosity = name;
}