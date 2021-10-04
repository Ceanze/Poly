#include "polypch.h"
#include "Poly/Rendering/Scene.h"
#include "RenderData.h"
#include "ResourceCache.h"

namespace Poly
{
	RenderData::RenderData(Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams)
	{
		m_pResourceCache	= pResourceCache;
		m_DefaultParams		= defaultParams;
	}
	
	void RenderData::ExecuteScene(const RenderContext& context) const
	{
		m_pScene->Execute(context);
	}

	Resource* RenderData::GetResourceNonConst(const std::string& resourceName) const
	{
		return m_pResourceCache->GetResource(m_RenderPassName + "." + resourceName).get();
	}

	const Resource* RenderData::operator[] (const std::string& resourceName) const
	{
		return m_pResourceCache->GetResource(m_RenderPassName + "." + resourceName).get();
	}
}