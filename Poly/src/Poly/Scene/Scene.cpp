#include "Scene.h"
#include "Entity.h"
#include "Components.h"

namespace Poly
{
	Scene::Scene(const std::string& name) : m_ResourceGroup("scene"), m_Name(name)
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
		return CreateEntityWithID(PolyID());
	}

	Entity Scene::CreateEntityWithID(PolyID id)
	{
		entt::entity entity = m_Registry.create();

		m_Registry.emplace<TransformComponent>(entity);
		m_Registry.emplace<HierarchyComponent>(entity);
		m_Registry.emplace<IDComponent>(entity, id);

		return Entity(this, entity);
	}

	Entity Scene::GetOrCreateEntityWithID(PolyID id)
	{
		auto view = m_Registry.view<const IDComponent>();

		entt::entity enttEntity = entt::null;
		for (auto [entity, IDComp] : view.each())
		{
			if (IDComp.ID == id)
			{
				enttEntity = entity;
				break;
			}
		}

		if (enttEntity == entt::null)
			return CreateEntityWithID(id);

		return Entity(this, enttEntity);
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	PolyID Scene::GetIdOfEntity(entt::entity entity)
	{
		if (entity == entt::null)
			return PolyID::None();

		if (m_Registry.valid(entity))
		{
			if (m_Registry.any_of<IDComponent>(entity))
				return m_Registry.get<IDComponent>(entity).ID;

			POLY_CORE_WARN("Cannot get ID of entity {}, entity does not have IDComponent", entity);
		}

		POLY_CORE_WARN("Cannot get entity {}, identifer is not valid", entity);
		return PolyID::None();
	}
}