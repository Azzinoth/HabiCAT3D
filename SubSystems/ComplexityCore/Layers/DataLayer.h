#pragma once

#include "../AnalysisObjectManager.h"
using namespace FocalEngine;

struct DebugEntry
{
	DebugEntry();
	DebugEntry(std::string Type, int Size, char* RawData);

	std::string Name;
	char* RawData = nullptr;
	std::string Type;
	int Size;

	std::string ToString();
};

struct DataLayerDebugInfo
{
	uint64_t StartCalculationsTime;
	uint64_t EndCalculationsTime;

	std::string Type = "DataLayerDebugInfo";

	virtual std::string ToString();

	virtual void FromFile(std::fstream& File);
	virtual void ToFile(std::fstream& File);

	std::vector<DebugEntry> Entries;

	void AddEntry(std::string Name, bool Data);
	void AddEntry(std::string Name, int Data);
	void AddEntry(std::string Name, float Data);
	void AddEntry(std::string Name, double Data);
	void AddEntry(std::string Name, uint64_t Data);
	void AddEntry(std::string Name, std::string Data);
};

enum class LAYER_TYPE
{
	UNKNOWN = 0,
	HEIGHT = 1,
	TRIANGLE_EDGE = 2,
	TRIANGLE_AREA = 3,
	COMPARE = 4,
	STANDARD_DEVIATION = 5,
	RUGOSITY = 6,
	VECTOR_DISPERSION = 7,
	FRACTAL_DIMENSION = 8,
	TRIANGLE_DENSITY = 9,

	// Point cloud specific types
	POINT_DENSITY = 10
};

class DataLayer
{
	friend class LayerManager;

	std::string ID;
	std::string Caption = "Layer caption";
	std::string UserNote;
	LAYER_TYPE Type = LAYER_TYPE::UNKNOWN;
	DATA_SOURCE_TYPE DataSourceType = DATA_SOURCE_TYPE::MESH;

	float Max = -FLT_MAX;
	float Min = FLT_MAX;
	float Mean = -FLT_MAX;
	float Median = -FLT_MAX;

	void ComputeStatistics();

	float SelectedRangeMin = 0.0f;
	float SelectedRangeMax = 0.0f;
public:
	DataLayer();
	DataLayer(DATA_SOURCE_TYPE SourceType);
	DataLayer(DATA_SOURCE_TYPE SourceType, std::vector<float> ElementsToData);
	~DataLayer();

	static DATA_SOURCE_TYPE GetDataSourceTypeForLayerType(LAYER_TYPE Type);

	void FillRawData();

	std::string GetID();
	void ForceID(std::string ID);

	DataLayerDebugInfo* DebugInfo = nullptr;

	std::string GetCaption();
	void SetCaption(std::string NewValue);

	std::string GetNote();
	void SetNote(std::string NewValue);

	float GetMax();
	float GetMin();
	float GetMean();
	float GetMedian();

	float MinVisible = FLT_MAX;
	float MaxVisible = FLT_MAX;

	std::vector<float> RawData;
	// This vector contains the data for each triangle/point.
	std::vector<float> ElementsToData;

	// Mesh specific data.
	std::vector<std::tuple<double, double, int>> ValueTriangleAreaAndIndex = std::vector<std::tuple<double, double, int>>();

	LAYER_TYPE GetType();
	void SetType(LAYER_TYPE NewValue);

	DATA_SOURCE_TYPE GetDataSourceType();
	void SetDataSourceType(DATA_SOURCE_TYPE NewValue);

	float GetSelectedRangeMin();
	void SetSelectedRangeMin(float NewValue);

	float GetSelectedRangeMax();
	void SetSelectedRangeMax(float NewValue);
};