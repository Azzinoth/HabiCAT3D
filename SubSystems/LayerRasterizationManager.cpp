#include "LayerRasterizationManager.h"
using namespace FocalEngine;

LayerRasterizationManager::LayerRasterizationManager() {}
LayerRasterizationManager::~LayerRasterizationManager() {}

glm::vec3 LayerRasterizationManager::ConvertToClosestAxis(const glm::vec3& Vector)
{
	// Calculate the absolute values of the vector components
	float AbsoluteX = glm::abs(Vector.x);
	float AbsoluteY = glm::abs(Vector.y);
	float AbsoluteZ = glm::abs(Vector.z);

	// Determine the largest component
	if (AbsoluteX > AbsoluteY && AbsoluteX > AbsoluteZ)
	{
		// X component is largest, so vector is closest to the X axis
		return glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else if (AbsoluteY > AbsoluteX && AbsoluteY > AbsoluteZ)
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

std::vector<std::vector<LayerRasterizationManager::GridCell>> LayerRasterizationManager::GenerateGridProjection(const glm::vec3& Axis)
{
	std::vector<std::vector<GridCell>> Grid;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Grid;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Grid;

	if (Axis.x + Axis.y + Axis.z != 1.0f)
		return Grid;

	glm::vec2 MinMaxResolutionInMeters = GetMinMaxResolutionInMeters(Axis);
	if (CurrentResolutionInMeters > MinMaxResolutionInMeters.x)
		CurrentResolutionInMeters = MinMaxResolutionInMeters.x;

	if (CurrentResolutionInMeters < MinMaxResolutionInMeters.y)
		CurrentResolutionInMeters = MinMaxResolutionInMeters.y;

	FEAABB MeshAABB = CurrentMeshAnalysisData->GetAABB();
	float CellSize = CurrentResolutionInMeters;

	if (CellSize <= 0.0f)
		return Grid;

	glm::vec3 CellDimension = glm::vec3(CellSize);

	if (Axis.x > 0.0)
	{
		CellDimension.x = MeshAABB.GetSize().x;
	}
	else if (Axis.y > 0.0)
	{
		CellDimension.y = MeshAABB.GetSize().y;
	}
	else if (Axis.z > 0.0)
	{
		CellDimension.z = MeshAABB.GetSize().z;
	}

	glm::uvec2 ResolutionXY = GetResolutionInPixelsBasedOnResolutionInMeters(Axis, CurrentResolutionInMeters);
	CurrentResolution = glm::max(ResolutionXY.x, ResolutionXY.y);

	Grid.resize(CurrentResolution);
	for (int i = 0; i < CurrentResolution; i++)
	{
		Grid[i].resize(CurrentResolution);
	}

	// Loop through each division to create the grid
	for (int i = 0; i < CurrentResolution; i++)
	{
		for (int j = 0; j < CurrentResolution; j++)
		{
			// Calculate min and max for the current cell
			glm::vec3 CellMin = MeshAABB.GetMin();
			glm::vec3 CellMax = MeshAABB.GetMin() + CellDimension;

			if (Axis.x > 0.0f)
			{
				CellMin.y += CellDimension.y * i;
				CellMax.y += CellDimension.y * i;

				CellMin.z += CellDimension.z * j;
				CellMax.z += CellDimension.z * j;
			}
			else if (Axis.y > 0.0f)
			{
				CellMin.x += CellDimension.x * i;
				CellMax.x += CellDimension.x * i;

				CellMin.z += CellDimension.z * j;
				CellMax.z += CellDimension.z * j;
			}
			else if (Axis.z > 0.0f)
			{
				CellMin.x += CellDimension.x * i;
				CellMax.x += CellDimension.x * i;

				CellMin.y += CellDimension.y * j;
				CellMax.y += CellDimension.y * j;
			}

			// Create a new AABB for the grid cell
			GridCell NewCell;
			NewCell.AABB = FEAABB(CellMin, CellMax);
			Grid[i][j] = NewCell;
		}
	}

	return Grid;
}

void LayerRasterizationManager::GridRasterizationThread(void* InputData, void* OutputData)
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

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
		for (int l = FirstIndexInTriangleArray; l <= LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(CurrentMeshAnalysisData->Triangles[l]);

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
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, CurrentMeshAnalysisData->Triangles[l]))
					{
						if (LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Mean ||
							LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Cumulative)
						{
							double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
							if (CurrentTrianlgeArea > 0)
							{
								Output->push_back(GridUpdateTask(static_cast<int>(i), static_cast<int>(j), l, CurrentTrianlgeArea));
							}
						}
						else
						{
							Output->push_back(GridUpdateTask(static_cast<int>(i), static_cast<int>(j), l));
						}
					}
				}
			}
		}
	}
	if (UpAxis.y > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l <= LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(CurrentMeshAnalysisData->Triangles[l]);

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
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, CurrentMeshAnalysisData->Triangles[l]))
					{
						if (LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Mean ||
							LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Cumulative)
						{
							double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
							if (CurrentTrianlgeArea > 0)
							{
								Output->push_back(GridUpdateTask(static_cast<int>(i), static_cast<int>(j), l, CurrentTrianlgeArea));
							}
						}
						else
						{
							Output->push_back(GridUpdateTask(static_cast<int>(i), static_cast<int>(j), l));
						}
					}
				}
			}
		}
	}
	else if (UpAxis.z > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l <= LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(CurrentMeshAnalysisData->Triangles[l]);

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
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, CurrentMeshAnalysisData->Triangles[l]))
					{
						if (LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Mean ||
							LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Cumulative)
						{
							double CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetTriangleIntersectionArea(i, j, l);
							if (CurrentTrianlgeArea > 0)
							{
								Output->push_back(GridUpdateTask(static_cast<int>(i), static_cast<int>(j), l, CurrentTrianlgeArea));
							}
						}
						else
						{
							Output->push_back(GridUpdateTask(static_cast<int>(i), static_cast<int>(j), l));
						}
					}
				}
			}
		}
	}
}

