#pragma once
#include "../FECoreIncludes.h"
using namespace FocalEngine;

namespace FocalEngine
{
	class FETransformComponent
	{
	public:
		FETransformComponent();
		FETransformComponent(glm::mat4 matrix);
		~FETransformComponent();

		FETransformComponent combine(FETransformComponent& other);

		glm::vec3 GetPosition();
		glm::vec3 GetRotation();
		glm::quat getQuaternion();
		glm::vec3 GetScale();

		void SetPosition(glm::vec3 newPosition);
		void SetRotation(glm::vec3 newRotation);
		void rotateByQuaternion(glm::quat quaternion);
		void SetScale(glm::vec3 newScale);

		void changeScaleUniformlyBy(float delta);
		void ChangeXScaleBy(float delta);
		void ChangeYScaleBy(float delta);
		void ChangeZScaleBy(float delta);

		glm::mat4 getTransformMatrix();
		void forceSetTransformMatrix(glm::mat4 newValue);
		void update();

		bool isDirty();
		void setDirty(bool isDirty);

		bool uniformScaling = true;
	private:
		bool dirtyFlag = false;
		glm::vec3 position;
		glm::quat rotationQuaternion;
		glm::vec3 rotationAngles;
		
		glm::vec3 scale;

		glm::mat4 transformMatrix;
		glm::mat4 previousTransformMatrix;

		void rotateQuaternion(float angle, glm::vec3 axis);
	};
}