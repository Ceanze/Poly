#include "PassResID.h"

#include "Poly/Poly/Format.h"

namespace Poly
{
	PassResID::PassResID()
	    : m_Pass("")
	    , m_Resource("")
	{}

	PassResID::PassResID(const std::string& passName, const std::string& resourceName)
	    : m_Pass(passName.empty() ? "$" : passName)
	    , m_Resource(resourceName)
	{}

	PassResID::PassResID(const PassID& pass, const ResID& resource)
	    : m_Pass(pass)
	    , m_Resource(resource)
	{}

	PassResID::PassResID(const PassResID& other)
	    : m_Pass(other.m_Pass)
	    , m_Resource(other.m_Resource)
	{}

	PassResID::PassResID(PassResID&& other)
	    : m_Pass(std::move(other.m_Pass))
	    , m_Resource(std::move(other.m_Resource))
	{}

	PassResID& PassResID::operator=(const PassResID& other)
	{
		m_Pass     = other.m_Pass;
		m_Resource = other.m_Resource;
		return *this;
	}

	PassResID& PassResID::operator=(PassResID&& other)
	{
		m_Pass     = std::move(other.m_Pass);
		m_Resource = std::move(other.m_Resource);
		return *this;
	}

	const PassID& PassResID::GetPass() const
	{
		return m_Pass;
	}

	const ResID& PassResID::GetResource() const
	{
		return m_Resource;
	}

	std::string PassResID::GetFullName() const
	{
		return Poly::Format("{}.{}", m_Pass.GetName(), m_Resource.GetName());
	}

	bool PassResID::IsExternal() const
	{
		return m_Pass.IsValid() && m_Pass.GetName() == "$";
	}

	bool PassResID::HasResource() const
	{
		return m_Resource.IsValid();
	}

	bool PassResID::operator==(const PassResID& other) const noexcept
	{
		const bool bothExternal = IsExternal() && other.IsExternal();

		if (bothExternal)
			return m_Resource == other.m_Resource;
		else
			return m_Pass == other.m_Pass && m_Resource == other.m_Resource;
	}
} // namespace Poly