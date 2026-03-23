#pragma once

namespace Poly
{
	class PassResID;

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
		bool IsValid() const;

		bool operator==(const PassID& other) const noexcept;

	private:
		std::string m_Name;
	};

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
		bool IsValid() const;
		std::string GetNameAsExternal() const;
		PassResID GetAsExternal() const;

		bool operator==(const ResID& other) const noexcept;

	private:
		std::string m_Name;
	};

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
		const ResID& GetResource() const;
		std::string GetFullName() const;

		bool IsExternal() const;
		bool HasResource() const;

		bool operator==(const PassResID& other) const noexcept;

	private:
		PassID m_Pass;
		ResID m_Resource;
	};
}

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

	template<>
	struct hash<Poly::ResID>
	{
		size_t operator()(const Poly::ResID& resID) const noexcept
		{
			return hash<std::string>()(resID.GetName());
		}
	};

	template<>
	struct hash<Poly::PassResID>
	{
		size_t operator()(const Poly::PassResID& passResID) const noexcept
		{
			return hash<std::string>()(passResID.GetFullName());
		}
	};
}