#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace Poly
{
	template<typename T>
	class Registry
	{
	public:
		T& Register(std::string_view name)
		{
			return m_Entries.try_emplace(std::string(name), name).first->second;
		}

		T* Get(std::string_view name)
		{
			auto it = m_Entries.find(std::string(name));
			return it != m_Entries.end() ? &it->second : nullptr;
		}

		const T* Get(std::string_view name) const
		{
			auto it = m_Entries.find(std::string(name));
			return it != m_Entries.end() ? &it->second : nullptr;
		}

		const std::unordered_map<std::string, T>& GetAll() const { return m_Entries; }

	private:
		std::unordered_map<std::string, T> m_Entries;
	};
} // namespace Poly
