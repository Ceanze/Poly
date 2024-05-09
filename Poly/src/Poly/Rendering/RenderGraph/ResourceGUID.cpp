#include "ResourceGUID.h"

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
	ResourceGUID::ResourceGUID(std::string resourceGUID)
		: m_Pass(SeparateStrings(resourceGUID, '.').first)
		, m_Resource(SeparateStrings(resourceGUID, '.').second) {}

	ResourceGUID::ResourceGUID(std::string pass, std::string resource)
		: m_Pass(pass.empty() ? "$" : pass)
		, m_Resource(resource) {}

	ResourceGUID ResourceGUID::Invalid()
	{
		return ResourceGUID("", "");
	}

	const std::string& ResourceGUID::GetPassName() const
	{
		return m_Pass;
	}

	const std::string& ResourceGUID::GetResourceName() const
	{
		return m_Resource;
	}

	std::string ResourceGUID::GetFullName() const
	{
		return std::format("{}.{}", m_Pass, m_Resource);
	}

	bool ResourceGUID::IsExternal() const
	{
		return m_Pass == "$";
	}

	bool ResourceGUID::IsValid() const
	{
		return !m_Resource.empty();
	}

	bool ResourceGUID::operator==(const ResourceGUID& other) const
	{
		const bool bothExternal = IsExternal() && other.IsExternal();
		
		if (bothExternal)
			return m_Resource == other.m_Resource;
		else
			return m_Pass == other.m_Pass && m_Resource == other.m_Resource;
	}
}