#pragma once

#include "../SubSystems/ComplexityCore/Layers/Producers/RugosityLayerProducer.h"
#include "../SubSystems/DeveloperMode.h"

class UICore
{
public:
	SINGLETON_PUBLIC_PART(UICore)

	std::string GetHabiCAT3DVersion();
	int GetHabiCAT3DBuildNumber();
	std::string GetHabiCAT3DBuildTimestamp();
	std::string GetHabiCAT3DBuildInfo();
	std::string GetHabiCAT3DFullVersion();
	
	std::string TruncateAfterDot(std::string FloatingPointNumber, const int DigitCount = 2);
	void ShowToolTip(const char* Text);
	void ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform);
private:
	SINGLETON_PRIVATE_PART(UICore)


	

};

#define UI_CORE UICore::GetInstance()