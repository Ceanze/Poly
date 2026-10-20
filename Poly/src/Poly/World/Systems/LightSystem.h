#pragma once

#include "Poly/Core/Core.h"
#include "Poly/RenderGraph/ResourceManager.h"

namespace Poly
{
	class World;

	/*
	 * Builds the GPU-side light data for all lights of a world and provides it to the render graph through
	 * the world's RenderResourceTable. The buffer is always provided, a world without lights has a count of zero.
	 * Must run after the TransformSystem, i.e. be added in the PostUpdate phase.
	 *
	 * Usage:
	 *
	 *   world.AddSystem<LightSystem>(World::Phase::PostUpdate);
	 *
	 *   pass.MapGlobal(LightSystem::LIGHTS_RESOURCE_NAME, "lights");
	 */
	class LightSystem
	{
	public:
		static constexpr const char* LIGHTS_RESOURCE_NAME = "scene.lights";

		LightSystem()  = default;
		~LightSystem() = default;
		CLASS_REMOVE_COPY(LightSystem);

		void Update(World& world);
		void OnShutdown(World& world);

	private:
		bool m_NeedsRebuild = true;

		BufferHandle m_LightsBufferHandle;
	};
} // namespace Poly
