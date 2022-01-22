#include "Entity.h"

namespace Poly
{
	void Entity::SetParent(Entity parent, uint8 siblingIndex)
	{
		HierarchyComponent& selfHierarchy = m_pScene->m_Registry.get_or_emplace<HierarchyComponent>(m_Entity);
		if (selfHierarchy.Parent != entt::null)
			RemoveFromParent();

		if (parent.m_Entity != entt::null)
			PlaceInParent(parent, siblingIndex);
	}

	void Entity::SetSiblingIndex(uint8 index)
	{
		HierarchyComponent* selfHierarchy = m_pScene->m_Registry.try_get<HierarchyComponent>(m_Entity);
		if (!selfHierarchy || selfHierarchy->Parent == entt::null)
		{
			POLY_CORE_WARN("Cannot set sibling index of entity {}, no parent has been set", m_Entity);
			return;
		}

		RemoveFromParent();
		PlaceInParent(selfHierarchy->Parent, index);
	}

	void Entity::RemoveFromParent()
	{
		HierarchyComponent& selfHierarchy = m_pScene->m_Registry.get<HierarchyComponent>(m_Entity);
		if (selfHierarchy.Parent == entt::null)
			return;

		m_pScene->m_Registry.patch<HierarchyComponent>(selfHierarchy.Previous, [selfHierarchy](auto& comp) { comp.Next = selfHierarchy.Next; });
		m_pScene->m_Registry.patch<HierarchyComponent>(selfHierarchy.Next, [selfHierarchy](auto& comp) { comp.Previous = selfHierarchy.Previous; });
		HierarchyComponent& parentHierarchy = m_pScene->m_Registry.get<HierarchyComponent>(selfHierarchy.Parent);
		parentHierarchy.ChildrenCount--;
		if (parentHierarchy.First == m_Entity)
			parentHierarchy.First == selfHierarchy.Next;
		selfHierarchy.Parent = entt::null;
	}

	void Entity::PlaceInParent(entt::entity parent, uint8 index)
	{
		HierarchyComponent& selfHierarchy = m_pScene->m_Registry.get_or_emplace<HierarchyComponent>(m_Entity);
		if (selfHierarchy.Parent != entt::null)
			RemoveFromParent();

		HierarchyComponent& parentHierarchy = m_pScene->m_Registry.get_or_emplace<HierarchyComponent>(parent);

		if (index >= parentHierarchy.ChildrenCount)
		{
			PlaceLastInParent(parent);
			return;
		}

		HierarchyComponent& currHierarchy = m_pScene->m_Registry.get<HierarchyComponent>(parentHierarchy.First);
		entt::entity currEntity = parentHierarchy.First;
		for (uint8 i = 0; i < parentHierarchy.ChildrenCount; i++)
		{
			if (i == index)
			{
				m_pScene->m_Registry.patch<HierarchyComponent>(currHierarchy.Previous, [&](auto& comp) { comp.Next = m_Entity; });
				currHierarchy.Previous = m_Entity;
				selfHierarchy.Previous = currHierarchy.Previous;
				selfHierarchy.Next = currEntity;
				selfHierarchy.Parent = currHierarchy.Parent;
				parentHierarchy.ChildrenCount++;

				if (currEntity == parentHierarchy.First)
					parentHierarchy.First = m_Entity;

				break;
			}

			currEntity = currHierarchy.Next;
			currHierarchy = m_pScene->m_Registry.get<HierarchyComponent>(currEntity);
		}
	}

	void Entity::PlaceLastInParent(entt::entity parent)
	{
		HierarchyComponent& selfHierarchy = m_pScene->m_Registry.get<HierarchyComponent>(m_Entity);
		if (selfHierarchy.Parent != entt::null)
			RemoveFromParent();

		HierarchyComponent& parentHierarchy = m_pScene->m_Registry.get_or_emplace<HierarchyComponent>(parent);
		if (parentHierarchy.First == entt::null)
		{
			parentHierarchy.First = m_Entity;
			parentHierarchy.ChildrenCount++;
			selfHierarchy.Next = m_Entity;
			selfHierarchy.Previous = m_Entity;
			return;
		}

		HierarchyComponent& firstHierarchy = m_pScene->m_Registry.get<HierarchyComponent>(parentHierarchy.First);
		HierarchyComponent& lastHierarchy = m_pScene->m_Registry.get<HierarchyComponent>(firstHierarchy.Previous);

		firstHierarchy.Previous = m_Entity;
		lastHierarchy.Next = m_Entity;
		selfHierarchy.Previous = firstHierarchy.Previous;
		selfHierarchy.Next = parentHierarchy.First;
		parentHierarchy.ChildrenCount++;
	}
}