glm::dvec3 LayerRasterizationManager::CalculateCentroid(const std::vector<glm::dvec3>& Points)
{
	glm::dvec3 Centroid(0.0, 0.0, 0.0);
	for (const auto& Point : Points)
		Centroid += Point;

	Centroid /= static_cast<double>(Points.size());
	return Centroid;
}

bool LayerRasterizationManager::CompareAngles(const glm::dvec3& A, const glm::dvec3& B, const glm::dvec3& Centroid)
{
	double AngleA = 0.0;
	double AngleB = 0.0;

	if (CurrentProjectionVector.x > 0.0)
	{
		AngleA = atan2(A.z - Centroid.z, A.y - Centroid.y);
		AngleB = atan2(B.z - Centroid.z, B.y - Centroid.y);
	}
	else if (CurrentProjectionVector.y > 0.0)
	{
		AngleA = atan2(A.z - Centroid.z, A.x - Centroid.x);
		AngleB = atan2(B.z - Centroid.z, B.x - Centroid.x);
	}
	else if (CurrentProjectionVector.z > 0.0)
	{
		AngleA = atan2(A.y - Centroid.y, A.x - Centroid.x);
		AngleB = atan2(B.y - Centroid.y, B.x - Centroid.x);
	}
	
	return AngleA < AngleB;
}

void LayerRasterizationManager::SortPointsByAngle(std::vector<glm::dvec3>& Points)
{
	glm::dvec3 Centroid = CalculateCentroid(Points);

	std::sort(Points.begin(), Points.end(), [&](const glm::dvec3& A, const glm::dvec3& B) {
		return CompareAngles(A, B, Centroid);
	});
}

double LayerRasterizationManager::CalculatePolygonArea(const std::vector<glm::dvec2>& Points)
{
	Polygon_2 Polygon;
	for (const auto& Point : Points)
		Polygon.push_back(Point_2(Point.x, Point.y));

	// Check if the polygon is simple (does not self-intersect)
	if (!Polygon.is_simple()) 
		return 0.0;

	double Area = CGAL::to_double(Polygon.area());
	return Area;
}

double LayerRasterizationManager::GetArea(std::vector<glm::dvec3>& Points)
{
	SortPointsByAngle(Points);

	std::vector<glm::dvec2> ProjectedPoints;
	for (size_t i = 0; i < Points.size(); i++)
	{
		if (CurrentProjectionVector.x > 0.0)
			ProjectedPoints.push_back(glm::dvec2(Points[i].y, Points[i].z));
		else if (CurrentProjectionVector.y > 0.0)
			ProjectedPoints.push_back(glm::dvec2(Points[i].x, Points[i].z));
		else if (CurrentProjectionVector.z > 0.0)
			ProjectedPoints.push_back(glm::dvec2(Points[i].x, Points[i].y));
	}

	return abs(CalculatePolygonArea(std::vector<glm::dvec2>(Points.begin(), Points.end())));
}

