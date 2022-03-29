#include "Scene.h"
#include "Entity.h"
#include "Components.h"

namespace Poly
{
	Scene::Scene() : m_ResourceGroup("scene")
	{
		m_ResourceGroup.AddResource(VERTICES_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(INSTANCE_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(MATERIAL_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(ALBEDO_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(NORMAL_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(COMBINED_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(METALLIC_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(ROUGHNESS_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(AO_TEX_RESOURCE_NAME, false);
	}

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