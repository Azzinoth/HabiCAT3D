#include "RugosityLayerProducer.h"
using namespace FocalEngine;

RugosityLayerProducer* RugosityLayerProducer::Instance = nullptr;
float RugosityLayerProducer::LastTimeTookForCalculation = 0.0f;
void(*RugosityLayerProducer::OnRugosityCalculationsStartCallbackImpl)(void) = nullptr;
void(*RugosityLayerProducer::OnRugosityCalculationsEndCallbackImpl)(MeshLayer) = nullptr;

RugosityLayerProducer::RugosityLayerProducer()
{
	RugosityAlgorithmList.push_back("Average normal");
	RugosityAlgorithmList.push_back("Min Rugosity(default)");
	RugosityAlgorithmList.push_back("Least square fitting");

	OrientationSetNamesForMinRugosityList.push_back("1");
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

RugosityLayerProducer::~RugosityLayerProducer() {}

bool RugosityLayerProducer::ShouldDeletedOutlier()
{
	return bDeleteOutliers;
}

void RugosityLayerProducer::SetDeleteOutliers(bool NewValue)
{
	bDeleteOutliers = NewValue;
}

bool RugosityLayerProducer::ShouldCalculateStandardDeviation()
{
	return bCalculateStandardDeviation;
}

void RugosityLayerProducer::SetCalculateStandardDeviation(bool NewValue)
{
	bCalculateStandardDeviation = NewValue;
}

double RugosityLayerProducer::CGALCalculateArea(const Polygon_set_2& PolygonSet)
{
	typedef CGAL::Polygon_with_holes_2<Kernel2> Polygon_with_holes_2;
	typedef std::vector<Polygon_with_holes_2>   Pwh_vector;

	double Area = 0;
	Pwh_vector ResultPolygons;
	PolygonSet.polygons_with_holes(std::back_inserter(ResultPolygons));

	for (const Polygon_with_holes_2& Polygon : ResultPolygons)
	{
		Area += CGAL::to_double(Polygon.outer_boundary().area());
		for (auto it = Polygon.holes_begin(); it != Polygon.holes_end(); ++it)
		{
			Area -= CGAL::to_double(it->area());
		}
	}

	return Area;
}

void RugosityLayerProducer::CreateLocalCoordinateSystem(const glm::dvec3& Normal, glm::dvec3& U, glm::dvec3& V)
{
	glm::dvec3 Temp(1, 0, 0);
	if (glm::length(glm::cross(Normal, Temp)) < 0.01)
	{
		Temp = glm::dvec3(0, 1, 0);
	}

	U = glm::normalize(glm::cross(Normal, Temp));
	V = glm::cross(Normal, U);
}

Point_2 RugosityLayerProducer::ProjectToLocalCoordinates(const glm::dvec3& Point, const glm::dvec3& U, const glm::dvec3& V)
{
	double X = glm::dot(Point, U);
	double Y = glm::dot(Point, V);
	return Point_2(X, Y);
}

Point_2 RugosityLayerProducer::ProjectPointOntoPlane(const Point_3& Point, const Plane_3& Plane)
{
	// Project the point onto the plane
	Point_3 ProjectedPoint = Plane.projection(Point);

	// Construct an orthonormal basis for the plane
	Vector_3 Base1 = Plane.base1();
	Vector_3 Base2 = Plane.base2();

	// Express the projected point in the local coordinate system of the plane
	Vector_3 DifferenceVector = ProjectedPoint - Plane.point();
	double X = DifferenceVector * Base1;
	double Y = DifferenceVector * Base2;

	return Point_2(X, Y);
}

void RugosityLayerProducer::CalculateOneNodeRugosity(GridNode* CurrentNode)
{
	if (CurrentNode->TrianglesInCell.empty())
		return;

	float TotalArea = 0.0f;
	for (size_t l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
	{
		TotalArea += static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[l]]);
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

		if (RUGOSITY_LAYER_PRODUCER.bUniqueProjectedArea)
		{
			CGALCorrectTotalArea = TotalArea;
			double TotalProjectedArea = 0.0;

			if (RUGOSITY_LAYER_PRODUCER.bUniqueProjectedAreaApproximation)
			{
				Point_3 PointA(0.0f, 0.0f, 0.0f);
				Plane_3 PlaneToProjectOnto(PointA, Vector_3(PlaneNormal.x, PlaneNormal.y, PlaneNormal.z));

				std::vector<Point_2> ProjectedPoints, ConvexHullOfProjectedPoints;
				for (int i = 0; i < CurrentNode->TrianglesInCell.size(); i++)
				{
					if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[i]] == 0.0)
						continue;

					std::vector<glm::dvec3> CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[CurrentNode->TrianglesInCell[i]];
					for (size_t j = 0; j < CurrentTriangle.size(); j++)
					{
						ProjectedPoints.push_back(RUGOSITY_LAYER_PRODUCER.ProjectPointOntoPlane(Point_3(CurrentTriangle[j].x, CurrentTriangle[j].y, CurrentTriangle[j].z), PlaneToProjectOnto));
					}
				}

				CGAL::convex_hull_2(ProjectedPoints.begin(), ProjectedPoints.end(), std::back_inserter(ConvexHullOfProjectedPoints));

				Polygon_2 Polygon;
				for (const auto& CurrentPoint : ConvexHullOfProjectedPoints)
					Polygon.push_back(CurrentPoint);

				TotalProjectedArea = abs(CGAL::to_double(Polygon.area()));
			}
			else
			{
				Polygon_vector Triangles;

				for (int i = 0; i < CurrentNode->TrianglesInCell.size(); i++)
				{
					if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[i]] == 0.0)
						continue;

					std::vector<glm::dvec3> CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[CurrentNode->TrianglesInCell[i]];

					Polygon_2 TempTriangle;
					
#ifndef CGAL_FOR_PROJECTION
					glm::dvec3 U, V;
					RUGOSITY_LAYER_PRODUCER.CreateLocalCoordinateSystem(PlaneNormal, U, V);

					glm::dvec3 AProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[0]);
					glm::dvec3 BProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[1]);
					glm::dvec3 CProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[2]);
					
					TempTriangle.push_back(RUGOSITY_LAYER_PRODUCER.ProjectToLocalCoordinates(AProjection, U, V));
					TempTriangle.push_back(RUGOSITY_LAYER_PRODUCER.ProjectToLocalCoordinates(BProjection, U, V));
					TempTriangle.push_back(RUGOSITY_LAYER_PRODUCER.ProjectToLocalCoordinates(CProjection, U, V));
					
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
						CGALCorrectTotalArea -= static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[i]]);
					}
					else
					{
						Triangles.push_back(TempTriangle);
					}
				}

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
							CGALFailedIndexes.push_back(static_cast<int>(i));
						}
					}

					for (size_t i = 0; i < CGALFailedIndexes.size(); i++)
					{
						CGALCorrectTotalArea -= static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[CGALFailedIndexes[i]]]);
					}
				}

				TotalProjectedArea = RUGOSITY_LAYER_PRODUCER.CGALCalculateArea(TriangleGroup);
			}

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
			Point_3 PointA(0.0f, 0.0f, 0.0f);
			Plane_3 PlaneToProjectOnto(PointA, Vector_3(PlaneNormal.x, PlaneNormal.y, PlaneNormal.z));

			for (int l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
			{
				std::vector<glm::dvec3> CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[CurrentNode->TrianglesInCell[l]];

				double ProjectionArea = 0.0;
				double OriginalArea = 0.0;

				if (!RUGOSITY_LAYER_PRODUCER.bUseCGALInMin)
				{
					glm::dvec3 AProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[0]);
					glm::dvec3 BProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[1]);
					glm::dvec3 CProjection = ProjectionPlane->ProjectPoint(CurrentTriangle[2]);

					ProjectionArea = GEOMETRY.CalculateTriangleArea(AProjection, BProjection, CProjection);
					OriginalArea = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[l]];
					Rugosities.push_back(static_cast<float>(OriginalArea / ProjectionArea));
				}
				else
				{
					OriginalArea = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[l]];

					try
					{
						std::vector<Point_2> ProjectedPoints;
						for (size_t j = 0; j < CurrentTriangle.size(); j++)
						{
							ProjectedPoints.push_back(RUGOSITY_LAYER_PRODUCER.ProjectPointOntoPlane(Point_3(CurrentTriangle[j].x, CurrentTriangle[j].y, CurrentTriangle[j].z), PlaneToProjectOnto));
						}

						ProjectionArea = abs(CGAL::to_double(CGAL::area(ProjectedPoints[0], ProjectedPoints[1], ProjectedPoints[2])));
					}
					catch (...)
					{
						ProjectionArea = OriginalArea;
					}
					
					Rugosities.push_back(static_cast<float>(OriginalArea / ProjectionArea));
				}

				if (OriginalArea == 0.0 || ProjectionArea == 0.0 || OriginalArea < FLT_EPSILON || ProjectionArea < FLT_EPSILON)
					Rugosities.back() = 1.0f;

				if (Rugosities.back() > 100.0f)
					Rugosities.back() = 100.0f;
			}

			// Weighted by triangle area rugosity.
			for (int l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
			{
				const float CurrentTriangleCoef = static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[l]] / TotalArea);
				Result += Rugosities[l] * CurrentTriangleCoef;

				if (isnan(Result))
					Result = 1.0f;
			}
		}

		// In very rare cases, 0.001% of area would have rugosity below 1.
		if (Result < 1.0)
			Result = 1.0;

		delete ProjectionPlane;
		return Result;
	};

	if (RUGOSITY_LAYER_PRODUCER.bUseCGALVariant)
	{
		std::vector<double> FEVerticesFinal;
		std::vector<int> FEIndicesFinal;

		for (int l = 0; l < CurrentNode->TrianglesInCell.size(); l++)
		{
			const int TriangleIndex = CurrentNode->TrianglesInCell[l];

			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][0][0]);
			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][0][1]);
			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][0][2]);
			FEIndicesFinal.push_back(l * 3);

			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][1][0]);
			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][1][1]);
			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][1][2]);
			FEIndicesFinal.push_back(l * 3 + 1);

			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][2][0]);
			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][2][1]);
			FEVerticesFinal.push_back(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex][2][2]);
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
		Kernel::FT quality = linear_least_squares_fitting_3(result.points().begin(), result.points().end(), plane, centroid, CGAL::Dimension_tag<0>());

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
		std::vector<glm::dvec3> CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[CurrentNode->TrianglesInCell[l]];
		std::vector<glm::vec3> CurrentTriangleNormals = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesNormals[CurrentNode->TrianglesInCell[l]];

		if (RUGOSITY_LAYER_PRODUCER.bWeightedNormals)
		{
			const float CurrentTriangleCoef = static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[CurrentNode->TrianglesInCell[l]] / TotalArea);

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

	if (!RUGOSITY_LAYER_PRODUCER.bWeightedNormals)
		CurrentNode->AverageCellNormal /= CurrentNode->TrianglesInCell.size() * 3;

	if (RUGOSITY_LAYER_PRODUCER.bNormalizedNormals)
		CurrentNode->AverageCellNormal = glm::normalize(CurrentNode->AverageCellNormal);
	CurrentNode->CellTrianglesCentroid /= CurrentNode->TrianglesInCell.size() * 3;
	// ******* Getting average normal END *******

	if (RUGOSITY_LAYER_PRODUCER.bUseFindSmallestRugosity && RUGOSITY_LAYER_PRODUCER.OrientationSetForMinRugosity != "1")
	{
		std::unordered_map<int, float> TriangleNormalsToRugosity;
		TriangleNormalsToRugosity[-1] = static_cast<float>(CalculateCellRugosity(CurrentNode->CellTrianglesCentroid, CurrentNode->AverageCellNormal));

		if (OrientationSetOptions.find(RUGOSITY_LAYER_PRODUCER.OrientationSetForMinRugosity) == OrientationSetOptions.end())
			RUGOSITY_LAYER_PRODUCER.OrientationSetForMinRugosity = "91";

		std::vector<glm::vec3> OrientationSet = OrientationSetOptions[RUGOSITY_LAYER_PRODUCER.OrientationSetForMinRugosity];
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
}

