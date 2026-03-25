#pragma once

#include <string>

namespace Poly
{
	class PassID
	{
	public:
		PassID();
		explicit PassID(const std::string& passName);
		PassID(const PassID& other);
		PassID(PassID&& other);
		PassID& operator=(const PassID& other);
		PassID& operator=(PassID&& other);
		~PassID() = default;

		static PassID Invalid() { return PassID(); }

		const std::string& GetName() const;
		bool               IsValid() const;

		bool operator==(const PassID& other) const noexcept;

	private:
		std::string m_Name;
	};
} // namespace Poly

namespace std
{
	template<>
	struct hash<Poly::PassID>
	{
		size_t operator()(const Poly::PassID& passID) const noexcept
		{
			return hash<std::string>()(passID.GetName());
		}
	};
} // namespace std