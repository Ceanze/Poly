#pragma once

#include "Scene.h"
#include "Components.h"

namespace Poly
{
	class Entity
	{
	public:
		static const uint8 LAST_SIBLING_INDEX = UINT8_MAX;

	public:
		~Entity() = default;

		void SetParent(Entity parent, uint8 siblingIndex = LAST_SIBLING_INDEX);

		void SetSiblingIndex(uint8 index);

		Entity GetParent()
		{
			HierarchyComponent& hierarchyComp = m_pScene->m_Registry.get_or_emplace<HierarchyComponent>(m_Entity);
			return Entity(m_pScene, hierarchyComp.Parent);
		}

		bool HasParent()
		{
			return GetParent().m_Entity != entt::null;
		}

		template <typename Component>
		bool HasComponent()
		{
			m_pScene->m_Registry.any_of<Component>(m_Entity);
		}

		template <typename Component, typename... Args>
		Component& AddComponent(Args&&... args)
		{
			POLY_VALIDATE(HasComponent<Component>(), "Cannot add component, entity {} already has it", m_Entity);
			m_pScene->m_Registry.emplace<Component>(m_Entity, std::forward<Args>(args)...);
		}

		template <typename Component>
		void RemoveComponent()
		{
			POLY_VALIDATE(HasComponent<Component>(), "Cannot remove component, entity {} does not have it", m_Entity);
			m_pScene->m_Registry.remove<Component>(m_Entity);
		}

		template <typename Component>
		Component& GetComponent()
		{
			POLY_VALIDATE(HasComponent<Component>(), "Cannot get component, entity {} does not have it", m_Entity);
			return m_pScene->m_Registry.get<Component>(m_Entity);
		}

		operator entt::entity() const { return m_Entity; }
		operator uint32() const { return entt::to_integral(m_Entity); }

	private:
		friend class Scene;

		Entity(Scene* pScene, entt::entity entity) : m_pScene(pScene), m_Entity(entity) {}

		void RemoveFromParent();
		void PlaceInParent(entt::entity parent, uint8 index);
		void PlaceLastInParent(entt::entity parent);

		Scene* m_pScene			= nullptr;
		entt::entity m_Entity	= entt::null;
	};
}