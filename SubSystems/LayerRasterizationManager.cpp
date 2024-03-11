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
					{
						if (LAYER_RASTERIZATION_MANAGER.GridRasterizationMode == LAYER_RASTERIZATION_MANAGER.GridRasterizationModeMean ||
							LAYER_RASTERIZATION_MANAGER.GridRasterizationMode == LAYER_RASTERIZATION_MANAGER.GridRasterizationModeCumulative)
						{
							double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
							if (CurrentTrianlgeArea > 0)
							{
								Output->push_back(GridUpdateTask(i, j, l, CurrentTrianlgeArea));
							}
						}
						else
						{
							Output->push_back(GridUpdateTask(i, j, l));
						}
						/*double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
						Output->push_back(GridUpdateTask(i, j, l));*/
					}
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
					{
						if (LAYER_RASTERIZATION_MANAGER.GridRasterizationMode == LAYER_RASTERIZATION_MANAGER.GridRasterizationModeMean ||
							LAYER_RASTERIZATION_MANAGER.GridRasterizationMode == LAYER_RASTERIZATION_MANAGER.GridRasterizationModeCumulative)
						{
							double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
							if (CurrentTrianlgeArea > 0)
							{
								Output->push_back(GridUpdateTask(i, j, l, CurrentTrianlgeArea));
							}
						}
						else
						{
							Output->push_back(GridUpdateTask(i, j, l));
						}
						/*double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
						Output->push_back(GridUpdateTask(i, j, l));*/
					}
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
					{
						if (LAYER_RASTERIZATION_MANAGER.GridRasterizationMode == LAYER_RASTERIZATION_MANAGER.GridRasterizationModeMean ||
							LAYER_RASTERIZATION_MANAGER.GridRasterizationMode == LAYER_RASTERIZATION_MANAGER.GridRasterizationModeCumulative)
						{
							double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
							if (CurrentTrianlgeArea > 0)
							{
								Output->push_back(GridUpdateTask(i, j, l, CurrentTrianlgeArea));
							}
						}
						else
						{
							Output->push_back(GridUpdateTask(i, j, l));
						}
						/*double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
						Output->push_back(GridUpdateTask(i, j, l));*/
					}
				}
			}
		}
	}
}

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
	SortPointsByAngle(points);

	std::vector<glm::dvec2> glmPoints;
	for (size_t i = 0; i < points.size(); i++)
	{
		if (CurrentUpAxis.x > 0.0)
			glmPoints.push_back(glm::dvec2(points[i].y, points[i].z));
		else if (CurrentUpAxis.y > 0.0)
			glmPoints.push_back(glm::dvec2(points[i].x, points[i].z));
		else if (CurrentUpAxis.z > 0.0)
			glmPoints.push_back(glm::dvec2(points[i].x, points[i].y));
	}

	return abs(CalculatePolygonArea(std::vector<glm::dvec2>(points.begin(), points.end())));
}

#include <CGAL/Bbox_3.h>
#include <CGAL/Triangle_3.h>
#include <CGAL/intersections.h>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;
typedef Kernel::Triangle_3 Triangle_3;
typedef CGAL::Bbox_3 Bbox_3;

