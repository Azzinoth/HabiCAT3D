#pragma once

#include "../SubSystems/ComplexityCore/Layers/Producers/RugosityLayerProducer.h"
#include "../SubSystems/DeveloperMode.h"

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
private:
	SINGLETON_PRIVATE_PART(UICore)
};

#define UI_CORE UICore::GetInstance()