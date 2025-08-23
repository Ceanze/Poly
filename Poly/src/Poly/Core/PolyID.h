#pragma once

namespace Poly
{
	class PolyID
	{
	public:
		PolyID();
		PolyID(uint64 id) : m_Id(id) {}
		PolyID(const PolyID& other) : m_Id(other.m_Id) {}

		static PolyID None() { return PolyID(0); }

		operator const uint64() const { return m_Id; }

		bool operator<(const PolyID& other) const noexcept
		{
			return m_Id < other.m_Id;
		}

		bool operator==(const PolyID& other) const noexcept
		{
			return m_Id == other.m_Id;
		}

		bool operator!=(const PolyID& other) const noexcept
		{
			return !(*this == other);
		}

	private:
		uint64 m_Id;
	};
}

namespace std
{
	template <>
	struct hash<Poly::PolyID>
	{
		std::size_t operator()(const Poly::PolyID& id) const
		{
			return hash<uint64_t>()((uint64_t)id);
		}
	};
}