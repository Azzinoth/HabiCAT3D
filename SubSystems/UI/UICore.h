#pragma once

#include "../SubSystems/ComplexityCore/Layers/Producers/RugosityLayerProducer.h"
#include "../SubSystems/DeveloperMode.h"

struct LabeledColor
{
	std::string Label;
	glm::vec4 Color = glm::vec4(1.0f);
};

class UICore
{
public:
	SINGLETON_PUBLIC_PART(UICore)

	std::string GetVersion();
	int GetBuildNumber();
	std::string GetBuildTimestamp();
	std::string GetBuildInfo();
	std::string GetFullVersion();

	std::string TruncateAfterDot(std::string FloatingPointNumber, const int DigitCount = 2);
	void ShowToolTip(const char* Text);
	void ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform);

	bool ShowColorPickerButton(const char* ID, glm::vec4& Color);
	int ShowLabeledColorTable(const char* TableID, std::vector<LabeledColor>& Rows, float Height = 160.0f);
private:
	SINGLETON_PRIVATE_PART(UICore)
};

#define UI_CORE UICore::GetInstance()