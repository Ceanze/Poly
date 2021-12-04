#pragma once

#include "polypch.h"
#include "Timestamp.h"

namespace Poly
{

	class Camera
	{
	public:
		// Creates a camera with default settings, use the setters to override these
		Camera();
		~Camera();

		void Update(Timestamp dt);

		void SetNearPlane(float nearPlane) { m_ZNear = nearPlane; UpdateProjection(); }
		void SetFarPlane(float farPlane) { m_ZFar = farPlane; UpdateProjection(); }
		void SetFov(float fov) { m_FOV = fov; UpdateProjection(); }
		void SetAspect(float aspect) { m_Aspect = aspect; UpdateProjection(); }
		void SetMouseSense(float sense) { m_MouseSense = sense; }
		void SetMovementSpeed(float speed) { m_MovementSpeed = speed; }
		void SetSprintSpeed(float speed) { m_SprintSpeed = speed; }

		glm::mat4 GetMatrix();
		glm::vec4 GetPosition() { return glm::vec4(m_Pos, 1.0); }

	private:
		void UpdateView();
		void UpdateProjection();

		glm::mat4 m_View, m_Proj;

		glm::vec3 m_Pos, m_Up, m_Right, m_Forward;
		const glm::vec3 m_GlobalUp = { 0.f, 1.f, 0.f };
		float m_Yaw		= 0.f;
		float m_Pitch	= 0.f;

		float m_ZNear			= 0.01f;
		float m_ZFar			= 1000.f;
		float m_FOV				= 45.f;
		float m_MouseSense		= 2.f;
		float m_Aspect			= 1;
		float m_SprintSpeed		= 4.f;
		float m_MovementSpeed	= 2.f;
	};

}