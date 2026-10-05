#pragma once

#include "Poly/Core/Result.h"

#include <string>
#include <string_view>

namespace Poly::Json
{
	/**
	 * @return the text as a quoted and escaped json string
	 */
	std::string Quote(std::string_view text);

	/**
	 * @return the text of a json string, the json itself if it is not a string
	 */
	std::string Unquote(std::string_view json);

	/**
	 * @return json object with a single member - {"key": value}
	 */
	std::string MakeObject(std::string_view key, std::string_view valueJson);

	/**
	 * @return the json of the member with the given key of a json object
	 */
	Result<std::string> GetMember(std::string_view objectJson, std::string_view key);
} // namespace Poly::Json
