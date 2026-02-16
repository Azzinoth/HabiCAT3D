#pragma once

#include "COLMAPCameras.h"
using namespace FocalEngine;

struct COLMAPImage
{
	friend class COLMAPProject;
	friend class COLMAPDataManager;
private:
	COLMAPImage();
	~COLMAPImage();

	int ID;
	glm::dquat OriginalRotation;
	glm::quat Rotation;
	glm::dvec3 OriginalTranslation;
	glm::vec3 Position;
	int CameraID;
	std::string Name;
public:
	int GetID() const;
	int GetCameraID() const;
	std::string GetName() const;
	glm::dquat GetOriginalRotation() const;
	glm::quat GetRotation() const;
	glm::dvec3 GetOriginalTranslation() const;
	glm::vec3 GetPosition() const;
};