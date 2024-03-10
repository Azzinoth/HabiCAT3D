#include "LayerRasterizationManager.h"
using namespace FocalEngine;

LayerRasterizationManager* LayerRasterizationManager::Instance = nullptr;

LayerRasterizationManager::LayerRasterizationManager() {}
LayerRasterizationManager::~LayerRasterizationManager() {}

glm::vec3 LayerRasterizationManager::ConvertToClosestAxis(const glm::vec3& Vector)
{
	// Calculate the absolute values of the vector components
	float AbsX = glm::abs(Vector.x);
	float AbsY = glm::abs(Vector.y);
	float AbsZ = glm::abs(Vector.z);

	// Determine the largest component
	if (AbsX > AbsY && AbsX > AbsZ)
	{
		// X component is largest, so vector is closest to the X axis
		return glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else if (AbsY > AbsX && AbsY > AbsZ)
	{
		// Y component is largest, so vector is closest to the Y axis
		return glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else
	{
		// Z component is largest (or it's a tie, in which case we default to Z), so vector is closest to the Z axis
		return glm::vec3(0.0f, 0.0f, 1.0f);
	}
}

std::vector<std::vector<LayerRasterizationManager::GridCell>> LayerRasterizationManager::GenerateGridProjection(FEAABB& OriginalAABB, const glm::vec3& Axis, int Resolution)
{
	std::vector<std::vector<GridCell>> Grid;

	if (Axis.x + Axis.y + Axis.z != 1.0f)
		return Grid;

	Grid.resize(Resolution);
	for (int i = 0; i < Resolution; i++)
	{
		Grid[i].resize(Resolution);
	}

	// Get the original AABB min and max vectors
	glm::vec3 Min = OriginalAABB.GetMin();
	glm::vec3 Max = OriginalAABB.GetMax();

	// Determine the number of divisions along each axis
	glm::vec3 Size = Max - Min;
	glm::vec3 DivisionSize = Size / static_cast<float>(Resolution);

	// Fix the division size for the specified axis to cover the full length of the original AABB
	if (Axis.x > 0.0) DivisionSize.x = Size.x;
	if (Axis.y > 0.0) DivisionSize.y = Size.y;
	if (Axis.z > 0.0) DivisionSize.z = Size.z;

	// Loop through each division to create the grid
	for (int i = 0; i < Resolution; i++)
	{
		for (int j = 0; j < Resolution; j++)
		{
			// Calculate min and max for the current cell
			glm::vec3 CellMin = Min;
			glm::vec3 CellMax = Min + DivisionSize;

			if (Axis.x > 0.0f)
			{
				CellMin.y += DivisionSize.y * i;
				CellMax.y += DivisionSize.y * i;

				CellMin.z += DivisionSize.z * j;
				CellMax.z += DivisionSize.z * j;
			}
			else if (Axis.y > 0.0f)
			{
				CellMin.x += DivisionSize.x * i;
				CellMax.x += DivisionSize.x * i;

				CellMin.z += DivisionSize.z * j;
				CellMax.z += DivisionSize.z * j;
			}
			else if (Axis.z > 0.0f)
			{
				CellMin.x += DivisionSize.x * i;
				CellMax.x += DivisionSize.x * i;

				CellMin.y += DivisionSize.y * j;
				CellMax.y += DivisionSize.y * j;
			}

			// Ensure we don't exceed original bounds due to floating point arithmetic
			CellMax = glm::min(CellMax, Max);

			// Create a new AABB for the grid cell
			GridCell NewCell;
			NewCell.AABB = FEAABB(CellMin, CellMax);
			//NewCell.AABBVolume = NewCell.AABB.GetVolume();
			Grid[i][j] = NewCell;
		}
	}

	return Grid;
}

void LayerRasterizationManager::GridRasterizationThread(void* InputData, void* OutputData)
{
	GridRasterizationThreadData* Input = reinterpret_cast<GridRasterizationThreadData*>(InputData);
	std::vector<GridUpdateTask>* Output = reinterpret_cast<std::vector<GridUpdateTask>*>(OutputData);

	std::vector<std::vector<GridCell>>& Grid = *Input->Grid;
	const glm::vec3 UpAxis = Input->UpAxis;
	const int Resolution = Input->Resolution;
	const int FirstIndexInTriangleArray = Input->FirstIndexInTriangleArray;
	const int LastIndexInTriangleArray = Input->LastIndexInTriangleArray;

	glm::vec3 CellSize = Grid[0][0].AABB.GetSize();
	const glm::vec3 GridMin = Grid[0][0].AABB.GetMin();
	const glm::vec3 GridMax = Grid[Resolution - 1][Resolution - 1].AABB.GetMax();

	int FirstAxisStartIndex = 0;
	int FirstAxisEndIndex = Resolution;

	int SecondAxisStartIndex = 0;
	int SecondAxisEndIndex = Resolution;

	//auto CheckIfTriangleIsInsideCell = [&](const int GridX, const int GridY, const int TriangleIndex) -> void {
	//	if (GEOMETRY.IsAABBIntersectTriangle(Grid[GridX][GridY].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex]))
	//	{
	//		Output->push_back(GridUpdateTask(GridX, GridY, TriangleIndex));
	//	}
	//};

	if (UpAxis.x > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l < LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			FirstAxisEndIndex = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
			FirstAxisStartIndex = static_cast<int>(Distance / CellSize.y) - 1;
			if (FirstAxisStartIndex < 0)
				FirstAxisStartIndex = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
			FirstAxisEndIndex -= static_cast<int>(Distance / CellSize.y);
			FirstAxisEndIndex++;
			if (FirstAxisEndIndex >= Resolution)
				FirstAxisEndIndex = Resolution - 1;

			for (size_t i = FirstAxisStartIndex; i < FirstAxisEndIndex; i++)
			{
				SecondAxisEndIndex = Resolution;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
				SecondAxisStartIndex = static_cast<int>(Distance / CellSize.z) - 1;
				if (SecondAxisStartIndex < 0)
					SecondAxisStartIndex = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
				SecondAxisEndIndex -= static_cast<int>(Distance / CellSize.z);
				SecondAxisEndIndex++;
				if (SecondAxisEndIndex >= Resolution)
					SecondAxisEndIndex = Resolution - 1;

				for (size_t j = SecondAxisStartIndex; j < SecondAxisEndIndex; j++)
				{
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Output->push_back(GridUpdateTask(i, j, l));
				}
			}
		}
	}
	if (UpAxis.y > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l < LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			FirstAxisEndIndex = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			FirstAxisStartIndex = static_cast<int>(Distance / CellSize.x) - 1;
			if (FirstAxisStartIndex < 0)
				FirstAxisStartIndex = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			FirstAxisEndIndex -= static_cast<int>(Distance / CellSize.x);
			FirstAxisEndIndex++;
			if (FirstAxisEndIndex >= Resolution)
				FirstAxisEndIndex = Resolution - 1;

			for (size_t i = FirstAxisStartIndex; i < FirstAxisEndIndex; i++)
			{
				SecondAxisEndIndex = Resolution;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
				SecondAxisStartIndex = static_cast<int>(Distance / CellSize.z) - 1;
				if (SecondAxisStartIndex < 0)
					SecondAxisStartIndex = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
				SecondAxisEndIndex -= static_cast<int>(Distance / CellSize.z);
				SecondAxisEndIndex++;
				if (SecondAxisEndIndex >= Resolution)
					SecondAxisEndIndex = Resolution - 1;

				for (size_t j = SecondAxisStartIndex; j < SecondAxisEndIndex; j++)
				{
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Output->push_back(GridUpdateTask(i, j, l));
				}
			}
		}
	}
	else if (UpAxis.z > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l < LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			FirstAxisEndIndex = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			FirstAxisStartIndex = static_cast<int>(Distance / CellSize.x) - 1;
			if (FirstAxisStartIndex < 0)
				FirstAxisStartIndex = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			FirstAxisEndIndex -= static_cast<int>(Distance / CellSize.x);
			FirstAxisEndIndex++;
			if (FirstAxisEndIndex >= Resolution)
				FirstAxisEndIndex = Resolution - 1;

			for (size_t i = FirstAxisStartIndex; i < FirstAxisEndIndex; i++)
			{
				SecondAxisEndIndex = Resolution;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
				SecondAxisStartIndex = static_cast<int>(Distance / CellSize.y) - 1;
				if (SecondAxisStartIndex < 0)
					SecondAxisStartIndex = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
				SecondAxisEndIndex -= static_cast<int>(Distance / CellSize.y);
				SecondAxisEndIndex++;
				if (SecondAxisEndIndex >= Resolution)
					SecondAxisEndIndex = Resolution - 1;

				for (size_t j = SecondAxisStartIndex; j < SecondAxisEndIndex; j++)
				{
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Output->push_back(GridUpdateTask(i, j, l));
				}
			}
		}
	}
}


