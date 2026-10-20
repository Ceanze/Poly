#pragma once

#include "Poly/Core/EnumNames.h"

#include <glaze/glaze.hpp>
#include <glaze/yaml.hpp>

// Glaze serialization of enums by the names of their values, see Poly/Core/EnumNames.h.
// - Plain enums are a single name, e.g. "TRIANGLE_LIST"
// - Flag enums are their set flags joined by '|', e.g. "VERTEX|FRAGMENT"
// A value without a name fails to be written.
//
// Heavy include, only use from the source files that serialize

namespace Poly::GlazeDetail
{
	template<uint32_t Format, typename E>
	struct FlagsFrom
	{
		template<auto Opts>
		static void op(E& value, glz::is_context auto&& ctx, auto&& it, auto&& end)
		{
			std::string text;
			glz::parse<Format>::template op<Opts>(text, ctx, it, end);
			if (bool(ctx.error))
				return;

			const std::optional<E> parsed = FromString<E>(text);
			if (!parsed)
			{
				ctx.error                = glz::error_code::constraint_violated;
				ctx.custom_error_message = "unknown flag name";
				return;
			}

			value = *parsed;
		}
	};

	template<uint32_t Format, typename E>
	struct FlagsTo
	{
		template<auto Opts>
		static void op(const E& value, glz::is_context auto&& ctx, auto&& b, auto&& ix)
		{
			const std::string text = ToString(value);
			if (text.empty())
			{
				ctx.error                = glz::error_code::constraint_violated;
				ctx.custom_error_message = "flag value has bits without a name";
				return;
			}

			glz::serialize<Format>::template op<Opts>(text, ctx, b, ix);
		}
	};
} // namespace Poly::GlazeDetail

namespace glz
{
	// Plain enums go through glaze's own named enum support
	template<typename E>
	    requires(std::is_enum_v<E> && !Poly::FlagEnum<E>)
	struct meta<E>
	{
		static constexpr auto keys  = magic_enum::enum_names<E>();
		static constexpr auto value = magic_enum::enum_values<E>();
	};

	// Flag enums opt out of glaze's enum support, they are read and written by the specializations below
	template<Poly::FlagEnum E>
	struct meta<E>
	{
		static constexpr bool custom_read  = true;
		static constexpr bool custom_write = true;
	};

	// clang-format off
	template<Poly::FlagEnum E> struct from<JSON, E> : Poly::GlazeDetail::FlagsFrom<JSON, E> {};
	template<Poly::FlagEnum E> struct to<JSON, E> : Poly::GlazeDetail::FlagsTo<JSON, E> {};
	template<Poly::FlagEnum E> struct from<YAML, E> : Poly::GlazeDetail::FlagsFrom<YAML, E> {};
	template<Poly::FlagEnum E> struct to<YAML, E> : Poly::GlazeDetail::FlagsTo<YAML, E> {};
	// clang-format on
} // namespace glz
