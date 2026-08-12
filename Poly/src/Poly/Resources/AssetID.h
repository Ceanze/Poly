#pragma once

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
		explicit constexpr AssetID(std::string_view vfsPath);
		explicit constexpr AssetID(uint64 id);
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