//// Function to clip a line segment against an AABB
//std::vector<glm::vec3> clipLineSegment(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& min, const glm::vec3& max) {
//	std::vector<glm::vec3> clippedPoints;
//
//	float tEnter = 0.0f;
//	float tLeave = 1.0f;
//
//	for (int i = 0; i < 3; i++) {
//		if (p1[i] < min[i] && p2[i] < min[i]) return clippedPoints;
//		if (p1[i] > max[i] && p2[i] > max[i]) return clippedPoints;
//
//		float tEnterPlane = (min[i] - p1[i]) / (p2[i] - p1[i]);
//		float tLeavePlane = (max[i] - p1[i]) / (p2[i] - p1[i]);
//
//		if (tEnterPlane > tEnter) tEnter = tEnterPlane;
//		if (tLeavePlane < tLeave) tLeave = tLeavePlane;
//	}
//
//	if (tEnter <= tLeave) {
//		clippedPoints.push_back(p1 + tEnter * (p2 - p1));
//		clippedPoints.push_back(p1 + tLeave * (p2 - p1));
//	}
//
//	return clippedPoints;
//}
//
//// Function to clip a triangle against an AABB
//std::vector<glm::vec3> clipTriangle(const std::vector<glm::vec3>& triangle, const glm::vec3& min, const glm::vec3& max) {
//	std::vector<glm::vec3> clippedPolygon;
//
//	clippedPolygon = clipLineSegment(triangle[0], triangle[1], min, max);
//	std::vector<glm::vec3> tempPolygon = clipLineSegment(triangle[1], triangle[2], min, max);
//	clippedPolygon.insert(clippedPolygon.end(), tempPolygon.begin(), tempPolygon.end());
//	tempPolygon = clipLineSegment(triangle[2], triangle[0], min, max);
//	clippedPolygon.insert(clippedPolygon.end(), tempPolygon.begin(), tempPolygon.end());
//
//	return clippedPolygon;
//}
//
//// Function to calculate the area of a polygon
//float calculatePolygonArea(const std::vector<glm::vec3>& polygon) {
//	float area = 0.0f;
//
//	for (size_t i = 0; i < polygon.size(); i++) {
//		const glm::vec3& p1 = polygon[i];
//		const glm::vec3& p2 = polygon[(i + 1) % polygon.size()];
//		area += p1.x * p2.y - p2.x * p1.y;
//	}
//
//	return std::abs(area) * 0.5f;
//}
//
//// Function to calculate the area of a triangle inside an AABB
//float calculateTriangleAreaInsideAABB(const std::vector<glm::vec3>& triangle, const glm::vec3& min, const glm::vec3& max) {
//	std::vector<glm::vec3> clippedPolygon = clipTriangle(triangle, min, max);
//	return calculatePolygonArea(clippedPolygon);
//}


