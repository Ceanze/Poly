#pragma once

#include "Poly/API/Types.h"
#include "Poly/Core/Result.h"
#include "Poly/Reflection/ComponentDesc.h"

namespace Poly
{
	class FieldVisitor;
}

namespace Poly::API
{
	class EngineContext;

	/*
	 * Adding, removing, reading and changing the components of entities, by the names they are registered with
	 * in the ComponentRegistry. Values are passed as json. Every change is an undoable command, see CommandHistory.
	 *
	 * Usage:
	 * @code
	 * components.Add(id, "MeshAsset");
	 * components.SetField(id, "Transform", "Scale", "[2, 2, 2]");
	 * Result<std::string> transform = components.Get(id, "Transform"); // {"Translation":[0,0,0],...}
	 * @endcode
	 */
	class ComponentService
	{
	public:
		explicit ComponentService(EngineContext& context);

		/**
		 * @param fieldsJson - json object of the fields to set after adding, empty to keep the defaults
		 */
		Result<void> Add(PolyID id, std::string_view component, std::string_view fieldsJson = "");

		Result<void> Remove(PolyID id, std::string_view component);

		/**
		 * @param valueJson - new value of the field as json, e.g. [1, 2, 3] for a vec3 or "text" for a string
		 */
		Result<void> SetField(PolyID id, std::string_view component, std::string_view field, std::string_view valueJson);

		/**
		 * @return all fields of the component as a json object
		 */
		Result<std::string> Get(PolyID id, std::string_view component) const;

		/**
		 * @return the value of a single field of the component as json
		 */
		Result<std::string> GetField(PolyID id, std::string_view component, std::string_view field) const;

		/**
		 * Visits the fields of a component with their real types, and sets the ones the visitor changes.
		 * Used by editors to draw and edit a component without going through json for every field every frame
		 * @return true if the visitor changed any field
		 */
		Result<bool> Inspect(PolyID id, std::string_view component, FieldVisitor& visitor);

		/**
		 * @return all component types that are registered
		 */
		std::vector<ComponentTypeInfo> GetTypes() const;

		/**
		 * @return names of the components that can be added to the entity, i.e. the addable ones it does not already have
		 */
		Result<std::vector<std::string>> GetAddable(PolyID id) const;

	private:
		EngineContext& m_Context;
	};
} // namespace Poly::API
