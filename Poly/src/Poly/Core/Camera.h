#pragma once

#include "polypch.h"

namespace Poly
{

	class Camera
	{
	public:
		// Creates a camera with default settings, use the setters to override these
		Camera();
		~Camera();

		void update(float dt);

		void setNearPlane(float nearPlane) { this->zNear = nearPlane; updateProjection(); }
		void setFarPlane(float farPlane) { this->zFar = farPlane; updateProjection(); }
		void setFov(float fov) { this->fov = fov; updateProjection(); }
		void setAspect(float aspect) { this->aspect = aspect; updateProjection(); }
		void setMouseSense(float sense) { this->mouseSense = sense; }
		void setMovementSpeed(float speed) { this->movementSpeed = speed; }
		void setSprintSpeed(float speed) { this->sprintSpeed = speed; }

		glm::mat4 getMatrix();

	private:
		void updateView();
		void updateProjection();

		glm::mat4 view, proj;

		glm::vec3 pos, up, right, forward;
		const glm::vec3 globalUp = { 0.f, 1.f, 0.f };
		float yaw, pitch;

		float zNear, zFar;
		float fov;
		float mouseSense;
		float aspect;
		float sprintSpeed, movementSpeed;
	};

}