glm::dvec3 CalculateCentroid(const std::vector<glm::dvec3>& points) {
	glm::dvec3 centroid(0.0, 0.0, 0.0);
	for (const auto& point : points) {
		centroid += point;
	}
	centroid /= static_cast<double>(points.size());
	return centroid;
}

bool CompareAngles(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& centroid) {
	double angleA = atan2(a.z - centroid.z, a.x - centroid.x);
	double angleB = atan2(b.z - centroid.z, b.x - centroid.x);
	return angleA < angleB;
}

void SortPointsByAngle(std::vector<glm::dvec3>& points) {
	glm::dvec3 centroid = CalculateCentroid(points);
	std::sort(points.begin(), points.end(), [&centroid](const glm::dvec3& a, const glm::dvec3& b) {
		return CompareAngles(a, b, centroid);
		});
}

// Function to calculate the area of a triangle given its vertices
double TriangleArea(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c) {
	glm::dvec3 crossProduct = glm::cross(b - a, c - a);
	return 0.5 * glm::length(crossProduct);
}

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_2_algorithms.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2_;
typedef CGAL::Polygon_2<K> Polygon_2_;

// Function to calculate the area of a quadrilateral given its vertices
double QuadrilateralArea(const std::vector<glm::dvec3>& points) {
	// Ensure there are exactly four vertices
	if (points.size() != 4) return 0.0;

	// Split the quadrilateral into two triangles and calculate each area
	double area1 = TriangleArea(points[0], points[1], points[2]);
	double area2 = TriangleArea(points[2], points[3], points[0]);

	// Sum the areas of the two triangles
	return area1 + area2;
}

double CalculatePolygonArea(const std::vector<glm::dvec2>& glmPoints) {
	Polygon_2_ polygon;
	for (const auto& p : glmPoints) {
		polygon.push_back(Point_2_(p.x, p.y));
	}

	// Check if the polygon is simple (does not self-intersect)
	if (!polygon.is_simple()) {
		std::cerr << "Polygon is not simple." << std::endl;
		return 0.0;
	}

	// Calculate the area
	double area = CGAL::to_double(polygon.area());

	return area;
}

double LayerRasterizationManager::GetArea(std::vector<glm::dvec3>& points)
{
	////if (points.size() == 3)
	////{
	////	return TriangleArea(points[0], points[1], points[2]);
	////}
	/////*else if (points.size() == 4)
	////{
	////	return QuadrilateralArea_(points);
	////}*/
	////else if (points.size() > 3)
	////{
	//	SortPointsByAngle(points);

	//	double Area = 0.0;
	//	for (size_t p = 0; p < points.size(); p++)
	//	{
	//		const glm::dvec3& v1 = points[p];
	//		const glm::dvec3& v2 = points[(p + 1) % points.size()];
	//		Area += v1.x * v2.z - v2.x * v1.z;
	//	}
	//	Area = std::abs(Area) * 0.5;
	//	return Area;
	///*}
	//else
	//{
	//	return 0.0;
	//}*/


	// CGAL

	SortPointsByAngle(points);

	// FIX ME
	std::vector<glm::dvec2> glmPoints;
	for (size_t i = 0; i < points.size(); i++)
	{
		if (CurrentUpAxis.x > 0.0)
			glmPoints.push_back(glm::dvec2(points[i].y, points[i].z));
		else if (CurrentUpAxis.y > 0.0)
			glmPoints.push_back(glm::dvec2(points[i].x, points[i].z));
		else if (CurrentUpAxis.z > 0.0)
			glmPoints.push_back(glm::dvec2(points[i].x, points[i].y));

		//glmPoints.push_back(glm::dvec2(points[i].x, points[i].z));
	}

	return abs(CalculatePolygonArea(std::vector<glm::dvec2>(points.begin(), points.end())));
}

// Function to calculate the Box-Cox transformation
float boxCoxTransform(float value, float lambda)
{
	if (lambda == 0)
		return log(value);
	else
		return (pow(value, lambda) - 1) / lambda;
}

// Find the optimal lambda value using the Box-Cox method
float findOptimalLambda(std::vector<float>& data)
{
	// Implement the Box-Cox method to find the optimal lambda value
	// You can use an optimization algorithm or a grid search
	// to find the lambda value that maximizes the log-likelihood function
	// Here, you can use a library or write your own implementation

	// For simplicity, let's assume you found the optimal lambda value
	float optimalLambda = 0.5; // Replace with your actual optimal lambda value

	return optimalLambda;
}

