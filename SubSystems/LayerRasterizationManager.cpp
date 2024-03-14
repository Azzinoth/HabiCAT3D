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

std::vector<std::vector<LayerRasterizationManager::GridCell>> LayerRasterizationManager::GenerateGridProjection(const glm::vec3& Axis, int Resolution)
{
	std::vector<std::vector<GridCell>> Grid;

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return Grid;

	if (Axis.x + Axis.y + Axis.z != 1.0f)
		return Grid;

	glm::vec2 MinMaxResolutionInMeters = GetMinMaxResolutionInMeters(Axis);
	if (CurrentResolutionInMeters > MinMaxResolutionInMeters.x)
		CurrentResolutionInMeters = MinMaxResolutionInMeters.x;

	if (CurrentResolutionInMeters < MinMaxResolutionInMeters.y)
		CurrentResolutionInMeters = MinMaxResolutionInMeters.y;

	FEAABB MeshAABB = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB;
	float CellSize = JITTER_MANAGER.GetLowestPossibleResolution() / 4.0f/*CurrentResolutionInMeters*/;

	glm::vec3 CellDimension = glm::vec3(CellSize);
	unsigned int CountOfCellToCoverAABB = 0;

	if (Axis.x > 0.0)
	{
		float UsableSize = glm::max(MeshAABB.GetSize().y, MeshAABB.GetSize().z);
		CountOfCellToCoverAABB = UsableSize / CellSize;

		CellDimension.x = MeshAABB.GetSize().x;
	}
	else if (Axis.y > 0.0)
	{
		float UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().z);
		CountOfCellToCoverAABB = UsableSize / CellSize;

		CellDimension.y = MeshAABB.GetSize().y;
	}
	else if (Axis.z > 0.0)
	{
		float UsableSize = glm::max(MeshAABB.GetSize().x, MeshAABB.GetSize().y);
		CountOfCellToCoverAABB = UsableSize / CellSize;

		CellDimension.z = MeshAABB.GetSize().z;
	}

	CountOfCellToCoverAABB += 1;
	glm::uvec2 ResolutionXY = glm::uvec2(CountOfCellToCoverAABB, CountOfCellToCoverAABB);

	LAYER_RASTERIZATION_MANAGER.CurrentResolution = glm::max(ResolutionXY.x, ResolutionXY.y);
	Resolution = LAYER_RASTERIZATION_MANAGER.CurrentResolution;

	Grid.resize(Resolution);
	for (int i = 0; i < Resolution; i++)
	{
		Grid[i].resize(Resolution);
	}

	// Loop through each division to create the grid
	for (int i = 0; i < Resolution; i++)
	{
		for (int j = 0; j < Resolution; j++)
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
						if (LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Mean ||
							LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Cumulative)
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
						if (LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Mean ||
							LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Cumulative)
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
						if (LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Mean ||
							LAYER_RASTERIZATION_MANAGER.Mode == GridRasterizationMode::Cumulative)
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
	double AngleA = atan2(A.z - Centroid.z, A.x - Centroid.x);
	double AngleB = atan2(B.z - Centroid.z, B.x - Centroid.x);

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
						float CurrentValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
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
						float CurrentValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
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
							FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
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
							double CurrentTriangleValue = CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]];
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
		MinForColorMap = UnitArea;
		MaxForColorMap = 4.0 * UnitArea;
	}

	std::vector<unsigned char> ImageRawData;
	std::vector<unsigned char> FinalImageRawData;
	ImageRawData.reserve(CurrentResolution * CurrentResolution * 4);
	FinalImageRawData.reserve(CurrentResolution * CurrentResolution * 4);

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
			}
		}
	}

	if (CurrentProjectionVector.x > 0.0)
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

		FinalImageRawData = FlippedImageRawData;
	}
	if (CurrentProjectionVector.y > 0.0)
	{
		FinalImageRawData = ImageRawData;
	}
	if (CurrentProjectionVector.z > 0.0)
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

		FinalImageRawData = FlippedImageRawData;
	}

	ResultPreview = RESOURCE_MANAGER.RawDataToFETexture(FinalImageRawData.data(), CurrentResolution, CurrentResolution);
}

