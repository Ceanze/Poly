#include "Entity.h"

namespace Poly
{
	void Entity::SetParent(Entity parent, uint32 siblingIndex)
	{
		entt::registry&    registry  = *m_Handle.registry();
		const entt::entity self      = m_Handle.entity();
		const entt::entity newParent = parent.m_Handle.entity();

		if (!m_Handle.all_of<HierarchyComponent>())
			m_Handle.emplace<HierarchyComponent>();

		// Parenting to self or to a descendant would create a cycle in the hierarchy
		for (entt::entity ancestor = newParent; ancestor != entt::null; ancestor = registry.get_or_emplace<HierarchyComponent>(ancestor).Parent)
		{
			if (ancestor == self)
			{
				POLY_CORE_WARN("Cannot parent entity {} to {}, it would create a cycle in the hierarchy", static_cast<uint32>(self), static_cast<uint32>(newParent));
				return;
			}
		}

		RemoveFromParent();

		if (newParent != entt::null)
			PlaceInParent(newParent, siblingIndex);
	}

	void Entity::SetSiblingIndex(uint32 index)
	{
		HierarchyComponent* pSelfHierarchy = m_Handle.try_get<HierarchyComponent>();
		if (!pSelfHierarchy || pSelfHierarchy->Parent == entt::null)
		{
			POLY_CORE_WARN("Cannot set sibling index of entity {}, no parent has been set", static_cast<uint32>(m_Handle.entity()));
			return;
		}

		const entt::entity parent = pSelfHierarchy->Parent;
		RemoveFromParent();
		PlaceInParent(parent, index);
	}

	void Entity::RemoveFromParent()
	{
		entt::registry&     registry      = *m_Handle.registry();
		const entt::entity  self          = m_Handle.entity();
		HierarchyComponent& selfHierarchy = registry.get<HierarchyComponent>(self);
		if (selfHierarchy.Parent == entt::null)
			return;

		HierarchyComponent& parentHierarchy = registry.get<HierarchyComponent>(selfHierarchy.Parent);
		if (selfHierarchy.Next == self)
		{
			// Only child
			parentHierarchy.First = entt::null;
		}
		else
		{
			registry.get<HierarchyComponent>(selfHierarchy.Previous).Next = selfHierarchy.Next;
			registry.get<HierarchyComponent>(selfHierarchy.Next).Previous = selfHierarchy.Previous;

			if (parentHierarchy.First == self)
				parentHierarchy.First = selfHierarchy.Next;
		}

		parentHierarchy.ChildrenCount--;

		selfHierarchy.Parent   = entt::null;
		selfHierarchy.Next     = entt::null;
		selfHierarchy.Previous = entt::null;
	}

	void Entity::PlaceInParent(entt::entity parent, uint32 index)
	{
		entt::registry&     registry        = *m_Handle.registry();
		const entt::entity  self            = m_Handle.entity();
		HierarchyComponent& parentHierarchy = registry.get_or_emplace<HierarchyComponent>(parent);
		HierarchyComponent& selfHierarchy   = registry.get<HierarchyComponent>(self);

		if (parentHierarchy.First == entt::null)
		{
			parentHierarchy.First  = self;
			selfHierarchy.Next     = self;
			selfHierarchy.Previous = self;
		}
		else
		{
			// Insert before the child currently at index - inserting before First is the same as appending last,
			// since the list is circular
			const bool   append = index >= parentHierarchy.ChildrenCount;
			entt::entity next   = parentHierarchy.First;
			if (!append)
			{
				for (uint32 i = 0; i < index; i++)
					next = registry.get<HierarchyComponent>(next).Next;
			}

			HierarchyComponent& nextHierarchy = registry.get<HierarchyComponent>(next);
			const entt::entity  previous      = nextHierarchy.Previous;

			selfHierarchy.Next                              = next;
			selfHierarchy.Previous                          = previous;
			registry.get<HierarchyComponent>(previous).Next = self;
			nextHierarchy.Previous                          = self;

			if (!append && index == 0)
				parentHierarchy.First = self;
		}

		selfHierarchy.Parent = parent;
		parentHierarchy.ChildrenCount++;
	}
} // namespace Poly
