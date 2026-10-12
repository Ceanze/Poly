#include "RenderProgramBuilder.h"

#include "Feature/FeaturePort.h"
#include "Resource/ResourceUsage.h"

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace Poly
{
	struct PassNode
	{
		size_t              flatIndex;
		std::vector<size_t> dependsOn;
		bool                mandatory = false;
	};

	namespace
	{
		// The window is target slot 0 and keeps the plain semantic name
		std::string ToTargetName(uint32 targetSlot)
		{
			const std::string colorName(ToSemanticName(EFeaturePort::Color));
			return targetSlot == 0 ? colorName : "target#" + std::to_string(targetSlot) + "." + colorName;
		}

		// Gives the ports of a pass the names they are known by in the program, see ResolvedPort::ResolvedName
		void ScopePorts(ResolvedPass& pass, uint32 viewSlot, const ProgramViewDesc* pView)
		{
			pass.ViewSlot = viewSlot;

			for (ResolvedPort& port : pass.Ports)
			{
				if (port.Semantic == EFeaturePort::Color)
				{
					port.TargetSlot   = pView ? pView->TargetSlot : 0;
					port.ResolvedName = ToTargetName(port.TargetSlot);
					continue;
				}

				// Passes of frame features have no view or world to scope by
				if (!pView || port.Scope == EResourceScope::Global)
				{
					port.ResolvedName = port.Name;
					continue;
				}

				if (port.Scope == EResourceScope::World)
				{
					port.ResolvedName = "world#" + std::to_string(pView->WorldIndex) + "." + port.Name;
					continue;
				}

				// A texture the view has bound to the target of another view is that target, which makes the
				// pass depend on the passes rendering to it
				if (!port.IsWrite && port.IsExternal && IsTextureResourceType(port.ResourceType))
				{
					const auto it = std::ranges::find_if(pView->TargetReads, [&port](const auto& read) { return read.first == port.Name; });
					if (it != pView->TargetReads.end())
					{
						port.TargetSlot   = it->second;
						port.ResolvedName = ToTargetName(port.TargetSlot);
						port.IsExternal   = false;
						continue;
					}
				}

				port.ResolvedName = "view#" + std::to_string(viewSlot) + "." + port.Name;
			}
		}
	} // namespace

	// Phase 1: Lay out the passes of the plan in order: the passes of a frame feature once, the passes of a
	// view stage once for every view of the stage.
	std::vector<ResolvedPass> RenderProgramBuilder::InstantiatePasses(const RenderPlan& plan, const ProgramKey& key)
	{
		std::vector<ResolvedPass> flat;

		const auto& entries = plan.GetEntries();
		for (uint32 entryIndex = 0; entryIndex < entries.size(); entryIndex++)
		{
			const RenderPlanEntry& entry = entries[entryIndex];

			if (entry.Type == RenderPlanEntry::EType::FrameFeature)
			{
				for (const ResolvedPass& pass : entry.Passes)
					ScopePorts(flat.emplace_back(pass), 0, nullptr);

				continue;
			}

			for (uint32 viewIndex = 0; viewIndex < key.size(); viewIndex++)
			{
				if (key[viewIndex].EntryIndex != entryIndex)
					continue;

				for (const ResolvedPass& pass : entry.Passes)
					ScopePorts(flat.emplace_back(pass), viewIndex + 1, &key[viewIndex]);
			}
		}

		return flat;
	}

	// Phase 2: Build a culled dependency graph.
	// Passes that don't contribute to a color target are excluded.
	std::vector<PassNode> RenderProgramBuilder::BuildDAG(const std::vector<ResolvedPass>& flat)
	{
		const size_t          n = flat.size();
		std::vector<PassNode> nodes(n);
		for (size_t i = 0; i < n; ++i)
			nodes[i].flatIndex = i;

		// Seed: the last pass that writes a color target (the window or an offscreen target of a view) is a mandatory sink
		std::unordered_set<std::string> seenTargets;
		for (int i = static_cast<int>(n) - 1; i >= 0; --i)
		{
			for (const auto& port : flat[i].Ports)
			{
				if (port.IsWrite && port.Semantic == EFeaturePort::Color && seenTargets.insert(port.ResolvedName).second)
					nodes[i].mandatory = true;
			}
		}

		// availableInputs: resolvedName -> indices of the mandatory passes that read it and have no producer yet
		std::unordered_map<std::string, std::vector<size_t>> availableInputs;

		// Reverse scan: pull in producers of resources that mandatory passes need
		for (int i = static_cast<int>(n) - 1; i >= 0; --i)
		{
			const auto& pass = flat[i];

			// Check if any of this pass's writes satisfy a pending read
			if (!nodes[i].mandatory)
			{
				for (const auto& port : pass.Ports)
				{
					if (port.IsWrite && availableInputs.count(port.ResolvedName))
					{
						nodes[i].mandatory = true;
						break;
					}
				}
			}

			if (!nodes[i].mandatory)
				continue;

			// Wire writes → consuming passes as dependencies.
			// A write without a clear is an implicit passthrough (read-then-write), so it
			// also appears in availableInputs and will pull in whichever earlier pass produced it.
			for (const auto& port : pass.Ports)
			{
				if (!port.IsWrite)
					continue;

				auto it = availableInputs.find(port.ResolvedName);
				if (it != availableInputs.end())
				{
					for (size_t consumer : it->second)
						nodes[consumer].dependsOn.push_back((size_t)i);

					availableInputs.erase(it);
				}
			}

			// Register all ports as pending: explicit reads and write-passthroughs.
			// An earlier pass that writes the same resource name will satisfy the passthrough.
			for (const auto& port : pass.Ports)
			{
				std::vector<size_t>& consumers = availableInputs[port.ResolvedName];
				if (consumers.empty() || consumers.back() != (size_t)i)
					consumers.push_back((size_t)i);
			}
		}

		return nodes;
	}

	// Phase 3: Topological sort, scheduled as-late-as-possible (ALAP).
	// Passes with deeper dependency chains are scheduled first so resources
	// have the shortest possible lifetime.
	std::vector<ResolvedPass> RenderProgramBuilder::TopoSortALAP(const std::vector<PassNode>& nodes, const std::vector<ResolvedPass>& flat)
	{
		const size_t n = nodes.size();

		// Memoized depth: number of edges on the longest path from this node to a leaf
		std::vector<int>           depth(n, -1);
		std::function<int(size_t)> computeDepth = [&](size_t idx) -> int {
			if (depth[idx] >= 0)
				return depth[idx];

			if (nodes[idx].dependsOn.empty())
				return depth[idx] = 0;

			int maxDep = 0;
			for (size_t dep : nodes[idx].dependsOn)
				maxDep = std::max(maxDep, computeDepth(dep));

			return depth[idx] = 1 + maxDep;
		};

		for (size_t i = 0; i < n; ++i)
			if (nodes[i].mandatory)
				computeDepth(i);

		// Sinks: mandatory nodes that no other mandatory node depends on
		std::unordered_set<size_t> hasDependents;
		for (size_t i = 0; i < n; ++i)
		{
			if (!nodes[i].mandatory)
				continue;

			for (size_t dep : nodes[i].dependsOn)
				hasDependents.insert(dep);
		}

		std::vector<ResolvedPass>  result;
		std::unordered_set<size_t> visited;

		std::function<void(size_t)> visit = [&](size_t idx) {
			if (visited.count(idx))
				return;

			visited.insert(idx);
			// Deepest dependency chains first → latest possible scheduling
			auto deps = nodes[idx].dependsOn;
			std::sort(deps.begin(), deps.end(), [&](size_t a, size_t b) {
				return depth[a] > depth[b];
			});

			for (size_t dep : deps)
				visit(dep);

			result.push_back(flat[idx]);
		};

		for (size_t i = 0; i < n; ++i)
			if (nodes[i].mandatory && !hasDependents.count(i))
				visit(i);

		return result;
	}

	// Phase 4: Plan explicit synchronization for the sorted pass list.
	// Tracks, per resolved resource name, the last known layout/access/stage/owning-queue and compares
	// it against what each port needs. Same-queue transitions batch into one BarrierGroup per consuming
	// pass (grouped syncs); a resource already in the required state produces no entry at all (indirect
	// syncs). Cross-queue reads/writes are split into a Release (on the resource's previous owning pass)
	// and an Acquire (on the consuming pass), paired via a SyncPoint wait whose value is collapsed to the
	// highest value already awaited on that queue pair (cross-queue indirect sync elision).
	// Inspiration source: "Organizing GPU Work with Directed Acyclic Graphs" by Pavlo Muratov https://levelup.gitconnected.com/organizing-gpu-work-with-directed-acyclic-graphs-f3fd5f2c2af3
	struct ResourceTrackState
	{
		bool           IsTracked     = false;
		bool           IsTexture     = false;
		ETextureLayout Layout        = ETextureLayout::UNDEFINED;
		FAccessFlag    Access        = FAccessFlag::NONE;
		FPipelineStage Stage         = FPipelineStage::NONE;
		FQueueType     Queue         = FQueueType::GRAPHICS;
		bool           LastWasWrite  = false;
		size_t         LastPassIndex = 0;
	};

	SyncPlan RenderProgramBuilder::PlanSynchronization(const std::vector<ResolvedPass>& passes, const FinalStates& finalStates)
	{
		std::unordered_map<std::string, ResourceTrackState>                      state;
		std::unordered_map<FQueueType, uint64_t>                                 queueSubmitCounter;
		std::unordered_map<FQueueType, std::unordered_map<FQueueType, uint64_t>> highestWaited;

		std::vector<PassSyncPlan> passPlans(passes.size());

		for (size_t i = 0; i < passes.size(); ++i)
		{
			const ResolvedPass& pass  = passes[i];
			const FQueueType    queue = pass.Queue;

			passPlans[i].PassIndex       = i;
			passPlans[i].SubmissionIndex = ++queueSubmitCounter[queue];

			const FPipelineStage                     passStages = DerivePassShaderStages(pass.Shaders);
			std::unordered_map<FQueueType, uint64_t> neededWaits;
			std::unordered_set<std::string>          seenThisPass;

			for (const ResolvedPort& port : pass.Ports)
			{
				if (!seenThisPass.insert(port.ResolvedName).second)
				{
					POLY_CORE_ERROR("Pass '{}' reads and writes resource '{}' within the same pass; "
					                "intra-pass synchronization isn't supported, skipping.",
					                pass.Name, port.ResolvedName);
					continue;
				}

				const bool isAttachmentSemantic = IsAttachmentSemanticPort(port.Semantic);
				const bool isTexture            = isAttachmentSemantic || IsTextureResourceType(port.ResourceType);

				if (!isAttachmentSemantic && port.ResourceType == EResourceType::None)
					POLY_CORE_WARN("Pass '{}' port '{}' has no resource type; it will not be synchronized.", pass.Name,
					               port.ResolvedName);

				const ResourceUsage needed = port.UsageState != FResourceState::Unknown
				                                 ? ConvertResourceState(port.UsageState)
				                                 : DeriveResourceUsage(port.ResourceType, port.IsWrite, port.Semantic, passStages);

				ResourceTrackState& rs           = state[port.ResolvedName];
				const bool          isFirstTouch = !rs.IsTracked;
				rs.IsTexture                     = isTexture;
				if (!rs.IsTracked)
				{
					rs.IsTracked = true;
					rs.Queue     = queue; // first touch: assume no incoming cross-queue dependency

					if (port.InitialState != FResourceState::Unknown)
					{
						const ResourceUsage seed = ConvertResourceState(port.InitialState);
						rs.Layout                = seed.Layout;
						rs.Access                = seed.Access;
						rs.Stage                 = seed.Stage;
					}
				}

				// Attachment load op: a pass declaration can force one explicitly (LoadOpOverride); otherwise
				// infer from program order - the first write to a resolved attachment name has nothing to
				// preserve (CLEAR), any later write must preserve what an earlier pass already wrote (LOAD).
				if (port.IsWrite && isAttachmentSemantic)
				{
					passPlans[i].AttachmentLoadOps[port.ResolvedName] =
					    port.LoadOpOverride != ELoadOp::NONE ? port.LoadOpOverride : (isFirstTouch ? ELoadOp::CLEAR : ELoadOp::LOAD);
				}

				const bool queueDiffers  = rs.Queue != queue;
				const bool layoutDiffers = isTexture && rs.Layout != needed.Layout;
				const bool isHazard      = rs.LastWasWrite || port.IsWrite || layoutDiffers;

				if (queueDiffers)
				{
					passPlans[rs.LastPassIndex].PostReleases.push_back(
					    {port.ResolvedName, isTexture, rs.Layout, needed.Layout, rs.Access, rs.Stage, queue});
					passPlans[i].Acquires.push_back(
					    {port.ResolvedName, isTexture, rs.Layout, needed.Layout, needed.Access, needed.Stage, rs.Queue});

					uint64_t& wait = neededWaits[rs.Queue];
					wait           = std::max(wait, passPlans[rs.LastPassIndex].SubmissionIndex);
				}
				else if (isHazard)
				{
					if (isTexture)
						passPlans[i].PreBarriers.Textures.push_back({port.ResolvedName, rs.Layout, needed.Layout, rs.Access,
						                                             needed.Access, rs.Stage, needed.Stage, needed.AspectMask});
					else
						passPlans[i].PreBarriers.Buffers.push_back(
						    {port.ResolvedName, rs.Access, needed.Access, rs.Stage, needed.Stage});
				}
				// else: resource is already exactly where it needs to be -- indirect sync, nothing to do.

				rs.Layout        = needed.Layout;
				rs.Access        = needed.Access;
				rs.Stage         = needed.Stage;
				rs.Queue         = queue;
				rs.LastWasWrite  = port.IsWrite;
				rs.LastPassIndex = i;
			}

			for (const auto& [srcQueue, neededValue] : neededWaits)
			{
				uint64_t& already = highestWaited[queue][srcQueue];
				if (neededValue > already)
				{
					passPlans[i].RequiredWaits[srcQueue] = neededValue;
					already                              = neededValue;
				}
				// else: this queue already waited far enough -- cross-queue indirect sync elision.
			}
		}

		// Closing transitions: leave each resource with a final state in that state after its last use
		// in the program (e.g. "$Color" -> Present)
		for (const auto& [resolvedName, finalState] : finalStates)
		{
			auto it = state.find(resolvedName);
			if (it == state.end() || !it->second.IsTracked)
			{
				POLY_CORE_WARN("A final state is required for '{}', but no pass in this program touches it.", resolvedName);
				continue;
			}

			ResourceTrackState& rs            = it->second;
			const ResourceUsage target        = ConvertResourceState(finalState);
			const bool          layoutDiffers = rs.IsTexture && rs.Layout != target.Layout;

			if (!layoutDiffers && rs.Access == target.Access && rs.Stage == target.Stage)
				continue; // already exactly where it needs to end up

			if (rs.IsTexture)
				passPlans[rs.LastPassIndex].PostBarriers.Textures.push_back(
				    {resolvedName, rs.Layout, target.Layout, rs.Access, target.Access, rs.Stage, target.Stage, target.AspectMask});
			else
				passPlans[rs.LastPassIndex].PostBarriers.Buffers.push_back(
				    {resolvedName, rs.Access, target.Access, rs.Stage, target.Stage});
		}

		return SyncPlan(std::move(passPlans));
	}

	Ref<RenderProgram> RenderProgramBuilder::Build(const RenderPlan& plan, const ProgramKey& key)
	{
		auto flat   = InstantiatePasses(plan, key);
		auto nodes  = BuildDAG(flat);
		auto sorted = TopoSortALAP(nodes, flat);

		// What a color target is left as follows from what it is: the window is presented, an offscreen
		// target is sampled by a later program or by the UI
		FinalStates finalStates;
		for (const ResolvedPass& pass : sorted)
		{
			for (const ResolvedPort& port : pass.Ports)
			{
				if (port.IsWrite && port.Semantic == EFeaturePort::Color)
					finalStates[port.ResolvedName] = port.TargetSlot == 0 ? FResourceState::Present : FResourceState::ShaderResource;
			}
		}

		auto syncPlan = PlanSynchronization(sorted, finalStates);
		return CreateRef<RenderProgram>(std::move(sorted), std::move(syncPlan));
	}
} // namespace Poly
