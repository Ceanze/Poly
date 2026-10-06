#pragma once

#include "Poly/Reflection/ComponentRegistry.h"
#include "Poly/Reflection/FieldVisitor.h"
#include "Poly/Reflection/GlazeAdapters.h"

// Heavy include (glaze), only use from the source files that register components

namespace Poly
{
	struct ComponentOptions
	{
		bool Addable   = true;
		bool Removable = true;
	};

	namespace ReflectionDetail
	{
		// clang-format off
		inline const char* TypeName(const bool&)        { return "bool"; }
		inline const char* TypeName(const int32&)       { return "int32"; }
		inline const char* TypeName(const uint32&)      { return "uint32"; }
		inline const char* TypeName(const float&)       { return "float"; }
		inline const char* TypeName(const glm::vec2&)   { return "vec2"; }
		inline const char* TypeName(const glm::vec3&)   { return "vec3"; }
		inline const char* TypeName(const glm::vec4&)   { return "vec4"; }
		inline const char* TypeName(const glm::quat&)   { return "quat"; }
		inline const char* TypeName(const std::string&) { return "string"; }
		template<typename AssetType>
		const char* TypeName(const Handle<AssetType>&)  { return "asset"; }
		// clang-format on

		/**
		 * @return the new value as json if the visitor changed the field
		 */
		template<typename TField>
		std::optional<std::string> VisitField(FieldVisitor& visitor, std::string_view name, TField& field)
		{
			if (!visitor.Visit(name, field))
				return std::nullopt;

			return glz::write_json(field).value_or("null");
		}

		template<typename AssetType>
		std::optional<std::string> VisitField(FieldVisitor& visitor, std::string_view name, Handle<AssetType>& field)
		{
			std::string path = AssetHandler::GetPath(field);
			if (!visitor.VisitAsset(name, path))
				return std::nullopt;

			return glz::write_json(path).value_or("null");
		}
	} // namespace ReflectionDetail

	/**
	 * Registers a component in the ComponentRegistry, making it serializable and accessible by name.
	 *
	 * The fields are the data members of the component, found by glaze: either all members of an aggregate or the
	 * ones listed in a glz::meta specialization. Every field must be of a type supported by FieldVisitor.
	 *
	 * Example:
	 * @code
	 * struct HealthComponent
	 * {
	 *     float Value = 100.0f;
	 *     bool  Invincible = false;
	 * };
	 *
	 * RegisterComponent<HealthComponent>("Health");
	 * @endcode
	 *
	 * @tparam TComponent - component type, must be default constructible
	 * @param name - name of the component in files and over the API, must be unique and stable across versions
	 * @param options - what is allowed to be done with the component from the outside
	 */
	template<typename TComponent>
	void RegisterComponent(std::string_view name, ComponentOptions options = {})
	{
		ComponentDesc desc;
		desc.TypeInfo.Name      = name;
		desc.TypeInfo.Addable   = options.Addable;
		desc.TypeInfo.Removable = options.Removable;

		{
			TComponent component{};
			size_t     index = 0;
			glz::for_each_field(component, [&](auto&& field) {
				desc.TypeInfo.Fields.push_back({std::string(glz::reflect<TComponent>::keys[index++]), ReflectionDetail::TypeName(field)});
			});
		}

		desc.Has = [](Entity entity) {
			return entity.HasComponent<TComponent>();
		};

		desc.Add = [](Entity entity) {
			entity.AddComponent<TComponent>();
			entity.MarkDirty();
		};

		desc.Remove = [](Entity entity) {
			entity.RemoveComponent<TComponent>();
			entity.MarkDirty();
		};

		desc.ToJson = [](Entity entity) {
			return glz::write_json(entity.GetComponent<TComponent>()).value_or("{}");
		};

		desc.ToYaml = [](Entity entity) {
			return glz::write_yaml(entity.GetComponent<TComponent>()).value_or("{}");
		};

		desc.FromJson = [](Entity entity, std::string_view json) -> Result<void> {
			// Read into a copy, so invalid json leaves the component untouched
			TComponent        component = entity.GetComponent<TComponent>();
			const std::string buffer(json);
			if (const glz::error_ctx error = glz::read_json(component, buffer))
				return MakeError(ErrorCode::InvalidValue, glz::format_error(error, buffer));

			entity.GetComponent<TComponent>() = std::move(component);
			entity.MarkDirty();
			return {};
		};

		desc.VisitFields = [](Entity entity, FieldVisitor& visitor, std::vector<FieldEdit>& edits) {
			TComponent component = entity.GetComponent<TComponent>();
			size_t     index     = 0;
			glz::for_each_field(component, [&](auto&& field) {
				const std::string_view name = glz::reflect<TComponent>::keys[index++];
				if (std::optional<std::string> json = ReflectionDetail::VisitField(visitor, name, field))
					edits.push_back({std::string(name), std::move(*json)});
			});
		};

		ComponentRegistry::Add(std::move(desc));
	}
} // namespace Poly
