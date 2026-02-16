#include "COLMAPImage.h"
using namespace FocalEngine;

COLMAPImage::COLMAPImage() {}
COLMAPImage::~COLMAPImage() {}

int COLMAPImage::GetID() const
{
	return ID;
}

int COLMAPImage::GetCameraID() const
{
	return CameraID;
}

std::string COLMAPImage::GetName() const
{
	return Name;
}

glm::dquat COLMAPImage::GetOriginalRotation() const
{
	return OriginalRotation;
}

glm::dvec3 COLMAPImage::GetOriginalTranslation() const
{
	return OriginalTranslation;
}

glm::quat COLMAPImage::GetRotation() const
{
	return Rotation;
}

glm::vec3 COLMAPImage::GetPosition() const
{
	return Position;
}