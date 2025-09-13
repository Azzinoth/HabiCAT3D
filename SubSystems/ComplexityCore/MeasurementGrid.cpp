#include "MeasurementGrid.h"
using namespace FocalEngine;

MeasurementGrid::MeasurementGrid() {}

MeasurementGrid::~MeasurementGrid()
{
	Data.clear();
}

void MeasurementGrid::InitializeSegment(size_t BeginIndex, size_t EndIndex, size_t Dimensions, FEAABB GridAABB, float CellSize)
{
	const glm::vec3 Start = GridAABB.GetMin();
	for (size_t i = BeginIndex; i < EndIndex; i++)
	{
		Data[i].resize(Dimensions);
		for (size_t j = 0; j < Dimensions; j++)
		{
			Data[i][j].resize(Dimensions);
			for (size_t k = 0; k < Dimensions; k++)
			{
				glm::vec3 CurrentAABBMin = Start + glm::vec3(CellSize * i, CellSize * j, CellSize * k);
				Data[i][j][k].AABB = FEAABB(CurrentAABBMin, CurrentAABBMin + glm::vec3(CellSize));
			}
		}
	}
}

void MeasurementGrid::Init(int Dimensions, FEAABB AABB, const float ResolutionInM)
{
	TIME.BeginTimeStamp("Measurement grid Generation");

	const glm::vec3 Center = AABB.GetCenter();
	int AdditionalDimensions = 0;
	Dimensions = 1;

	if (ResolutionInM > 0)
	{
		AdditionalDimensions = 2;
		const int MinDimensions = static_cast<int>(AABB.GetLongestAxisLength() / ResolutionInM);
		Dimensions = MinDimensions + AdditionalDimensions;

		if (Dimensions < 1 || Dimensions > 4096)
			return;
	}

	FEAABB GridAABB;
	if (ResolutionInM > 0.0f)
	{
		GridAABB = FEAABB(Center - glm::vec3(ResolutionInM * Dimensions / 2.0f), Center + glm::vec3(ResolutionInM * Dimensions / 2.0f));
	}
	else
	{
		GridAABB = FEAABB(Center - glm::vec3(AABB.GetLongestAxisLength() / 2.0f), Center + glm::vec3(AABB.GetLongestAxisLength() / 2.0f));
	}

	float CellSize;
	if (ResolutionInM > 0)
	{
		CellSize = ResolutionInM;
	}
	else
	{
		CellSize = GridAABB.GetLongestAxisLength();
	}

	Data.resize(Dimensions);
	if (bUsingMultiThreading)
	{
		size_t ThreadCount = std::thread::hardware_concurrency();
		// Using dedicated threads instead of the thread pool to make less changes to the existing code
		std::vector<std::thread> Threads(ThreadCount);

		size_t ChunkSize = Dimensions / ThreadCount;  // Divide the work into chunks per thread
		for (size_t i = 0; i < ThreadCount; ++i)
		{
			size_t Start = i * ChunkSize;
			size_t End = (i + 1) * ChunkSize;
			if (i == ThreadCount - 1)
				End = Dimensions;  // Make sure the last thread covers all remaining elements

			Threads[i] = std::thread([=]() {
				InitializeSegment(Start, End, Dimensions, GridAABB, CellSize);
			});
		}

		// Wait for all threads to finish
		for (auto& CurrentThread : Threads)
			CurrentThread.join();
	}
	else
	{
		for (size_t i = 0; i < Dimensions; i++)
		{
			Data[i].resize(Dimensions);
			for (size_t j = 0; j < Dimensions; j++)
				Data[i][j].resize(Dimensions);
		}

		const glm::vec3 Start = GridAABB.GetMin();
		for (size_t i = 0; i < Dimensions; i++)
		{
			for (size_t j = 0; j < Dimensions; j++)
			{
				for (size_t k = 0; k < Dimensions; k++)
				{
					glm::vec3 CurrentAABBMin = Start + glm::vec3(CellSize * i, CellSize * j, CellSize * k);
					Data[i][j][k].AABB = FEAABB(CurrentAABBMin, CurrentAABBMin + glm::vec3(CellSize));
				}
			}
		}
	}
}

