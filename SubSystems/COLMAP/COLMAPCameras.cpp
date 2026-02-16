#include "COLMAPCameras.h"
using namespace FocalEngine;

COLMAPPhysicalCamera::COLMAPPhysicalCamera()
{
	ID = APPLICATION.GetUniqueHexID();
	Width = 0;
	Height = 0;
}

COLMAPPhysicalCamera::~COLMAPPhysicalCamera() {}

std::string COLMAPPhysicalCamera::GetID() const
{
	return ID;
}

std::string COLMAPPhysicalCamera::GetModel() const
{
	return Model;
}

int COLMAPPhysicalCamera::GetWidth() const
{
	return Width;
}

int COLMAPPhysicalCamera::GetHeight() const
{
	return Height;
}

double COLMAPPhysicalCamera::GetParameter(int Index) const
{
	if (Index < 0 || Index >= Parameters.size())
		return -DBL_MAX;

	return Parameters[Index];
}

FEEntity* COLMAPPhysicalCamera::GetSceneEntity() const
{
	return MAIN_SCENE_MANAGER.GetMainScene()->GetEntity(SceneEntityID);
}

bool COLMAPPhysicalCamera::EquvialentTo(COLMAPPhysicalCamera* OtherCamera) const
{
	if (OtherCamera == nullptr)
		return false;

	if (Model != OtherCamera->Model ||
		Width != OtherCamera->Width ||
		Height != OtherCamera->Height ||
		Parameters.size() != OtherCamera->Parameters.size())
		return false;

	for (size_t i = 0; i < Parameters.size(); i++)
	{
		if (Parameters[i] != OtherCamera->Parameters[i])
			return false;
	}

	return true;
}

COLMAPCamera::COLMAPCamera() {}
COLMAPCamera::~COLMAPCamera() {}

int COLMAPCamera::GetID() const
{
	return ID;
}

COLMAPPhysicalCamera* COLMAPCamera::GetPhysicalCamera()
{
	return PhysicalCamera;
}