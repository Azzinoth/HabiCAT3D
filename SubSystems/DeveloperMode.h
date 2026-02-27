#pragma once

#include "../SubSystems/ComplexityCore/Layers/Producers/RugosityLayerProducer.h"

class DeveloperMode
{
public:
	SINGLETON_PUBLIC_PART(DeveloperMode)

	void Initialize();

	bool IsOn();
	void SetIsOn(bool NewValue);

	MeasurementGrid* GetDebugGrid();
	void InitDebugGrid(size_t JitterIndex);
	void ClearDebugGrid();

	std::vector<GridInitData_Jitter> ReadJitterSettingsFromDebugInfo(DataLayerDebugInfo* DebugInfo);

	void AddOnDebugGridSelectedCellChangedCallback(std::function<void(glm::vec3 SelectedCellIndex)> Callback);

	void ShowLayerDebugUI();
private:
	SINGLETON_PRIVATE_PART(DeveloperMode)

	bool bIsOn = false;
	MeasurementGrid* DebugGrid = nullptr;

	int CurrentJitterStepIndexVisualize = 0;
	void UpdateRenderingMode(int NewRenderingMode);

	static void OnDebugGridSelectedCellChanged(glm::vec3 NewSelectedCell);
	std::vector<std::function<void(glm::vec3 SelectedCellIndex)>> ClientOnDebugGridSelectedCellChangedCallbacks;

	double MouseX;
	double MouseY;
	static void MouseButtonCallback(int button, int action, int mods);
	static void MouseMoveCallback(double XPos, double YPos);

	static void OnLayerChange();

	static void OnJitterCalculationsEnd(DataLayer* NewLayer);
};

#define DEVELOPER_MODE DeveloperMode::GetInstance()