void MeasurementGrid::GridFillingThread(void* InputData, void* OutputData)
{
	GridThreadData* Input = reinterpret_cast<GridThreadData*>(InputData);
	std::vector<GridUpdateTask>* Output = reinterpret_cast<std::vector<GridUpdateTask>*>(OutputData);

	const float CellSize = Data[0][0][0].AABB.GetLongestAxisLength();
	const glm::vec3 GridMin = Data[0][0][0].AABB.GetMin();
	const glm::vec3 GridMax = Data[Data.size() - 1][Data.size() - 1][Data.size() - 1].AABB.GetMax();

	for (int l = Input->FirstIndexInTriangleArray; l <= Input->LastIndexInTriangleArray; l++)
	{
		FEAABB TriangleAABB = FEAABB(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles[l]);

		int XEnd = static_cast<int>(Data.size());

		float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
		int XBegin = static_cast<int>(Distance / CellSize) - 1;
		if (XBegin < 0)
			XBegin = 0;

		Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
		XEnd -= static_cast<int>(Distance / CellSize);
		XEnd++;
		if (XEnd > Data.size())
			XEnd = static_cast<int>(Data.size());

		for (size_t i = XBegin; i < XEnd; i++)
		{
			int YEnd = static_cast<int>(Data.size());

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
			int YBegin = static_cast<int>(Distance / CellSize) - 1;
			if (YBegin < 0)
				YBegin = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
			YEnd -= static_cast<int>(Distance / CellSize);
			YEnd++;
			if (YEnd > Data.size())
				YEnd = static_cast<int>(Data.size());

			for (size_t j = YBegin; j < YEnd; j++)
			{
				int ZEnd = static_cast<int>(Data.size());

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
				int ZBegin = static_cast<int>(Distance / CellSize) - 1;
				if (ZBegin < 0)
					ZBegin = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
				ZEnd -= static_cast<int>(Distance / CellSize);
				ZEnd++;
				if (ZEnd > Data.size())
					ZEnd = static_cast<int>(Data.size());

				for (size_t k = ZBegin; k < ZEnd; k++)
				{
					if (Data[i][j][k].AABB.AABBIntersect(TriangleAABB))
					{
						if (GEOMETRY.IsAABBIntersectTriangle(Data[i][j][k].AABB, ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles[l]))
						{
							Output->push_back(GridUpdateTask(static_cast<int>(i), static_cast<int>(j), static_cast<int>(k), l));
						}
					}
				}
			}
		}
	}
}

