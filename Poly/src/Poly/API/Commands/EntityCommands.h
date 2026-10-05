#pragma once

#include "Poly/API/Command.h"
#include "Poly/Scene/Entity.h"
#include "Poly/World/Serialization/WorldSerializer.h"

namespace Poly::API
{
	/**
	 * Creates an entity, optionally named and parented
	 */
	class CreateEntityCommand : public Command
	{
	public:
		CreateEntityCommand(PolyID parent, std::string_view name, uint32 siblingIndex = Entity::LAST_SIBLING_INDEX);

		Result<void>     Execute(EngineContext& context) override;
		void             Undo(EngineContext& context) override;
		std::string_view GetName() const override { return "Create Entity"; }

		/**
		 * @return ID of the created entity, valid after the first Execute()
		 */
		PolyID GetID() const { return m_ID; }

	private:
		PolyID      m_ID = PolyID::None();
		PolyID      m_Parent;
		std::string m_Name;
		uint32      m_SiblingIndex;
	};

	/**
	 * Destroys an entity together with all of its descendants
	 */
	class DestroyEntityCommand : public Command
	{
	public:
		explicit DestroyEntityCommand(PolyID id);

		Result<void>     Execute(EngineContext& context) override;
		void             Undo(EngineContext& context) override;
		std::string_view GetName() const override { return "Destroy Entity"; }

	private:
		PolyID m_ID;

		// The destroyed subtree, parents before their children and siblings in order
		std::vector<EntityData> m_Entities;
		uint32                  m_SiblingIndex = 0;
	};

	/**
	 * Moves an entity to a new parent, or to the root with PolyID::None(), at the given index among its new siblings
	 */
	class SetParentCommand : public Command
	{
	public:
		SetParentCommand(PolyID id, PolyID parent, uint32 siblingIndex = Entity::LAST_SIBLING_INDEX);

		Result<void>     Execute(EngineContext& context) override;
		void             Undo(EngineContext& context) override;
		std::string_view GetName() const override { return "Set Parent"; }

	private:
		PolyID m_ID;
		PolyID m_Parent;
		uint32 m_SiblingIndex;

		PolyID m_OldParent       = PolyID::None();
		uint32 m_OldSiblingIndex = 0;
	};
} // namespace Poly::API
