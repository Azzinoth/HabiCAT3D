#include "UIWeightedHistogram.h"

FEWeightedHistogram::FEWeightedHistogram()
{
}

ImVec2 FEWeightedHistogram::GetPosition() const
{
	return Position;
}

void FEWeightedHistogram::SetPosition(ImVec2 NewValue)
{
	Position = NewValue;
	Graph.SetPosition(Position);
}

ImVec2 FEWeightedHistogram::GetSize() const
{
	return Size;
}

void FEWeightedHistogram::SetSize(ImVec2 NewValue)
{
	Size = NewValue;
	Graph.SetSize(NewValue);
}

FEWeightedHistogram::~FEWeightedHistogram()
{
}

void FEWeightedHistogram::Render()
{
	Graph.Render();
}

void FEWeightedHistogram::Clear()
{
	Graph.Clear();

	MinValue = DBL_MAX;
	MaxValue = -DBL_MAX;
}

void FEWeightedHistogram::SetLegendCaption(float NormalizedPosition, std::string Text)
{
	Graph.Legend.SetCaption(NormalizedPosition, Text);
}

std::vector<float> FEWeightedHistogram::GetDataPoints() const
{
	return Graph.GetDataPoints();
}

void FEWeightedHistogram::SetDataPoints(std::vector<float> NewValue)
{
	Graph.SetDataPoints(NewValue);
}

float FEWeightedHistogram::GetCeiling()
{
	return Graph.GetCeiling();
}

void FEWeightedHistogram::SetCeiling(float NewValue)
{
	Graph.SetCeiling(NewValue);
}

void FEWeightedHistogram::FillDataBins(const std::vector<double>& Values, const std::vector<double>& Weights, size_t BinsCount)
{
	std::vector<float> DataPoints;
	std::vector<float> MinRugosity;
	MinRugosity.resize(BinsCount);
	std::vector<float> MaxRugosity;
	MaxRugosity.resize(BinsCount);

	MinValue = *std::min_element(Values.begin(), Values.end());
	MaxValue = *std::max_element(Values.begin(), Values.end());

	for (size_t i = 0; i < BinsCount; i++)
	{
		const double NormalizedPixelPosition = double(i) / (BinsCount);
		const double NextNormalizedPixelPosition = double(i + 1) / (BinsCount);

		MinRugosity[i] = MinValue + (MaxValue - MinValue) * NormalizedPixelPosition;
		MaxRugosity[i] = MinValue + (MaxValue - MinValue) * NextNormalizedPixelPosition;
	}

	for (size_t i = 0; i < BinsCount; i++)
	{
		double CurrentArea = 0.0;
		for (int j = 0; j < Values.size(); j++)
		{
			const double CurrentValue = Values[j];
			if (CurrentValue >= MinRugosity[i] && (i == BinsCount - 1 ? CurrentValue <= MaxRugosity[i] : CurrentValue < MaxRugosity[i]))
			{
				CurrentArea += Weights[j];
			}
		}

		DataPoints.push_back(CurrentArea);
	}

	double TotalArea = 0.0;
	float MaxValue = -FLT_MAX;
	for (size_t i = 0; i < BinsCount; i++)
	{
		TotalArea += DataPoints[i];
		MaxValue = std::max(DataPoints[i], MaxValue);
	}

	SetDataPoints(DataPoints);
	SetCeiling(MaxValue * 1.2f);
}

bool FEWeightedHistogram::IsUsingInterpolation()
{
	return Graph.IsUsingInterpolation();
}

void FEWeightedHistogram::SetIsUsingInterpolation(bool NewValue)
{
	Graph.SetIsUsingInterpolation(NewValue);
}

int FEWeightedHistogram::GetCurrentBinCount() const
{
	return Graph.GetDataPoints().size();
}