void MeasurementGrid::FillCellsWithTriangleInfo()
{
	TIME.BeginTimeStamp("Fill cells with triangle info");

	if (bUsingMultiThreading)
	{
		int LocalThreadCount = THREAD_POOL.GetThreadCount();
		int NumberOfTrianglesPerThread = static_cast<int>(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size() / LocalThreadCount);
		
		if (LocalThreadCount > NumberOfTrianglesPerThread)
		{
			LocalThreadCount = 1;
			NumberOfTrianglesPerThread = static_cast<int>(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size());
		}

		std::vector<std::string> ThreadIDs;
		std::vector<GridThreadData*> ThreadData;
		std::vector<std::vector<GridUpdateTask>*> AllOutputTasks;

		for (int i = 0; i < LocalThreadCount; i++)
		{
			GridThreadData* NewThreadData = new GridThreadData();
			ThreadData.push_back(NewThreadData);
			NewThreadData->FirstIndexInTriangleArray = i * NumberOfTrianglesPerThread + 1;

			if (i == 0)
				NewThreadData->FirstIndexInTriangleArray = 0;

			if (i == LocalThreadCount - 1)
				NewThreadData->LastIndexInTriangleArray = static_cast<int>(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size() - 1);
			else
				NewThreadData->LastIndexInTriangleArray = (i + 1) * NumberOfTrianglesPerThread;

			std::vector<GridUpdateTask>* OutputTasks = new std::vector<GridUpdateTask>();
			AllOutputTasks.push_back(OutputTasks);

			ThreadIDs.push_back(THREAD_POOL.CreateLightThread());
			THREAD_POOL.ExecuteLightThread(ThreadIDs.back(), [=]() {
				GridFillingThread(NewThreadData, OutputTasks);
			});
		}

		for (size_t i = 0; i < ThreadIDs.size(); i++)
		{
			THREAD_POOL.WaitForLightThread(ThreadIDs[i]);
		}

		for (size_t i = 0; i < ThreadIDs.size(); i++)
		{
			THREAD_POOL.RemoveLightThread(ThreadIDs[i]);
		}

		for (size_t i = 0; i < ThreadData.size(); i++)
		{
			delete ThreadData[i];
		}

		for (int i = 0; i < AllOutputTasks.size(); i++)
		{
			for (int j = 0; j < AllOutputTasks[i]->size(); j++)
			{
				const int FirstIndex = AllOutputTasks[i]->at(j).FirstIndex;
				const int SecondIndex = AllOutputTasks[i]->at(j).SecondIndex;
				const int ThirdIndex = AllOutputTasks[i]->at(j).ThirdIndex;
				const int TriangleIndexToAdd = AllOutputTasks[i]->at(j).TriangleIndexToAdd;

				Data[FirstIndex][SecondIndex][ThirdIndex].TrianglesInCell.push_back(TriangleIndexToAdd);
			}

			delete AllOutputTasks[i];
		}

		AllOutputTasks.clear();
	}
	else
	{
		const float CellSize = Data[0][0][0].AABB.GetLongestAxisLength();
		const glm::vec3 GridMin = Data[0][0][0].AABB.GetMin();
		const glm::vec3 GridMax = Data[Data.size() - 1][Data.size() - 1][Data.size() - 1].AABB.GetMax();

		DebugTotalTrianglesInCells = 0;

		for (int l = 0; l < ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size(); l++)
		{
			FEAABB TriangleAABB = FEAABB(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles[l]);

			int XEnd = static_cast<int>(Data.size());

			float Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().x - GridMin.x, 2.0)));
			int XBegin = static_cast<int>(Distance / CellSize) - 1;
			if (XBegin < 0)
				XBegin = 0;

			Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().x - GridMax.x, 2.0)));
			XEnd -= static_cast<int>(Distance / CellSize);
			XEnd++;
			if (XEnd > Data.size())
				XEnd = static_cast<int>(Data.size());

			for (size_t i = XBegin; i < XEnd; i++)
			{
				int YEnd = static_cast<int>(Data.size());

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().y - GridMin.y, 2.0)));
				int YBegin = static_cast<int>(Distance / CellSize) - 1;
				if (YBegin < 0)
					YBegin = 0;

				Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().y - GridMax.y, 2.0)));
				YEnd -= static_cast<int>(Distance / CellSize);
				YEnd++;
				if (YEnd > Data.size())
					YEnd = static_cast<int>(Data.size());

				for (size_t j = YBegin; j < YEnd; j++)
				{
					int ZEnd = static_cast<int>(Data.size());

					Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMin().z - GridMin.z, 2.0)));
					int ZBegin = static_cast<int>(Distance / CellSize) - 1;
					if (ZBegin < 0)
						ZBegin = 0;

					Distance = static_cast<float>(sqrt(pow(TriangleAABB.GetMax().z - GridMax.z, 2.0)));
					ZEnd -= static_cast<int>(Distance / CellSize);
					ZEnd++;
					if (ZEnd > Data.size())
						ZEnd = static_cast<int>(Data.size());

					for (size_t k = ZBegin; k < ZEnd; k++)
					{
						if (Data[i][j][k].AABB.AABBIntersect(TriangleAABB))
						{
							if (GEOMETRY.IsAABBIntersectTriangle(Data[i][j][k].AABB, ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles[l]))
							{
								Data[i][j][k].TrianglesInCell.push_back(l);
								DebugTotalTrianglesInCells++;
							}
						}
					}
				}
			}
		}
	}

	TimeTakenFillCellsWithTriangleInfo = static_cast<float>(TIME.EndTimeStamp("Fill cells with triangle info"));
}

