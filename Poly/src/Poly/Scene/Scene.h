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

	private:
		friend class Entity;

		entt::registry m_Registry;
	};
}