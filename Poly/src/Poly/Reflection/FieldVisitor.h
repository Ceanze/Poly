#pragma once

#include <string>
#include <string_view>

namespace Poly
{
	/*
	 * Visits the fields of a component one by one with their real type, see ComponentDesc::VisitFields.
	 * The fields belong to a copy of the component, a Visit() that changes the value returns true to have the
	 * change reported back as a FieldEdit - the component itself is never written to by the visitor.
	 *
	 * Used by editors to draw a widget per field without knowing the component type.
	 */
	class FieldVisitor
	{
	public:
		virtual ~FieldVisitor() = default;

		virtual bool Visit(std::string_view name, bool& value)        = 0;
		virtual bool Visit(std::string_view name, int32& value)       = 0;
		virtual bool Visit(std::string_view name, uint32& value)      = 0;
		virtual bool Visit(std::string_view name, float& value)       = 0;
		virtual bool Visit(std::string_view name, glm::vec2& value)   = 0;
		virtual bool Visit(std::string_view name, glm::vec3& value)   = 0;
		virtual bool Visit(std::string_view name, glm::vec4& value)   = 0;
		virtual bool Visit(std::string_view name, glm::quat& value)   = 0;
		virtual bool Visit(std::string_view name, std::string& value) = 0;

		/**
		 * Visits an asset handle field
		 * @param path - VFS path of the asset the handle refers to, empty if the handle is invalid
		 */
		virtual bool VisitAsset(std::string_view name, std::string& path) = 0;
	};
} // namespace Poly