void MeasurementGrid::FillCellsWithPointInfo()
{
	bTriangleMode = false;
	TIME.BeginTimeStamp("Fill cells with points info");

	/*if (bUsingMultiThreading)
	{*/
		/*int LocalThreadCount = THREAD_POOL.GetThreadCount();
		int NumberOfTrianglesPerThread = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() / LocalThreadCount);

		if (LocalThreadCount > NumberOfTrianglesPerThread)
		{
			LocalThreadCount = 1;
			NumberOfTrianglesPerThread = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size());
		}

		std::vector<std::string> ThreadIDs;
		std::vector<GridThreadData*> ThreadData;
		std::vector<std::vector<GridUpdateTask>*> AllOutputTasks;

		for (int i = 0; i < LocalThreadCount; i++)
		{
			GridThreadData* NewThreadData = new GridThreadData();
			ThreadData.push_back(NewThreadData);
			NewThreadData->FirstIndexInTriangleArray = i * NumberOfTrianglesPerThread + 1;

			if (i == 0)
				NewThreadData->FirstIndexInTriangleArray = 0;

			if (i == LocalThreadCount - 1)
				NewThreadData->LastIndexInTriangleArray = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() - 1);
			else
				NewThreadData->LastIndexInTriangleArray = (i + 1) * NumberOfTrianglesPerThread;

			std::vector<GridUpdateTask>* OutputTasks = new std::vector<GridUpdateTask>();
			AllOutputTasks.push_back(OutputTasks);

			ThreadIDs.push_back(THREAD_POOL.CreateLightThread());
			THREAD_POOL.ExecuteLightThread(ThreadIDs.back(), [=]() {
				GridFillingThread(NewThreadData, OutputTasks);
				});
		}

		for (size_t i = 0; i < ThreadIDs.size(); i++)
		{
			THREAD_POOL.WaitForLightThread(ThreadIDs[i]);
		}

		for (size_t i = 0; i < ThreadIDs.size(); i++)
		{
			THREAD_POOL.RemoveLightThread(ThreadIDs[i]);
		}

		for (size_t i = 0; i < ThreadData.size(); i++)
		{
			delete ThreadData[i];
		}

		for (int i = 0; i < AllOutputTasks.size(); i++)
		{
			for (int j = 0; j < AllOutputTasks[i]->size(); j++)
			{
				const int FirstIndex = AllOutputTasks[i]->at(j).FirstIndex;
				const int SecondIndex = AllOutputTasks[i]->at(j).SecondIndex;
				const int ThirdIndex = AllOutputTasks[i]->at(j).ThirdIndex;
				const int TriangleIndexToAdd = AllOutputTasks[i]->at(j).TriangleIndexToAdd;

				Data[FirstIndex][SecondIndex][ThirdIndex].TrianglesInCell.push_back(TriangleIndexToAdd);
			}

			delete AllOutputTasks[i];
		}

		AllOutputTasks.clear();*/
	/*}
	else
	{*/
		const float CellSize = Data[0][0][0].AABB.GetLongestAxisLength();
		const glm::vec3 GridMin = Data[0][0][0].AABB.GetMin();
		const glm::vec3 GridMax = Data[Data.size() - 1][Data.size() - 1][Data.size() - 1].AABB.GetMax();

		//DebugTotalTrianglesInCells = 0;

		/*for (size_t i = 0; i < COMPLEXITY_METRIC_MANAGER.RawPointCloudData.size(); i++)
		{
			if (Data[i][j][k].AABB.AABBIntersect(TriangleAABB))
			{
				if (GEOMETRY.IsAABBIntersectTriangle(Data[i][j][k].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
				{
					Data[i][j][k].TrianglesInCell.push_back(l);
					DebugTotalTrianglesInCells++;
				}
			}
		}*/

		DebugTotalPointsInCells = 0;

		PointCloudGeometryData* CurrentPointCloudData = ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData;
		for (int l = 0; l < CurrentPointCloudData->RawPointCloudData.size(); l++)
		{
			glm::vec3 CurrentPoint = glm::vec3(CurrentPointCloudData->RawPointCloudData[l].X, CurrentPointCloudData->RawPointCloudData[l].Y, CurrentPointCloudData->RawPointCloudData[l].Z);

		//	for (size_t i = 0; i < Data.size(); i++)
		//	{
		//		for (size_t j = 0; j < Data[i].size(); j++)
		//		{
		//			for (size_t k = 0; k < Data[i][j].size(); k++)
		//			{
		//				if (Data[i][j][k].AABB.ContainsPoint(CurrentPoint))
		//				{
		//					Data[i][j][k].PointsInCell.push_back(l);
		//					DebugTotalPointsInCells++;
		//				}
		//			}
		//		}
		//	}
		//}



			int XEnd = static_cast<int>(Data.size());

			float Distance = CurrentPoint.x - GridMin.x;
			int XBegin = static_cast<int>(Distance / CellSize) - 2;
			if (XBegin < 0)
				XBegin = 0;

			XEnd = XBegin + 3;
			if (XEnd > Data.size())
				XEnd = static_cast<int>(Data.size());

			for (size_t i = XBegin; i < XEnd; i++)
			{
				int YEnd = static_cast<int>(Data.size());

				Distance = CurrentPoint.y - GridMin.y;
				int YBegin = static_cast<int>(Distance / CellSize) - 2;
				if (YBegin < 0)
					YBegin = 0;

				YEnd = YBegin + 3;
				if (YEnd > Data.size())
					YEnd = static_cast<int>(Data.size());

				for (size_t j = YBegin; j < YEnd; j++)
				{
					int ZEnd = static_cast<int>(Data.size());

					Distance = CurrentPoint.z - GridMin.z;
					int ZBegin = static_cast<int>(Distance / CellSize) - 2;
					if (ZBegin < 0)
						ZBegin = 0;

					ZEnd = ZBegin + 3;
					if (ZEnd > Data.size())
						ZEnd = static_cast<int>(Data.size());

					for (size_t k = ZBegin; k < ZEnd; k++)
					{
						if (Data[i][j][k].AABB.ContainsPoint(CurrentPoint))
						{
							Data[i][j][k].PointsInCell.push_back(l);
							DebugTotalPointsInCells++;
						}
					}

					/*for (size_t i = 0; i < Data.size(); i++)
					{
						for (size_t j = 0; j < Data[i].size(); j++)
						{
							for (size_t k = 0; k < Data[i][j].size(); k++)
							{
								if (Data[i][j][k].AABB.ContainsPoint(CurrentPoint))
								{
									Data[i][j][k].PointsInCell.push_back(l);
									DebugTotalPointsInCells++;
								}
							}
						}
					}*/
				}
			}
		}
	//}

	TimeTakenFillCellsWithTriangleInfo = static_cast<float>(TIME.EndTimeStamp("Fill cells with points info"));
}