void LayerRasterizationManager::AfterAllGridRasterizationThreadFinished()
{

	for (size_t i = 0; i < TemporaryThreadDataArray.size(); i++)
	{
		delete TemporaryThreadDataArray[i];
	}
	TemporaryThreadDataArray.clear();

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

				if (Mode == GridRasterizationMode::Min)
				{
					float MinValue = FLT_MAX;

					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						float CurrentValue = CurrentLayer->ElementsToData[Grid[i][j].TrianglesInCell[k]];
						if (CurrentValue < MinValue)
							MinValue = CurrentValue;
					}

					FinalResult = MinValue;
				}
				else if (Mode == GridRasterizationMode::Max)
				{
					float MaxValue = -FLT_MAX;

					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						float CurrentValue = CurrentLayer->ElementsToData[Grid[i][j].TrianglesInCell[k]];
						if (CurrentValue > MaxValue)
							MaxValue = CurrentValue;
					}

					FinalResult = MaxValue;

				}
				else if (Mode == GridRasterizationMode::Mean)
				{
					int TriangleWithAreaCount = 0;

					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						double CurrentTrianlgeArea = Grid[i][j].TrianglesInCellArea[k];

						if (CurrentTrianlgeArea != 0.0 && !isnan(CurrentTrianlgeArea))
						{
							TriangleWithAreaCount++;
							FinalResult += CurrentLayer->ElementsToData[Grid[i][j].TrianglesInCell[k]];
						}
					}

					FinalResult /= static_cast<float>(TriangleWithAreaCount);
				}
				else if (Mode == GridRasterizationMode::Cumulative)
				{
					for (int k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						double CurrentTrianlgeArea = Grid[i][j].TrianglesInCellArea[k];
						if (CurrentTrianlgeArea < 0.0)
							CurrentTrianlgeArea = 0.0;

						if (CurrentTrianlgeArea != 0.0 && !isnan(CurrentTrianlgeArea))
						{
							Debug_TotalAreaUsed += CurrentTrianlgeArea;
							double CurrentTriangleValue = CurrentLayer->ElementsToData[Grid[i][j].TrianglesInCell[k]];
							FinalResult += static_cast<float>(CurrentTriangleValue * CurrentTrianlgeArea);
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
	PrepareRawImageData();
	OnCalculationsEnd();
}

void LayerRasterizationManager::PrepareRawImageData()
{
	std::vector<std::vector<float>> RawDataCopy;
	RawDataCopy.resize(Grid.size());
	for (int i = 0; i < Grid.size(); i++)
	{
		RawDataCopy[i].resize(Grid[i].size());
	}

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			RawDataCopy[i][j] = Grid[i][j].Value;
		}
	}

	float MinForColorMap = CurrentLayer->MinVisible;
	float MaxForColorMap = CurrentLayer->MaxVisible;

	if (Mode == GridRasterizationMode::Cumulative)
	{
		FEAABB TempAABB = Grid[0][0].AABB;

		double AABBWidth = 0.0;
		double AABBHeight = 0.0;
		if (CurrentProjectionVector.x > 0.0)
		{
			AABBWidth = TempAABB.GetMax().y - TempAABB.GetMin().y;
			AABBHeight = TempAABB.GetMax().z - TempAABB.GetMin().z;
		}
		else if (CurrentProjectionVector.y > 0.0)
		{
			AABBWidth = TempAABB.GetMax().x - TempAABB.GetMin().x;
			AABBHeight = TempAABB.GetMax().z - TempAABB.GetMin().z;
		}
		else if (CurrentProjectionVector.z > 0.0)
		{
			AABBWidth = TempAABB.GetMax().x - TempAABB.GetMin().x;
			AABBHeight = TempAABB.GetMax().y - TempAABB.GetMin().y;
		}

		double UnitArea = (AABBWidth * AABBHeight);
		MinForColorMap = static_cast<float>(UnitArea);

		std::vector<float> FlattenGridValues;
		FlattenGridValues.reserve(Grid.size() * Grid[0].size());

		std::vector<float> FlattenGridArea;
		FlattenGridArea.reserve(Grid.size() * Grid[0].size());

		for (int i = 0; i < Grid.size(); i++)
		{
			for (int j = 0; j < Grid[i].size(); j++)
			{
				FlattenGridValues.push_back(Grid[i][j].Value);

				float AreaInCell = 0.0f;
				for (size_t k = 0; k < Grid[i][j].TrianglesInCellArea.size(); k++)
				{
					AreaInCell += static_cast<float>(Grid[i][j].TrianglesInCellArea[k]);
				}

				FlattenGridArea.push_back(AreaInCell);
			}
		}

		MaxForColorMap = JITTER_MANAGER.GetValueThatHaveAtLeastThisPercentOfArea(FlattenGridValues, FlattenGridArea, PercentOfAreaThatWouldBeRed / 100.0f);

		if (MaxForColorMap <= MinForColorMap)
			MaxForColorMap = MinForColorMap + FLT_EPSILON * 4;
	}

	std::vector<unsigned char> ImageRawData;
	ImageRawData.reserve(CurrentResolution * CurrentResolution * 4);
	FinalImageRawData.reserve(CurrentResolution * CurrentResolution * 4);

	ImageRawData32Bits.clear();
	ImageRawData32Bits.reserve(CurrentResolution * CurrentResolution);

	for (int i = 0; i < RawDataCopy.size(); i++)
	{
		for (int j = 0; j < RawDataCopy[i].size(); j++)
		{
			// Using RawDataCopy[j][i] instead of RawDataCopy[i][j] to rotate the image by 90 degrees
			if (Grid[j][i].TrianglesInCell.empty() || RawDataCopy[j][i] == 0.0f)
			{
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));
				ImageRawData.push_back(static_cast<unsigned char>(0));

				ImageRawData32Bits.push_back(0.0f);
			}
			else
			{
				// Normalize the value to the range [0, 1] (0 = min, 1 = max)
				float NormalizedValue = (RawDataCopy[j][i] - MinForColorMap) / (MaxForColorMap - MinForColorMap);

				// NormalizedValue could be out of range due to fact that we need to supress the outliers
				if (NormalizedValue > 1.0f)
					NormalizedValue = 1.0f;

				if (NormalizedValue < 0.0f)
					NormalizedValue = 0.0f;

				glm::vec3 Color = GetTurboColorMap(NormalizedValue);

				unsigned char R = static_cast<unsigned char>(Color.x * 255.0f);
				ImageRawData.push_back(R);
				unsigned char G = static_cast<unsigned char>(Color.y * 255.0f);
				ImageRawData.push_back(G);
				unsigned char B = static_cast<unsigned char>(Color.z * 255.0f);
				ImageRawData.push_back(B);
				ImageRawData.push_back(static_cast<unsigned char>(255));

				ImageRawData32Bits.push_back(RawDataCopy[j][i]);
			}
		}
	}

	if (CurrentProjectionVector.x > 0.0)
	{
		// Flip the image diagonally
		std::vector<unsigned char> FlippedImageRawData;
		FlippedImageRawData.reserve(CurrentResolution * CurrentResolution * 4);
		std::vector<float> FlippedImageRawData32Bits;
		FlippedImageRawData32Bits.reserve(CurrentResolution * CurrentResolution);

		for (int i = 0; i < CurrentResolution; i++)
		{
			for (int j = 0; j < CurrentResolution; j++)
			{
				int index = ((CurrentResolution - 1 - j) * CurrentResolution + (CurrentResolution - 1 - i)) * 4;
				FlippedImageRawData.push_back(ImageRawData[index + 0]);
				FlippedImageRawData.push_back(ImageRawData[index + 1]);
				FlippedImageRawData.push_back(ImageRawData[index + 2]);
				FlippedImageRawData.push_back(ImageRawData[index + 3]);

				FlippedImageRawData32Bits.push_back(ImageRawData32Bits[(CurrentResolution - 1 - j) * CurrentResolution + (CurrentResolution - 1 - i)]);
			}
		}

		FinalImageRawData = FlippedImageRawData;
		ImageRawData32Bits = FlippedImageRawData32Bits;
	}
	if (CurrentProjectionVector.y > 0.0)
	{
		FinalImageRawData = ImageRawData;
	}
	if (CurrentProjectionVector.z > 0.0)
	{
		// Flip the image vertically
		std::vector<unsigned char> FlippedImageRawData;
		FlippedImageRawData.reserve(CurrentResolution* CurrentResolution * 4);
		std::vector<float> FlippedImageRawData32Bits;
		FlippedImageRawData32Bits.reserve(CurrentResolution* CurrentResolution);

		for (int i = CurrentResolution - 1; i >= 0; i--)
		{
			for (int j = 0; j < CurrentResolution; j++)
			{
				int index = (i * CurrentResolution + j) * 4;
				FlippedImageRawData.push_back(ImageRawData[index + 0]);
				FlippedImageRawData.push_back(ImageRawData[index + 1]);
				FlippedImageRawData.push_back(ImageRawData[index + 2]);
				FlippedImageRawData.push_back(ImageRawData[index + 3]);

				FlippedImageRawData32Bits.push_back(ImageRawData32Bits[i * CurrentResolution + j]);
			}
		}

		FinalImageRawData = FlippedImageRawData;
		ImageRawData32Bits = FlippedImageRawData32Bits;
	}

	if (!APPLICATION.HasConsoleWindow())
	{
		ResultPreview = RESOURCE_MANAGER.RawDataToFETexture(FinalImageRawData.data(), CurrentResolution, CurrentResolution);

		ResultPreview->Bind();
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		ResultPreview->UnBind();

		FinalImageRawData.clear();
	}
}