void RugosityLayerProducer::CalculateWithJitterAsync()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	RUGOSITY_LAYER_PRODUCER.bWaitForJitterResult = true;
	JITTER_MANAGER.CalculateWithGridJitterAsync(CalculateOneNodeRugosity);
}

void RugosityLayerProducer::CalculateOnWholeModel()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	RUGOSITY_LAYER_PRODUCER.bWaitForJitterResult = true;
	JITTER_MANAGER.CalculateOnWholeModel(CalculateOneNodeRugosity);
}

std::vector<std::string> RugosityLayerProducer::GetRugosityAlgorithmList()
{
	return RugosityAlgorithmList;
}

std::vector<std::string> RugosityLayerProducer::GetOrientationSetNamesForMinRugosityList()
{
	return OrientationSetNamesForMinRugosityList;
}

std::string RugosityLayerProducer::GetUsedRugosityAlgorithmName()
{
	if (bUseFindSmallestRugosity)
		return RugosityAlgorithmList[1];

	if (bUseCGALVariant)
		return RugosityAlgorithmList[2];

	return RugosityAlgorithmList[0];
}

void RugosityLayerProducer::SetUsedRugosityAlgorithmName(std::string name)
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

bool RugosityLayerProducer::GetUseFindSmallestRugosity()
{
	return bUseFindSmallestRugosity;
}

