#pragma once

#include "RenderPlan.h"
#include "RenderProgram.h"
#include "Resource/ResourceState.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Poly
{
	/*
	 * How one view takes part in a render program. Only describes structure: which stage the view is
	 * rendered by and which of the other views it shares or reads resources with. Nothing in it identifies
	 * a specific view, world or texture, so the same program is reused for any views with the same structure.
	 */
	struct ProgramViewDesc
	{
		// Index of the view stage in RenderPlan::GetEntries()
		uint32 EntryIndex = 0;

		// The view slot whose color target the view renders to: 0 for the window, otherwise the slot of the
		// first view rendering to the same offscreen target, which is the slot of the view itself unless it shares one
		uint32 TargetSlot = 0;

		// Views with the same index render the same world
		uint32 WorldIndex = 0;

		// View scoped resources bound to the target of an earlier view: name of the resource and the target slot
		// of that view. Sorted by name.
		std::vector<std::pair<std::string, uint32>> TargetReads;

		bool operator==(const ProgramViewDesc& other) const = default;
	};

	// The views of a render program in the order they are rendered, the view at index i has view slot i + 1.
	// Together with the render plan this decides the program completely.
	using ProgramKey = std::vector<ProgramViewDesc>;

	/*
	 * Compiles a render plan and the views of a frame into a RenderProgram: the passes of every frame feature
	 * once and the passes of every view stage once per view, with resource names scoped per view, world and
	 * target, then culled, sorted and given a synchronization plan.
	 */
	class RenderProgramBuilder
	{
	public:
		CLASS_STATIC(RenderProgramBuilder);

		/*
		 * @param plan The render plan to build the program from.
		 * @param key The views to render, ordered by the stage they belong to and with any view reading
		 *            the target of another view after that view.
		 */
		static Ref<RenderProgram> Build(const RenderPlan& plan, const ProgramKey& key);

	private:
		using FinalStates = std::unordered_map<std::string, FResourceState>;

		static std::vector<ResolvedPass>    InstantiatePasses(const RenderPlan& plan, const ProgramKey& key);
		static std::vector<struct PassNode> BuildDAG(const std::vector<ResolvedPass>& flat);
		static std::vector<ResolvedPass>    TopoSortALAP(const std::vector<struct PassNode>& nodes, const std::vector<ResolvedPass>& flat);
		static SyncPlan                     PlanSynchronization(const std::vector<ResolvedPass>& passes, const FinalStates& finalStates);
	};
} // namespace Poly
