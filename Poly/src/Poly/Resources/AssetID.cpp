#include "AssetID.h"

namespace Poly
{
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