void RugosityLayerProducer::SetUseFindSmallestRugosity(bool NewValue)
{
	bUseFindSmallestRugosity = NewValue;
}

bool RugosityLayerProducer::GetUseCGALVariant()
{
	return bUseCGALVariant;
}

void RugosityLayerProducer::SetUseCGALVariant(bool NewValue)
{
	bUseCGALVariant = NewValue;
}

float RugosityLayerProducer::GetLastTimeTookForCalculation()
{
	return LastTimeTookForCalculation;
}

void RugosityLayerProducer::SetOnRugosityCalculationsStartCallback(void(*Func)(void))
{
	OnRugosityCalculationsStartCallbackImpl = Func;
}

void RugosityLayerProducer::OnRugosityCalculationsStart()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	JITTER_MANAGER.SetFallbackValue(1.0f);
	RUGOSITY_LAYER_PRODUCER.bWaitForJitterResult = true;

	TIME.BeginTimeStamp("CalculateRugorsityTotal");
	RUGOSITY_LAYER_PRODUCER.StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	if (OnRugosityCalculationsStartCallbackImpl != nullptr)
		OnRugosityCalculationsStartCallbackImpl();
}

void RugosityLayerProducer::SetOnRugosityCalculationsEndCallback(void(*Func)(MeshLayer))
{
	OnRugosityCalculationsEndCallbackImpl = Func;
}

