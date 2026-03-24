#include "ExternalPass.h"

namespace Poly
{
	ExternalPass::ExternalPass() : Pass(Pass::Type::EXTERNAL, "$") {}

	void ExternalPass::RegisterResource(const ResID& guid, const ResourceInfo& info)
	{
		m_Resources[guid] = info;
	}

	void ExternalPass::RemoveResource(const ResID& guid)
	{
		m_Resources.erase(guid);
	}

	bool ExternalPass::HasResource(const ResID& guid) const
	{
		return m_Resources.contains(guid);
	}

	const std::unordered_map<ResID, ResourceInfo>& ExternalPass::GetResources() const
	{
		return m_Resources;
	}
}