#pragma once

#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace Poly
{
	template<typename T>
	class Registry
	{
	public:
		struct Entry
		{
			T           Value;
			std::string Source;
		};

		/*
		 * @param kind What the registry holds, e.g. "Pass", used in log messages.
		 */
		explicit Registry(std::string_view kind)
		    : m_Kind(kind)
		{}

		/*
		 * Registers a new, empty declaration. Replaces a declaration already registered with the same name.
		 * @param name The name of the declaration.
		 * @param source The path of the file the declaration is read from, empty if registered from code.
		 * @return The new declaration.
		 */
		T& Register(std::string_view name, std::string_view source = {})
		{
			auto it = m_Entries.find(name);
			if (it == m_Entries.end())
				return m_Entries.emplace(std::string(name), Entry{T(name), std::string(source)}).first->second.Value;

			POLY_CORE_WARN("{} '{}' was already registered{}, it is replaced", m_Kind, name, it->second.Source.empty() ? "" : " by " + it->second.Source);
			it->second = Entry{T(name), std::string(source)};
			return it->second.Value;
		}

		/*
		 * @return True if there was a declaration with the name.
		 */
		bool Remove(std::string_view name)
		{
			auto it = m_Entries.find(name);
			if (it == m_Entries.end())
				return false;

			m_Entries.erase(it);
			return true;
		}

		/*
		 * Removes every declaration registered from a file, e.g. before the file is read again.
		 * @param source The path of the file.
		 * @return The number of removed declarations.
		 */
		size_t RemoveFromSource(std::string_view source)
		{
			if (source.empty())
				return 0;

			return std::erase_if(m_Entries, [source](const auto& pair) { return pair.second.Source == source; });
		}

		void Clear() { m_Entries.clear(); }

		T* Get(std::string_view name)
		{
			auto it = m_Entries.find(name);
			return it != m_Entries.end() ? &it->second.Value : nullptr;
		}

		const T* Get(std::string_view name) const
		{
			auto it = m_Entries.find(name);
			return it != m_Entries.end() ? &it->second.Value : nullptr;
		}

		const Entry* GetEntry(std::string_view name) const
		{
			auto it = m_Entries.find(name);
			return it != m_Entries.end() ? &it->second : nullptr;
		}

		const std::map<std::string, Entry, std::less<>>& GetAll() const { return m_Entries; }

	private:
		std::string                               m_Kind;
		std::map<std::string, Entry, std::less<>> m_Entries;
	};
} // namespace Poly
