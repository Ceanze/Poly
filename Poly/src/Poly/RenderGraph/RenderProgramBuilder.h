#pragma once

#include "RenderCatalog.h"
#include "RenderProgram.h"
#include "Resource/ResourceState.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Poly
{
	class RenderProgramBuilder
	{
	public:
		explicit RenderProgramBuilder(Ref<RenderCatalog> catalog);

		RenderProgramBuilder& AddFeature(std::string_view name);

		/*
		 * Requires a resolved port/resource (e.g. "$Color", or any registered resource name) to start in
		 * the given state, without needing it to be registered on the RenderGraph. Takes precedence over
		 * a matching IResourceDeclaration::WithInitialState() if both are set.
		 * @param resolvedName The resolved port/resource name, following the same naming as ports (e.g. "$Color").
		 * @param state The state the resource is assumed to already be in the first time this program touches it.
		 * @return Reference to the builder for chaining.
		 */
		RenderProgramBuilder& WithInitialState(std::string_view resolvedName, FResourceState state);

		/*
		 * Requires a resolved port/resource (e.g. "$Color", or any registered resource name) to end up in
		 * the given state after its last use in this program - e.g. FResourceState::Present on "$Color" for
		 * a program that presents to a swapchain, so the image is left in the layout vkQueuePresentKHR
		 * requires. Unlike IResourceDeclaration::WithInitialState(), there is no registry-based equivalent
		 * for this, since semantic ports like "$Color" are never registered as resources at all - this is
		 * the only way to declare it for them.
		 * @param resolvedName The resolved port/resource name, following the same naming as ports (e.g. "$Color").
		 * @param state The state the resource must be in once this program is done with it.
		 * @return Reference to the builder for chaining.
		 */
		RenderProgramBuilder& WithFinalState(std::string_view resolvedName, FResourceState state);

		Ref<RenderProgram> Build();

	private:
		std::vector<ResolvedPass>    FlattenFeatures();
		std::vector<struct PassNode> BuildDAG(const std::vector<ResolvedPass>& flat) const;
		std::vector<ResolvedPass>    TopoSortALAP(const std::vector<struct PassNode>& nodes,
		                                          const std::vector<ResolvedPass>&    flat) const;
		void                         AssignBindlessSlots(std::vector<ResolvedPass>& passes) const;
		SyncPlan                     PlanSynchronization(const std::vector<ResolvedPass>& passes) const;

		Ref<RenderCatalog>       m_Catalog;
		std::vector<std::string> m_Features;

		std::unordered_map<std::string, FResourceState> m_InitialStates;
		std::unordered_map<std::string, FResourceState> m_FinalStates;
	};
} // namespace Poly
