#include "Entity.h"

namespace Poly
{
	void Entity::SetParent(Entity parent, uint8 siblingIndex)
	{
		HierarchyComponent& selfHierarchy = m_Handle.get_or_emplace<HierarchyComponent>();
		if (selfHierarchy.Parent != entt::null)
			RemoveFromParent();

		if (parent.m_Handle.entity() != entt::null)
			PlaceInParent(parent, siblingIndex);
	}

	void Entity::SetSiblingIndex(uint8 index)
	{
		HierarchyComponent* selfHierarchy = m_Handle.try_get<HierarchyComponent>();
		if (!selfHierarchy || selfHierarchy->Parent == entt::null)
		{
			POLY_CORE_WARN("Cannot set sibling index of entity {}, no parent has been set", static_cast<uint32>(m_Handle.entity()));
			return;
		}

		RemoveFromParent();
		PlaceInParent(selfHierarchy->Parent, index);
	}

	void Entity::RemoveFromParent()
	{
		HierarchyComponent& selfHierarchy = m_Handle.get<HierarchyComponent>();
		if (selfHierarchy.Parent == entt::null)
			return;

		m_Handle.registry()->patch<HierarchyComponent>(selfHierarchy.Previous, [selfHierarchy](auto& comp) { comp.Next = selfHierarchy.Next; });
		m_Handle.registry()->patch<HierarchyComponent>(selfHierarchy.Next, [selfHierarchy](auto& comp) { comp.Previous = selfHierarchy.Previous; });
		HierarchyComponent& parentHierarchy = m_Handle.registry()->get<HierarchyComponent>(selfHierarchy.Parent);
		parentHierarchy.ChildrenCount--;
		if (parentHierarchy.First == m_Handle.entity())
			parentHierarchy.First == selfHierarchy.Next;
		selfHierarchy.Parent = entt::null;
	}

	void Entity::PlaceInParent(entt::entity parent, uint8 index)
	{
		HierarchyComponent& selfHierarchy = m_Handle.get_or_emplace<HierarchyComponent>();
		if (selfHierarchy.Parent != entt::null)
			RemoveFromParent();

		HierarchyComponent& parentHierarchy = m_Handle.registry()->get_or_emplace<HierarchyComponent>(parent);

		if (index >= parentHierarchy.ChildrenCount)
		{
			PlaceLastInParent(parent);
			return;
		}

		HierarchyComponent& currHierarchy = m_Handle.registry()->get<HierarchyComponent>(parentHierarchy.First);
		entt::entity        currEntity    = parentHierarchy.First;
		for (uint8 i = 0; i < parentHierarchy.ChildrenCount; i++)
		{
			if (i == index)
			{
				m_Handle.registry()->patch<HierarchyComponent>(currHierarchy.Previous, [&](auto& comp) { comp.Next = m_Handle.entity(); });
				currHierarchy.Previous = m_Handle.entity();
				selfHierarchy.Previous = currHierarchy.Previous;
				selfHierarchy.Next     = currEntity;
				selfHierarchy.Parent   = currHierarchy.Parent;
				parentHierarchy.ChildrenCount++;

				if (currEntity == parentHierarchy.First)
					parentHierarchy.First = m_Handle.entity();

				break;
			}

			currEntity    = currHierarchy.Next;
			currHierarchy = m_Handle.registry()->get<HierarchyComponent>(currEntity);
		}
	}

	void Entity::PlaceLastInParent(entt::entity parent)
	{
		HierarchyComponent& selfHierarchy = m_Handle.get<HierarchyComponent>();
		if (selfHierarchy.Parent != entt::null)
			RemoveFromParent();

		HierarchyComponent& parentHierarchy = m_Handle.registry()->get_or_emplace<HierarchyComponent>(parent);
		if (parentHierarchy.First == entt::null)
		{
			parentHierarchy.First = m_Handle.entity();
			parentHierarchy.ChildrenCount++;
			selfHierarchy.Next     = m_Handle.entity();
			selfHierarchy.Previous = m_Handle.entity();
			selfHierarchy.Parent   = parent;
			return;
		}

		HierarchyComponent& firstHierarchy = m_Handle.registry()->get<HierarchyComponent>(parentHierarchy.First);
		HierarchyComponent& lastHierarchy  = m_Handle.registry()->get<HierarchyComponent>(firstHierarchy.Previous);

		firstHierarchy.Previous = m_Handle.entity();
		lastHierarchy.Next      = m_Handle.entity();
		selfHierarchy.Previous  = firstHierarchy.Previous;
		selfHierarchy.Next      = parentHierarchy.First;
		selfHierarchy.Parent    = parent;
		parentHierarchy.ChildrenCount++;
	}
} // namespace Poly