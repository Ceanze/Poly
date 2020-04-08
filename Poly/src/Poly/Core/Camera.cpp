#include "polypch.h"
#include "Camera.h"

#include "Input.h"
#include <GLFW/glfw3.h>

namespace Poly
{

	Camera::Camera() : zNear(0.1f), zFar(100.f), fov(45.f), mouseSense(2.f), yaw(0), pitch(0), view(0), movementSpeed(2.f), sprintSpeed(4.f), aspect(1)
	{
		this->proj = glm::perspective(this->fov, this->aspect, this->zNear, this->zFar);
		this->proj[1][1] *= -1;
		this->pos = { 0.f, 0.f, -1.f };
		this->up = { 0.f, 1.f, 0.f };
		this->forward = { 0.f, 0.f, 1.f };
		this->right = { 1.f, 0.f, 0.f };
	}

	Camera::~Camera()
	{
	}

	void Camera::update(float dt)
	{
		glm::vec2 mouseDelta = Input::getMouseDelta();

		this->yaw += mouseDelta.x * dt * this->mouseSense;
		this->pitch += mouseDelta.y * dt * this->mouseSense;

		float extraSpeed = 0.0f;
		if (Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT))
			extraSpeed = this->sprintSpeed;

		if (Input::isKeyPressed(GLFW_KEY_W))
			this->pos += this->forward * dt * (this->movementSpeed + extraSpeed);
		if (Input::isKeyPressed(GLFW_KEY_S))
			this->pos -= this->forward * dt * (this->movementSpeed + extraSpeed);
		if (Input::isKeyPressed(GLFW_KEY_A))
			this->pos += this->right * dt * (this->movementSpeed + extraSpeed);
		if (Input::isKeyPressed(GLFW_KEY_D))
			this->pos -= this->right * dt * (this->movementSpeed + extraSpeed);
		if (Input::isKeyPressed(GLFW_KEY_SPACE))
			this->pos += this->globalUp * dt * (this->movementSpeed + extraSpeed);
		if (Input::isKeyPressed(GLFW_KEY_LEFT_CONTROL))
			this->pos -= this->globalUp * dt * (this->movementSpeed + extraSpeed);

		updateView();
	}

	glm::mat4 Camera::getMatrix()
	{
		return this->proj * this->view;
	}

	void Camera::updateView()
	{
		glm::quat qYaw = glm::angleAxis(-this->yaw, this->globalUp);
		glm::quat qPitch = glm::angleAxis(this->pitch, this->right);
		glm::quat rotation = qPitch * qYaw;
		this->forward = glm::normalize(glm::rotate(rotation, this->forward));
		this->right = glm::normalize(glm::cross(this->globalUp, this->forward));
		this->up = glm::cross(this->forward, -this->right);
		this->yaw = this->pitch = 0.f;

		this->view = glm::lookAt(this->pos, this->pos + this->forward, this->globalUp);
	}

	void Camera::updateProjection()
	{
		this->proj = glm::perspective(this->fov, this->aspect, this->zNear, this->zFar);
		this->proj[1][1] *= -1;
	}

}