#include "UIComponents.h"

class FEWeightedHistogram
{
	double MinValue = DBL_MAX;
	double MaxValue = -DBL_MAX;

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

	int GetCurrentBinCount() const;

	void Clear();

	void SetLegendCaption(float NormalizedPosition, std::string Text);

	float GetCeiling();
	void SetCeiling(float NewValue);

	bool IsUsingInterpolation();
	void SetIsUsingInterpolation(bool NewValue);

	std::vector<float> GetDataPoints() const;
	void SetDataPoints(std::vector<float> NewValue);

	void FillDataBins(const std::vector<double>& Values, const std::vector<double>& Weights, size_t BinsCount);
};