void LayerRasterizationManager::GDALCopyProjectionAndGeoTransform(GDALDataset* DataSetToChange, std::string ExampleFile)
{
	if (DataSetToChange == nullptr)
		return;

	if (ExampleFile == "")
		return;

	if (!FILE_SYSTEM.DoesFileExist(ExampleFile.c_str()))
		return;

	// Open source dataset
	GDALDataset* SourceDataSet = static_cast<GDALDataset*>(GDALOpen(ExampleFile.c_str(), GA_ReadOnly));
	if (SourceDataSet == nullptr)
		return;

	// Extract projection and geotransformation from source dataset
	const char* Projection = SourceDataSet->GetProjectionRef();
	double GeoTransform[6];
	if (SourceDataSet->GetGeoTransform(GeoTransform) != CE_None)
	{
		GDALClose(SourceDataSet);
		return;
	}

	// Set projection and geotransformation to the target dataset
	DataSetToChange->SetProjection(Projection);
	if (DataSetToChange->SetGeoTransform(GeoTransform) != CE_None)
	{
		GDALClose(SourceDataSet);
		return;
	}

	GDALClose(SourceDataSet);
}

bool LayerRasterizationManager::SaveToFile(std::string FilePath, SaveMode SaveMode)
{
	if (Grid.size() == 0)
		return false;

	if (!APPLICATION.HasConsoleWindow() && ResultPreview == nullptr)
		return false;

	if (SaveMode != SaveAsPNG && SaveMode != SaveAsTIF && SaveMode != SaveAs32bitTIF)
		return false;

	if (SaveMode == SaveAs32bitTIF && ImageRawData32Bits.size() != CurrentResolution * CurrentResolution)
		return false;

	// If the file path does not have an extension, add the appropriate one
	if (FILE_SYSTEM.GetFileExtension(FilePath.c_str()) == "")
	{
		if (SaveMode == SaveAsTIF || SaveMode == SaveAs32bitTIF)
			FilePath += ".tif";
		else
			FilePath += ".png";
	}

	if (SaveMode == SaveAs32bitTIF)
	{
		GDALAllRegister();

		int ImageWidth = CurrentResolution;
		int ImageHeight = CurrentResolution;
		int BandsCount = 1;
		GDALDataType Type = GDT_Float32;

		// Get the GeoTIFF driver
		GDALDriver* Driver = GetGDALDriverManager()->GetDriverByName("GTiff");
		if (Driver == nullptr)
			return false;

		// Create a new GeoTIFF file
		GDALDataset* Dataset = Driver->Create(FilePath.c_str(), ImageWidth, ImageHeight, BandsCount, Type, nullptr);
		if (Dataset == nullptr)
			return false;

		// Set geotransform and projection if necessary
		double GeoTransform[6] = { 0, 1, 0, 0, 0, -1 }; // Generic transformation
		Dataset->SetGeoTransform(GeoTransform);

		// Set the projection using the WKT representation
		const char* WKT = "PROJCS[\"WGS 84 / UTM zone 18N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-75],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32618\"]]";
		Dataset->SetProjection(WKT);

		// Write data to the first band
		GDALRasterBand* Band = Dataset->GetRasterBand(1);
		// Set the NODATA value for the band
		Band->SetNoDataValue(0.0);
		CPLErr Error = Band->RasterIO(GF_Write, 0, 0, ImageWidth, ImageHeight, ImageRawData32Bits.data(), ImageWidth, ImageHeight, Type, 0, 0);
		if (Error != CE_None)
		{
			GDALClose(Dataset);
			return false;
		}

		GDALClose(Dataset);
		return true;
	}

	unsigned char* RawImageData = nullptr;
	if (APPLICATION.HasConsoleWindow())
	{
		RawImageData = FinalImageRawData.data();
	}
	else
	{
		RawImageData = ResultPreview->GetRawData();
	}

	if (SaveMode == SaveAsPNG)
	{
		lodepng::encode(FilePath, RawImageData, CurrentResolution, CurrentResolution);
	}
	else if (SaveMode == SaveAsTIF)
	{
		GDALAllRegister();

		int ImageWidth = CurrentResolution;
		int ImageHeight = CurrentResolution;
		int BandsCount = 4;
		GDALDataType Type = GDT_Byte;

		// Get the GeoTIFF driver
		GDALDriver* Driver = GetGDALDriverManager()->GetDriverByName("GTiff");
		if (Driver == nullptr)
		{
			if (!APPLICATION.HasConsoleWindow())
				delete[] RawImageData;
			return false;
		}

		// Create a new GeoTIFF file
		GDALDataset* Dataset = Driver->Create(FilePath.c_str(), ImageWidth, ImageHeight, BandsCount, Type, nullptr);
		if (Dataset == nullptr)
		{
			if (!APPLICATION.HasConsoleWindow())
				delete[] RawImageData;
			return false;
		}

		// Set geotransform and projection if necessary
		double GeoTransform[6] = { 0, 1, 0, 0, 0, -1 }; // Generic transformation
		Dataset->SetGeoTransform(GeoTransform);

		// Set the projection using the WKT representation
		const char* WKT = "PROJCS[\"WGS 84 / UTM zone 18N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-75],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32618\"]]";
		Dataset->SetProjection(WKT);

		// Write data to the RGBA bands
		for (int CurrentBand = 1; CurrentBand <= 4; CurrentBand++)
		{
			GDALRasterBand* Band = Dataset->GetRasterBand(CurrentBand);
			CPLErr Error = Band->RasterIO(GF_Write, 0, 0, ImageWidth, ImageHeight, RawImageData + (CurrentBand - 1), ImageWidth, ImageHeight, Type, 4, 4 * ImageWidth);
			if (Error != CE_None)
			{
				GDALClose(Dataset);
				if (!APPLICATION.HasConsoleWindow())
					delete[] RawImageData;
				return false;
			}
		}

		GDALClose(Dataset);
	}

	if (!APPLICATION.HasConsoleWindow())
		delete[] RawImageData;
	return true;
}