#include <CGAL/Partition_traits_2.h>
#include <CGAL/partition_2.h>

#include <CGAL/Bbox_3.h>
#include <CGAL/Triangle_3.h>
#include <CGAL/intersections.h>

typedef CGAL::Simple_cartesian<double>					   Kernel;

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
//typedef K::Point_2 Point_2;
//typedef CGAL::Polygon_2<K> Polygon_2;

typedef CGAL::Partition_traits_2<K> Traits;
typedef std::list<Polygon_2_> Polygon_list;

typedef Kernel::Point_3 Point_3;
typedef Kernel::Triangle_3 Triangle_3;
typedef CGAL::Bbox_3 Bbox_3;

void LayerRasterizationManager::AfterAllGridRasterizationThreadFinished()
{
	Debug_TotalAreaUsed = 0.0;
	//MessageBoxA(NULL, "GatherGridRasterizationThreadWork!", "Mouse Position", MB_OK);

	for (int i = 0; i < MainThreadGridUpdateTasks.size(); i++)
	{
		Grid[MainThreadGridUpdateTasks[i].FirstIndex][MainThreadGridUpdateTasks[i].SecondIndex].TrianglesInCell.push_back(MainThreadGridUpdateTasks[i].TriangleIndexToAdd);
	}

	//double TotalArea = 0.0;
	int DebugCount = 0;
	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			if (!Grid[i][j].TrianglesInCell.empty())
			{
				double CurrentCellTotalArea = 0.0;
				double CurrentCellTotalArea_alternative = 0.0;
				std::vector<glm::dvec3> DebugResult;

				if (GridRasterizationMode == /*GridRasterizationModeCumulative*/GridRasterizationModeMean)
				{
					for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Grid[i][j].TrianglesInCell[k]];
						if (CurrentTriangle.size() != 3)
							continue;

						std::vector<glm::dvec3> CurrentTriangleDouble;
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z));
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z));
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z));

						std::vector<glm::dvec3> DebugResult = GEOMETRY.GetIntersectionPoints(Grid[i][j].AABB, CurrentTriangleDouble);

						for (size_t l = 0; l < CurrentTriangleDouble.size(); l++)
						{
							if (Grid[i][j].AABB.ContainsPoint(CurrentTriangleDouble[l]))
							{
								bool bAlreadyExists = false;
								int PointsThatAreNotSame = 0;
								for (size_t q = 0; q < DebugResult.size(); q++)
								{
									if (abs(DebugResult[q] - CurrentTriangleDouble[l]).x > glm::dvec3(DBL_EPSILON).x ||
										abs(DebugResult[q] - CurrentTriangleDouble[l]).y > glm::dvec3(DBL_EPSILON).y ||
										abs(DebugResult[q] - CurrentTriangleDouble[l]).z > glm::dvec3(DBL_EPSILON).z)
									{
										PointsThatAreNotSame++;
									}
								}

								if (PointsThatAreNotSame != DebugResult.size())
									bAlreadyExists = true;

								if (!bAlreadyExists)
									DebugResult.push_back(CurrentTriangle[l]);
							}
						}


						double CurrentTrianlgeArea = 0.0;
						CurrentTrianlgeArea = GetArea(DebugResult);

						if (CurrentTrianlgeArea != 0.0 && !isnan(CurrentTrianlgeArea))
						{
							CurrentCellTotalArea += CurrentTrianlgeArea;
						}
					}
				}
				else
				{
					for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						CurrentCellTotalArea += static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]]);
					}
				}

				/*for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
				{
					CurrentCellTotalArea += static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]]);
				}*/

				//TotalArea += CurrentCellTotalArea;

				/*CurrentCellTotalArea = 0.0;
				for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
				{
					CurrentCellTotalArea += static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]]);
				}*/

				//if (CurrentCellTotalArea_alternative != 0.0f && CurrentCellTotalArea == 0.0f /*&& Grid[i][j].TrianglesInCell.size() == 1*/)
				//{
				//	auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Grid[i][j].TrianglesInCell[0]];
				//	if (CurrentTriangle.size() == 3)
				//	{
				//		std::string Output = "glm::vec3 FirstPoint = glm::vec3(" + std::to_string(CurrentTriangle[0].x) + ", " + std::to_string(CurrentTriangle[0].y) + ", " + std::to_string(CurrentTriangle[0].z) + ") * Multiplicator;\n";
				//		Output += "glm::vec3 SecondPoint = glm::vec3(" + std::to_string(CurrentTriangle[1].x) + ", " + std::to_string(CurrentTriangle[1].y) + ", " + std::to_string(CurrentTriangle[1].z) + ") * Multiplicator;\n";
				//		Output += "glm::vec3 ThirdPoint = glm::vec3(" + std::to_string(CurrentTriangle[2].x) + ", " + std::to_string(CurrentTriangle[2].y) + ", " + std::to_string(CurrentTriangle[2].z) + ") * Multiplicator;\n";

				//		Output += "glm::vec3 Min = glm::vec3(" + std::to_string(Grid[i][j].AABB.GetMin().x) + ", " + std::to_string(Grid[i][j].AABB.GetMin().y) + ", " + std::to_string(Grid[i][j].AABB.GetMin().z) + ") * Multiplicator;\n";
				//		Output += "glm::vec3 Max = glm::vec3(" + std::to_string(Grid[i][j].AABB.GetMax().x) + ", " + std::to_string(Grid[i][j].AABB.GetMax().y) + ", " + std::to_string(Grid[i][j].AABB.GetMax().z) + ") * Multiplicator;\n";
				//		
				//		int a = 0;
				//		a++;
				//	}
				//}

				if (CurrentCellTotalArea == 0.0f)
				{
					Grid[i][j].Value = 0.0f;
					continue;
				}

				float FinalResult = 0.0f;

				if (GridRasterizationMode == GridRasterizationModeMin)
				{
					float MinValue = FLT_MAX;

					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						float CurrentValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
						if (CurrentValue < MinValue)
							MinValue = CurrentValue;
					}

					FinalResult = MinValue;
				}
				else if (GridRasterizationMode == GridRasterizationModeMax)
				{
					float MaxValue = -FLT_MAX;

					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						float CurrentValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
						if (CurrentValue > MaxValue)
							MaxValue = CurrentValue;
					}

					FinalResult = MaxValue;

				}
				else if (GridRasterizationMode == GridRasterizationModeMean)
				{
					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Grid[i][j].TrianglesInCell[k]];
						if (CurrentTriangle.size() != 3)
							continue;

						std::vector<glm::dvec3> CurrentTriangleDouble;
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z));
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z));
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z));

						std::vector<glm::dvec3> DebugResult = GEOMETRY.GetIntersectionPoints(Grid[i][j].AABB, CurrentTriangleDouble);

						for (size_t l = 0; l < CurrentTriangleDouble.size(); l++)
						{
							if (Grid[i][j].AABB.ContainsPoint(CurrentTriangleDouble[l]))
							{
								bool bAlreadyExists = false;
								int PointsThatAreNotSame = 0;
								for (size_t q = 0; q < DebugResult.size(); q++)
								{
									if (abs(DebugResult[q] - CurrentTriangleDouble[l]).x > glm::dvec3(DBL_EPSILON).x ||
										abs(DebugResult[q] - CurrentTriangleDouble[l]).y > glm::dvec3(DBL_EPSILON).y ||
										abs(DebugResult[q] - CurrentTriangleDouble[l]).z > glm::dvec3(DBL_EPSILON).z)
									{
										PointsThatAreNotSame++;
									}
								}

								if (PointsThatAreNotSame != DebugResult.size())
									bAlreadyExists = true;

								if (!bAlreadyExists)
									DebugResult.push_back(CurrentTriangle[l]);
							}
						}



						double CurrentTrianlgeArea = 0.0;
						CurrentTrianlgeArea = GetArea(DebugResult);
						if (CurrentTrianlgeArea != 0.0 && !isnan(CurrentTrianlgeArea))
						{
							float CurrentTriangleCoef = CurrentTrianlgeArea / CurrentCellTotalArea;
							FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]] * CurrentTriangleCoef;
						}
					}

					

				}
				else if (GridRasterizationMode == GridRasterizationModeCumulative)
				{
					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						/*float CurrentTrianlgeArea = calculateTriangleAreaInsideAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Grid[i][j].TrianglesInCell[k]], Grid[i][j].AABB.GetMin(), Grid[i][j].AABB.GetMax());
						if (CurrentTrianlgeArea == 0.0f)
							continue;

						float CurrentTriangleCoef = CurrentTrianlgeArea / CurrentCellTotalArea;
						if (CurrentTrianlgeArea == 0.0f || isnan(CurrentTriangleCoef))
							continue;*/


						auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Grid[i][j].TrianglesInCell[k]];
						if (CurrentTriangle.size() != 3)
							continue;

						std::vector<glm::dvec3> CurrentTriangleDouble;
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z));
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z));
						CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z));

						std::vector<glm::dvec3> DebugResult = GEOMETRY.GetIntersectionPoints(Grid[i][j].AABB, CurrentTriangleDouble);

						for (size_t l = 0; l < CurrentTriangleDouble.size(); l++)
						{
							if (Grid[i][j].AABB.ContainsPoint(CurrentTriangleDouble[l]))
							{
								bool bAlreadyExists = false;
								int PointsThatAreNotSame = 0;
								for (size_t q = 0; q < DebugResult.size(); q++)
								{
									if (abs(DebugResult[q] - CurrentTriangleDouble[l]).x > glm::dvec3(DBL_EPSILON).x ||
										abs(DebugResult[q] - CurrentTriangleDouble[l]).y > glm::dvec3(DBL_EPSILON).y ||
										abs(DebugResult[q] - CurrentTriangleDouble[l]).z > glm::dvec3(DBL_EPSILON).z)
									{
										PointsThatAreNotSame++;
									}
								}

								if (PointsThatAreNotSame != DebugResult.size())
									bAlreadyExists = true;

								if (!bAlreadyExists)
									DebugResult.push_back(CurrentTriangle[l]);
							}
						}



						double CurrentTrianlgeArea = 0.0;
						CurrentTrianlgeArea = GetArea(DebugResult);

						if (CurrentTrianlgeArea == 0.0)
						{
							std::string Output = "glm::vec3 FirstPoint = glm::vec3(" + std::to_string(CurrentTriangle[0].x) + ", " + std::to_string(CurrentTriangle[0].y) + ", " + std::to_string(CurrentTriangle[0].z) + ") * Multiplicator;\n";
							Output += "glm::vec3 SecondPoint = glm::vec3(" + std::to_string(CurrentTriangle[1].x) + ", " + std::to_string(CurrentTriangle[1].y) + ", " + std::to_string(CurrentTriangle[1].z) + ") * Multiplicator;\n";
							Output += "glm::vec3 ThirdPoint = glm::vec3(" + std::to_string(CurrentTriangle[2].x) + ", " + std::to_string(CurrentTriangle[2].y) + ", " + std::to_string(CurrentTriangle[2].z) + ") * Multiplicator;\n";

							Output += "glm::vec3 Min = glm::vec3(" + std::to_string(Grid[i][j].AABB.GetMin().x) + ", " + std::to_string(Grid[i][j].AABB.GetMin().y) + ", " + std::to_string(Grid[i][j].AABB.GetMin().z) + ") * Multiplicator;\n";
							Output += "glm::vec3 Max = glm::vec3(" + std::to_string(Grid[i][j].AABB.GetMax().x) + ", " + std::to_string(Grid[i][j].AABB.GetMax().y) + ", " + std::to_string(Grid[i][j].AABB.GetMax().z) + ") * Multiplicator;\n";

							int y = 0;
							y++;

							if (DebugResult.size() > 1)
							{
								int y = 0;
								y++;
							}
						}





						//// Define a triangle
						//Point_3 p1(CurrentTriangleDouble[0].x, CurrentTriangleDouble[0].y, CurrentTriangleDouble[0].z);
						//Point_3 p2(CurrentTriangleDouble[1].x, CurrentTriangleDouble[1].y, CurrentTriangleDouble[1].z);
						//Point_3 p3(CurrentTriangleDouble[2].x, CurrentTriangleDouble[2].y, CurrentTriangleDouble[2].z);
						//Triangle_3 triangle(p1, p2, p3);

						//// Define an AABB
						//Bbox_3 aabb(Grid[i][j].AABB.GetMin().x, Grid[i][j].AABB.GetMin().y, Grid[i][j].AABB.GetMin().z, Grid[i][j].AABB.GetMax().x, Grid[i][j].AABB.GetMax().y, Grid[i][j].AABB.GetMax().z);


						//// Compute the intersection between the triangle and the AABB
						//auto result = CGAL::intersection(triangle, aabb);

						////double CGALArea = 0.0;
						//// Check the type of the intersection result and calculate the area
						//if (result) {
						//	if (const Triangle_3* intersect_triangle = boost::get<Triangle_3>(&*result)) {

						//		// Project the 3D triangle onto a 2D plane
						//		Kernel::Point_2 p1_2d(intersect_triangle->vertex(0).x(), intersect_triangle->vertex(0).z());
						//		Kernel::Point_2 p2_2d(intersect_triangle->vertex(1).x(), intersect_triangle->vertex(1).z());
						//		Kernel::Point_2 p3_2d(intersect_triangle->vertex(2).x(), intersect_triangle->vertex(2).z());

						//		// Using absolute value to ensure the area is positive
						//		// CGAL will return a negative area if the vertices are ordered clockwise
						//		CurrentTrianlgeArea = abs(CGAL::to_double(CGAL::area(p1_2d, p2_2d, p3_2d)));
						//		//std::cout << "Intersection is a triangle with area: " << area << std::endl;
						//		//int y = 0;
						//		//y++;
						//	}
						//	else if (const Kernel::Segment_3* intersect_segment = boost::get<Kernel::Segment_3>(&*result)) {
						//		std::cout << "Intersection is a segment. Area is zero." << std::endl;
						//	}
						//	else if (const Point_3* intersect_point = boost::get<Point_3>(&*result)) {
						//		std::cout << "Intersection is a point. Area is zero." << std::endl;
						//	}
						//	else if (const std::vector<Point_3>* intersect_points = boost::get<std::vector<Point_3>>(&*result)) {
						//		// Create a polygon from the intersection points
						//		Polygon_2_ polygon;
						//		for (const auto& point : *intersect_points) {
						//			polygon.push_back(Point_2_(point.x(), point.z()));
						//		}

						//		// Using absolute value to ensure the area is positive
						//		// CGAL will return a negative area if the vertices are ordered clockwise
						//		CurrentTrianlgeArea = abs(CGAL::to_double(polygon.area()));
						//		//std::cout << "Intersection is a polygon with area: " << CGALArea << std::endl;
						//	}
						//}
						//else {
						//	std::cout << "No intersection. Area is zero." << std::endl;
						//}


















						

						if (CurrentTrianlgeArea != 0.0 && !isnan(CurrentTrianlgeArea))
						{
							Debug_TotalAreaUsed += CurrentTrianlgeArea;
							//double CurrentTriangleCoef = CurrentTrianlgeArea / CurrentCellTotalArea;
							//double CurrentTriangleCoef = CurrentTrianlgeArea / COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]];
							double CurrentTriangleValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
							FinalResult += CurrentTriangleValue * /*CGALArea*/CurrentTrianlgeArea/*CurrentTriangleCoef*/;

							/*float CurrentTriangleCoef = CurrentTrianlgeArea / CurrentCellTotalArea;
							FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]] * CurrentTriangleCoef;*/
						}
					}
				}

				Grid[i][j].Value = FinalResult;
				if (isnan(Grid[i][j].Value))
					Grid[i][j].Value = 0.0f;
			}
		}
	}

	//TotalArea;

	// Getting all debug values
	/*std::vector<float> FlattenGrid;
	FlattenGrid.reserve(Grid.size()* Grid[0].size());

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			FlattenGrid.push_back(Grid[i][j].Value);
		}
	}*/

	UpdateGridDebugDistributionInfo();
	// Getting all debug values END


	

	float MinForColorMap = CurrentLayer->MinVisible;
	float MaxForColorMap = CurrentLayer->MaxVisible;
	if (GridRasterizationMode == GridRasterizationModeCumulative)
	{
		std::vector<float> FlattenGrid;
		FlattenGrid.reserve(Grid.size() * Grid[0].size());

		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				// FIX ME
				//if (Grid[i][j].Value < 1.0f)
					//Grid[i][j].Value += 1.0f;
				FlattenGrid.push_back(Grid[i][j].Value);
			}
		}

		//float lower_percentile = 50 - (50 / (1 + pow(Debug_ResultRawSkewness, 2) + Debug_ResultRawKurtosis));
		//float upper_percentile = 50 + (50 / (1 + pow(Debug_ResultRawSkewness, 2) + Debug_ResultRawKurtosis));


		//// Transform your data using the Box-Cox transformation
		//std::vector<float> transformedData;
		//float optimalLambda = findOptimalLambda(FlattenGrid);

		//for (int i = 0; i < Grid.size(); i++)
		//{
		//	for (int j = 0; j < Grid[i].size(); j++)
		//	{
		//		float transformedValue = boxCoxTransform(Grid[i][j].Value, optimalLambda);
		//		Grid[i][j].Value = transformedValue;
		//	}
		//}


		JITTER_MANAGER.AdjustOutliers(FlattenGrid, CumulativeOutliersLower / 100.0f, CumulativeOutliersUpper / 100.0f);

		// Updated the grid values
		int Index = 0;
		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				Grid[i][j].Value = FlattenGrid[Index];
				// FIX ME
				/*if (Grid[i][j].Value < 1.0f)
					Grid[i][j].Value = 1.0f;*/
				Index++;
			}
		}

		float MaxValue = -FLT_MAX;
		float MinValue = FLT_MAX;

		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				if (Grid[i][j].Value > MaxValue)
					MaxValue = Grid[i][j].Value;
				if (Grid[i][j].Value < MinValue)
					MinValue = Grid[i][j].Value;
			}
		}

		MinForColorMap = MinValue;
		MaxForColorMap = MaxValue;

		if (MinForColorMap == MaxForColorMap)
		{
			MaxForColorMap += FLT_EPSILON * 4;
		}
	}

	UpdateGridDebugDistributionInfo();

	std::vector<unsigned char> ImageRawData;
	ImageRawData.reserve(CurrentResolution * CurrentResolution * 4);
	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			// Using Grid[j][i] instead of Grid[i][j] to rotate the image by 90 degrees
			if (Grid[j][i].TrianglesInCell.empty() || Grid[j][i].Value == 0.0f)
			{
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));
			}
			else
			{
				// Normalize the value to the range [0, 1] (0 = min, 1 = max)
				float NormalizedValue = (Grid[j][i].Value - MinForColorMap) / (MaxForColorMap - MinForColorMap);

				// It could be more than 1 because I am using user set max value.
				if (NormalizedValue > 1.0f)
					NormalizedValue = 1.0f;

				glm::vec3 Color = GetTurboColorMap(NormalizedValue);

				unsigned char R = static_cast<unsigned char>(Color.x * 255.0f);
				ImageRawData.push_back(R);
				unsigned char G = static_cast<unsigned char>(Color.y * 255.0f);
				ImageRawData.push_back(G);
				unsigned char B = static_cast<unsigned char>(Color.z * 255.0f);
				ImageRawData.push_back(B);
				ImageRawData.push_back(static_cast<unsigned char>(255));
			}
		}
	}

	if (CurrentUpAxis.x > 0.0)
	{
		lodepng::encode("test.png", ImageRawData, CurrentResolution, CurrentResolution);
	}
	if (CurrentUpAxis.y > 0.0)
	{
		lodepng::encode("test.png", ImageRawData, CurrentResolution, CurrentResolution);
	}
	if (CurrentUpAxis.z > 0.0)
	{
		// Flip the image vertically
		std::vector<unsigned char> FlippedImageRawData;
		FlippedImageRawData.reserve(CurrentResolution * CurrentResolution * 4);

		for (int i = CurrentResolution - 1; i >= 0; i--)
		{
			for (int j = 0; j < CurrentResolution; j++)
			{
				int index = (i * CurrentResolution + j) * 4;
				FlippedImageRawData.push_back(ImageRawData[index + 0]);
				FlippedImageRawData.push_back(ImageRawData[index + 1]);
				FlippedImageRawData.push_back(ImageRawData[index + 2]);
				FlippedImageRawData.push_back(ImageRawData[index + 3]);
			}
		}

		lodepng::encode("test.png", FlippedImageRawData, CurrentResolution, CurrentResolution);
	}

	Grid.clear();
	MainThreadGridUpdateTasks.clear();
	GatherGridRasterizationThreadCount = 0;
	CurrentLayer = nullptr;
	CurrentUpAxis = glm::vec3(0.0f);
}