void RugosityLayerProducer::OnJitterCalculationsEnd(MeshLayer NewLayer)
{
	if (!RUGOSITY_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	NewLayer.SetType(RUGOSITY);
	RUGOSITY_LAYER_PRODUCER.bWaitForJitterResult = false;
	NewLayer.DebugInfo->Type = "RugosityMeshLayerDebugInfo";

	std::string AlgorithmUsed = RUGOSITY_LAYER_PRODUCER.RugosityAlgorithmList[0];
	if (RUGOSITY_LAYER_PRODUCER.bUseFindSmallestRugosity)
		AlgorithmUsed = RUGOSITY_LAYER_PRODUCER.RugosityAlgorithmList[1];

	if (RUGOSITY_LAYER_PRODUCER.bUseCGALVariant)
		AlgorithmUsed = RUGOSITY_LAYER_PRODUCER.RugosityAlgorithmList[2];

	NewLayer.DebugInfo->AddEntry("Algorithm used", AlgorithmUsed);

	if (AlgorithmUsed == "Min Rugosity(default)")
		NewLayer.DebugInfo->AddEntry("Orientation set name", RUGOSITY_LAYER_PRODUCER.GetOrientationSetForMinRugosityName());
	
	std::string DeleteOutliers = "No";
	// Remove outliers.
	if (RUGOSITY_LAYER_PRODUCER.bDeleteOutliers || (RUGOSITY_LAYER_PRODUCER.bUseFindSmallestRugosity && RUGOSITY_LAYER_PRODUCER.OrientationSetForMinRugosity == "1"))
	{
		DeleteOutliers = "Yes";
		JITTER_MANAGER.AdjustOutliers(NewLayer.TrianglesToData, 0.00f, 0.99f);
	}
	NewLayer.DebugInfo->AddEntry("Delete outliers", DeleteOutliers);

	std::string OverlapAware = "No";
	if (RUGOSITY_LAYER_PRODUCER.bUniqueProjectedArea)
		OverlapAware = "Yes";
	NewLayer.DebugInfo->AddEntry("Unique projected area (very slow)", OverlapAware);

	std::string OverlapAwareApproximation = "No";
	if (RUGOSITY_LAYER_PRODUCER.bUniqueProjectedAreaApproximation)
		OverlapAwareApproximation = "Yes";
	NewLayer.DebugInfo->AddEntry("Approximation of unique projected area", OverlapAwareApproximation);

	LastTimeTookForCalculation = float(TIME.EndTimeStamp("CalculateRugorsityTotal"));

	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(NewLayer);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetType(LAYER_TYPE::RUGOSITY);
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Rugosity"));

	LAYER_MANAGER.SetActiveLayerIndex(static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 1));

	if (RUGOSITY_LAYER_PRODUCER.bCalculateStandardDeviation)
	{
		uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
		std::vector<float> TrianglesToStandardDeviation = JITTER_MANAGER.ProduceStandardDeviationData();
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(TrianglesToStandardDeviation);
		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Standard deviation"));

		COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo = new MeshLayerDebugInfo();
		MeshLayerDebugInfo* DebugInfo = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back().DebugInfo;
		DebugInfo->Type = "RugosityStandardDeviationLayerDebugInfo";
		DebugInfo->AddEntry("Start time", StartTime);
		DebugInfo->AddEntry("End time", TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS));
		DebugInfo->AddEntry("Source layer ID", COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 2].GetID());
		DebugInfo->AddEntry("Source layer caption", COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size() - 2].GetCaption());
	}

	if (OnRugosityCalculationsEndCallbackImpl != nullptr)
		OnRugosityCalculationsEndCallbackImpl(NewLayer);
}

