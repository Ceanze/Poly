#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace Poly::Internal
{
	// Helpers backing AssetID's constexpr constructor - kept visible in the header since a
	// constexpr function's definition (and everything it calls) must be visible in every TU
	// that uses it; splitting them into AssetID.cpp caused unresolved externals elsewhere.
	constexpr std::string AssetID_NormalizeForHashing(std::string_view vfsPath)
	{
		std::string normalized(vfsPath);

		std::replace(normalized.begin(), normalized.end(), '\\', '/');

		std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		while (normalized.size() > 1 && normalized.back() == '/')
			normalized.pop_back();

		return normalized;
	}

	constexpr uint64 ASSET_ID_FNV_OFFSET_BASIS = 14695981039346656037ull;
	constexpr uint64 ASSET_ID_FNV_PRIME        = 1099511628211ull;

	constexpr uint64 AssetID_FNV1a64(std::string_view data)
	{
		uint64 hash = ASSET_ID_FNV_OFFSET_BASIS;
		for (unsigned char c : data)
		{
			hash ^= c;
			hash *= ASSET_ID_FNV_PRIME;
		}
		return hash;
	}
} // namespace Poly::Internal

namespace Poly
{
	class AssetID
	{
	public:
		/**
		 * Deterministically derives an AssetID from a virtual (VFS) path.
		 * Given the same VFS path, this always produces the same id for the given VFS
		 * @param vfsPath - virtual path of the asset, e.g. "assets/textures/foo.png"
		 */
		explicit constexpr AssetID(std::string_view vfsPath)
		    : m_Id(Internal::AssetID_FNV1a64(Internal::AssetID_NormalizeForHashing(vfsPath)))
		{}

		explicit constexpr AssetID(uint64 id)
		    : m_Id(id)
		{}

		AssetID()                     = default;
		AssetID(const AssetID& other) = default;

		operator const uint64() const;

		bool operator<(const AssetID& other) const noexcept;
		bool operator==(const AssetID& other) const noexcept;
		bool operator!=(const AssetID& other) const noexcept;

		uint64 m_Id = 0;
	};
} // namespace Poly

namespace std
{
	template<>
	struct hash<Poly::AssetID>
	{
		std::size_t operator()(const Poly::AssetID& id) const
		{
			return hash<uint64_t>()((uint64_t)id);
		}
	};
} // namespace std

namespace fmt
{
	template<>
	struct formatter<Poly::AssetID> : formatter<std::string>
	{
		auto format(Poly::AssetID my, format_context& ctx) const -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "[AssetID={}]", static_cast<uint64_t>(my));
		}
	};
} // namespace fmt
