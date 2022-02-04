#include "Scene.h"
#include "Entity.h"
#include "Components.h"

namespace Poly
{
	Entity Scene::CreateEntity()
	{
		entt::entity entity = m_Registry.create();

		m_Registry.emplace<TransformComponent>(entity);
		m_Registry.emplace<HierarchyComponent>(entity);
		// m_Registry.emplace<IDComponent>(entity);

		return Entity(this, entity);
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}
}