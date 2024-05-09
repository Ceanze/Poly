#pragma once

namespace Poly
{
	class ResourceGUID
	{
	public:
		ResourceGUID(std::string resourceGUID);
		ResourceGUID(std::string pass, std::string resource);

		static ResourceGUID Invalid();

		const std::string& GetPassName() const;
		const std::string& GetResourceName() const;
		std::string GetFullName() const;
		
		bool IsExternal() const;
		bool IsValid() const;

		bool operator==(const ResourceGUID& other) const;
	
	private:
		const std::string m_Pass;
		const std::string m_Resource;
	};
}