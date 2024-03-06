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

	//double TotalTime = 0.0;
	//double TimeTakenFillCellsWithTriangleInfo = 0.0;

	//TIME.BeginTimeStamp("Total time");


	if (UpAxis.y > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l < LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			int XEnd = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			int XBegin = static_cast<int>(Distance / CellSize.x) - 1;
			if (XBegin < 0)
				XBegin = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			XEnd -= static_cast<int>(Distance / CellSize.x);
			XEnd++;
			if (XEnd >= Resolution)
				XEnd = Resolution;

			for (size_t i = XBegin; i < XEnd; i++)
			{
				int ZEnd = Resolution;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
				int ZBegin = static_cast<int>(Distance / CellSize.z) - 1;
				if (ZBegin < 0)
					ZBegin = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
				ZEnd -= static_cast<int>(Distance / CellSize.z);
				ZEnd++;
				if (ZEnd >= Resolution)
					ZEnd = Resolution;

				for (size_t j = ZBegin; j < ZEnd; j++)
				{
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Output->push_back(GridUpdateTask(i, j, l)); //Grid[i][j].TrianglesInCell.push_back(l);
				}
			}
		}

	}
	else if (UpAxis.z > 0.0)
	{
		for (int l = FirstIndexInTriangleArray; l < LastIndexInTriangleArray; l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

			int XEnd = Resolution;

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			int XBegin = static_cast<int>(Distance / CellSize.x) - 1;
			if (XBegin < 0)
				XBegin = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			XEnd -= static_cast<int>(Distance / CellSize.x);
			XEnd++;
			if (XEnd >= Resolution)
				XEnd = Resolution;

			for (size_t i = XBegin; i < XEnd; i++)
			{
				int YEnd = static_cast<int>(Resolution);

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
				int YBegin = static_cast<int>(Distance / CellSize.y) - 1;
				if (YBegin < 0)
					YBegin = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
				YEnd -= static_cast<int>(Distance / CellSize.y);
				YEnd++;
				if (YEnd >= Resolution)
					YEnd = Resolution;

				for (size_t j = YBegin; j < YEnd; j++)
				{
					if (GEOMETRY.IsAABBIntersectTriangle(Grid[i][j].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						Output->push_back(GridUpdateTask(i, j, l)); //Grid[i][j].TrianglesInCell.push_back(l);
				}
			}
		}
	}
}




void LayerRasterizationManager::AfterAllGridRasterizationThreadFinished()
{
	//MessageBoxA(NULL, "GatherGridRasterizationThreadWork!", "Mouse Position", MB_OK);

	for (int i = 0; i < MainThreadGridUpdateTasks.size(); i++)
	{
		Grid[MainThreadGridUpdateTasks[i].FirstIndex][MainThreadGridUpdateTasks[i].SecondIndex].TrianglesInCell.push_back(MainThreadGridUpdateTasks[i].TriangleIndexToAdd);
	}

	for (int i = 0; i < Grid.size(); i++)
	{
		for (int j = 0; j < Grid[i].size(); j++)
		{
			if (!Grid[i][j].TrianglesInCell.empty())
			{
				float CurrentCellTotalArea = 0.0f;
				for (size_t k = 0; k < Grid[i][j].TrianglesInCell.size(); k++)
				{
					CurrentCellTotalArea += static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]]);
				}

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
						float CurrentTriangleCoef = static_cast<float>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->TrianglesArea[Grid[i][j].TrianglesInCell[k]] / CurrentCellTotalArea);
						//float CurrentTriangleCoef = 1.0f;
						//float CurrentTriangleCoef = Grid[i][j].TriangleAABBOverlaps[k];
						FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]] * CurrentTriangleCoef;
					}

					//FinalResult /= static_cast<float>(Grid[i][j].TrianglesInCell.size());

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

						FinalResult += CurrentLayer->TrianglesToData[Grid[i][j].TrianglesInCell[k]] /** CurrentTriangleCoef*/;
					}
				}

				Grid[i][j].Value = FinalResult;
				if (isnan(Grid[i][j].Value))
					Grid[i][j].Value = 0.0f;
			}
		}
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

		JITTER_MANAGER.AdjustOutliers(FlattenGrid, 0.0f, (100 - CumulativeOutliers) / 100.0f);

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
	}

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