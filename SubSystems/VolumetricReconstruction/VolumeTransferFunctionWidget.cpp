#include "VolumeTransferFunctionWidget.h"

const std::vector<VolumeTransferFunctionWidget::ColorPreset> VolumeTransferFunctionWidget::ColorPresets =
{
	{ "Rainbow",      { { 0.0f, 0, 0, 1}, {0.25f, 0, 1, 1}, {0.5f, 0, 1, 0}, {0.75f, 1, 1, 0}, {1.0f, 1, 0, 0} } },
	{ "Grayscale",    { { 0.0f, 0, 0, 0}, {1.0f, 1, 1, 1} } },
	{ "Cool to Warm", { { 0.0f, 0.23f, 0.30f, 0.75f}, {0.5f, 0.87f, 0.87f, 0.87f}, {1.0f, 0.71f, 0.02f, 0.15f} } },
	{ "Viridis",      { { 0.0f, 0.27f, 0.0f, 0.33f}, {0.33f, 0.21f, 0.37f, 0.55f}, {0.66f, 0.13f, 0.57f, 0.55f}, {1.0f, 0.99f, 0.91f, 0.14f} } },
	// Google Turbo (Apache-2.0, Anton Mikhailov), subsampled to 17 evenly spaced stops.
	{ "Turbo",        { { 0.0f, 0.18995f, 0.07176f, 0.23217f }, { 0.0627f, 0.25107f, 0.25237f, 0.63374f }, { 0.1255f, 0.27628f, 0.42118f, 0.89123f }, { 0.1882f, 0.25862f, 0.57958f, 0.99876f }, { 0.2510f, 0.15844f, 0.73551f, 0.92305f }, { 0.3137f, 0.09267f, 0.86554f, 0.76230f }, { 0.3765f, 0.19659f, 0.94901f, 0.59466f }, { 0.4392f, 0.42778f, 0.99419f, 0.38575f }, { 0.5020f, 0.64362f, 0.98999f, 0.23356f }, { 0.5647f, 0.80473f, 0.92452f, 0.20459f }, { 0.6275f, 0.93301f, 0.81236f, 0.22667f }, { 0.6902f, 0.99314f, 0.67408f, 0.20348f }, { 0.7529f, 0.98360f, 0.49291f, 0.12849f }, { 0.8157f, 0.92105f, 0.31489f, 0.05475f }, { 0.8784f, 0.81608f, 0.18462f, 0.01809f }, { 0.9412f, 0.66449f, 0.08436f, 0.00424f }, { 1.0f, 0.47960f, 0.01583f, 0.01055f } } }
};

void VolumeTransferFunctionWidget::ApplyColorPreset(FEEntity* CurrentEntity, int PresetIndex)
{
	if (CurrentEntity == nullptr)
		return;

	if (PresetIndex < 0 || PresetIndex >= static_cast<int>(ColorPresets.size()))
		return;

	std::vector<FETransferFunctionColorPoint>& ColorPoints = VOLUME_SYSTEM.GetTransferFunctionColorPoints(CurrentEntity);
	ColorPoints.clear();
	for (const ColorPresetStop& Stop : ColorPresets[PresetIndex].Stops)
		ColorPoints.push_back({ Stop.Position, glm::vec3(Stop.R, Stop.G, Stop.B) });
}

void VolumeTransferFunctionWidget::SelectColorPreset(FEEntity* CurrentEntity, int PresetIndex)
{
	ApplyColorPreset(CurrentEntity, PresetIndex);
	SelectedColorPoint = -1;
	bChangedThisFrame = true;
}

