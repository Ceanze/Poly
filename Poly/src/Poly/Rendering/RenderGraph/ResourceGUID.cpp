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
	ResourceGUID::ResourceGUID()
		: m_Pass("")
		, m_Resource("") {}

	ResourceGUID::ResourceGUID(const std::string& resourceGUID)
		: m_Pass(SeparateStrings(resourceGUID, '.').first)
		, m_Resource(SeparateStrings(resourceGUID, '.').second) {}

	ResourceGUID::ResourceGUID(std::string pass, std::string resource)
		: m_Pass(pass.empty() ? "$" : std::move(pass))
		, m_Resource(std::move(resource)) {}

	ResourceGUID::ResourceGUID(const ResourceGUID& other)
		: m_Pass(other.m_Pass)
		, m_Resource(other.m_Resource) {}

	ResourceGUID::ResourceGUID(ResourceGUID&& other)
		: m_Pass(std::move(other.m_Pass))
		, m_Resource(std::move(other.m_Resource)) {}

	ResourceGUID& ResourceGUID::operator=(const ResourceGUID& other)
	{
		m_Pass		= other.m_Pass;
		m_Resource	= other.m_Resource;
		return *this;
	}

	ResourceGUID& ResourceGUID::operator=(ResourceGUID&& other)
	{
		m_Pass		= std::move(other.m_Pass);
		m_Resource	= std::move(other.m_Resource);
		return *this;
	}

	ResourceGUID::~ResourceGUID() = default;

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
		return Poly::Format("{}.{}", m_Pass, m_Resource);
	}

	bool ResourceGUID::IsExternal() const
	{
		return m_Pass == "$";
	}

	bool ResourceGUID::HasResource() const
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

	ResourceGUID::operator std::string() const
	{
		return GetFullName();
	}
}
