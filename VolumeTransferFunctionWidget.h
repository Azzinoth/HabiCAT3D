#pragma once

#include "../SubSystems/FocalEngine/FEngine.h"

class VolumeTransferFunctionWidget
{
public:
	void Render(FEEntity* CurrentEntity, float DataValueLow, float DataValueHigh);

	void SelectColorPreset(FEEntity* CurrentEntity, int PresetIndex);
private:
	// A named color-map preset: a list of (position, RGB) stops used to populate the transfer function.
	struct ColorPresetStop
	{
		float Position;
		float R;
		float G;
		float B;
	};

	struct ColorPreset
	{
		std::string Name;
		std::vector<ColorPresetStop> Stops;
	};

	static const std::vector<ColorPreset> ColorPresets;

	void ApplyColorPreset(FEEntity* CurrentEntity, int PresetIndex);

	int SelectedColorPoint = -1;
	int DraggedColorPoint = -1;
	int DraggedOpacityPoint = -1;
	int CurrentPreset = 0;
	bool bChangedThisFrame = false;
};