void LayerRasterizationManager::UpdateGridDebugDistributionInfo()
{
	Debug_ResultRawMax = -FLT_MAX;
	Debug_ResultRawMin = FLT_MAX;
	Debug_ResultRawMean = 0.0;
	Debug_ResultRawStandardDeviation = 0.0;
	Debug_ResultRawSkewness = 0.0;
	Debug_ResultRawKurtosis = 0.0;

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			if (Grid[i][j].Value > Debug_ResultRawMax)
				Debug_ResultRawMax = Grid[i][j].Value;
			if (Grid[i][j].Value < Debug_ResultRawMin)
				Debug_ResultRawMin = Grid[i][j].Value;

			Debug_ResultRawMean += Grid[i][j].Value;
		}
	}

	Debug_ResultRawMean /= (Grid.size() * Grid[0].size());

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			Debug_ResultRawStandardDeviation += pow(Grid[i][j].Value - Debug_ResultRawMean, 2);
		}
	}

	Debug_ResultRawStandardDeviation /= (Grid.size() * Grid[0].size());
	Debug_ResultRawStandardDeviation = sqrt(Debug_ResultRawStandardDeviation);

	/*
	 * Skewness:
	 *   - If skewness is close to 0, the distribution is symmetric (similar to a normal distribution).
	 *   - If skewness is positive, the distribution is skewed to the right (has a longer right tail).
	 *   - If skewness is negative, the distribution is skewed to the left (has a longer left tail).
	 *
	 * Kurtosis:
	 *   - If kurtosis is close to 3, the distribution has a similar tail heaviness to a normal distribution.
	 *   - If kurtosis is greater than 3, the distribution has heavier tails than a normal distribution (more outliers).
	 *   - If kurtosis is less than 3, the distribution has lighter tails than a normal distribution (fewer outliers).
	 *
	 * The further the skewness is from 0 and the kurtosis is from 3, the more your distribution deviates from a normal distribution.
	 *
	 */

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			double CurrentDeviation = Grid[i][j].Value - Debug_ResultRawMean;
			Debug_ResultRawSkewness += pow(CurrentDeviation, 3);
			Debug_ResultRawKurtosis += pow(CurrentDeviation, 4);
		}
	}

	Debug_ResultRawSkewness /= (Grid.size() * Grid[0].size() * pow(Debug_ResultRawStandardDeviation, 3));
	Debug_ResultRawKurtosis /= (Grid.size() * Grid[0].size() * pow(Debug_ResultRawStandardDeviation, 4));

	int y = 0;
	y++;
}

