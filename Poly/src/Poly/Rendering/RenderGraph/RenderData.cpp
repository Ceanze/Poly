#include "RenderData.h"

#include "Poly/Rendering/RenderGraph/RenderContext.h"
#include "Poly/Rendering/SceneRenderer.h"
#include "Poly/Scene/Scene.h"
#include "polypch.h"
#include "ResourceCache.h"

namespace Poly
{
	RenderData::RenderData(Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams)
	{
		m_pResourceCache = pResourceCache;
		m_DefaultParams  = defaultParams;
	}

	Resource* RenderData::GetResourceNonConst(const std::string& resourceName) const
	{
		// PassResID is created within the function to avoid the pass to get resources outside of pass scope
		return m_pResourceCache->GetResource({m_RenderPassName, resourceName});
	}

	const Resource* RenderData::operator[](const std::string& resourceName) const
	{
		// PassResID is created within the function to avoid the pass to get resources outside of pass scope
		return m_pResourceCache->GetResource({m_RenderPassName, resourceName});
	}
} // namespace Poly