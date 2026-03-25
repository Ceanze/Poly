#include "PassID.h"

namespace Poly
{
	PassID::PassID()
	    : m_Name("")
	{}

	PassID::PassID(const std::string& passName)
	    : m_Name(passName.empty() ? "$" : passName)
	{}

	PassID::PassID(const PassID& other)
	    : m_Name(other.m_Name)
	{}

	PassID::PassID(PassID&& other)
	    : m_Name(std::move(other.m_Name))
	{}

	PassID& PassID::operator=(const PassID& other)
	{
		m_Name = other.m_Name;
		return *this;
	}

	PassID& PassID::operator=(PassID&& other)
	{
		m_Name = std::move(other.m_Name);
		return *this;
	}

	const std::string& PassID::GetName() const
	{
		return m_Name;
	}

	bool PassID::IsValid() const
	{
		return !m_Name.empty();
	}

	bool PassID::operator==(const PassID& other) const noexcept
	{
		return m_Name == other.m_Name;
	}
} // namespace Poly