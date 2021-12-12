#include "polypch.h"
#include "Camera.h"

#include "input/Input.h"
#include <GLFW/glfw3.h>

namespace Poly
{

	Camera::Camera()
	{
		m_Proj = glm::perspective(m_FOV, m_Aspect, m_ZNear, m_ZFar);
		m_Proj[1][1] *= -1;
		m_Pos = { 0.f, 0.f, -1.f };
		m_Up = { 0.f, 1.f, 0.f };
		m_Forward = { 0.f, 0.f, 1.f };
		m_Right = { 1.f, 0.f, 0.f };

		UpdateView();
	}

	Camera::~Camera()
	{
	}

	void Camera::Update(Timestamp dt)
	{
		glm::vec2 mouseDelta = Input::GetMouseDelta();

		float dtSeconds = float(dt.Seconds());

		m_Yaw += mouseDelta.x * dtSeconds * m_MouseSense;
		m_Pitch += mouseDelta.y * dtSeconds * m_MouseSense;

		float extraSpeed = 0.0f;
		if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
			extraSpeed = m_SprintSpeed;

		if (Input::IsKeyPressed(GLFW_KEY_W))
			m_Pos += m_Forward * dtSeconds * (m_MovementSpeed + extraSpeed);
		if (Input::IsKeyPressed(GLFW_KEY_S))
			m_Pos -= m_Forward * dtSeconds * (m_MovementSpeed + extraSpeed);
		if (Input::IsKeyPressed(GLFW_KEY_A))
			m_Pos += m_Right * dtSeconds * (m_MovementSpeed + extraSpeed);
		if (Input::IsKeyPressed(GLFW_KEY_D))
			m_Pos -= m_Right * dtSeconds * (m_MovementSpeed + extraSpeed);
		if (Input::IsKeyPressed(GLFW_KEY_SPACE))
			m_Pos += m_GlobalUp * dtSeconds * (m_MovementSpeed + extraSpeed);
		if (Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
			m_Pos -= m_GlobalUp * dtSeconds * (m_MovementSpeed + extraSpeed);

		UpdateView();
	}

	glm::mat4 Camera::GetMatrix()
	{
		return m_Proj * m_View;
	}

	void Camera::UpdateView()
	{
		glm::quat qYaw = glm::angleAxis(-m_Yaw, m_GlobalUp);
		glm::quat qPitch = glm::angleAxis(m_Pitch, m_Right);
		glm::quat rotation = qPitch * qYaw;
		glm::vec3 newForward = glm::normalize(glm::rotate(rotation, m_Forward));
		if (abs(newForward.y) > 0.999999)
			newForward = m_Forward;
		m_Forward = newForward; //glm::normalize(glm::rotate(rotation, m_Forward));
		m_Right = glm::normalize(glm::cross(m_GlobalUp, m_Forward));
		m_Up = glm::cross(m_Forward, -m_Right);
		m_Yaw = m_Pitch = 0.f;

		m_View = glm::lookAt(m_Pos, m_Pos + m_Forward, m_GlobalUp);
	}

	void Camera::UpdateProjection()
	{
		m_Proj = glm::perspective(m_FOV, m_Aspect, m_ZNear, m_ZFar);
		m_Proj[1][1] *= -1;
	}

}