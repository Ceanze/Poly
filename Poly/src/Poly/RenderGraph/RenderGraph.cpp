#include "RenderGraph.h"

#include "Serialization/RenderGraphSerializer.h"

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

	Ref<RenderPlan> RenderGraph::LoadRenderPlan(std::string_view vfsPath)
	{
		Result<RenderPlanDesc> plan = RenderGraphSerializer::ReadPlan(vfsPath);
		if (!plan)
		{
			POLY_CORE_ERROR("{}", plan.error().Message);
			return nullptr;
		}

		for (const CatalogIssue& issue : m_Catalog->ValidatePlan(*plan))
		{
			if (issue.Severity == CatalogIssue::ESeverity::Error)
				POLY_CORE_ERROR("{}: {}", vfsPath, issue.Message);
			else
				POLY_CORE_WARN("{}: {}", vfsPath, issue.Message);
		}

		return BeginRenderPlan(std::move(*plan)).Build();
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