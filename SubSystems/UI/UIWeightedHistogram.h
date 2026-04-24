#include "UIComponents.h"

class FEWeightedHistogram
{
	double MinValue = std::numeric_limits<double>::max();
	double MaxValue = -std::numeric_limits<double>::max();

	int CurrentBinCount = 128;

	ImVec2 Position = ImVec2(10, 10);
	ImVec2 Size = ImVec2(100, 100);

	FEGraphRender Graph;
public:
	FEWeightedHistogram();
	~FEWeightedHistogram();

	void Render();

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewValue);

	ImVec2 GetSize() const;
	void SetSize(ImVec2 NewValue);

	int GetBinCount() const;

	void Clear();

	void SetLegendCaption(float NormalizedPosition, std::string Text);

	float GetCeiling();
	void SetCeiling(float NewValue);

	void FillDataBins(const std::vector<double>& Values, const std::vector<double>& Weights, size_t BinsCount);
	std::vector<FEGraphDataPoint> ConvertToDataPoints(const std::vector<double>& Values,
													  const std::vector<double>& Weights,
													  size_t BinsCount,
													  double OverrideMinValue = std::numeric_limits<double>::max(),
													  double OverrideMaxValue = -std::numeric_limits<double>::max());

	FEGraphRender* GetGraphPointer();
};