void RugosityLayerProducer::RenderDebugInfoForSelectedNode(MeasurementGrid* Grid)
{
	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	Grid->UpdateRenderedLines();

	GridNode* CurrentlySelectedCell = &Grid->Data[int(Grid->SelectedCell.x)][int(Grid->SelectedCell.y)][int(Grid->SelectedCell.z)];
	for (size_t i = 0; i < CurrentlySelectedCell->TrianglesInCell.size(); i++)
	{
		const auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[CurrentlySelectedCell->TrianglesInCell[i]];

		std::vector<glm::dvec3> TranformedTrianglePoints = CurrentTriangle;
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
		LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
	}

	LINE_RENDERER.SyncWithGPU();
}

std::string RugosityLayerProducer::GetOrientationSetForMinRugosityName()
{
	return OrientationSetForMinRugosity;
}

void RugosityLayerProducer::SetOrientationSetForMinRugosityName(std::string name)
{
	if (OrientationSetOptions.find(name) != OrientationSetOptions.end())
		OrientationSetForMinRugosity = name;
}

bool RugosityLayerProducer::GetIsUsingUniqueProjectedArea()
{
	return bUniqueProjectedArea;
}

void RugosityLayerProducer::SetIsUsingUniqueProjectedArea(bool NewValue)
{
	bUniqueProjectedArea = NewValue;
}

bool RugosityLayerProducer::GetIsUsingUniqueProjectedAreaApproximation()
{
	return bUniqueProjectedAreaApproximation;
}

void RugosityLayerProducer::SetIsUsingUniqueProjectedAreaApproximation(bool NewValue)
{
	bUniqueProjectedAreaApproximation = NewValue;
}