const COMDLG_FILTERSPEC RASTERIZATION_EXPORT_FILE_FILTER[] =
{
	{ L"PNG file (*.png)", L"*.png"},
	{ L"GeoTiff file (*.tif)", L"*.tif" },
	{ L"GeoTiff file 32-bit gray scale (*.tif)", L"*.tif" }
};

bool LayerRasterizationManager::PromptUserForSaveLocation()
{
	int Index = -1;
	std::string FilePath;
	FILE_SYSTEM.ShowFileSaveDialog(FilePath, RASTERIZATION_EXPORT_FILE_FILTER, 3, &Index);

	if (!FilePath.empty())
	{
		LAYER_RASTERIZATION_MANAGER.SaveToFile(FilePath, static_cast<LayerRasterizationManager::SaveMode>(Index));
		return true;
	}
	
	return false;
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

void LayerRasterizationManager::PrepareLayerForExport(DataLayer* LayerToExport, glm::vec3 ForceProjectionVector)
{
	CurrentLayer = LayerToExport;
	if (CurrentLayer == nullptr)
		return;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	OnCalculationsStart();

	if (!GLMVec3Equal(ForceProjectionVector, glm::vec3(0.0f)))
		CurrentProjectionVector = ForceProjectionVector;
	else if (GLMVec3Equal(CurrentProjectionVector, glm::vec3(0.0f)))
		CurrentProjectionVector = ConvertToClosestAxis(CurrentMeshAnalysisData->GetAverageNormal());

	Grid = GenerateGridProjection(CurrentProjectionVector);

	int NumberOfTrianglesPerThread = static_cast<int>(CurrentMeshAnalysisData->Triangles.size() / LAYER_RASTERIZATION_MANAGER.THREAD_COUNT);

	if (LAYER_RASTERIZATION_MANAGER.THREAD_COUNT > NumberOfTrianglesPerThread)
	{
		LAYER_RASTERIZATION_MANAGER.THREAD_COUNT = 1;
		NumberOfTrianglesPerThread = static_cast<int>(CurrentMeshAnalysisData->Triangles.size());
	}

	TemporaryThreadDataArray.clear();
	for (int i = 0; i < LAYER_RASTERIZATION_MANAGER.THREAD_COUNT; i++)
	{
		GridRasterizationThreadData* NewThreadData = new GridRasterizationThreadData();
		NewThreadData->Grid = &Grid;
		NewThreadData->UpAxis = CurrentProjectionVector;
		NewThreadData->Resolution = CurrentResolution;
		NewThreadData->FirstIndexInTriangleArray = i * NumberOfTrianglesPerThread;

		if (i == LAYER_RASTERIZATION_MANAGER.THREAD_COUNT - 1)
			NewThreadData->LastIndexInTriangleArray = static_cast<int>(CurrentMeshAnalysisData->Triangles.size() - 1);
		else
			NewThreadData->LastIndexInTriangleArray = (i + 1) * NumberOfTrianglesPerThread;

		std::vector<GridUpdateTask>* OutputTasks = new std::vector<GridUpdateTask>();
		TemporaryThreadDataArray.push_back(NewThreadData);

		THREAD_POOL.Execute(GridRasterizationThread, NewThreadData, OutputTasks, GatherGridRasterizationThreadWork);
	}
}

LayerRasterizationManager::GridRasterizationMode LayerRasterizationManager::GetGridRasterizationMode()
{
	return Mode;
}

void LayerRasterizationManager::SetGridRasterizationMode(GridRasterizationMode NewValue)
{
	if (NewValue < 0 || NewValue > 3)
		return;

	Mode = NewValue;
}

int LayerRasterizationManager::GetGridResolution()
{
	return CurrentResolution;
}

float LayerRasterizationManager::GetCumulativeModePercentOfAreaThatWouldBeRed()
{
	return PercentOfAreaThatWouldBeRed;
}

void LayerRasterizationManager::SetCumulativeModePercentOfAreaThatWouldBeRed(float NewValue)
{
	if (NewValue < 0.0f || NewValue > 99.99f)
		return;

	PercentOfAreaThatWouldBeRed = NewValue;
}

void LayerRasterizationManager::OnCalculationsStart()
{
	LAYER_RASTERIZATION_MANAGER.THREAD_COUNT = THREAD_POOL.GetThreadCount() - 1;
	if (LAYER_RASTERIZATION_MANAGER.THREAD_COUNT < 1)
		LAYER_RASTERIZATION_MANAGER.THREAD_COUNT = 1;

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
	LAYER_RASTERIZATION_MANAGER.ClearDataAfterCalculation();

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

Point_2 LayerRasterizationManager::ProjectPointOntoPlane(const Point_3& Point, const Plane_3& Plane)
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

double LayerRasterizationManager::GetTriangleIntersectionArea(size_t GridX, size_t GridY, int TriangleIndex)
{
	double Result = 0.0;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	auto CurrentTriangle = CurrentMeshAnalysisData->Triangles[TriangleIndex];
	if (CurrentTriangle.size() != 3)
		return Result;

	if (CurrentMeshAnalysisData->TrianglesArea[TriangleIndex] == 0.0)
		return Result;

	if (bUsingCGAL)
	{
		// Define a triangle
		Point_3 PointA(CurrentTriangle[0].x, CurrentTriangle[0].y, CurrentTriangle[0].z);
		Point_3 PointB(CurrentTriangle[1].x, CurrentTriangle[1].y, CurrentTriangle[1].z);
		Point_3 PointC(CurrentTriangle[2].x, CurrentTriangle[2].y, CurrentTriangle[2].z);
		Triangle_3 Triangle(PointA, PointB, PointC);

		// Define an AABB
		Bbox_3 AABB(Grid[GridX][GridY].AABB.GetMin().x, Grid[GridX][GridY].AABB.GetMin().y, Grid[GridX][GridY].AABB.GetMin().z,
					Grid[GridX][GridY].AABB.GetMax().x, Grid[GridX][GridY].AABB.GetMax().y, Grid[GridX][GridY].AABB.GetMax().z);

		// Compute the intersection between the triangle and the AABB
		auto CGALIntersection = CGAL::intersection(Triangle, AABB);

		glm::dvec3 Edge_0 = CurrentTriangle[2] - CurrentTriangle[1];
		glm::dvec3 Edge_1 = CurrentTriangle[2] - CurrentTriangle[0];

		glm::dvec3 Normal = glm::normalize(glm::cross(Edge_1, Edge_0));
		if (isnan(Normal.x) || isnan(Normal.y) || isnan(Normal.z))
			return Result;

		// We are using the normal of the triangle to project the 3D triangle onto a 2D plane
		// not a projection vector to make sure that sum of pixel values will be same for all projections, for scientific measurements.
		Plane_3 PlaneToProject(PointA, Vector_3(Normal.x, Normal.y, Normal.z));

		// Check the type of the intersection result and calculate the area
		if (CGALIntersection)
		{
			if (const Triangle_3* IntersectionAsTriangle = boost::get<Triangle_3>(&*CGALIntersection))
			{
				// Project the 3D triangle onto a 2D plane
				Point_2 p1_2d = ProjectPointOntoPlane(IntersectionAsTriangle->vertex(0), PlaneToProject);
				Point_2 p2_2d = ProjectPointOntoPlane(IntersectionAsTriangle->vertex(1), PlaneToProject);
				Point_2 p3_2d = ProjectPointOntoPlane(IntersectionAsTriangle->vertex(2), PlaneToProject);

				// Using absolute value to ensure the area is positive
				// CGAL will return a negative area if the vertices are ordered clockwise
				Result = abs(CGAL::to_double(CGAL::area(p1_2d, p2_2d, p3_2d)));
			}
			else if (const Kernel::Segment_3* IntersectionAsSegment = boost::get<Kernel::Segment_3>(&*CGALIntersection))
			{
				// No need to calculate the area, as it will be zero.
			}
			else if (const Point_3* IntersectionAsPoint = boost::get<Point_3>(&*CGALIntersection))
			{
				// No need to calculate the area, as it will be zero.
			}
			else if (const std::vector<Point_3>* IntersectionAsPolygonPoints = boost::get<std::vector<Point_3>>(&*CGALIntersection))
			{
				std::vector<glm::dvec3> IntersectionPoints;
				Polygon_2 Polygon;
				for (const auto& Point : *IntersectionAsPolygonPoints)
				{
					Polygon.push_back(ProjectPointOntoPlane(Point, PlaneToProject));
					IntersectionPoints.push_back(glm::dvec3(Point.x(), Point.y(), Point.z()));
				}

				// Check if the polygon is simple
				// And try to 'fix' it by sorting the points by angle
				if (!Polygon.is_simple())
				{
					SortPointsByAngle(IntersectionPoints);

					Polygon = Polygon_2();
					for (const auto& Point : IntersectionPoints)
					{
						Point_3 CurrentPoint = Point_3(Point[0], Point[1], Point[2]);
						Polygon.push_back(ProjectPointOntoPlane(CurrentPoint, PlaneToProject));
					}
				}
	
				// Using absolute value to ensure the area is positive
				// CGAL will return a negative area if the vertices are ordered clockwise
				Result = abs(CGAL::to_double(Polygon.area()));
			}
		}
	}
	else
	{
		std::vector<glm::dvec3> IntersectionPoints = GEOMETRY.GetIntersectionPoints(Grid[GridX][GridY].AABB, CurrentTriangle);

		for (size_t i = 0; i < CurrentTriangle.size(); i++)
		{
			if (Grid[GridX][GridY].AABB.ContainsPoint(CurrentTriangle[i]))
			{
				bool bAlreadyExists = false;
				int PointsThatAreNotSame = 0;
				for (size_t j = 0; j < IntersectionPoints.size(); j++)
				{
					if (abs(IntersectionPoints[j] - CurrentTriangle[i]).x > glm::dvec3(DBL_EPSILON).x ||
						abs(IntersectionPoints[j] - CurrentTriangle[i]).y > glm::dvec3(DBL_EPSILON).y ||
						abs(IntersectionPoints[j] - CurrentTriangle[i]).z > glm::dvec3(DBL_EPSILON).z)
					{
						PointsThatAreNotSame++;
					}
				}

				if (PointsThatAreNotSame != IntersectionPoints.size())
					bAlreadyExists = true;

				if (!bAlreadyExists)
					IntersectionPoints.push_back(CurrentTriangle[i]);
			}
		}

		Result = GetArea(IntersectionPoints);
	}

	return Result;
}

void LayerRasterizationManager::ClearAllData()
{
	if (ResultPreview != nullptr)
	{
		delete ResultPreview;
		ResultPreview = nullptr;
	}

	Grid.clear();
	MainThreadGridUpdateTasks.clear();
	GatherGridRasterizationThreadCount = 0;
	CurrentLayer = nullptr;
	CurrentProjectionVector = glm::vec3(0.0f);
}

void LayerRasterizationManager::ClearDataAfterCalculation()
{
	MainThreadGridUpdateTasks.clear();
	GatherGridRasterizationThreadCount = 0;
}

int LayerRasterizationManager::GetTexturePreviewID()
{
	if (ResultPreview == nullptr)
		return -1;

	return ResultPreview->GetTextureID();
}

glm::vec3 LayerRasterizationManager::GetProjectionVector()
{
	return CurrentProjectionVector;
}

float LayerRasterizationManager::GetResolutionInMeters()
{
	return CurrentResolutionInMeters;
}
void LayerRasterizationManager::SetResolutionInMeters(float NewValue)
{
	glm::vec2 MinMaxResolutionInMeters = GetMinMaxResolutionInMeters();

	if (NewValue > MinMaxResolutionInMeters.x)
		NewValue = MinMaxResolutionInMeters.x;

	if (NewValue < MinMaxResolutionInMeters.y)
		NewValue = MinMaxResolutionInMeters.y;

	CurrentResolutionInMeters = NewValue;
	
	if (GLMVec3Equal(CurrentProjectionVector, glm::vec3(0.0f)))
		UpdateProjectionVector();

	glm::uvec2 ResolutionXY = GetResolutionInPixelsBasedOnResolutionInMeters(CurrentProjectionVector, CurrentResolutionInMeters);
	CurrentResolution = glm::max(ResolutionXY.x, ResolutionXY.y);
}

glm::vec2 LayerRasterizationManager::GetMinMaxResolutionInMeters(glm::vec3 ProjectionVector)
{
	glm::vec2 Result = glm::vec2(0.0f);

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	glm::vec3 Axis = CurrentProjectionVector;
	if (!GLMVec3Equal(ProjectionVector, glm::vec3(0.0f)))
		Axis = ProjectionVector;
	
	if (GLMVec3Equal(Axis, glm::vec3(0.0f)))
	{
		UpdateProjectionVector();
		Axis = CurrentProjectionVector;
	}

	if (GLMVec3Equal(Axis, glm::vec3(0.0f)))
		return Result;
		
	FEAABB MeshAABB = CurrentMeshAnalysisData->GetAABB();

	float UsableSize = 0.0f;
	if (Axis.x > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().y, MeshAABB.GetSize().z);
	}
	else if (Axis.y > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().z);
	}
	else if (Axis.z > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().y);
	}

	Result.x = UsableSize / (RASTERIZATION_MIN_RESOLUTION + 1);
	Result.y = UsableSize / (RASTERIZATION_MAX_RESOLUTION + 1);

	return Result;
}

