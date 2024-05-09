#include "polypch.h"
#include "Poly/Scene/Scene.h"
#include "RenderData.h"
#include "ResourceCache.h"
#include "Poly/Rendering/SceneRenderer.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"

namespace Poly
{
	RenderData::RenderData(Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams)
	{
		m_pResourceCache	= pResourceCache;
		m_DefaultParams		= defaultParams;
	}
	
	void RenderData::ExecuteScene(const RenderContext& context) const
	{
		m_pSceneRenderer->Render(context);
	}

	Resource* RenderData::GetResourceNonConst(const std::string& resourceName) const
	{
		return m_pResourceCache->GetResource(m_RenderPassName + "." + resourceName);
	}

	const Resource* RenderData::operator[] (const std::string& resourceName) const
	{
		return m_pResourceCache->GetResource(m_RenderPassName + "." + resourceName);
	}
}