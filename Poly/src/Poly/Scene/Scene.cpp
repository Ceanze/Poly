#include "Scene.h"
#include "Entity.h"

namespace Poly
{
	Entity Scene::CreateEntity()
	{
		entt::entity entity = m_Registry.create();
		return Entity(this, entity);
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}
}