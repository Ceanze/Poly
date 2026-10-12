#pragma once

#include "Poly/Rendering/RenderCamera.h"

#include <optional>

namespace Poly
{
	class World;

	/*
	 * Resolves the primary CameraComponent of a world into a RenderCamera, ready to be submitted for rendering.
	 * Must run after the TransformSystem, i.e. be added in the PostUpdate phase.
	 *
	 * Usage:
	 *
	 *   CameraSystem& cameraSystem = world.AddSystem<CameraSystem>(World::Phase::PostUpdate);
	 *
	 *   // Every frame, after world.Update()
	 *   renderer.Submit({.pWorld = &world, .Camera = cameraSystem.GetPrimaryCamera()});
	 */
	class CameraSystem
	{
	public:
		void Update(World& world);

		/**
		 * @return the primary camera of the world as of the last update, empty if the world has none
		 */
		const std::optional<RenderCamera>& GetPrimaryCamera() const { return m_PrimaryCamera; }

	private:
		bool m_NeedsResolve = true;

		std::optional<RenderCamera> m_PrimaryCamera;
	};
} // namespace Poly