void VolumeTransferFunctionWidget::Render(FEEntity* CurrentEntity, float DataValueLow, float DataValueHigh)
{
	if (CurrentEntity == nullptr)
		return;

	std::vector<FETransferFunctionColorPoint>& ColorPoints = VOLUME_SYSTEM.GetTransferFunctionColorPoints(CurrentEntity);
	std::vector<FETransferFunctionOpacityPoint>& OpacityPoints = VOLUME_SYSTEM.GetTransferFunctionOpacityPoints(CurrentEntity);
	if (ColorPoints.empty() || OpacityPoints.empty())
		return;

	bChangedThisFrame = false;

	const float CanvasWidth = glm::max(ImGui::GetContentRegionAvail().x, 100.0f);
	const float CanvasHeight = 140.0f;
	const float MarkerStripHeight = 16.0f;
	const ImVec2 CanvasOrigin = ImGui::GetCursorScreenPos();
	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	ImGui::InvisibleButton("##TransferFunctionCanvas", ImVec2(CanvasWidth, CanvasHeight + MarkerStripHeight));
	const bool bCanvasHovered = ImGui::IsItemHovered();
	const ImVec2 MousePos = ImGui::GetIO().MousePos;

	auto PositionToX = [&](float Position) {
		return CanvasOrigin.x + Position * CanvasWidth;
	};

	auto OpacityToY  = [&](float Opacity)  {
		return CanvasOrigin.y + (1.0f - Opacity) * CanvasHeight;
	};

	auto XToPosition = [&](float X) {
		return glm::clamp((X - CanvasOrigin.x) / CanvasWidth, 0.0f, 1.0f);
	};

	auto YToOpacity  = [&](float Y) {
		return glm::clamp(1.0f - (Y - CanvasOrigin.y) / CanvasHeight, 0.0f, 1.0f);
	};

	const float HandleRadius = 5.0f;
	auto IsNearOpacityHandle = [&](int Index) {
		const ImVec2 Handle(PositionToX(OpacityPoints[Index].Position), OpacityToY(OpacityPoints[Index].Opacity));
		return fabs(MousePos.x - Handle.x) <= HandleRadius + 2.0f && fabs(MousePos.y - Handle.y) <= HandleRadius + 2.0f;
	};

	// Color gradient background (full alpha so the color map is visible regardless of opacity).
	const int StripCount = 128;
	for (int i = 0; i < StripCount; i++)
	{
		const float P0 = static_cast<float>(i) / static_cast<float>(StripCount);
		const float P1 = static_cast<float>(i + 1) / static_cast<float>(StripCount);
		const glm::vec3 Color = VOLUME_SYSTEM.EvaluateTransferFunctionColor(CurrentEntity, (P0 + P1) * 0.5f);
		const ImU32 PackedColor = ImGui::ColorConvertFloat4ToU32(ImVec4(Color.x, Color.y, Color.z, 1.0f));
		DrawList->AddRectFilled(ImVec2(PositionToX(P0), CanvasOrigin.y), ImVec2(PositionToX(P1), CanvasOrigin.y + CanvasHeight), PackedColor);
	}
	DrawList->AddRect(CanvasOrigin, ImVec2(CanvasOrigin.x + CanvasWidth, CanvasOrigin.y + CanvasHeight), IM_COL32(200, 200, 200, 255));

	// Mouse interaction.
	if (bCanvasHovered && ImGui::IsMouseClicked(0))
	{
		DraggedOpacityPoint = -1;
		DraggedColorPoint = -1;

		for (int i = 0; i < static_cast<int>(OpacityPoints.size()); i++)
		{
			if (IsNearOpacityHandle(i))
			{
				DraggedOpacityPoint = i;
				break;
			}
		}

		if (DraggedOpacityPoint == -1 && MousePos.y >= CanvasOrigin.y + CanvasHeight)
		{
			for (int i = 0; i < static_cast<int>(ColorPoints.size()); i++)
			{
				if (fabs(MousePos.x - PositionToX(ColorPoints[i].Position)) <= 7.0f)
				{
					DraggedColorPoint = i;
					SelectedColorPoint = i;
					break;
				}
			}
		}
	}

	if (ImGui::IsMouseReleased(0))
	{
		DraggedOpacityPoint = -1;
		DraggedColorPoint = -1;
	}

	if (DraggedOpacityPoint != -1 && ImGui::IsMouseDragging(0))
	{
		OpacityPoints[DraggedOpacityPoint].Opacity = YToOpacity(MousePos.y);
		const bool bEndpoint = (DraggedOpacityPoint == 0 || DraggedOpacityPoint == static_cast<int>(OpacityPoints.size()) - 1);
		if (!bEndpoint)
		{
			const float LeftLimit = OpacityPoints[DraggedOpacityPoint - 1].Position + 0.001f;
			const float RightLimit = OpacityPoints[DraggedOpacityPoint + 1].Position - 0.001f;
			OpacityPoints[DraggedOpacityPoint].Position = glm::clamp(XToPosition(MousePos.x), LeftLimit, RightLimit);
		}
		bChangedThisFrame = true;
	}

	if (DraggedColorPoint != -1 && ImGui::IsMouseDragging(0))
	{
		// Endpoints may move freely within [0, 1]; data beyond the moved endpoints is out of range and renders as black.
		const int LastIndex = static_cast<int>(ColorPoints.size()) - 1;
		const float LeftLimit = (DraggedColorPoint == 0) ? 0.0f : ColorPoints[DraggedColorPoint - 1].Position + 0.001f;
		const float RightLimit = (DraggedColorPoint == LastIndex) ? 1.0f : ColorPoints[DraggedColorPoint + 1].Position - 0.001f;
		ColorPoints[DraggedColorPoint].Position = glm::clamp(XToPosition(MousePos.x), LeftLimit, RightLimit);
		bChangedThisFrame = true;
	}

	// Double-click on empty curve area adds an opacity point.
	if (bCanvasHovered && ImGui::IsMouseDoubleClicked(0) && MousePos.y < CanvasOrigin.y + CanvasHeight)
	{
		bool bOnHandle = false;
		for (int i = 0; i < static_cast<int>(OpacityPoints.size()); i++)
			if (IsNearOpacityHandle(i)) { bOnHandle = true; break; }

		if (!bOnHandle)
		{
			FETransferFunctionOpacityPoint NewPoint;
			NewPoint.Position = XToPosition(MousePos.x);
			NewPoint.Opacity = YToOpacity(MousePos.y);

			int InsertIndex = static_cast<int>(OpacityPoints.size());
			for (int i = 0; i < static_cast<int>(OpacityPoints.size()); i++)
				if (OpacityPoints[i].Position > NewPoint.Position) { InsertIndex = i; break; }

			OpacityPoints.insert(OpacityPoints.begin() + InsertIndex, NewPoint);
			bChangedThisFrame = true;
		}
	}

	// Right-click removes points, an opacity handle on the curve, or a color stop.
	if (bCanvasHovered && ImGui::IsMouseClicked(1))
	{
		if (MousePos.y >= CanvasOrigin.y + CanvasHeight)
		{
			for (int i = 1; i < static_cast<int>(ColorPoints.size()) - 1; i++)
			{
				if (fabs(MousePos.x - PositionToX(ColorPoints[i].Position)) <= 7.0f)
				{
					ColorPoints.erase(ColorPoints.begin() + i);
					if (SelectedColorPoint == i)
						SelectedColorPoint = -1;
					else if (SelectedColorPoint > i)
						SelectedColorPoint--;
					bChangedThisFrame = true;
					break;
				}
			}
		}
		else
		{
			for (int i = 1; i < static_cast<int>(OpacityPoints.size()) - 1; i++)
			{
				if (IsNearOpacityHandle(i))
				{
					OpacityPoints.erase(OpacityPoints.begin() + i);
					bChangedThisFrame = true;
					break;
				}
			}
		}
	}

	// Draw overlays.
	for (size_t i = 1; i < OpacityPoints.size(); i++)
	{
		DrawList->AddLine(
			ImVec2(PositionToX(OpacityPoints[i - 1].Position), OpacityToY(OpacityPoints[i - 1].Opacity)),
			ImVec2(PositionToX(OpacityPoints[i].Position), OpacityToY(OpacityPoints[i].Opacity)),
			IM_COL32(255, 255, 255, 255), 2.0f);
	}

	for (size_t i = 0; i < OpacityPoints.size(); i++)
	{
		const ImVec2 Handle(PositionToX(OpacityPoints[i].Position), OpacityToY(OpacityPoints[i].Opacity));
		DrawList->AddCircleFilled(Handle, HandleRadius, IM_COL32(255, 255, 255, 255));
		DrawList->AddCircle(Handle, HandleRadius, IM_COL32(0, 0, 0, 255));
	}

	for (int i = 0; i < static_cast<int>(ColorPoints.size()); i++)
	{
		const float X = PositionToX(ColorPoints[i].Position);
		const float Y = CanvasOrigin.y + CanvasHeight;
		const glm::vec3 Color = ColorPoints[i].Color;
		const ImU32 Fill = ImGui::ColorConvertFloat4ToU32(ImVec4(Color.x, Color.y, Color.z, 1.0f));
		const ImU32 Border = (i == SelectedColorPoint) ? IM_COL32(255, 255, 0, 255) : IM_COL32(0, 0, 0, 255);
		DrawList->AddTriangleFilled(ImVec2(X - 6.0f, Y + MarkerStripHeight), ImVec2(X + 6.0f, Y + MarkerStripHeight), ImVec2(X, Y), Fill);
		DrawList->AddTriangle(ImVec2(X - 6.0f, Y + MarkerStripHeight), ImVec2(X + 6.0f, Y + MarkerStripHeight), ImVec2(X, Y), Border, 1.5f);
	}

	ImGui::Text("Data range: %.4g  ..  %.4g", DataValueLow, DataValueHigh);

	// Controls below the canvas.
	if (SelectedColorPoint >= 0 && SelectedColorPoint < static_cast<int>(ColorPoints.size()))
	{
		if (ImGui::ColorEdit3("Selected stop color", &ColorPoints[SelectedColorPoint].Color.x))
			bChangedThisFrame = true;

		const bool bEndpoint = (SelectedColorPoint == 0 || SelectedColorPoint == static_cast<int>(ColorPoints.size()) - 1);
		if (!bEndpoint)
		{
			if (ImGui::Button("Remove stop"))
			{
				ColorPoints.erase(ColorPoints.begin() + SelectedColorPoint);
				SelectedColorPoint = -1;
				bChangedThisFrame = true;
			}
		}
	}

	if (ImGui::Button("Add color stop"))
	{
		// Insert at the midpoint of the widest gap so it never collides with an existing stop.
		int GapIndex = 0;
		float WidestGap = -1.0f;
		for (int i = 1; i < static_cast<int>(ColorPoints.size()); i++)
		{
			const float Gap = ColorPoints[i].Position - ColorPoints[i - 1].Position;
			if (Gap > WidestGap)
			{
				WidestGap = Gap;
				GapIndex = i;
			}
		}

		FETransferFunctionColorPoint NewPoint;
		NewPoint.Position = (ColorPoints[GapIndex].Position + ColorPoints[GapIndex - 1].Position) * 0.5f;
		NewPoint.Color = VOLUME_SYSTEM.EvaluateTransferFunctionColor(CurrentEntity, NewPoint.Position);
		ColorPoints.insert(ColorPoints.begin() + GapIndex, NewPoint);
		SelectedColorPoint = GapIndex;
		bChangedThisFrame = true;
	}

	// Presets.
	std::vector<const char*> PresetNames;
	for (const ColorPreset& Preset : ColorPresets)
		PresetNames.push_back(Preset.Name.c_str());

	ImGui::SetNextItemWidth(160.0f);
	if (ImGui::Combo("Color preset", &CurrentPreset, PresetNames.data(), static_cast<int>(PresetNames.size())))
		SelectColorPreset(CurrentEntity, CurrentPreset);
	
	if (bChangedThisFrame)
		VOLUME_SYSTEM.BakeTransferFunction(CurrentEntity);
}