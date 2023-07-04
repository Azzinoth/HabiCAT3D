#include "FETransformComponent.h"
using namespace FocalEngine;

FETransformComponent::FETransformComponent()
{
	position = glm::vec3(0.0f);
	rotationAngles = glm::vec3(0.0f);
	rotationQuaternion = glm::quat(1.0f, glm::vec3(0.0f));
	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	update();
}

FETransformComponent::FETransformComponent(glm::mat4 matrix)
{
	position = glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
	scale = glm::vec3(glm::length(matrix[0]), glm::length(matrix[1]), glm::length(matrix[2]));

	matrix[3][0] = 0.0f;
	matrix[3][1] = 0.0f;
	matrix[3][2] = 0.0f;

	matrix[0] /= scale.x;
	matrix[1] /= scale.y;
	matrix[2] /= scale.z;

	rotationQuaternion = glm::quat_cast(matrix);
	rotationAngles = glm::eulerAngles(rotationQuaternion) * 180.0f / glm::pi<float>();

	update();
}

FETransformComponent::~FETransformComponent()
{
}

glm::vec3 FETransformComponent::GetPosition()
{
	return position;
}

glm::vec3 FETransformComponent::GetRotation()
{
	return rotationAngles;
}

glm::vec3 FETransformComponent::GetScale()
{
	return scale;
}

void FETransformComponent::SetPosition(glm::vec3 newPosition)
{
	position = newPosition;
	update();
}

void FETransformComponent::rotateQuaternion(float angle, glm::vec3 axis)
{
	rotationQuaternion = glm::quat(cos(angle / 2),
								   axis.x * sin(angle / 2),
								   axis.y * sin(angle / 2),
								   axis.z * sin(angle / 2)) * rotationQuaternion;
}

void FETransformComponent::SetRotation(glm::vec3 newRotation)
{
	if (rotationAngles == newRotation)
		return;

	rotationQuaternion = glm::quat(1.0f, glm::vec3(0.0f));
	rotateQuaternion((float)(newRotation.x) * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
	rotateQuaternion((float)(newRotation.y) * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
	rotateQuaternion((float)(newRotation.z) * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));
	
	rotationAngles = newRotation;
	update();
}

void FETransformComponent::rotateByQuaternion(glm::quat quaternion)
{
	rotationQuaternion = quaternion * rotationQuaternion;
	glm::vec3 newRotationAngle = glm::eulerAngles(getQuaternion());

	newRotationAngle.x /= ANGLE_TORADIANS_COF;
	newRotationAngle.y /= ANGLE_TORADIANS_COF;
	newRotationAngle.z /= ANGLE_TORADIANS_COF;

	rotationAngles = newRotationAngle;
	update();
}

void FETransformComponent::changeScaleUniformlyBy(float delta)
{
	scale += delta;
	update();
}

void FETransformComponent::ChangeXScaleBy(float delta)
{
	if (uniformScaling)
	{
		changeScaleUniformlyBy(delta);
	}
	else
	{
		scale[0] += delta;
	}

	update();
}

void FETransformComponent::ChangeYScaleBy(float delta)
{
	if (uniformScaling)
	{
		changeScaleUniformlyBy(delta);
	}
	else
	{
		scale[1] += delta;
	}

	update();
}

void FETransformComponent::ChangeZScaleBy(float delta)
{
	if (uniformScaling)
	{
		changeScaleUniformlyBy(delta);
	}
	else
	{
		scale[2] += delta;
	}

	update();
}

void FETransformComponent::SetScale(glm::vec3 newScale)
{
	scale = newScale;
	update();
}

void FETransformComponent::update()
{
	transformMatrix = glm::mat4(1.0);
	transformMatrix = glm::translate(transformMatrix, position);

	transformMatrix *= glm::toMat4(rotationQuaternion);
	transformMatrix = glm::scale(transformMatrix, glm::vec3(scale[0], scale[1], scale[2]));

	if (previousTransformMatrix != transformMatrix)
		dirtyFlag = true;
	previousTransformMatrix = transformMatrix;
}

glm::mat4 FETransformComponent::getTransformMatrix()
{
	return transformMatrix;
}

glm::quat FETransformComponent::getQuaternion()
{
	return rotationQuaternion;
}

bool FETransformComponent::isDirty()
{
	return dirtyFlag;
}

void FETransformComponent::setDirty(bool isDirty)
{
	dirtyFlag = isDirty;
}

void FETransformComponent::forceSetTransformMatrix(glm::mat4 newValue)
{
	transformMatrix = newValue;
}

FETransformComponent FETransformComponent::combine(FETransformComponent& other)
{
	FETransformComponent result;

	result.SetPosition(GetPosition() + other.GetPosition());
	result.SetRotation(GetRotation() + other.GetRotation());
	result.SetScale(GetScale() * other.GetScale());

	return result;
}