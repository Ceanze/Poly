#pragma once

#include <entt/entt.hpp>
#include "Poly/Rendering/RenderGraph/ResourceGroup.h"

namespace Poly
{
	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		static Ref<Scene> Create() { return CreateRef<Scene>(); }

		/**
		 * Creates and adds an entity with transform component to the scene
		 * @return new entity
		 */
		Entity CreateEntity();

		/**
		 * Destroys a previously created entity
		 * @param entity - entity to destroy
		 */
		void DestroyEntity(Entity entity);


	private:
		friend class Entity;
		friend class SceneRenderer;

		entt::registry m_Registry;
		ResourceGroup m_ResourceGroup;
	};
}