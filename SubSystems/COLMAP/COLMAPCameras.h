#pragma once

#include "COLMAPCore.h"
using namespace FocalEngine;

struct COLMAPPhysicalCamera
{
	friend class COLMAPProject;
	friend class COLMAPDataManager;
private:
	COLMAPPhysicalCamera();
	~COLMAPPhysicalCamera();

	std::string ID;
	std::string Model;
	int Width, Height;
	std::vector<double> Parameters;

	std::string SceneEntityID = "";
public:
	std::string GetID() const;
	std::string GetModel() const;
	int GetWidth() const;
	int GetHeight() const;
	double GetParameter(int Index) const;

	FEEntity* GetSceneEntity() const;
	bool EquvialentTo(COLMAPPhysicalCamera* OtherCamera) const;
};

struct COLMAPCamera
{
	friend class COLMAPProject;
	friend class COLMAPDataManager;
private:
	COLMAPCamera();
	~COLMAPCamera();

	int ID;
	COLMAPPhysicalCamera* PhysicalCamera;
public:
	int GetID() const;

	COLMAPPhysicalCamera* GetPhysicalCamera();
};