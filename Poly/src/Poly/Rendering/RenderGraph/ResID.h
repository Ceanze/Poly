#pragma once

#include <string>

namespace Poly
{
	class PassResID;

	class ResID
	{
	public:
		ResID();
		explicit ResID(const std::string& resourceName);
		ResID(const ResID& other);
		ResID(ResID&& other);
		ResID& operator=(const ResID& other);
		ResID& operator=(ResID&& other);
		~ResID() = default;

		static ResID Invalid() { return ResID(); }

		const std::string& GetName() const;
		bool               IsValid() const;
		std::string        GetNameAsExternal() const;
		PassResID          GetAsExternal() const;

		bool operator==(const ResID& other) const noexcept;

	private:
		std::string m_Name;
	};
} // namespace Poly

namespace std
{
	template<>
	struct hash<Poly::ResID>
	{
		size_t operator()(const Poly::ResID& resID) const noexcept
		{
			return hash<std::string>()(resID.GetName());
		}
	};
} // namespace std