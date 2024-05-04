#include "MeasurementGrid.h"
using namespace FocalEngine;

MeasurementGrid::MeasurementGrid() {}

MeasurementGrid::~MeasurementGrid()
{
	Data.clear();
}

void MeasurementGrid::InitializeSegment(size_t beginIndex, size_t endIndex, size_t Dimensions, FEAABB GridAABB, float CellSize)
{
	const glm::vec3 Start = GridAABB.GetMin();

	for (size_t i = beginIndex; i < endIndex; i++)
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
	if (bUseingMultiThreading)
	{
		size_t numThreads = std::thread::hardware_concurrency();
		// Using dedicated threads instead of the thread pool to make less changes to the existing code
		std::vector<std::thread> threads(numThreads);

		size_t chunkSize = Dimensions / numThreads;  // Divide the work into chunks per thread

		for (size_t i = 0; i < numThreads; ++i)
		{
			size_t start = i * chunkSize;
			size_t end = (i + 1) * chunkSize;
			if (i == numThreads - 1)
			{
				end = Dimensions;  // Make sure the last thread covers all remaining elements
			}

			threads[i] = std::thread([=]() {
				InitializeSegment(start, end, Dimensions, GridAABB, CellSize);
			});
		}

		// Wait for all threads to finish
		for (auto& thread : threads)
		{
			thread.join();
		}
	}
	else
	{
		for (size_t i = 0; i < Dimensions; i++)
		{
			Data[i].resize(Dimensions);
			for (size_t j = 0; j < Dimensions; j++)
			{
				Data[i][j].resize(Dimensions);
			}
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
		FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

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
						if (GEOMETRY.IsAABBIntersectTriangle(Data[i][j][k].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
						{
							Output->push_back(GridUpdateTask(static_cast<int>(i), static_cast<int>(j), static_cast<int>(k), l));
						}
					}
				}
			}
		}
	}
}

void GatherGridFillingThreadWork(void* OutputData)
{

}

void MeasurementGrid::FillCellsWithTriangleInfo()
{
	TIME.BeginTimeStamp("Fill cells with triangle info");


	if (bUseingMultiThreading)
	{
		int THREAD_COUNT = THREAD_POOL.GetThreadCount();


		int NumberOfTrianglesPerThread = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() / THREAD_COUNT);

		if (THREAD_COUNT > NumberOfTrianglesPerThread)
		{
			THREAD_COUNT = 1;
			NumberOfTrianglesPerThread = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size());
		}

		// Alternative
		std::vector<std::thread> threads(THREAD_COUNT);

		std::vector<GridThreadData*> ThreadData;
		std::vector<std::vector<GridUpdateTask>*> AllOutputTasks;

		for (int i = 0; i < THREAD_COUNT; i++)
		{
			GridThreadData* NewThreadData = new GridThreadData();
			ThreadData.push_back(NewThreadData);
			NewThreadData->FirstIndexInTriangleArray = i * NumberOfTrianglesPerThread + 1;

			if (i == 0)
				NewThreadData->FirstIndexInTriangleArray = 0;

			if (i == THREAD_COUNT - 1)
				NewThreadData->LastIndexInTriangleArray = static_cast<int>(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size() - 1);
			else
				NewThreadData->LastIndexInTriangleArray = (i + 1) * NumberOfTrianglesPerThread;

			std::vector<GridUpdateTask>* OutputTasks = new std::vector<GridUpdateTask>();
			AllOutputTasks.push_back(OutputTasks);

			threads[i] = std::thread([=]() {
				GridFillingThread(NewThreadData, OutputTasks);
			});

			//THREAD_POOL.Execute(GridFillingThread, NewThreadData, OutputTasks, GatherGridFillingThreadWork);
		}

		// Wait for all threads to finish
		for (auto& thread : threads)
		{
			thread.join();
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

		for (int l = 0; l < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size(); l++)
		{
			FEAABB TriangleAABB = FEAABB(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]);

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
							if (GEOMETRY.IsAABBIntersectTriangle(Data[i][j][k].AABB, COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[l]))
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

				FEAABB FinalAABB = Data[i][j][k].AABB.Transform(TransformMat).Transform(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Position->GetTransformMatrix());
				if (FinalAABB.RayIntersect(ENGINE.GetCamera()->GetPosition(), ENGINE.ConstructMouseRay(), DistanceToCell))
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

void MeasurementGrid::FillMeshWithUserData()
{
	if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		return;

	TIME.BeginTimeStamp("FillMeshWithUserData");

	std::vector<int> TrianglesRugosityCount;
	TrianglesUserData.resize(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size());
	TrianglesRugosityCount.resize(COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles.size());

	for (size_t i = 0; i < Data.size(); i++)
	{
		for (size_t j = 0; j < Data[i].size(); j++)
		{
			for (size_t k = 0; k < Data[i][j].size(); k++)
			{
				for (size_t l = 0; l < Data[i][j][k].TrianglesInCell.size(); l++)
				{
					const int TriangleIndex = Data[i][j][k].TrianglesInCell[l];
					TrianglesRugosityCount[TriangleIndex]++;
					TrianglesUserData[TriangleIndex] += static_cast<float>(Data[i][j][k].UserData);
				}
			}
		}
	}

	for (size_t i = 0; i < TrianglesUserData.size(); i++)
	{
		// If triangle was not in any cell, omit it
		// it should not happen, but just in case.
		if (TrianglesUserData[i] == 0 || TrianglesRugosityCount[i] == 0)
			continue;

		TrianglesUserData[i] /= TrianglesRugosityCount[i];
	}

	TimeTakenToFillMeshWithUserData = static_cast<float>(TIME.EndTimeStamp("FillMeshWithUserData"));
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

					LINE_RENDERER.RenderAABB(Data[i][j][k].AABB.Transform(MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix()), Color);

					Data[i][j][k].bWasRenderedLastFrame = true;

					if (bShowTrianglesInCells && Data[i][j][k].bSelected)
					{
						for (size_t l = 0; l < Data[i][j][k].TrianglesInCell.size(); l++)
						{
							const auto CurrentTriangle = COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Triangles[Data[i][j][k].TrianglesInCell[l]];

							std::vector<glm::vec3> TranformedTrianglePoints = CurrentTriangle;
							for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
							{
								TranformedTrianglePoints[j] = MESH_MANAGER.ActiveEntity->Transform.GetTransformMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
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