void LayerRasterizationManager::AfterAllGridRasterizationThreadFinished()
{
	Debug_TotalAreaUsed = 0.0;

	for (int i = 0; i < MainThreadGridUpdateTasks.size(); i++)
	{
		Grid[MainThreadGridUpdateTasks[i].FirstIndex][MainThreadGridUpdateTasks[i].SecondIndex].TrianglesInCell.push_back(MainThreadGridUpdateTasks[i].TriangleIndexToAdd);
		Grid[MainThreadGridUpdateTasks[i].FirstIndex][MainThreadGridUpdateTasks[i].SecondIndex].TrianglesInCellArea.push_back(MainThreadGridUpdateTasks[i].TriangleArea);
	}

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			if (!Grid[i][j].TrianglesInCell.empty())
			{
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
					int TriangleWithAreaCount = 0;

					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						double CurrentTrianlgeArea = GetTriangleIntersectionArea(i, j, Grid[i][j].TrianglesInCell[k]);

						if (CurrentTrianlgeArea != 0.0 && !isnan(CurrentTrianlgeArea))
						{
							TriangleWithAreaCount++;
							//float CurrentTriangleCoef = CurrentTrianlgeArea / CurrentCellTotalArea;
							FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]] /** CurrentTriangleCoef*/;
						}
					}

					FinalResult /= static_cast<float>(TriangleWithAreaCount);
				}
				else if (GridRasterizationMode == GridRasterizationModeCumulative)
				{
					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						//double CurrentTrianlgeArea = GetTriangleIntersectionArea(i, j, Grid[i][j].TrianglesInCell[k]);

						
						double CurrentTrianlgeArea = Grid[i][j].TrianglesInCellArea[k];
						if (CurrentTrianlgeArea < 0.0)
							CurrentTrianlgeArea = 0.0;

						if (CurrentTrianlgeArea != 0.0 && !isnan(CurrentTrianlgeArea))
						{
							Debug_TotalAreaUsed += CurrentTrianlgeArea;
							//double CurrentTriangleCoef = CurrentTrianlgeArea / CurrentCellTotalArea;
							double CurrentTriangleCoef = CurrentTrianlgeArea / COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]];
							double CurrentTriangleValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
							FinalResult += CurrentTriangleValue * CurrentTrianlgeArea;

							//FinalResult += CurrentTriangleValue * CurrentTriangleCoef;

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

	UpdateGridDebugDistributionInfo();

	SaveGridRasterizationToFile("test.png");
	OnCalculationsEnd();
}

void LayerRasterizationManager::SaveGridRasterizationToFile(const std::string FilePath)
{
	if (b16BitsExport)
	{
		std::vector<unsigned char> ImageRawData;
		ImageRawData.resize(Grid.size() * Grid[0].size() * 2);
		
		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				float NormalizedValue = (Grid[i][j].Value - Debug_ResultRawMin) / (Debug_ResultRawMax - Debug_ResultRawMin);

				unsigned short Value = static_cast<unsigned short>(NormalizedValue * 65535.0f);
				ImageRawData[(i * Grid[i].size() + j) * 2 + 0] = static_cast<unsigned char>(Value & 0x00FF);
				ImageRawData[(i * Grid[i].size() + j) * 2 + 1] = static_cast<unsigned char>((Value & 0xFF00) >> 8);
			}
		}

		lodepng::encode(FilePath, ImageRawData, Grid.size(), Grid[0].size(), LCT_GREY, 16);
		return;
	}

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
				FlattenGrid.push_back(Grid[i][j].Value);
			}
		}

		JITTER_MANAGER.AdjustOutliers(FlattenGrid, CumulativeOutliersLower / 100.0f, CumulativeOutliersUpper / 100.0f);

		// Updated the grid values
		int Index = 0;
		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				Grid[i][j].Value = FlattenGrid[Index];
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
		// Flip the image diagonally
		std::vector<unsigned char> FlippedImageRawData;
		FlippedImageRawData.reserve(CurrentResolution * CurrentResolution * 4);

		for (int i = 0; i < CurrentResolution; i++)
		{
			for (int j = 0; j < CurrentResolution; j++)
			{
				int index = ((CurrentResolution - 1 - j) * CurrentResolution + (CurrentResolution - 1 - i)) * 4;
				FlippedImageRawData.push_back(ImageRawData[index + 0]);
				FlippedImageRawData.push_back(ImageRawData[index + 1]);
				FlippedImageRawData.push_back(ImageRawData[index + 2]);
				FlippedImageRawData.push_back(ImageRawData[index + 3]);
			}
		}

		lodepng::encode("test.png", FlippedImageRawData, CurrentResolution, CurrentResolution);
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
}

void LayerRasterizationManager::GatherGridRasterizationThreadWork(void* OutputData)
{
	std::vector<GridUpdateTask>* Result = reinterpret_cast<std::vector<GridUpdateTask>*>(OutputData);

	LAYER_RASTERIZATION_MANAGER.MainThreadGridUpdateTasks.insert(LAYER_RASTERIZATION_MANAGER.MainThreadGridUpdateTasks.end(), Result->begin(), Result->end());
	LAYER_RASTERIZATION_MANAGER.GatherGridRasterizationThreadCount++;
	delete Result;

	LAYER_RASTERIZATION_MANAGER.Progress = static_cast<float>(LAYER_RASTERIZATION_MANAGER.GatherGridRasterizationThreadCount) / static_cast<float>(LAYER_RASTERIZATION_MANAGER.THREAD_COUNT);

	if (LAYER_RASTERIZATION_MANAGER.GatherGridRasterizationThreadCount == LAYER_RASTERIZATION_MANAGER.THREAD_COUNT)
		LAYER_RASTERIZATION_MANAGER.AfterAllGridRasterizationThreadFinished();
}

