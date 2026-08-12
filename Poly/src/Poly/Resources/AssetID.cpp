#include "AssetID.h"

#include <algorithm>
#include <cctype>

namespace
{
	constexpr std::string NormalizeForHashing(std::string_view vfsPath)
	{
		std::string normalized(vfsPath);

		std::replace(normalized.begin(), normalized.end(), '\\', '/');

		std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		while (normalized.size() > 1 && normalized.back() == '/')
			normalized.pop_back();

		return normalized;
	}

	constexpr uint64 FNV_OFFSET_BASIS = 14695981039346656037ull;
	constexpr uint64 FNV_PRIME        = 1099511628211ull;

	constexpr uint64 FNV1a64(std::string_view data)
	{
		uint64 hash = FNV_OFFSET_BASIS;
		for (unsigned char c : data)
		{
			hash ^= c;
			hash *= FNV_PRIME;
		}
		return hash;
	}
} // namespace

namespace Poly
{
	AssetID::AssetID(std::string_view vfsPath)
	    : m_Id(FNV1a64(NormalizeForHashing(vfsPath)))
	{}

	AssetID::AssetID(uint64 id)
	    : m_Id(id)
	{}

	AssetID::operator const uint64() const
	{
		return m_Id;
	}

	bool AssetID::operator<(const AssetID& other) const noexcept
	{
		return m_Id < other.m_Id;
	}

	bool AssetID::operator==(const AssetID& other) const noexcept
	{
		return m_Id == other.m_Id;
	}

	bool AssetID::operator!=(const AssetID& other) const noexcept
	{
		return !(*this == other);
	}

} // namespace Poly
