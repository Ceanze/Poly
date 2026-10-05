#pragma once

#include "Poly/Core/Handle.h"
#include "Poly/Resources/AssetHandler.h"

#include <glaze/glaze.hpp>
#include <glaze/yaml.hpp>

#include <array>
#include <charconv>

// Glaze serialization of the engine types that cannot be reflected as is.
// - glm vectors and quaternions are arrays, quaternions in [x, y, z, w] order. Flow sequences in yaml
// - PolyID is a string, 64 bit integers do not survive json readers that store numbers as doubles
// - Asset handles are the VFS path of the asset, reading one loads the asset if it is not already
//
// Heavy include, only use from the source files that register components or serialize

namespace Poly::GlazeDetail
{
	// T is a glm float vector, it is a plain array of floats
	template<typename T>
	using FloatArray = std::array<float, sizeof(T) / sizeof(float)>;

	template<typename T>
	FloatArray<T> ToArray(const T& value)
	{
		FloatArray<T> array;
		for (size_t i = 0; i < array.size(); i++)
			array[i] = value[static_cast<glm::length_t>(i)];
		return array;
	}

	inline std::array<float, 4> ToArray(const glm::quat& value)
	{
		return {value.x, value.y, value.z, value.w};
	}

	template<typename T>
	void FromArray(T& value, const FloatArray<T>& array)
	{
		for (size_t i = 0; i < array.size(); i++)
			value[static_cast<glm::length_t>(i)] = array[i];
	}

	inline void FromArray(glm::quat& value, const std::array<float, 4>& array)
	{
		value.x = array[0];
		value.y = array[1];
		value.z = array[2];
		value.w = array[3];
	}

	template<typename T>
	struct FloatArrayFromJson
	{
		template<auto Opts>
		static void op(T& value, glz::is_context auto&& ctx, auto&& it, auto&& end)
		{
			auto array = ToArray(value);
			glz::parse<glz::JSON>::op<Opts>(array, ctx, it, end);
			FromArray(value, array);
		}
	};

	template<typename T>
	struct FloatArrayToJson
	{
		template<auto Opts>
		static void op(const T& value, glz::is_context auto&& ctx, auto&& b, auto&& ix)
		{
			const auto array = ToArray(value);
			glz::serialize<glz::JSON>::op<Opts>(array, ctx, b, ix);
		}
	};

	template<typename T>
	struct FloatArrayToYaml
	{
		template<auto Opts>
		static void op(const T& value, glz::is_context auto&& ctx, auto&& b, auto&& ix)
		{
			const auto array = ToArray(value);
			glz::serialize<glz::YAML>::op<glz::yaml::flow_context_on<Opts>()>(array, ctx, b, ix);
		}
	};
} // namespace Poly::GlazeDetail

namespace glz
{
	// clang-format off
	template<> struct from<JSON, glm::vec2> : Poly::GlazeDetail::FloatArrayFromJson<glm::vec2> {};
	template<> struct from<JSON, glm::vec3> : Poly::GlazeDetail::FloatArrayFromJson<glm::vec3> {};
	template<> struct from<JSON, glm::vec4> : Poly::GlazeDetail::FloatArrayFromJson<glm::vec4> {};
	template<> struct from<JSON, glm::quat> : Poly::GlazeDetail::FloatArrayFromJson<glm::quat> {};

	template<> struct to<JSON, glm::vec2> : Poly::GlazeDetail::FloatArrayToJson<glm::vec2> {};
	template<> struct to<JSON, glm::vec3> : Poly::GlazeDetail::FloatArrayToJson<glm::vec3> {};
	template<> struct to<JSON, glm::vec4> : Poly::GlazeDetail::FloatArrayToJson<glm::vec4> {};
	template<> struct to<JSON, glm::quat> : Poly::GlazeDetail::FloatArrayToJson<glm::quat> {};

	template<> struct to<YAML, glm::vec2> : Poly::GlazeDetail::FloatArrayToYaml<glm::vec2> {};
	template<> struct to<YAML, glm::vec3> : Poly::GlazeDetail::FloatArrayToYaml<glm::vec3> {};
	template<> struct to<YAML, glm::vec4> : Poly::GlazeDetail::FloatArrayToYaml<glm::vec4> {};
	template<> struct to<YAML, glm::quat> : Poly::GlazeDetail::FloatArrayToYaml<glm::quat> {};
	// clang-format on

	template<>
	struct from<JSON, Poly::PolyID>
	{
		template<auto Opts>
		static void op(Poly::PolyID& value, is_context auto&& ctx, auto&& it, auto&& end)
		{
			std::string text;
			parse<JSON>::op<Opts>(text, ctx, it, end);
			if (bool(ctx.error))
				return;

			uint64     id     = 0;
			const auto result = std::from_chars(text.data(), text.data() + text.size(), id);
			if (result.ec != std::errc() || result.ptr != text.data() + text.size())
			{
				ctx.error                = error_code::constraint_violated;
				ctx.custom_error_message = "a PolyID must be a string of digits";
				return;
			}

			value = Poly::PolyID(id);
		}
	};

	template<>
	struct to<JSON, Poly::PolyID>
	{
		template<auto Opts>
		static void op(const Poly::PolyID& value, is_context auto&& ctx, auto&& b, auto&& ix)
		{
			const std::string text = std::to_string(static_cast<uint64>(value));
			serialize<JSON>::op<Opts>(text, ctx, b, ix);
		}
	};

	template<typename AssetType>
	struct from<JSON, Poly::Handle<AssetType>>
	{
		template<auto Opts>
		static void op(Poly::Handle<AssetType>& value, is_context auto&& ctx, auto&& it, auto&& end)
		{
			std::string path;
			parse<JSON>::op<Opts>(path, ctx, it, end);
			if (bool(ctx.error))
				return;

			if (path.empty())
			{
				value = {};
				return;
			}

			const Poly::Handle<AssetType> handle = Poly::AssetHandler::Load<AssetType>(path);
			if (!handle.IsValid())
			{
				ctx.error                = error_code::constraint_violated;
				ctx.custom_error_message = "the asset could not be loaded";
				return;
			}

			value = handle;
		}
	};

	template<typename AssetType>
	struct to<JSON, Poly::Handle<AssetType>>
	{
		template<auto Opts>
		static void op(const Poly::Handle<AssetType>& value, is_context auto&& ctx, auto&& b, auto&& ix)
		{
			const std::string path = Poly::AssetHandler::GetPath(value);
			serialize<JSON>::op<Opts>(path, ctx, b, ix);
		}
	};

	template<typename AssetType>
	struct to<YAML, Poly::Handle<AssetType>>
	{
		template<auto Opts>
		static void op(const Poly::Handle<AssetType>& value, is_context auto&& ctx, auto&& b, auto&& ix)
		{
			const std::string path = Poly::AssetHandler::GetPath(value);
			serialize<YAML>::op<Opts>(path, ctx, b, ix);
		}
	};
} // namespace glz
