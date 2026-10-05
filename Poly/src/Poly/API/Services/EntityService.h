#pragma once

#include "Poly/API/Types.h"
#include "Poly/Core/Result.h"
#include "Poly/Scene/Entity.h"

namespace Poly::API
{
	class EngineContext;

	/*
	 * Creating, destroying, arranging and querying the entities of the world.
	 * Every change is an undoable command, see CommandHistory.
	 */
	class EntityService
	{
	public:
		explicit EntityService(EngineContext& context);

		/**
		 * @param parent - entity to parent the new entity to, PolyID::None() for a root entity
		 * @param name - name of the entity, empty to create it without a Name component
		 * @param siblingIndex - index among the children of the parent, last by default
		 * @return ID of the created entity
		 */
		Result<PolyID> Create(PolyID parent = PolyID::None(), std::string_view name = "Entity", uint32 siblingIndex = Entity::LAST_SIBLING_INDEX);

		/**
		 * Destroys an entity together with all of its descendants
		 */
		Result<void> Destroy(PolyID id);

		/**
		 * @param parent - new parent, PolyID::None() to make the entity a root entity
		 * @param siblingIndex - index among the children of the new parent, last by default
		 */
		Result<void> SetParent(PolyID id, PolyID parent, uint32 siblingIndex = Entity::LAST_SIBLING_INDEX);

		/**
		 * Sets the name of an entity, adding a Name component if it has none
		 */
		Result<void> Rename(PolyID id, std::string_view name);

		Result<EntityInfo> Get(PolyID id) const;

		bool Exists(PolyID id) const;

		/**
		 * @return IDs of all entities without a parent
		 */
		std::vector<PolyID> GetRoots() const;

	private:
		EngineContext& m_Context;
	};
} // namespace Poly::API