void LayerRasterizationManager::ShowDebugWindow()
{
	if (ImGui::Begin("Layer Rasterization Debug"))
	{
		if (ImGui::CollapsingHeader("Info"))
		{
			ImGui::Text("Progress: %.2f%%", Progress * 100.0f);
			ImGui::Separator();
			ImGui::Text("Grid Resolution: %d", CurrentResolution);
			ImGui::Text("Grid Resolution (m): %.2f", CurrentResolutionInMeters);
			ImGui::Separator();
			ImGui::Text("Mode: %s", std::to_string(Mode).c_str());
			ImGui::Separator();
			ImGui::Text("Total Area Used: %.2f", Debug_TotalAreaUsed);
			ImGui::Separator();
			ImGui::Text("Result Raw Max: %.2f", Debug_ResultRawMax);
			ImGui::Text("Result Raw Min: %.2f", Debug_ResultRawMin);
			ImGui::Text("Result Raw Mean: %.2f", Debug_ResultRawMean);
			ImGui::Text("Result Raw Standard Deviation: %.2f", Debug_ResultRawStandardDeviation);
			ImGui::Text("Result Raw Skewness: %.2f", Debug_ResultRawSkewness);
			ImGui::Text("Result Raw Kurtosis: %.2f", Debug_ResultRawKurtosis);
		}

		ImGui::Checkbox("Show only cell with triangles", &bDebugShowOnlyCellsWithTriangles);
		ImGui::Checkbox("Show only selected cell", &bDebugShowOnlySelectedCells);

		if (ImGui::Button("Render grid"))
		{
			DebugRenderGrid();
		}

		std::string SelectedCellText = "Selected cell - X : " + std::to_string(int(DebugSelectedCell.x)) + " Y : " + std::to_string(int(DebugSelectedCell.y));
		ImGui::Text(SelectedCellText.c_str());

		static int TempCellX = 0;
		static int TempCellY = 0;
		ImGui::InputInt("SelectedCellX", &TempCellX);

		int TempInt = static_cast<int>(DebugSelectedCell.y);
		ImGui::InputInt("SelectedCellY", &TempCellY);

		if (ImGui::Button("Select cell"))
		{
			DebugSelectCell(TempCellX, TempCellY);
		}


		ImGui::End();
	}
}

