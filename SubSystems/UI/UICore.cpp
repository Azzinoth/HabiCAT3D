#include "UICore.h"
using namespace FocalEngine;

UICore::UICore() {}
UICore::~UICore() {}

std::string UICore::GetVersion()
{
	return GetHabiCAT3D_VersionInfo().GetVersion();
}

int UICore::GetBuildNumber()
{
	return GetHabiCAT3D_VersionInfo().BuildNumber;
}

std::string UICore::GetBuildTimestamp()
{
	return GetHabiCAT3D_VersionInfo().BuildTimestamp;
}

std::string UICore::GetBuildInfo()
{
	return GetHabiCAT3D_VersionInfo().GetBuildInfo();
}

std::string UICore::GetFullVersion()
{
	return "HabiCAT3D " + GetHabiCAT3D_VersionInfo().GetFullVersionString();
}

std::string UICore::TruncateAfterDot(std::string FloatingPointNumber, const int DigitCount)
{
	int Count = 0;
	bool WasFound = false;
	for (size_t i = 0; i < FloatingPointNumber.size(); i++)
	{
		if (FloatingPointNumber[i] == '.')
		{
			WasFound = true;
			continue;
		}

		if (WasFound)
		{
			if (DigitCount == Count)
			{
				std::string Result = FloatingPointNumber.substr(0, i);
				return Result;
			}
			Count++;
		}
	}

	return FloatingPointNumber;
}

void UICore::ShowToolTip(const char* Text)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(Text);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void UICore::ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform)
{
	static float EditWidth = 70.0f;
	bool bModified = false;
	// ********************* POSITION *********************
	glm::vec3 Position = Transform->GetPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X pos : ") + Name).c_str(), &Position[0], 0.1f))
		bModified = true;
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y pos : ") + Name).c_str(), &Position[1], 0.1f))
		bModified = true;
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z pos : ") + Name).c_str(), &Position[2], 0.1f))
		bModified = true;
	ShowToolTip("Z position");

	if (bModified)
		Transform->SetPosition(Position);

	bModified = false;

	// ********************* WORLD POSITION *********************
	glm::vec3 WorldPosition = Transform->GetPosition(FE_WORLD_SPACE);
	ImGui::Text("World Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X pos : ") + Name).c_str(), &WorldPosition[0], 0.1f))
		bModified = true;
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y pos : ") + Name).c_str(), &WorldPosition[1], 0.1f))
		bModified = true;
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z pos : ") + Name).c_str(), &WorldPosition[2], 0.1f))
		bModified = true;
	ShowToolTip("Z position");

	if (bModified)
		Transform->SetPosition(WorldPosition, FE_WORLD_SPACE);

	bModified = false;

	// ********************* ROTATION *********************
	glm::vec3 Rotation = Transform->GetRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X rot : ") + Name).c_str(), &Rotation[0], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y rot : ") + Name).c_str(), &Rotation[1], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z rot : ") + Name).c_str(), &Rotation[2], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Z rotation");

	if (bModified)
		Transform->SetRotation(Rotation);

	bModified = false;

	// ********************* WORLD ROTATION *********************
	glm::vec3 WorldRotation = Transform->GetRotation(FE_WORLD_SPACE);
	ImGui::Text("World Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X rot : ") + Name).c_str(), &WorldRotation[0], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y rot : ") + Name).c_str(), &WorldRotation[1], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z rot : ") + Name).c_str(), &WorldRotation[2], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Z rotation");

	if (bModified)
		Transform->SetRotation(WorldRotation, FE_WORLD_SPACE);

	bModified = false;

	// ********************* SCALE *********************
	bool bUniformScaling = Transform->IsUniformScalingSet();
	ImGui::Checkbox("Uniform scaling", &bUniformScaling);
	Transform->SetUniformScaling(bUniformScaling);

	glm::vec3 Scale = Transform->GetScale();
	float ScaleChangeSpeed = Scale.x * 0.01f;
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X scale : ") + Name).c_str(), &Scale[0], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[1] = Scale[0];
			Scale[2] = Scale[0];
		}
	}
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y scale : ") + Name).c_str(), &Scale[1], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[0] = Scale[1];
			Scale[2] = Scale[1];
		}
	}
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z scale : ") + Name).c_str(), &Scale[2], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[0] = Scale[2];
			Scale[1] = Scale[2];
		}
	}
	ShowToolTip("Z scale");

	if (bModified)
		Transform->SetScale(Scale);

	bModified = false;

	// ********************* WORLD SCALE *********************
	glm::vec3 WorldScale = Transform->GetScale(FE_WORLD_SPACE);
	ScaleChangeSpeed = WorldScale.x * 0.01f;
	ImGui::Text("World Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X scale : ") + Name).c_str(), &WorldScale[0], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[1] = WorldScale[0];
			WorldScale[2] = WorldScale[0];
		}
	}
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y scale : ") + Name).c_str(), &WorldScale[1], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[0] = WorldScale[1];
			WorldScale[2] = WorldScale[1];
		}
	}
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z scale : ") + Name).c_str(), &WorldScale[2], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[0] = WorldScale[2];
			WorldScale[1] = WorldScale[2];
		}
	}
	ShowToolTip("Z scale");

	if (bModified)
		Transform->SetScale(WorldScale, FE_WORLD_SPACE);
}