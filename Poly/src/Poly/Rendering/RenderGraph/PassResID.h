#pragma once

#include "PassID.h"
#include "ResID.h"

#include <string>

namespace Poly
{
	class PassResID
	{
	public:
		PassResID();
		PassResID(const std::string& passName, const std::string& resourceName);
		PassResID(const PassID& pass, const ResID& resource);
		PassResID(const PassResID& other);
		PassResID(PassResID&& other);
		PassResID& operator=(const PassResID& other);
		PassResID& operator=(PassResID&& other);
		~PassResID() = default;

		static PassResID Invalid() { return PassResID(); }

		const PassID& GetPass() const;
		const ResID&  GetResource() const;
		std::string   GetFullName() const;

		bool IsExternal() const;
		bool HasResource() const;

		bool operator==(const PassResID& other) const noexcept;

	private:
		PassID m_Pass;
		ResID  m_Resource;
	};
} // namespace Poly

namespace std
{
	template<>
	struct hash<Poly::PassResID>
	{
		size_t operator()(const Poly::PassResID& passResID) const noexcept
		{
			return hash<std::string>()(passResID.GetFullName());
		}
	};
} // namespace std