void LayerRasterizationManager::ExportCurrentLayerAsMap(MeshLayer* LayerToExport)
{
	CurrentLayer = LayerToExport;
	if (CurrentLayer == nullptr)
		return;

	LAYER_RASTERIZATION_MANAGER.THREAD_COUNT = THREAD_POOL.GetThreadCount() - 1;
	if (LAYER_RASTERIZATION_MANAGER.THREAD_COUNT < 1)
		LAYER_RASTERIZATION_MANAGER.THREAD_COUNT = 1;

	OnCalculationsStart();

	CurrentUpAxis = ConvertToClosestAxis(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->GetAverageNormal());

	Grid = GenerateGridProjection(MESH_MANAGER.ActiveMesh->GetAABB(), CurrentUpAxis, CurrentResolution);

	int NumberOfTrianglesPerThread = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() / LAYER_RASTERIZATION_MANAGER.THREAD_COUNT);

	std::vector<GridRasterizationThreadData*> ThreadData;
	for (int i = 0; i < LAYER_RASTERIZATION_MANAGER.THREAD_COUNT; i++)
	{
		GridRasterizationThreadData* NewThreadData = new GridRasterizationThreadData();
		NewThreadData->Grid = &Grid;
		NewThreadData->UpAxis = CurrentUpAxis;
		NewThreadData->Resolution = CurrentResolution;
		NewThreadData->FirstIndexInTriangleArray = i * NumberOfTrianglesPerThread;

		if (i == LAYER_RASTERIZATION_MANAGER.THREAD_COUNT - 1)
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

void LayerRasterizationManager::OnCalculationsStart()
{
	LAYER_RASTERIZATION_MANAGER.Progress = 0.0f;

	for (size_t i = 0; i < LAYER_RASTERIZATION_MANAGER.OnCalculationsStartCallbacks.size(); i++)
	{
		if (LAYER_RASTERIZATION_MANAGER.OnCalculationsStartCallbacks[i] != nullptr)
			LAYER_RASTERIZATION_MANAGER.OnCalculationsStartCallbacks[i]();
	}
}

void LayerRasterizationManager::OnCalculationsEnd()
{
	LAYER_RASTERIZATION_MANAGER.Progress = 1.0f;

	LAYER_RASTERIZATION_MANAGER.Grid.clear();
	LAYER_RASTERIZATION_MANAGER.MainThreadGridUpdateTasks.clear();
	LAYER_RASTERIZATION_MANAGER.GatherGridRasterizationThreadCount = 0;
	LAYER_RASTERIZATION_MANAGER.CurrentLayer = nullptr;
	LAYER_RASTERIZATION_MANAGER.CurrentUpAxis = glm::vec3(0.0f);

	for (size_t i = 0; i < LAYER_RASTERIZATION_MANAGER.OnCalculationsEndCallbacks.size(); i++)
	{
		if (LAYER_RASTERIZATION_MANAGER.OnCalculationsEndCallbacks[i] != nullptr)
			LAYER_RASTERIZATION_MANAGER.OnCalculationsEndCallbacks[i]();
	}
}

void LayerRasterizationManager::SetOnCalculationsStartCallback(std::function<void()> Func)
{
	OnCalculationsStartCallbacks.push_back(Func);
}

void LayerRasterizationManager::SetOnCalculationsEndCallback(std::function<void()> Func)
{
	OnCalculationsEndCallbacks.push_back(Func);
}

float LayerRasterizationManager::GetProgress()
{
	return Progress;
}

double LayerRasterizationManager::GetTriangleIntersectionArea(int GridX, int GridY, int TriangleIndex)
{
	double Result = 0.0;

	auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[TriangleIndex];
	if (CurrentTriangle.size() != 3)
		return Result;

	std::vector<glm::dvec3> CurrentTriangleDouble;
	CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z));
	CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z));
	CurrentTriangleDouble.push_back(glm::dvec3(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z));

	if (bUsingCGAL)
	{
		// Define a triangle
		Point_3 PointA(CurrentTriangleDouble[0].x, CurrentTriangleDouble[0].y, CurrentTriangleDouble[0].z);
		Point_3 PointB(CurrentTriangleDouble[1].x, CurrentTriangleDouble[1].y, CurrentTriangleDouble[1].z);
		Point_3 PointC(CurrentTriangleDouble[2].x, CurrentTriangleDouble[2].y, CurrentTriangleDouble[2].z);
		Triangle_3 Triangle(PointA, PointB, PointC);

		// Define an AABB
		Bbox_3 AABB(Grid[GridX][GridY].AABB.GetMin().x, Grid[GridX][GridY].AABB.GetMin().y, Grid[GridX][GridY].AABB.GetMin().z,
					Grid[GridX][GridY].AABB.GetMax().x, Grid[GridX][GridY].AABB.GetMax().y, Grid[GridX][GridY].AABB.GetMax().z);

		// Compute the intersection between the triangle and the AABB
		auto CGALIntersection = CGAL::intersection(Triangle, AABB);

		// Check the type of the intersection result and calculate the area
		if (CGALIntersection)
		{
			if (const Triangle_3* intersect_triangle = boost::get<Triangle_3>(&*CGALIntersection))
			{
				// Project the 3D triangle onto a 2D plane
				Kernel::Point_2 p1_2d(intersect_triangle->vertex(0).x(), intersect_triangle->vertex(0).z());
				Kernel::Point_2 p2_2d(intersect_triangle->vertex(1).x(), intersect_triangle->vertex(1).z());
				Kernel::Point_2 p3_2d(intersect_triangle->vertex(2).x(), intersect_triangle->vertex(2).z());

				// Using absolute value to ensure the area is positive
				// CGAL will return a negative area if the vertices are ordered clockwise
				Result = abs(CGAL::to_double(CGAL::area(p1_2d, p2_2d, p3_2d)));
			}
			else if (const Kernel::Segment_3* intersect_segment = boost::get<Kernel::Segment_3>(&*CGALIntersection))
			{
				// No need to calculate the area, as it will be zero.
			}
			else if (const Point_3* intersect_point = boost::get<Point_3>(&*CGALIntersection))
			{
				// No need to calculate the area, as it will be zero.
			}
			else if (const std::vector<Point_3>* intersect_points = boost::get<std::vector<Point_3>>(&*CGALIntersection))
			{
				// Create a polygon from the intersection points
				Polygon_2_ polygon;
				for (const auto& point : *intersect_points) {
					polygon.push_back(Point_2_(point.x(), point.z()));
				}

				// Using absolute value to ensure the area is positive
				// CGAL will return a negative area if the vertices are ordered clockwise
				Result = abs(CGAL::to_double(polygon.area()));
			}
		}
	}
	else
	{
		std::vector<glm::dvec3> IntersectionPoints = GEOMETRY.GetIntersectionPoints(Grid[GridX][GridY].AABB, CurrentTriangleDouble);

		for (size_t l = 0; l < CurrentTriangleDouble.size(); l++)
		{
			if (Grid[GridX][GridY].AABB.ContainsPoint(CurrentTriangleDouble[l]))
			{
				bool bAlreadyExists = false;
				int PointsThatAreNotSame = 0;
				for (size_t q = 0; q < IntersectionPoints.size(); q++)
				{
					if (abs(IntersectionPoints[q] - CurrentTriangleDouble[l]).x > glm::dvec3(DBL_EPSILON).x ||
						abs(IntersectionPoints[q] - CurrentTriangleDouble[l]).y > glm::dvec3(DBL_EPSILON).y ||
						abs(IntersectionPoints[q] - CurrentTriangleDouble[l]).z > glm::dvec3(DBL_EPSILON).z)
					{
						PointsThatAreNotSame++;
					}
				}

				if (PointsThatAreNotSame != IntersectionPoints.size())
					bAlreadyExists = true;

				if (!bAlreadyExists)
					IntersectionPoints.push_back(CurrentTriangle[l]);
			}
		}

		Result = GetArea(IntersectionPoints);
	}

	return Result;
}