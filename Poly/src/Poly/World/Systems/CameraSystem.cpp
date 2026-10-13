#include "CameraSystem.h"

#include "Poly/World/Components.h"
#include "Poly/World/Components/CameraComponent.h"
#include "Poly/World/World.h"

namespace Poly
{
	void CameraSystem::Update(World& world)
	{
		if (!m_NeedsResolve && !world.HasPendingChanges())
			return;

		m_NeedsResolve = false;
		m_PrimaryCamera.reset();

		for (auto [entity, camera, transform] : world.View<CameraComponent, WorldTransformComponent>().each())
		{
			if (!camera.Primary)
				continue;

			// The view only follows the position and orientation of the entity, not its scale
			glm::mat4 cameraToWorld = transform.Matrix;
			for (glm::length_t i = 0; i < 3; i++)
				cameraToWorld[i] = glm::vec4(glm::normalize(glm::vec3(cameraToWorld[i])), 0.0f);

			m_PrimaryCamera = RenderCamera{.View        = glm::inverse(cameraToWorld),
			                               .Position    = glm::vec3(cameraToWorld[3]),
			                               .FieldOfView = glm::radians(camera.FieldOfView),
			                               .NearPlane   = camera.NearPlane,
			                               .FarPlane    = camera.FarPlane};
			return;
		}
	}
} // namespace Poly
