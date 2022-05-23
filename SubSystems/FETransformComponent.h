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

		glm::vec3 getPosition();
		glm::vec3 getRotation();
		glm::quat getQuaternion();
		glm::vec3 getScale();

		void setPosition(glm::vec3 newPosition);
		void setRotation(glm::vec3 newRotation);
		void rotateByQuaternion(glm::quat quaternion);
		void setScale(glm::vec3 newScale);

		void changeScaleUniformlyBy(float delta);
		void changeXScaleBy(float delta);
		void changeYScaleBy(float delta);
		void changeZScaleBy(float delta);

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