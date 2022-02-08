#include "ResourceGroup.h"

#include "Resource.h"

namespace Poly
{
	void ResourceGroup::AddResource(Ref<Resource> pResource)
	{
		if (m_Resources.contains(pResource->GetName()))
		{
			POLY_CORE_WARN("Resource {} is already added to resource group {}!", pResource->GetName(), m_GroupName);
			return;
		}

		m_Resources[pResource->GetName()] = pResource;
	}

	void ResourceGroup::AddResource(const std::string& name)
	{
		if (m_Resources.contains(name))
		{
			POLY_CORE_WARN("Resource {} is already added to resource group {}!", name, m_GroupName);
			return;
		}

		m_Resources[name] = nullptr;
	}

	Ref<Resource> ResourceGroup::GetResource(const std::string& name) const
	{
		if (!m_Resources.contains(name))
		{
			POLY_CORE_WARN("Resource {} is not added to resource group {}!", name, m_GroupName);
			return nullptr;
		}

		return m_Resources.at(name);
	}
}