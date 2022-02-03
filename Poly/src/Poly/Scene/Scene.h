#pragma once

#include <entt/entt.hpp>

namespace Poly
{
	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		Entity CreateEntity();
		void DestroyEntity(Entity entity);

		static Ref<Scene> Create();

	private:
		friend class Entity;
		friend class SceneRenderer;

		entt::registry m_Registry;
	};
}