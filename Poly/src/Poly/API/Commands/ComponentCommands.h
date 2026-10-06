#pragma once

#include "Poly/API/Command.h"

namespace Poly::API
{
	/**
	 * Adds a component to an entity, default constructed and then filled with the given fields
	 */
	class AddComponentCommand : public Command
	{
	public:
		/**
		 * @param fieldsJson - json object of the fields to set, e.g. {"Name": "Light"}. Empty to keep the defaults
		 */
		AddComponentCommand(PolyID id, std::string_view component, std::string_view fieldsJson = "");

		Result<void>     Execute(EngineContext& context) override;
		void             Undo(EngineContext& context) override;
		std::string_view GetName() const override { return "Add Component"; }

	private:
		PolyID      m_ID;
		std::string m_Component;
		std::string m_FieldsJson;
	};

	/**
	 * Removes a component to an entity, only succeeds when there is a component of that type on the entity
	 */
	class RemoveComponentCommand : public Command
	{
	public:
		RemoveComponentCommand(PolyID id, std::string_view component);

		Result<void>     Execute(EngineContext& context) override;
		void             Undo(EngineContext& context) override;
		std::string_view GetName() const override { return "Remove Component"; }

	private:
		PolyID      m_ID;
		std::string m_Component;
		std::string m_RemovedJson;
	};

	/**
	 * Sets a single field of a component. Consecutive sets of the same field merge, see CommandHistory::BeginMerge()
	 */
	class SetFieldCommand : public Command
	{
	public:
		/**
		 * @param valueJson - new value of the field as json, e.g. [1, 2, 3] for a vec3 or "text" for a string
		 */
		SetFieldCommand(PolyID id, std::string_view component, std::string_view field, std::string_view valueJson);

		Result<void>     Execute(EngineContext& context) override;
		void             Undo(EngineContext& context) override;
		std::string_view GetName() const override { return "Set Field"; }
		bool             TryMerge(const Command& next) override;

	private:
		PolyID      m_ID;
		std::string m_Component;
		std::string m_Field;
		std::string m_ValueJson;
		std::string m_OldValueJson;
	};
} // namespace Poly::API
