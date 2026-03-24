#pragma once

#include "ResID.h"
#include "PassResID.h"

#include <string>

namespace Poly
{
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
}