#include "ResourceGroup.h"

#include "Resource.h"

namespace Poly
{
	void ResourceGroup::AddResource(Ref<Resource> pResource, bool autoBindDescriptor)
	{
		if (m_Resources.contains(pResource->GetName()))
		{
			POLY_CORE_WARN("Resource {} is already added to resource group {}!", pResource->GetName(), m_GroupName);
			return;
		}

		pResource->SetName(m_GroupName + ':' + pResource->GetName());
		m_Resources[pResource->GetName()] = { pResource, autoBindDescriptor };
	}

	void ResourceGroup::AddResource(const std::string& name, bool autoBindDescriptor)
	{
		if (m_Resources.contains(name))
		{
			POLY_CORE_WARN("Resource {} is already added to resource group {}!", name, m_GroupName);
			return;
		}

		m_Resources[name] = { nullptr, autoBindDescriptor };
	}

	void ResourceGroup::SetResource(const std::string& name, Ref<Resource> pResource)
	{
		if (!m_Resources.contains(name))
		{
			POLY_CORE_WARN("Resource {} is not added to resource group {}!", name, m_GroupName);
			return;
		}

		pResource->SetName(m_GroupName + ':' + pResource->GetName());
		m_Resources[name].pResource = pResource;
	}

	Ref<Resource> ResourceGroup::GetResource(const std::string& name) const
	{
		if (!m_Resources.contains(name))
		{
			POLY_CORE_WARN("Resource {} is not added to resource group {}!", name, m_GroupName);
			return nullptr;
		}

		return m_Resources.at(name).pResource;
	}

	const ResourceInfo& ResourceGroup::GetResourceInfo(const std::string& name) const
	{
		if (!m_Resources.contains(name))
		{
			POLY_CORE_WARN("Resource {} is not added to resource group {}!", name, m_GroupName);
			return {};
		}

		return m_Resources.at(name);
	}
}