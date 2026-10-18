#pragma once

#include <concepts>
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_flags.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

/*
 * Names of enum values, for anything that shows or stores an enum as text (files, logs, editors). The names are
 * the enumerators themselves, read from the compiler by magic_enum, so any enum has them.
 *
 * - A plain enum value is the name of its enumerator. Enumerators are found in the range [-128, 127]
 * - A flag enum (an enum with ENABLE_BITMASK_OPERATORS) value is its set flags joined by '|', e.g. "VERTEX|FRAGMENT",
 *   and the value 0 the name of the enumerator equal to 0. Enumerators that are not a single flag (e.g.
 *   FQueueType::ALL) have no name
 *
 * An enum with values outside of that needs a magic_enum::customize::enum_range specialization.
 */

namespace Poly
{
	template<typename E>
	concept FlagEnum = std::is_enum_v<E> && requires(E a, E b) {
		{ a | b } -> std::same_as<E>;
	};

	/*
	 * @return The name of the value, empty if it (or one of its flags) has no name.
	 */
	template<typename E>
	    requires std::is_enum_v<E>
	std::string ToString(E value)
	{
		if constexpr (FlagEnum<E>)
		{
			if (static_cast<std::underlying_type_t<E>>(value) != 0)
				return magic_enum::enum_flags_name(value, '|');
		}

		return std::string(magic_enum::enum_name(value));
	}

	/*
	 * @return The value with the name, for flag enums also flags joined by '|'. nullopt if a name is not a value of the enum.
	 */
	template<typename E>
	    requires std::is_enum_v<E>
	std::optional<E> FromString(std::string_view text)
	{
		if constexpr (FlagEnum<E>)
		{
			if (const std::optional<E> flags = magic_enum::enum_flags_cast<E>(text))
				return flags;
		}

		return magic_enum::enum_cast<E>(text);
	}
} // namespace Poly
