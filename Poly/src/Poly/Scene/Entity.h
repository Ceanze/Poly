#pragma once

#include "Components.h"

namespace Poly
{
	class Scene;

	class Entity
	{
	public:
		static const uint8 LAST_SIBLING_INDEX = UINT8_MAX;

	public:
		~Entity() = default;

		static Entity None() { return Entity({}); }

		void SetParent(Entity parent, uint8 siblingIndex = LAST_SIBLING_INDEX);

		void SetSiblingIndex(uint8 index);

		Entity GetParent()
		{
			HierarchyComponent& hierarchyComp = m_Handle.get_or_emplace<HierarchyComponent>();
			return Entity({*m_Handle.registry(), hierarchyComp.Parent});
		}

		bool HasParent()
		{
			return GetParent().m_Handle.entity() != entt::null;
		}

		// TODO: Consider removal
		Scene* GetScene() const { return m_Handle.registry()->ctx().get<Scene*>(); }

		template<typename Component>
		bool HasComponent() const
		{
			return m_Handle.any_of<Component>();
		}

		template<typename Component, typename... Args>
		Component& AddComponent(Args&&... args)
		{
			POLY_VALIDATE(!HasComponent<Component>(), "Cannot add component, entity {} already has it", static_cast<uint32_t>(m_Handle.entity()));
			return m_Handle.emplace<Component>(std::forward<Args>(args)...);
		}

		template<typename Component>
		void RemoveComponent()
		{
			POLY_VALIDATE(HasComponent<Component>(), "Cannot remove component, entity {} does not have it", static_cast<uint32_t>(m_Handle.entity()));
			m_Handle.remove<Component>();
		}

		template<typename Component>
		Component& GetComponent() const
		{
			POLY_VALIDATE(HasComponent<Component>(), "Cannot get component, entity {} does not have it", static_cast<uint32_t>(m_Handle.entity()));
			return m_Handle.get<Component>();
		}

		PolyID GetPolyID() const { return GetComponent<IDComponent>().ID; }

		operator entt::entity() const { return m_Handle.entity(); }
		operator uint32() const { return entt::to_integral(m_Handle.entity()); }

		bool operator==(const Entity& other) const { return other.m_Handle == m_Handle; }

	private:
		friend class Scene;
		friend class World;
		friend class SceneSerializer;

		Entity(entt::handle handle)
		    : m_Handle(handle)
		{}

		void RemoveFromParent();
		void PlaceInParent(entt::entity parent, uint8 index);
		void PlaceLastInParent(entt::entity parent);

		entt::handle m_Handle;
	};
} // namespace Poly