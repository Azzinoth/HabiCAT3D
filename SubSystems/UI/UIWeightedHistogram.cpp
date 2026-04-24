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

	MinValue = std::numeric_limits<double>::max();
	MaxValue = -std::numeric_limits<double>::max();
}

void FEWeightedHistogram::SetLegendCaption(float NormalizedPosition, std::string Text)
{
	Graph.XLegend.SetCaption(NormalizedPosition, Text);
}

float FEWeightedHistogram::GetCeiling()
{
	return Graph.GetCeiling();
}

void FEWeightedHistogram::SetCeiling(float NewValue)
{
	Graph.SetCeiling(NewValue);
}

std::vector<FEGraphDataPoint> FEWeightedHistogram::ConvertToDataPoints(const std::vector<double>& Values,
																	   const std::vector<double>& Weights,
																	   size_t BinsCount,
																	   double OverrideMinValue,
																	   double OverrideMaxValue)
{
	std::vector<FEGraphDataPoint> Result;
	Result.resize(BinsCount);

	std::vector<float> DataPoints;
	std::vector<float> BinLowerBounds;
	BinLowerBounds.resize(BinsCount);
	std::vector<float> BinUpperBounds;
	BinUpperBounds.resize(BinsCount);

	if (OverrideMinValue != std::numeric_limits<double>::max())
	{
		MinValue = OverrideMinValue;
	}
	else
	{
		MinValue = *std::min_element(Values.begin(), Values.end());
	}

	if (OverrideMaxValue != -std::numeric_limits<double>::max())
	{
		MaxValue = OverrideMaxValue;
	}
	else
	{
		MaxValue = *std::max_element(Values.begin(), Values.end());
	}

	for (size_t i = 0; i < BinsCount; i++)
	{
		const double NormalizedPixelPosition = double(i) / (BinsCount);
		const double NextNormalizedPixelPosition = double(i + 1) / (BinsCount);

		BinLowerBounds[i] = static_cast<float>(MinValue + (MaxValue - MinValue) * NormalizedPixelPosition);
		BinUpperBounds[i] = static_cast<float>(MinValue + (MaxValue - MinValue) * NextNormalizedPixelPosition);

		Result[i].XValue = BinLowerBounds[i];
	}

	for (size_t i = 0; i < BinsCount; i++)
	{
		double CurrentBinWeight = 0.0;
		for (int j = 0; j < Values.size(); j++)
		{
			const double CurrentValue = Values[j];
			if (CurrentValue >= BinLowerBounds[i] && (i == BinsCount - 1 ? CurrentValue <= BinUpperBounds[i] : CurrentValue < BinUpperBounds[i]))
			{
				CurrentBinWeight += Weights[j];
			}
		}

		DataPoints.push_back(static_cast<float>(CurrentBinWeight));

		Result[i].YValue = CurrentBinWeight;
		Result[i].StackID = 0;
	}

	return Result;
}

void FEWeightedHistogram::FillDataBins(const std::vector<double>& Values, const std::vector<double>& Weights, size_t BinsCount)
{
	std::vector<FEGraphDataPoint> GraphDataPoints = ConvertToDataPoints(Values, Weights, BinsCount);

	double TotalWeight = 0.0;
	float MaxValue = -FLT_MAX;
	for (size_t i = 0; i < GraphDataPoints.size(); i++)
	{
		TotalWeight += GraphDataPoints[i].YValue;
		MaxValue = std::max(float(GraphDataPoints[i].YValue), MaxValue);
	}

	Graph.AddDataPoints(GraphDataPoints);

	SetCeiling(MaxValue * 1.2f);
}

int FEWeightedHistogram::GetBinCount() const
{
	return CurrentBinCount;
}

FEGraphRender* FEWeightedHistogram::GetGraphPointer()
{ 
	return &Graph;
}