void LayerRasterizationManager::GatherGridRasterizationThreadWork(void* OutputData)
{
	std::vector<GridUpdateTask>* Result = reinterpret_cast<std::vector<GridUpdateTask>*>(OutputData);

	LAYER_RASTERIZATION_MANAGER.MainThreadGridUpdateTasks.insert(LAYER_RASTERIZATION_MANAGER.MainThreadGridUpdateTasks.end(), Result->begin(), Result->end());
	LAYER_RASTERIZATION_MANAGER.GatherGridRasterizationThreadCount++;
	delete Result;

	if (LAYER_RASTERIZATION_MANAGER.GatherGridRasterizationThreadCount == 10)
		LAYER_RASTERIZATION_MANAGER.AfterAllGridRasterizationThreadFinished();
}

void LayerRasterizationManager::ExportCurrentLayerAsMap(MeshLayer* LayerToExport)
{
	CurrentLayer = LayerToExport;
	if (CurrentLayer == nullptr)
		return;

	CurrentUpAxis = ConvertToClosestAxis(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->GetAverageNormal());

	Grid = GenerateGridProjection(MESH_MANAGER.ActiveMesh->GetAABB(), CurrentUpAxis, CurrentResolution);

	int NumberOfTrianglesPerThread = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() / 10);

	std::vector<GridRasterizationThreadData*> ThreadData;
	for (int i = 0; i < 10; i++)
	{
		GridRasterizationThreadData* NewThreadData = new GridRasterizationThreadData();
		NewThreadData->Grid = &Grid;
		NewThreadData->UpAxis = CurrentUpAxis;
		NewThreadData->Resolution = CurrentResolution;
		NewThreadData->FirstIndexInTriangleArray = i * NumberOfTrianglesPerThread;

		if (i == 9)
			NewThreadData->LastIndexInTriangleArray = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() - 1;
		else
			NewThreadData->LastIndexInTriangleArray = (i + 1) * NumberOfTrianglesPerThread;

		std::vector<GridUpdateTask>* OutputTasks = new std::vector<GridUpdateTask>();
		ThreadData.push_back(NewThreadData);

		THREAD_POOL.Execute(GridRasterizationThread, NewThreadData, OutputTasks, GatherGridRasterizationThreadWork);
	}
}

int LayerRasterizationManager::GetGridRasterizationMode()
{
	return GridRasterizationMode;
}

void LayerRasterizationManager::SetGridRasterizationMode(int NewValue)
{
	if (NewValue < 0 || NewValue > 3)
		return;

	GridRasterizationMode = NewValue;
}

int LayerRasterizationManager::GetGridResolution()
{
	return CurrentResolution;
}

void LayerRasterizationManager::SetGridResolution(int NewValue)
{
	if (NewValue < 2 || NewValue > 4096)
		return;

	CurrentResolution = NewValue;
}

int LayerRasterizationManager::GetCumulativeOutliers()
{
	return CumulativeOutliersUpper;
}

void LayerRasterizationManager::SetCumulativeOutliers(int NewValue)
{
	if (NewValue < 0 || NewValue > 99)
		return;

	CumulativeOutliersUpper = NewValue;
}