bool LayerRasterizationManager::SaveToFile(std::string FilePath, SaveMode SaveMode)
{
	if (Grid.size() == 0)
		return false;

	if (ResultPreview == nullptr)
		return false;

	if (SaveMode != SaveAsPNG && SaveMode != SaveAsTIF && SaveMode != SaveAs32bitTIF)
		return false;

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
		Dataset->SetProjection("WGS84");

		// Allocate data buffer for a single band
		float* RawData = (float*)CPLMalloc(sizeof(float) * ImageWidth * ImageHeight);

		for (size_t i = 0; i < ImageWidth; i++)
		{
			for (size_t j = 0; j < ImageHeight; j++)
			{
				RawData[i * ImageWidth + j] = Grid[j][i].Value;
			}
		}

		// Write data to the first band
		GDALRasterBand* Band = Dataset->GetRasterBand(1);
		CPLErr Error = Band->RasterIO(GF_Write, 0, 0, ImageWidth, ImageHeight, RawData, ImageWidth, ImageHeight, Type, 0, 0);
		if (Error != CE_None)
		{
			CPLFree(RawData);
			GDALClose(Dataset);
			return false;
		}

		// Cleanup
		CPLFree(RawData);
		GDALClose(Dataset);

		return true;
	}

	unsigned char* RawImageData = ResultPreview->GetRawData();

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
			delete[] RawImageData;
			return false;
		}

		// Create a new GeoTIFF file
		GDALDataset* Dataset = Driver->Create(FilePath.c_str(), ImageWidth, ImageHeight, BandsCount, Type, nullptr);
		if (Dataset == nullptr)
		{
			delete[] RawImageData;
			return false;
		}

		// Set geotransform and projection if necessary
		double GeoTransform[6] = { 0, 1, 0, 0, 0, -1 }; // Generic transformation
		Dataset->SetGeoTransform(GeoTransform);
		Dataset->SetProjection("WGS84");

		// Write data to the RGBA bands
		for (int CurrentBand = 1; CurrentBand <= 4; CurrentBand++)
		{
			GDALRasterBand* Band = Dataset->GetRasterBand(CurrentBand);
			CPLErr Error = Band->RasterIO(GF_Write, 0, 0, ImageWidth, ImageHeight, RawImageData + (CurrentBand - 1), ImageWidth, ImageHeight, Type, 4, 4 * ImageWidth);
			if (Error != CE_None)
			{
				GDALClose(Dataset);
				delete[] RawImageData;
				return false;
			}
		}

		// Cleanup
		GDALClose(Dataset);
	}

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

void LayerRasterizationManager::PrepareCurrentLayerForExport(MeshLayer* LayerToExport, glm::vec3 ForceProjectionVector)
{
	CurrentLayer = LayerToExport;
	if (CurrentLayer == nullptr)
		return;

	OnCalculationsStart();

	if (ForceProjectionVector != glm::vec3(0.0f))
		CurrentProjectionVector = ForceProjectionVector;
	else
		CurrentProjectionVector = ConvertToClosestAxis(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->GetAverageNormal());

	Grid = GenerateGridProjection(CurrentProjectionVector, CurrentResolution);

	int NumberOfTrianglesPerThread = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() / LAYER_RASTERIZATION_MANAGER.THREAD_COUNT);

	std::vector<GridRasterizationThreadData*> ThreadData;
	for (int i = 0; i < LAYER_RASTERIZATION_MANAGER.THREAD_COUNT; i++)
	{
		GridRasterizationThreadData* NewThreadData = new GridRasterizationThreadData();
		NewThreadData->Grid = &Grid;
		NewThreadData->UpAxis = CurrentProjectionVector;
		NewThreadData->Resolution = CurrentResolution;
		NewThreadData->FirstIndexInTriangleArray = i * NumberOfTrianglesPerThread;

		if (i == LAYER_RASTERIZATION_MANAGER.THREAD_COUNT - 1)
			NewThreadData->LastIndexInTriangleArray = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() - 1);
		else
			NewThreadData->LastIndexInTriangleArray = (i + 1) * NumberOfTrianglesPerThread;

		std::vector<GridUpdateTask>* OutputTasks = new std::vector<GridUpdateTask>();
		ThreadData.push_back(NewThreadData);

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

