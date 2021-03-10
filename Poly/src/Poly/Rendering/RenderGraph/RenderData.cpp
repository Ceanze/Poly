#include "polypch.h"
#include "RenderData.h"
#include "ResourceCache.h"

namespace Poly
{
	RenderData::RenderData(const std::string& renderPassName, Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams)
	{
		m_RenderPassName	= renderPassName;
		m_pResourceCache	= pResourceCache;
		m_DefaultParams		= defaultParams;
	}
	
	const Resource* RenderData::operator[] (const std::string& resourceName)
	{
		m_pResourceCache->GetResource(m_RenderPassName + "." + resourceName);
	}


}