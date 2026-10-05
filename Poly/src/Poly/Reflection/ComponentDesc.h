#pragma once

#include "Poly/Core/Result.h"
#include "Poly/Scene/Entity.h"

#include <functional>
#include <string_view>

namespace Poly
{
	class FieldVisitor;

	struct FieldInfo
	{
		std::string Name;
		std::string Type; // "bool", "int32", "uint32", "float", "vec2", "vec3", "vec4", "quat", "string" or "asset"
	};

	/**
	 * A field changed by a FieldVisitor, the new value as json
	 */
	struct FieldEdit
	{
		std::string Field;
		std::string Json;
	};

	struct ComponentTypeInfo
	{
		std::string            Name;
		bool                   Addable   = true;
		bool                   Removable = true;
		std::vector<FieldInfo> Fields;
	};

	/*
	 * Type-erased description of a component type, created by RegisterComponent<TComponent>() (ComponentRegistration.h).
	 * Lets serialization, the API and editors work with components by name without knowing their type.
	 */
	struct ComponentDesc
	{
		ComponentTypeInfo TypeInfo;

		std::function<bool(Entity)> Has;
		std::function<void(Entity)> Add;
		std::function<void(Entity)> Remove;

		std::function<std::string(Entity)> ToJson;
		std::function<std::string(Entity)> ToYaml;

		// Only the fields present in the json are written, the component is left untouched if the json is invalid
		std::function<Result<void>(Entity, std::string_view json)> FromJson;

		std::function<void(Entity, FieldVisitor&, std::vector<FieldEdit>& edits)> VisitFields;
	};
} // namespace Poly