//void LayerRasterizationManager::SetGridResolution(int NewValue)
//{
//	if (NewValue < 2 || NewValue > 4096)
//		return;
//
//	CurrentResolution = NewValue;
//}

float LayerRasterizationManager::GetCumulativeModeLowerOutlierPercentile()
{
	return CumulativeModeLowerOutlierPercentile;
}

void LayerRasterizationManager::SetCumulativeModeLowerOutlierPercentile(float NewValue)
{
	if (NewValue < 0.0f || NewValue > 99.99f)
		return;

	CumulativeModeLowerOutlierPercentile = NewValue;
}

float LayerRasterizationManager::GetCumulativeModeUpperOutlierPercentile()
{
	return CumulativeModeUpperOutlierPercentile;
}

void LayerRasterizationManager::SetCumulativeModeUpperOutlierPercentile(float NewValue)
{
	if (NewValue < 0.001f || NewValue > 100.0f)
		return;

	CumulativeModeUpperOutlierPercentile = NewValue;
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
	Vector_3 OrthogonalVector = Plane.orthogonal_vector();
	Vector_3 ToPoint = Point - Plane.point();
	double SignedDistance = ToPoint * OrthogonalVector / std::sqrt(OrthogonalVector.squared_length());
	Point_3 ProjectedPoint = Point - SignedDistance * OrthogonalVector / std::sqrt(OrthogonalVector.squared_length());
	return Point_2(ProjectedPoint.x(), ProjectedPoint.y());
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

		/*const auto CGALNormal = Triangle.supporting_plane().perpendicular_line(PointA);

		Vector_3 Normal = Vector_3(CGALNormal.direction().vector().x(),
								   CGALNormal.direction().vector().y(),
								   CGALNormal.direction().vector().z());*/

		Vector_3 Normal = Vector_3(CurrentProjectionVector.x , CurrentProjectionVector.y, CurrentProjectionVector.z);

		// Check the type of the intersection result and calculate the area
		if (CGALIntersection)
		{
			if (const Triangle_3* intersect_triangle = boost::get<Triangle_3>(&*CGALIntersection))
			{
				// Project the 3D triangle onto a 2D plane
				Point_2 p1_2d = ProjectPointOntoPlane(intersect_triangle->vertex(0), Triangle.supporting_plane());
				Point_2 p2_2d = ProjectPointOntoPlane(intersect_triangle->vertex(1), Triangle.supporting_plane());
				Point_2 p3_2d = ProjectPointOntoPlane(intersect_triangle->vertex(2), Triangle.supporting_plane());

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
				Polygon_2 polygon;
				for (const auto& point : *intersect_points) {
					polygon.push_back(ProjectPointOntoPlane(point, Triangle.supporting_plane()));
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

		for (size_t i = 0; i < CurrentTriangleDouble.size(); i++)
		{
			if (Grid[GridX][GridY].AABB.ContainsPoint(CurrentTriangleDouble[i]))
			{
				bool bAlreadyExists = false;
				int PointsThatAreNotSame = 0;
				for (size_t j = 0; j < IntersectionPoints.size(); j++)
				{
					if (abs(IntersectionPoints[j] - CurrentTriangleDouble[i]).x > glm::dvec3(DBL_EPSILON).x ||
						abs(IntersectionPoints[j] - CurrentTriangleDouble[i]).y > glm::dvec3(DBL_EPSILON).y ||
						abs(IntersectionPoints[j] - CurrentTriangleDouble[i]).z > glm::dvec3(DBL_EPSILON).z)
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
}

glm::vec2 LayerRasterizationManager::GetMinMaxResolutionInMeters(glm::vec3 ProjectionVector)
{
	glm::vec2 Result = glm::vec2(0.0f);

	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return Result;

	glm::vec3 Axis = CurrentProjectionVector;
	if (ProjectionVector != glm::vec3(0.0f))
		Axis = ProjectionVector;
		
	FEAABB MeshAABB = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->MeshData.AABB;

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