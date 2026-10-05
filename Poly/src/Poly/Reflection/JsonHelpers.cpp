#include "Poly/Reflection/JsonHelpers.h"

#include <glaze/glaze.hpp>

namespace Poly
{
	namespace Json
	{
		std::string Quote(std::string_view text)
		{
			return glz::write_json(text).value_or("\"\"");
		}

		std::string Unquote(std::string_view json)
		{
			std::string       text;
			const std::string buffer(json);
			if (glz::read_json(text, buffer))
				return buffer;

			return text;
		}

		std::string MakeObject(std::string_view key, std::string_view valueJson)
		{
			std::string object = "{";
			object += Quote(key);
			object += ':';
			object += valueJson;
			object += '}';
			return object;
		}

		Result<std::string> GetMember(std::string_view objectJson, std::string_view key)
		{
			glz::generic      object;
			const std::string buffer(objectJson);
			if (const glz::error_ctx error = glz::read_json(object, buffer))
				return MakeError(ErrorCode::InvalidValue, glz::format_error(error, buffer));

			if (!object.is_object() || !object.contains(key))
				return MakeError(ErrorCode::FieldNotFound, std::format("'{}' is not a member of {}", key, objectJson));

			return object[key].dump().value_or("null");
		}
	} // namespace Json
} // namespace Poly