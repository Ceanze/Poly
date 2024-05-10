#pragma once

namespace Poly
{
	class ResourceGUID
	{
	public:
		ResourceGUID();
		ResourceGUID(const std::string& resourceGUID);
		ResourceGUID(std::string pass, std::string resource);

		ResourceGUID(const ResourceGUID& other);
		ResourceGUID(ResourceGUID&& other);
		ResourceGUID& operator=(const ResourceGUID& other);
		ResourceGUID& operator=(ResourceGUID&& other);
		~ResourceGUID();


		static ResourceGUID Invalid();

		const std::string& GetPassName() const;
		const std::string& GetResourceName() const;
		std::string GetFullName() const;
		
		bool IsExternal() const;
		bool HasResource() const;

		bool operator==(const ResourceGUID& other) const;
		operator std::string() const;
	
	private:
		std::string m_Pass;
		std::string m_Resource;
	};

	struct ResourceGUIDHasher
	{
		size_t operator()(const ResourceGUID& key) const
		{
			return std::hash<std::string>()(key.GetFullName());
		}
	};
}