void MeasurementGrid::MouseClick(const double MouseX, const double MouseY, const glm::mat4 TransformMat)
{
	SelectedCell = glm::vec3(-1.0);

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				Data[i][j][k].bSelected = false;
			}
		}
	}

	float DistanceToCell = 999999.0f;
	float LastDistanceToCell = 999999.0f;
	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				Data[i][j][k].bSelected = false;
				if (!Data[i][j][k].bWasRenderedLastFrame)
					continue;

				FEAABB FinalAABB = Data[i][j][k].AABB.Transform(TransformMat).Transform(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Position->GetWorldMatrix());
				if (FinalAABB.RayIntersect(MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MAIN_SCENE_MANAGER.GetMouseRayDirection(), DistanceToCell))
				{
					if (LastDistanceToCell > DistanceToCell)
					{
						LastDistanceToCell = DistanceToCell;
						SelectedCell = glm::vec3(i, j, k);
					}
				}
			}
		}
	}

	if (DistanceToCell != 999999.0f)
		Data[static_cast<int>(SelectedCell.x)][static_cast<int>(SelectedCell.y)][static_cast<int>(SelectedCell.z)].bSelected = true;
}

void MeasurementGrid::FillPerTriangleMeasurementData()
{
	if (!ANALYSIS_OBJECT_MANAGER.HaveMeshData())
		return;

	TIME.BeginTimeStamp("FillMeasurementData");

	std::vector<int> PointDataCount;
	PerTriangleMeasurementData.resize(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size());
	PointDataCount.resize(ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles.size());

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				for (size_t l = 0; l < Data[i][j][k].TrianglesInCell.size(); l++)
				{
					const int PointIndex = Data[i][j][k].TrianglesInCell[l];
					PointDataCount[PointIndex]++;
					PerTriangleMeasurementData[PointIndex] += static_cast<float>(Data[i][j][k].UserData);
				}
			}
		}
	}

	for (size_t i = 0; i < PerTriangleMeasurementData.size(); i++)
	{
		// If triangle was not in any cell, omit it
		// it should not happen, but just in case.
		if (PerTriangleMeasurementData[i] == 0 || PointDataCount[i] == 0)
			continue;

		PerTriangleMeasurementData[i] /= PointDataCount[i];
	}

	TimeTakenToFillMeasurementData = static_cast<float>(TIME.EndTimeStamp("FillMeasurementData"));
}

