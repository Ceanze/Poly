#include "RenderGraph.h"

namespace Poly
{
	RenderGraph::RenderGraph()
	    : m_Catalog(std::make_shared<RenderCatalog>())
	{}

	RenderGraph::RenderGraph(Ref<RenderCatalog> catalog)
	    : m_Catalog(std::move(catalog))
	{}

	RenderPlanBuilder RenderGraph::BeginRenderPlan()
	{
		return RenderPlanBuilder(m_Catalog);
	}

	RenderPlanBuilder RenderGraph::BeginRenderPlan(RenderPlanDesc desc)
	{
		return RenderPlanBuilder(m_Catalog, std::move(desc));
	}

	IPassDeclaration& RenderGraph::RegisterPass(std::string_view name)
	{
		return m_Catalog->RegisterPass(name);
	}

	IFeatureDeclaration& RenderGraph::RegisterFeature(std::string_view name)
	{
		return m_Catalog->RegisterFeature(name);
	}

	IResourceDeclaration& RenderGraph::RegisterResource(std::string_view name)
	{
		return m_Catalog->RegisterResource(name);
	}
} // namespace Poly