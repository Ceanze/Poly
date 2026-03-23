#include "ResourceGUID.h"

#include "Poly/Poly/Format.h"

namespace
{
	inline std::pair<std::string, std::string> SeparateStrings(const std::string& value, char separator)
	{
		auto pos = value.find_first_of(separator);

		if (pos == std::string::npos)
			return { "$", value };

		return { value.substr(0, pos), value.substr(pos + 1) };
	}
}

namespace Poly
{
	/*
	* ---------------------
	* PassID implementation
	* ---------------------
	*/
	PassID::PassID()
		: m_Name("") {}

	PassID::PassID(const std::string& passName)
		: m_Name(passName.empty() ? "$" : passName) {}

	PassID::PassID(const PassID& other)
		: m_Name(other.m_Name) {}

	PassID::PassID(PassID&& other)
		: m_Name(std::move(other.m_Name)) {}

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

	const std::string &PassID::GetName() const
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

	/*
	* ---------------------
	* ResID implementation
	* ---------------------
	*/
	ResID::ResID()
		: m_Name("") {}

	ResID::ResID(const std::string& resourceName)
		: m_Name(resourceName) {}

	ResID::ResID(const ResID& other)
		: m_Name(other.m_Name) {}

	ResID::ResID(ResID&& other)
		: m_Name(std::move(other.m_Name)) {}

	ResID& ResID::operator=(const ResID& other)
	{
		m_Name = other.m_Name;
		return *this;
	}

	ResID& ResID::operator=(ResID&& other)
	{
		m_Name = std::move(other.m_Name);
		return *this;
	}

	const std::string &ResID::GetName() const
	{
		return m_Name;
	}

	bool ResID::IsValid() const
	{
		return !m_Name.empty();
	}

	std::string ResID::GetNameAsExternal() const
	{
		return m_Name;
	}

	PassResID ResID::GetAsExternal() const
	{
		return PassResID(PassID("$"), *this);
	}

	bool ResID::operator==(const ResID& other) const noexcept
	{
		return m_Name == other.m_Name;
	}

	/*
	* ---------------------
	* PassResID implementation
	* ---------------------
	*/

	PassResID::PassResID()
		: m_Pass("")
		, m_Resource("") {}

	PassResID::PassResID(const std::string& passName, const std::string& resourceName)
		: m_Pass(passName.empty() ? "$" : passName)
		, m_Resource(resourceName) {}

	PassResID::PassResID(const PassID& pass, const ResID& resource)
		: m_Pass(pass)
		, m_Resource(resource) {}

	PassResID::PassResID(const PassResID& other)
		: m_Pass(other.m_Pass)
		, m_Resource(other.m_Resource) {}

	PassResID::PassResID(PassResID&& other)
		: m_Pass(std::move(other.m_Pass))
		, m_Resource(std::move(other.m_Resource)) {}


	PassResID& PassResID::operator=(const PassResID& other)
	{
		m_Pass = other.m_Pass;
		m_Resource = other.m_Resource;
		return *this;
	}

	PassResID& PassResID::operator=(PassResID&& other)
	{
		m_Pass = std::move(other.m_Pass);
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
}
