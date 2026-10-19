#pragma once

#include "RenderCatalog.h"
#include "RenderPlanBuilder.h"

namespace Poly
{
	class RenderGraph
	{
	public:
		RenderGraph();
		explicit RenderGraph(Ref<RenderCatalog> catalog);
		~RenderGraph() = default;

		/*
		 * Starts a render plan: the description of what a frame renders, which the renderer compiles together
		 * with the views submitted each frame. See RenderPlanBuilder.
		 */
		RenderPlanBuilder BeginRenderPlan();

		/*
		 * Starts a render plan from a description, e.g. one read with RenderGraphSerializer::ReadPlan().
		 */
		RenderPlanBuilder BeginRenderPlan(RenderPlanDesc desc);

		IPassDeclaration& RegisterPass(std::string_view name);

		IFeatureDeclaration& RegisterFeature(std::string_view name);

		/*
		 * Registers a resource with the render graph.
		 * A registered resource is a contract that the resource will be available during the generated render program.
		 * Provided resources' lifetime are managed by the caller, the rest by the render program.
		 *
		 * @param name The name of the resource to register.
		 * @return A ResourceDeclaration that can optionally add additional information about the resource.
		 */
		IResourceDeclaration& RegisterResource(std::string_view name);

	private:
		Ref<RenderCatalog> m_Catalog;
	};
} // namespace Poly
