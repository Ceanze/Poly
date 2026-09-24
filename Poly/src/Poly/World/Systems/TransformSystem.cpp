#include "TransformSystem.h"

#include "Poly/Scene/Components.h"
#include "Poly/World/World.h"

#include <utility>
#include <vector>

namespace Poly
{
	void TransformSystem::Update(World& world)
	{
		// TODO: Only resolve the subtrees of dirty entities instead of the whole world
		if (world.View<DirtyTag>().empty())
			return;

		auto hierarchies = world.View<HierarchyComponent>();
		auto transforms  = world.View<TransformComponent, WorldTransformComponent>();

		// Depth first from the roots, carrying the parent's world matrix along, so a parent is always resolved before its children
		std::vector<std::pair<entt::entity, glm::mat4>> stack;
		for (entt::entity entity : transforms)
		{
			if (hierarchies.get<HierarchyComponent>(entity).Parent == entt::null)
				stack.emplace_back(entity, glm::mat4(1.0f));
		}

#ifdef POLY_DEBUG
		size_t resolvedCount = 0;
#endif

		while (!stack.empty())
		{
			const auto [entity, parentMatrix] = stack.back();
			stack.pop_back();

			glm::mat4& matrix = transforms.get<WorldTransformComponent>(entity).Matrix;
			matrix            = parentMatrix * transforms.get<TransformComponent>(entity).GetTransform();

#ifdef POLY_DEBUG
			resolvedCount++;
#endif

			// Children form a circular list starting at First
			const entt::entity first = hierarchies.get<HierarchyComponent>(entity).First;
			for (entt::entity child = first; child != entt::null;)
			{
				stack.emplace_back(child, matrix);

				child = hierarchies.get<HierarchyComponent>(child).Next;
				if (child == first)
					break;
			}
		}

#ifdef POLY_DEBUG
		// A mismatch means the hierarchy links are corrupt, e.g. a subtree that isn't reachable from a root or a child listed twice
		const size_t entityCount = transforms.size_hint();
		if (resolvedCount != entityCount)
			POLY_CORE_WARN("TransformSystem resolved {} world transforms, but world '{}' has {} entities - the hierarchy is inconsistent", resolvedCount, world.GetName(), entityCount);
#endif
	}
} // namespace Poly
