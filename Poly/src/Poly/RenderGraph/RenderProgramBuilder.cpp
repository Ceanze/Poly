#include "RenderProgramBuilder.h"

#include "Feature/FeatureDeclaration.h"
#include "Feature/FeaturePort.h"
#include "Pass/PassDeclaration.h"
#include "Poly/Resources/Shader/ShaderManager.h"
#include "Resource/ResourceDeclaration.h"
#include "Resource/ResourceUsage.h"
#include "SetupContext.h"

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

	RenderProgramBuilder::RenderProgramBuilder(Ref<RenderCatalog> catalog)
	    : m_Catalog(std::move(catalog))
	{}

	RenderProgramBuilder& RenderProgramBuilder::AddFeature(std::string_view name)
	{
		if (!m_Catalog->GetFeatureRegistry().Get(name))
		{
			POLY_CORE_ERROR("Feature '{}' not found in render catalog.", name);
			return *this;
		}

		m_Features.emplace_back(name);
		return *this;
	}

	RenderProgramBuilder& RenderProgramBuilder::WithInitialState(std::string_view resolvedName, FResourceState state)
	{
		m_InitialStates[std::string(resolvedName)] = state;
		return *this;
	}

	RenderProgramBuilder& RenderProgramBuilder::WithFinalState(std::string_view resolvedName, FResourceState state)
	{
		m_FinalStates[std::string(resolvedName)] = state;
		return *this;
	}

	// Phase 1: Expand features into an ordered flat list of passes with resolved port names.
	// Import/export resource names are scoped per feature instance ("featureName#N.resName")
	// so that two uses of the same feature don't share internal resource connections.
	// Semantic ports (EFeaturePort) and global mappings are intentionally left unscoped.
	std::vector<ResolvedPass> RenderProgramBuilder::FlattenFeatures()
	{
		std::vector<ResolvedPass>               flat;
		std::unordered_map<std::string, size_t> instanceCount;

		for (const auto& featureName : m_Features)
		{
			const FeatureDeclaration* feature = m_Catalog->GetFeatureRegistry().Get(featureName);
			if (!feature)
			{
				POLY_CORE_ERROR("Feature '{}' not found during compilation.", featureName);
				continue;
			}

			const size_t      instanceIdx = instanceCount[featureName]++;
			const std::string scope       = featureName + "#" + std::to_string(instanceIdx) + ".";

			for (const auto& passName : feature->GetPasses())
			{
				PassDeclaration* pass = m_Catalog->GetPassRegistry().Get(passName);
				if (!pass)
				{
					POLY_CORE_ERROR("Pass '{}' referenced by feature '{}' not found during compilation.",
					                passName, featureName);
					continue;
				}

				SetupContext setupCtx(*pass);
				pass->CallSetupFn(setupCtx);

				ResolvedPass resolved;
				resolved.Name         = passName;
				resolved.Shaders      = pass->GetShaders();
				resolved.PipelineDesc = pass->GetGraphicsPipeline().GetDesc();
				resolved.ExecuteFn    = pass->GetExecuteFn();
				resolved.Queue        = pass->GetQueue();

				for (const ResourceMapping& mapping : pass->GetResourceMappings())
				{
					ResolvedPort port;
					port.ShaderName     = mapping.ShaderResourceName;
					port.ResolvedName   = std::string(ToSemanticName(mapping.Port));
					port.IsWrite        = true;
					port.LoadOpOverride = mapping.LoadOpOverride;
					resolved.Ports.push_back(std::move(port));
				}

				for (const auto& [globalName, shaderName] : pass->GetGlobalMappings())
					resolved.Ports.push_back({shaderName, globalName, /*IsWrite=*/false});

				for (const auto& [resName, shaderName] : pass->GetImportedResources())
					resolved.Ports.push_back({shaderName, scope + resName, /*IsWrite=*/false});

				for (const auto& [resName, shaderName] : pass->GetExportedResources())
					resolved.Ports.push_back({shaderName, scope + resName, /*IsWrite=*/true});

				// A port whose resolved name matches a resource registered on the RenderGraph with an
				// explicit size is graph-owned/allocated at that fixed size (e.g. a shadow map); one
				// registered without a size is assumed externally owned, supplied per-frame via
				// RenderProgramInstance::UpdateResource(). Everything unregistered is transient and
				// sized to the render target by the RenderProgramInstance.
				for (auto& p : resolved.Ports)
				{
					if (const ResourceDeclaration* resDecl = m_Catalog->GetResourceRegistry().Get(p.ResolvedName))
					{
						p.ResourceType = resDecl->GetResourceType();
						p.InitialState = resDecl->GetInitialState();
						p.Width        = resDecl->GetWidth();
						p.Height       = resDecl->GetHeight();
						p.IsExternal   = !resDecl->HasSize();
					}

					// Builder-level override
					if (auto it = m_InitialStates.find(p.ResolvedName); it != m_InitialStates.end())
						p.InitialState = it->second;
				}

				flat.push_back(std::move(resolved));
			}
		}

		return flat;
	}

	// Phase 2: Build a culled dependency graph.
	// Passes that don't contribute to the final $Color output or are marked non-mandatory are excluded.
	std::vector<PassNode> RenderProgramBuilder::BuildDAG(const std::vector<ResolvedPass>& flat) const
	{
		const size_t          n = flat.size();
		std::vector<PassNode> nodes(n);
		for (size_t i = 0; i < n; ++i)
			nodes[i].flatIndex = i;

		// Seed: the last pass that writes $Color is the mandatory sink
		for (int i = static_cast<int>(n) - 1; i >= 0; --i)
		{
			for (const auto& port : flat[i].Ports)
			{
				if (port.IsWrite && port.ResolvedName == "$Color")
				{
					nodes[i].mandatory = true;
					break;
				}
			}

			if (nodes[i].mandatory)
				break;
		}

		// availableInputs: resolvedName -> index of the mandatory pass that reads it
		std::unordered_map<std::string, size_t> availableInputs;

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
					nodes[it->second].dependsOn.push_back((size_t)i);
					availableInputs.erase(it);
				}
			}

			// Register all ports as pending: explicit reads and write-passthroughs.
			// An earlier pass that writes the same resource name will satisfy the passthrough.
			for (const auto& port : pass.Ports)
				availableInputs.emplace(port.ResolvedName, (size_t)i);
		}

		return nodes;
	}

	// Phase 3: Topological sort, scheduled as-late-as-possible (ALAP).
	// Passes with deeper dependency chains are scheduled first so resources
	// have the shortest possible lifetime.
	std::vector<ResolvedPass> RenderProgramBuilder::TopoSortALAP(const std::vector<PassNode>& nodes, const std::vector<ResolvedPass>& flat) const
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

	// Bindless slot assignment: walk each pass's ports in declaration order (the same order they
	// were pushed in FlattenFeatures - ImportResource/ExportResource/MapResource/MapGlobal calls,
	// in the order the pass author wrote them) and assign the next free textureIndices[]/
	// bufferAddresses[] slot to each texture-/buffer-shaped port. Each shader stage is a separately
	// compiled SPIR-V module, so a stage that never includes the shared BINDLESS_PUSH_CONSTANTS macro
	// (e.g. a vertex shader with no bindless needs while the fragment shader has some) simply won't
	// report a BindlessLayout - reflecting only one stage would silently miss slots another stage
	// needs. So every stage is reflected and merged: the layout is present if any stage declares it,
	// and PushConstantSize is the max across stages in case one stage's push-constant block has
	// trailing fields beyond the shared macro that another stage's doesn't. Offsets/counts still come
	// from the one shared macro and so are expected to agree wherever more than one stage declares
	// them; a mismatch is logged rather than silently picking one. Passes whose shaders don't use the
	// macro in any stage are left with empty slots and PushConstantSize == 0.
	void RenderProgramBuilder::AssignBindlessSlots(std::vector<ResolvedPass>& passes) const
	{
		for (ResolvedPass& pass : passes)
		{
			if (pass.Shaders.empty())
				continue;

			ShaderBindlessLayout layout;
			for (const auto& [shaderPath, shaderStage] : pass.Shaders)
			{
				const PolyID                shaderID    = ShaderManager::CreateShader(shaderPath, shaderStage);
				const ShaderData&           shaderData  = ShaderManager::GetShader(shaderID);
				const ShaderBindlessLayout& stageLayout = shaderData.Reflection.BindlessLayout;

				if (stageLayout.HasTextureSlots)
				{
					if (layout.HasTextureSlots && (layout.TextureSlotsOffset != stageLayout.TextureSlotsOffset ||
					                               layout.TextureSlotCount != stageLayout.TextureSlotCount))
						POLY_CORE_ERROR("Pass '{}' shader stages disagree on textureIndices[] layout ({} slots @ offset {} vs {} slots @ offset {}).",
						                pass.Name, layout.TextureSlotCount, layout.TextureSlotsOffset,
						                stageLayout.TextureSlotCount, stageLayout.TextureSlotsOffset);

					layout.HasTextureSlots    = true;
					layout.TextureSlotsOffset = stageLayout.TextureSlotsOffset;
					layout.TextureSlotCount   = stageLayout.TextureSlotCount;
				}

				if (stageLayout.HasBufferSlots)
				{
					if (layout.HasBufferSlots && (layout.BufferSlotsOffset != stageLayout.BufferSlotsOffset ||
					                              layout.BufferSlotCount != stageLayout.BufferSlotCount))
						POLY_CORE_ERROR("Pass '{}' shader stages disagree on bufferAddresses[] layout ({} slots @ offset {} vs {} slots @ offset {}).",
						                pass.Name, layout.BufferSlotCount, layout.BufferSlotsOffset,
						                stageLayout.BufferSlotCount, stageLayout.BufferSlotsOffset);

					layout.HasBufferSlots    = true;
					layout.BufferSlotsOffset = stageLayout.BufferSlotsOffset;
					layout.BufferSlotCount   = stageLayout.BufferSlotCount;
				}

				for (const auto& pushConstant : shaderData.Reflection.PushConstants)
					pass.PushConstantSize = std::max(pass.PushConstantSize, pushConstant.Offset + pushConstant.Size);
			}

			if (!layout.HasTextureSlots && !layout.HasBufferSlots)
				continue;

			pass.BufferSlotsOffset  = layout.BufferSlotsOffset;
			pass.TextureSlotsOffset = layout.TextureSlotsOffset;

			for (const ResolvedPort& port : pass.Ports)
			{
				if (IsTextureResourceType(port.ResourceType))
				{
					if (!layout.HasTextureSlots)
						continue;

					const uint32 slot = static_cast<uint32>(pass.TextureSlots.size());
					if (slot >= layout.TextureSlotCount)
					{
						POLY_CORE_ERROR("Pass '{}' has more texture resources ({}) than its shader's textureIndices[] array supports ({}); '{}' was not assigned a slot.",
						                pass.Name, slot + 1, layout.TextureSlotCount, port.ResolvedName);
						continue;
					}

					pass.TextureSlots.push_back({slot, port.ResolvedName});
				}
				else if (IsBufferResourceType(port.ResourceType))
				{
					if (!layout.HasBufferSlots)
						continue;

					const uint32 slot = static_cast<uint32>(pass.BufferSlots.size());
					if (slot >= layout.BufferSlotCount)
					{
						POLY_CORE_ERROR("Pass '{}' has more buffer resources ({}) than its shader's bufferAddresses[] array supports ({}); '{}' was not assigned a slot.",
						                pass.Name, slot + 1, layout.BufferSlotCount, port.ResolvedName);
						continue;
					}

					pass.BufferSlots.push_back({slot, port.ResolvedName});
				}
			}
		}
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

	SyncPlan RenderProgramBuilder::PlanSynchronization(const std::vector<ResolvedPass>& passes) const
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

				const bool isAttachmentSemantic = IsAttachmentSemanticPort(port.ResolvedName);
				const bool isTexture            = isAttachmentSemantic || IsTextureResourceType(port.ResourceType);

				if (!isAttachmentSemantic && port.ResourceType == EResourceType::None)
					POLY_CORE_WARN("Pass '{}' port '{}' has no resource type; it will not be synchronized.", pass.Name,
					               port.ResolvedName);

				const ResourceUsage needed = DeriveResourceUsage(port.ResourceType, port.IsWrite, port.ResolvedName, passStages);

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

		// Closing transitions: for each resolved name with a declared WithFinalState(), leave it in that
		// state after its last use in the program (e.g. "$Color" -> Present)
		for (const auto& [resolvedName, finalState] : m_FinalStates)
		{
			auto it = state.find(resolvedName);
			if (it == state.end() || !it->second.IsTracked)
			{
				POLY_CORE_WARN("WithFinalState was declared for '{}', but no pass in this program touches it.", resolvedName);
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

	Ref<RenderProgram> RenderProgramBuilder::Build()
	{
		auto flat   = FlattenFeatures();
		auto nodes  = BuildDAG(flat);
		auto sorted = TopoSortALAP(nodes, flat);
		AssignBindlessSlots(sorted);
		auto syncPlan = PlanSynchronization(sorted);
		return CreateRef<RenderProgram>(std::move(sorted), std::move(syncPlan));
	}
} // namespace Poly