void LayerRasterizationManager::DebugRenderGrid()
{
	//LINE_RENDERER.ClearAll();

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return;

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			LayerRasterizationManager::GridCell& Cell = Grid[i][j];
			glm::vec3 Color = glm::vec3(0.0f, 1.0f, 0.0f);

			if (bDebugShowOnlyCellsWithTriangles)
			{
				if (!Grid[i][j].TrianglesInCell.empty())
				{
					LINE_RENDERER.RenderAABB(Cell.AABB.Transform(ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix()), Color);
				}
			}
			else if (bDebugShowOnlySelectedCells)
			{
				if (DebugSelectedCell.x == i && DebugSelectedCell.y == j)
				{
					Color = glm::vec3(1.0f, 1.0f, 0.0f);
					LINE_RENDERER.RenderAABB(Cell.AABB.Transform(ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix()), Color);

					for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
					{
						const auto CurrentTriangle = CurrentMeshAnalysisData->Triangles[Grid[i][j].TrianglesInCell[k]];

						std::vector<glm::dvec3> TranformedTrianglePoints = CurrentTriangle;
						for (size_t l = 0; l < TranformedTrianglePoints.size(); l++)
						{
							TranformedTrianglePoints[l] = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[l], 1.0f);
						}

						LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 0.0f, 1.0f)));
						LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 0.0f, 1.0f)));
						LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 0.0f, 1.0f)));
					}
				}
			}
			else
			{
				LINE_RENDERER.RenderAABB(Cell.AABB.Transform(ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix()), Color);
			}
		}
	}

	LINE_RENDERER.SyncWithGPU();
}

void LayerRasterizationManager::DebugMouseClick()
{
	if (Grid.size() == 0)
		return;

	DebugSelectedCell = glm::vec2(-1.0);

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	float DistanceToCell = 999999.0f;
	float LastDistanceToCell = 999999.0f;
	for (size_t i = 0; i < Grid.size(); i++)
	{
		for (size_t j = 0; j < Grid[i].size(); j++)
		{
			FEAABB FinalAABB = Grid[i][j].AABB.Transform(CurrentMeshAnalysisData->Position->GetWorldMatrix());
			if (FinalAABB.RayIntersect(MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MAIN_SCENE_MANAGER.GetMouseRayDirection(), DistanceToCell))
			{
				if (LastDistanceToCell > DistanceToCell)
				{
					LastDistanceToCell = DistanceToCell;
					DebugSelectCell(static_cast<int>(i), static_cast<int>(j));
					return;
				}
			}
		}
	}
}