void MeasurementGrid::FillPerPointMeasurementData()
{
	PointCloudGeometryData* CurrentPointCloudData = ANALYSIS_OBJECT_MANAGER.CurrentPointCloudGeometryData;
	if (CurrentPointCloudData == nullptr)
		return;

	if (CurrentPointCloudData->RawPointCloudData.empty())
		return;

	TIME.BeginTimeStamp("FillMeasurementData");

	std::vector<int> PointDataCount;
	PerPointMeasurementData.resize(CurrentPointCloudData->RawPointCloudData.size());
	PointDataCount.resize(CurrentPointCloudData->RawPointCloudData.size());

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				for (size_t l = 0; l < Data[i][j][k].PointsInCell.size(); l++)
				{
					const int PointIndex = Data[i][j][k].PointsInCell[l];
					PointDataCount[PointIndex]++;
					PerPointMeasurementData[PointIndex] += static_cast<float>(Data[i][j][k].UserData);
				}
			}
		}
	}

	for (size_t i = 0; i < PerPointMeasurementData.size(); i++)
	{
		// If point was not in any cell, omit it
		// it should not happen, but just in case.
		if (PerPointMeasurementData[i] == 0 || PointDataCount[i] == 0)
			continue;

		PerPointMeasurementData[i] /= PointDataCount[i];
	}

	TimeTakenToFillMeasurementData = static_cast<float>(TIME.EndTimeStamp("FillMeasurementData"));
}

void MeasurementGrid::FillMeasurementData()
{
	if (bTriangleMode)
		FillPerTriangleMeasurementData();
	else
		FillPerPointMeasurementData();
}

void MeasurementGrid::AddLinesOfGrid()
{
	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				bool bNeedToRender = false;
				Data[i][j][k].bWasRenderedLastFrame = false;

				if (!Data[i][j][k].TrianglesInCell.empty() || RenderingMode == 2)
					bNeedToRender = true;

				if (bNeedToRender)
				{
					glm::vec3 Color = glm::vec3(0.1f, 0.6f, 0.1f);
					if (Data[i][j][k].bSelected)
						Color = glm::vec3(0.9f, 0.1f, 0.1f);

					LINE_RENDERER.RenderAABB(Data[i][j][k].AABB.Transform(SCENE_RESOURCES.ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix()), Color);

					Data[i][j][k].bWasRenderedLastFrame = true;

					if (bShowTrianglesInCells && Data[i][j][k].bSelected)
					{
						for (size_t l = 0; l < Data[i][j][k].TrianglesInCell.size(); l++)
						{
							const auto CurrentTriangle = ANALYSIS_OBJECT_MANAGER.CurrentMeshGeometryData->Triangles[Data[i][j][k].TrianglesInCell[l]];

							std::vector<glm::dvec3> TranformedTrianglePoints = CurrentTriangle;
							for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
							{
								TranformedTrianglePoints[j] = SCENE_RESOURCES.ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
							}

							LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[1], glm::vec3(1.0f, 1.0f, 0.0f)));
							LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[0], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
							LINE_RENDERER.AddLineToBuffer(FECustomLine(TranformedTrianglePoints[1], TranformedTrianglePoints[2], glm::vec3(1.0f, 1.0f, 0.0f)));
						}
					}
				}
			}
		}
	}
}

void MeasurementGrid::UpdateRenderedLines()
{
#ifndef NEW_LINES
	LINE_RENDERER.ClearAll();
	if (RenderingMode != 0)
		AddLinesOfGrid();
	LINE_RENDERER.SyncWithGPU();
#endif
}

void MeasurementGrid::RunOnAllNodes(std::function<void(GridNode* currentNode)> Func)
{
	if (Func == nullptr)
		return;

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				Func(&Data[i][j][k]);
			}
		}
	}
}

bool MeasurementGrid::IsInTriangleMode()
{
	return bTriangleMode;
}