void LayerRasterizationManager::DebugSelectCell(int X, int Y)
{
	if (Grid.size() == 0)
		return;

	if (X < 0 || X >= Grid.size() || Y < 0 || Y >= Grid[0].size())
		return;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return;

	LINE_RENDERER.ClearAll();

	DebugSelectedCell = glm::vec2(X, Y);
	for (size_t i = 0; i < Grid[X][Y].TrianglesInCell.size(); i++)
	{
		auto CurrentTriangle = CurrentMeshAnalysisData->Triangles[Grid[X][Y].TrianglesInCell[i]];
		if (CurrentTriangle.size() != 3)
			continue;

		std::vector<glm::dvec3> IntersectionPoints = GEOMETRY.GetIntersectionPoints(Grid[X][Y].AABB, CurrentTriangle);

		for (size_t l = 0; l < CurrentTriangle.size(); l++)
		{
			if (Grid[X][Y].AABB.ContainsPoint(CurrentTriangle[l]))
			{
				bool bAlreadyExists = false;
				int PointsThatAreNotSame = 0;
				for (size_t q = 0; q < IntersectionPoints.size(); q++)
				{
					if (abs(IntersectionPoints[q] - CurrentTriangle[l]).x > glm::dvec3(DBL_EPSILON).x ||
						abs(IntersectionPoints[q] - CurrentTriangle[l]).y > glm::dvec3(DBL_EPSILON).y ||
						abs(IntersectionPoints[q] - CurrentTriangle[l]).z > glm::dvec3(DBL_EPSILON).z)
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

		for (size_t l = 0; l < IntersectionPoints.size(); l++)
		{
			glm::dvec3 TransformedPoint = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(IntersectionPoints[l], 1.0f);
			LINE_RENDERER.AddLineToBuffer(FECustomLine(TransformedPoint, TransformedPoint + glm::dvec3(0.0, 1.0, 0.0), glm::vec3(1.0f, 0.0f, 0.0f)));
		}

		double CurrentTrianlgeArea = 0.0;
		CurrentTrianlgeArea = LAYER_RASTERIZATION_MANAGER.GetArea(IntersectionPoints);
	}

	DebugRenderGrid();
}

void LayerRasterizationManager::UpdateProjectionVector()
{
	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return;

	LAYER_RASTERIZATION_MANAGER.CurrentProjectionVector = ConvertToClosestAxis(CurrentMeshAnalysisData->GetAverageNormal());
}

glm::uvec2 LayerRasterizationManager::GetResolutionInPixelsBasedOnResolutionInMeters(glm::vec3 ProjectionVector, float ResolutionInMeters)
{
	glm::uvec2 Result = glm::uvec2(0);

	if (ResolutionInMeters <= 0.0f)
		return Result;
	
	if (GLMVec3Equal(ProjectionVector, glm::vec3(0.0f)))
		return Result;

	if (ProjectionVector.x + ProjectionVector.y + ProjectionVector.z != 1.0f)
		return Result;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	FEAABB MeshAABB = CurrentMeshAnalysisData->GetAABB();
	unsigned int CountOfCellsToCoverAABB = 0;
	float UsableSize = 0.0f;

	if (ProjectionVector.x > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().y, MeshAABB.GetSize().z);
	}
	else if (ProjectionVector.y > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().z);
	}
	else if (ProjectionVector.z > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().y);
	}

	CountOfCellsToCoverAABB = static_cast<unsigned int>(UsableSize / ResolutionInMeters);
	CountOfCellsToCoverAABB += 1;
	Result = glm::uvec2(CountOfCellsToCoverAABB, CountOfCellsToCoverAABB);

	return Result;
}

bool LayerRasterizationManager::GLMVec3Equal(const glm::vec3& A, const glm::vec3& B)
{
	auto Epsilon = glm::epsilon<float>();
	auto Equal = glm::all(glm::epsilonEqual(A, B, Epsilon));
	auto Result = Equal;
	return glm::all(glm::epsilonEqual(A, B, glm::epsilon<float>()));
}

float LayerRasterizationManager::GetResolutionInMetersBasedOnResolutionInPixels(int Pixels)
{
	float Result = -1.0f;

	if (Pixels <= 0)
		return Result;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	if (GLMVec3Equal(CurrentProjectionVector, glm::vec3(0.0f)))
		UpdateProjectionVector();
	
	if (GLMVec3Equal(CurrentProjectionVector, glm::vec3(0.0f)))
		return Result;

	FEAABB MeshAABB = CurrentMeshAnalysisData->GetAABB();
	unsigned int CountOfCellToCoverAABB = 0;

	float UsableSize = 0.0f;
	if (CurrentProjectionVector.x > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().y, MeshAABB.GetSize().z);
	}
	else if (CurrentProjectionVector.y > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().z);
	}
	else if (CurrentProjectionVector.z > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().y);
	}

	Result = UsableSize / Pixels;

	return Result;
}

void LayerRasterizationManager::ActivateAutomaticOutliersSuppression()
{
	SetCumulativeModePercentOfAreaThatWouldBeRed(5.0f);
}

int LayerRasterizationManager::GetResolutionInPixelsThatWouldGiveSuchResolutionInMeters(float Meters)
{
	int Result = -1;

	if (Meters <= 0.0f)
		return Result;

	AnalysisObject* CurrentObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (CurrentObject == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = static_cast<MeshAnalysisData*>(CurrentObject->GetGeometryData());
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	if (GLMVec3Equal(CurrentProjectionVector, glm::vec3(0.0f)))
		UpdateProjectionVector();

	if (GLMVec3Equal(CurrentProjectionVector, glm::vec3(0.0f)))
		return Result;

	FEAABB MeshAABB = CurrentMeshAnalysisData->GetAABB();
	unsigned int CountOfCellToCoverAABB = 0;
	float UsableSize = 0.0f;

	if (CurrentProjectionVector.x > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().y, MeshAABB.GetSize().z);
	}
	else if (CurrentProjectionVector.y > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().z);
	}
	else if (CurrentProjectionVector.z > 0.0)
	{
		UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().y);
	}

	CountOfCellToCoverAABB = static_cast<unsigned int>(UsableSize / Meters);
	CountOfCellToCoverAABB += 1;
	Result = CountOfCellToCoverAABB;

	return Result;
}