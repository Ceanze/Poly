#include "Poly/Rendering/RenderGraph/Compiler/RGCSynchroniser.h"

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Rendering/RenderGraph/Reflection/PassField.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/Pass.h"
#include "Poly/Rendering/RenderGraph/RenderPass.h"
#include "Poly/Rendering/RenderGraph/SyncPass.h"
#include "Poly/Rendering/RenderGraph/ResourceCache.h"
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Core/Utils/DirectedGraph.h"

namespace Poly
{
	// ---------------------------------------------------------------------------
	// Local structs
	// ---------------------------------------------------------------------------

	struct ResourceState
	{
		ETextureLayout	Layout		= ETextureLayout::UNDEFINED;
		FAccessFlag		AccessMask	= FAccessFlag::NONE;
		FPipelineStage	Stage		= FPipelineStage::NONE;
	};

	struct ResourceUsage
	{
		std::string		Name		= "";
		ETextureLayout	Layout		= ETextureLayout::UNDEFINED;
		FAccessFlag		AccessMask	= FAccessFlag::NONE;
		FPipelineStage	Stage		= FPipelineStage::NONE;
	};

	struct SyncContext
	{
		// Pre-computed per-pass usages
		std::unordered_map<uint32, std::vector<ResourceUsage>> PassInvalidates;  // inputs per node
		std::unordered_map<uint32, std::vector<ResourceUsage>> PassFlushes;      // outputs per node

		// Live state per physical resource index (ResourceCache::GetResourceIndex)
		// Absence in map = {UNDEFINED, NONE, NONE} (resource not yet touched)
		std::unordered_map<uint32, ResourceState> CurrentResourceState;
	};

	struct SyncPassData
	{
		Ref<SyncPass>               pSyncPass;
		std::unordered_set<uint32>  brokenEdgeIDs;
	};

	// ---------------------------------------------------------------------------
	// Helper functions
	// ---------------------------------------------------------------------------

	static FAccessFlag GetAccessFlag(FResourceBindPoint bindPoint, bool isReadOnly)
	{
		if (BitsSet(bindPoint, FResourceBindPoint::COLOR_ATTACHMENT))
			return isReadOnly ? FAccessFlag::COLOR_ATTACHMENT_READ : FAccessFlag::COLOR_ATTACHMENT_WRITE;
		if (BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL))
			return isReadOnly ? FAccessFlag::DEPTH_STENCIL_ATTACHMENT_READ : FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE;
		if (BitsSet(bindPoint, FResourceBindPoint::UNIFORM))
			return FAccessFlag::UNIFORM_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::SAMPLER))
			return FAccessFlag::SHADER_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::STORAGE))
			return isReadOnly ? FAccessFlag::SHADER_READ : FAccessFlag::SHADER_WRITE;
		if (BitsSet(bindPoint, FResourceBindPoint::VERTEX))
			return FAccessFlag::VERTEX_ATTRIBUTE_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::INDEX))
			return FAccessFlag::INDEX_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::INDIRECT))
			return FAccessFlag::INDIRECT_COMMAND_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::INPUT_ATTACHMENT))
			return FAccessFlag::INPUT_ATTACHMENT_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::EXTERNAL))
			return FAccessFlag::SHADER_READ;

		return FAccessFlag::NONE;
	}

	static FPipelineStage GetPipelineStage(FResourceBindPoint bindPoint, bool isReadOnly)
	{
		if (BitsSet(bindPoint, FResourceBindPoint::COLOR_ATTACHMENT))
			return FPipelineStage::COLOR_ATTACHMENT_OUTPUT;
		if (BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL))
			return isReadOnly ? FPipelineStage::EARLY_FRAGMENT_TEST : FPipelineStage::LATE_FRAGMENT_TEST;
		if (BitsSet(bindPoint, FResourceBindPoint::UNIFORM))
			return FPipelineStage::VERTEX_SHADER | FPipelineStage::FRAGMENT_SHADER;
		if (BitsSet(bindPoint, FResourceBindPoint::SAMPLER))
			return FPipelineStage::FRAGMENT_SHADER;
		if (BitsSet(bindPoint, FResourceBindPoint::STORAGE))
			return FPipelineStage::ALL_GRAPHICS;
		if (BitsSet(bindPoint, FResourceBindPoint::VERTEX))
			return FPipelineStage::VERTEX_SHADER;
		if (BitsSet(bindPoint, FResourceBindPoint::INDEX))
			return FPipelineStage::BOTTOM_OF_PIPE;
		if (BitsSet(bindPoint, FResourceBindPoint::INDIRECT))
			return FPipelineStage::DRAW_INDIRECT;
		if (BitsSet(bindPoint, FResourceBindPoint::INPUT_ATTACHMENT))
			return FPipelineStage::FRAGMENT_SHADER;
		if (BitsSet(bindPoint, FResourceBindPoint::EXTERNAL))
			return FPipelineStage::FRAGMENT_SHADER;

		return FPipelineStage::NONE;
	}

	static ETextureLayout GetTextureLayout(FResourceBindPoint bindPoint, bool isReadOnly)
	{
		if (BitsSet(bindPoint, FResourceBindPoint::COLOR_ATTACHMENT))
			return isReadOnly ? ETextureLayout::SHADER_READ_ONLY_OPTIMAL : ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;
		if (BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL))
			return isReadOnly ? ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL : ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		if (BitsSet(bindPoint, FResourceBindPoint::SAMPLER))
			return ETextureLayout::SHADER_READ_ONLY_OPTIMAL;
		if (BitsSet(bindPoint, FResourceBindPoint::INPUT_ATTACHMENT))
			return ETextureLayout::SHADER_READ_ONLY_OPTIMAL;

		// STORAGE and buffer-like bind points have no layout concept
		return ETextureLayout::UNDEFINED;
	}

	static bool HasWriteBits(FAccessFlag mask)
	{
		return BitsSet(mask, FAccessFlag::COLOR_ATTACHMENT_WRITE)
			|| BitsSet(mask, FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE)
			|| BitsSet(mask, FAccessFlag::SHADER_WRITE);
	}

	// ---------------------------------------------------------------------------
	// Phase 1: InitialiseResourceStates
	// ---------------------------------------------------------------------------

	void RGCSynchroniser::InitialiseResourceStates(RGCContext& ctx, SyncContext& syncCtx)
	{
		for (const auto& [guid, info] : ctx.RenderGraph.m_ExternalResources)
		{
			uint32 idx = ctx.pResourceCache->GetResourceIndex(guid);
			if (idx == UINT32_MAX)
				continue;

			syncCtx.CurrentResourceState[idx] = ResourceState{
				ETextureLayout::SHADER_READ_ONLY_OPTIMAL,
				FAccessFlag::SHADER_READ,
				FPipelineStage::FRAGMENT_SHADER
			};
		}
	}

	// ---------------------------------------------------------------------------
	// Phase 2: SetupPassUsages
	// ---------------------------------------------------------------------------

	void RGCSynchroniser::SetupPassUsages(RGCContext& ctx, SyncContext& syncCtx)
	{
		for (const auto& compiledPass : ctx.CompiledGraph.CompiledPasses)
		{
			const uint32 nodeIndex = compiledPass.GraphNodeIndex;
			const std::string& passName = compiledPass.pPass->GetName();

			// --- Inputs → PassInvalidates ---
			auto inputs = compiledPass.Reflection.GetFieldsFiltered(FFieldVisibility::INPUT, FResourceBindPoint::INTERNAL_USE);
			for (const PassField* field : inputs)
			{
				ETextureLayout layout = field->GetTextureLayout();
				if (layout == ETextureLayout::UNDEFINED)
					layout = GetTextureLayout(field->GetBindPoint(), true);

				ResourceUsage usage;
				usage.Name       = field->GetName();
				usage.Layout     = layout;
				usage.AccessMask = GetAccessFlag(field->GetBindPoint(), true);
				usage.Stage      = GetPipelineStage(field->GetBindPoint(), true);
				syncCtx.PassInvalidates[nodeIndex].push_back(usage);
			}

			// --- Outputs → PassFlushes ---
			auto outputs = compiledPass.Reflection.GetFieldsFiltered(FFieldVisibility::OUTPUT, FResourceBindPoint::INTERNAL_USE);
			uint32 attachmentIndex = 0;
			for (const PassField* field : outputs)
			{
				ETextureLayout layout = field->GetTextureLayout();
				if (layout == ETextureLayout::UNDEFINED)
					layout = GetTextureLayout(field->GetBindPoint(), false);

				ResourceUsage usage;
				usage.Name       = field->GetName();
				usage.Layout     = layout;
				usage.AccessMask = GetAccessFlag(field->GetBindPoint(), false);
				usage.Stage      = GetPipelineStage(field->GetBindPoint(), false);
				syncCtx.PassFlushes[nodeIndex].push_back(usage);

				// Register render pass attachments — default initial layout to UNDEFINED (clear/discard)
				if (compiledPass.pPass->GetPassType() == Pass::Type::RENDER && layout != ETextureLayout::UNDEFINED)
				{
					auto* pRenderPass = static_cast<RenderPass*>(compiledPass.pPass.get());
					pRenderPass->AddAttachment(field->GetName(), layout, attachmentIndex++, field->GetFormat());
					pRenderPass->SetAttachmentInital(field->GetName(), ETextureLayout::UNDEFINED);
				}
			}
		}
	}

	// ---------------------------------------------------------------------------
	// Phase 3: BuildSyncPasses
	// ---------------------------------------------------------------------------

	std::vector<SyncPassData> RGCSynchroniser::BuildSyncPasses(RGCContext& ctx, SyncContext& syncCtx)
	{
		std::vector<SyncPassData> addedSyncPasses;

		for (const auto& compiledPass : ctx.CompiledGraph.CompiledPasses)
		{
			const uint32 nodeIndex  = compiledPass.GraphNodeIndex;
			const std::string& passName = compiledPass.pPass->GetName();
			const bool isRenderPass = compiledPass.pPass->GetPassType() == Pass::Type::RENDER;

			Ref<SyncPass> pSyncPass = nullptr;
			std::unordered_set<uint32> brokenEdges;

			// ----------------------------------------------------------------
			// Step A — Determine barriers for inputs
			// ----------------------------------------------------------------
			const auto invalidatesIt = syncCtx.PassInvalidates.find(nodeIndex);
			if (invalidatesIt != syncCtx.PassInvalidates.end())
			{
				for (const ResourceUsage& usage : invalidatesIt->second)
				{
					ResourceGUID dstGUID(passName, usage.Name);
					uint32 idx = ctx.pResourceCache->GetResourceIndex(dstGUID);
					if (idx == UINT32_MAX)
						continue;

					ResourceState current = {};
					auto stateIt = syncCtx.CurrentResourceState.find(idx);
					if (stateIt != syncCtx.CurrentResourceState.end())
						current = stateIt->second;

					bool needsBarrier = false;
					FAccessFlag srcAccess = FAccessFlag::NONE;
					FAccessFlag dstAccess = usage.AccessMask;

					if (usage.Layout != ETextureLayout::UNDEFINED) // Texture
					{
						bool priorWrite   = HasWriteBits(current.AccessMask);
						bool neededWrite  = HasWriteBits(usage.AccessMask);
						bool layoutChange = current.Layout != usage.Layout;
						needsBarrier = priorWrite || neededWrite || layoutChange;

						if (needsBarrier)
						{
							srcAccess = priorWrite ? current.AccessMask : FAccessFlag::NONE;

							const std::string syncPassName = "Presync for " + passName;
							if (!pSyncPass)
								pSyncPass = SyncPass::Create(syncPassName);

							SyncPass::SyncData syncData = {};
							syncData.Type             = SyncPass::SyncType::TEXTURE;
							syncData.ResourceName     = usage.Name;
							syncData.SrcLayout        = current.Layout;
							syncData.DstLayout        = usage.Layout;
							syncData.SrcAccessFlag    = srcAccess;
							syncData.DstAccessFlag    = dstAccess;
							syncData.SrcPipelineStage = current.Stage;
							syncData.DstPipelineStage = usage.Stage;
							pSyncPass->AddSyncData(syncData);

							ctx.pResourceCache->RegisterSyncResource({ syncPassName, usage.Name }, dstGUID);

							// Find incoming edge for this resource and mark it broken
							const auto& incomingEdges = ctx.RenderGraph.m_pGraph->GetNode(nodeIndex)->GetIncommingEdges();
							for (uint32 edgeID : incomingEdges)
							{
								const auto& edge = ctx.RenderGraph.m_Edges[edgeID];
								if (edge.Dst.GetResourceName() == usage.Name)
								{
									brokenEdges.insert(edgeID);
									break;
								}
							}

							// If render pass: override initial layout to post-transition layout
							if (isRenderPass)
							{
								auto* pRenderPass = static_cast<RenderPass*>(compiledPass.pPass.get());
								pRenderPass->SetAttachmentInital(usage.Name, usage.Layout);
							}
						}
					}
					else // Buffer
					{
						bool priorWrite  = HasWriteBits(current.AccessMask);
						bool neededWrite = HasWriteBits(usage.AccessMask);
						needsBarrier = priorWrite || neededWrite;

						if (needsBarrier)
						{
							srcAccess = priorWrite ? current.AccessMask : FAccessFlag::NONE;

							const std::string syncPassName = "Presync for " + passName;
							if (!pSyncPass)
								pSyncPass = SyncPass::Create(syncPassName);

							SyncPass::SyncData syncData = {};
							syncData.Type             = SyncPass::SyncType::BUFFER;
							syncData.ResourceName     = usage.Name;
							syncData.SrcLayout        = ETextureLayout::UNDEFINED;
							syncData.DstLayout        = ETextureLayout::UNDEFINED;
							syncData.SrcAccessFlag    = srcAccess;
							syncData.DstAccessFlag    = dstAccess;
							syncData.SrcPipelineStage = current.Stage;
							syncData.DstPipelineStage = usage.Stage;
							pSyncPass->AddSyncData(syncData);

							ctx.pResourceCache->RegisterSyncResource({ syncPassName, usage.Name }, dstGUID);

							const auto& incomingEdges = ctx.RenderGraph.m_pGraph->GetNode(nodeIndex)->GetIncommingEdges();
							for (uint32 edgeID : incomingEdges)
							{
								const auto& edge = ctx.RenderGraph.m_Edges[edgeID];
								if (edge.Dst.GetResourceName() == usage.Name)
								{
									brokenEdges.insert(edgeID);
									break;
								}
							}
						}
					}

					// Update current state after processing this input
					syncCtx.CurrentResourceState[idx] = ResourceState{ usage.Layout, usage.AccessMask, usage.Stage };
				}
			}

			// ----------------------------------------------------------------
			// Step B — Write back output state (post-pass)
			// ----------------------------------------------------------------
			const auto flushesIt = syncCtx.PassFlushes.find(nodeIndex);
			if (flushesIt != syncCtx.PassFlushes.end())
			{
				for (const ResourceUsage& usage : flushesIt->second)
				{
					ResourceGUID outputGUID(passName, usage.Name);
					uint32 idx = ctx.pResourceCache->GetResourceIndex(outputGUID);
					if (idx == UINT32_MAX)
						continue;

					syncCtx.CurrentResourceState[idx] = ResourceState{ usage.Layout, usage.AccessMask, usage.Stage };
				}
			}

			// ----------------------------------------------------------------
			// Step C — Collect
			// ----------------------------------------------------------------
			if (pSyncPass)
			{
				SyncPassData data;
				data.pSyncPass     = pSyncPass;
				data.brokenEdgeIDs = brokenEdges;
				addedSyncPasses.push_back(std::move(data));
			}
		}

		return addedSyncPasses;
	}

	// ---------------------------------------------------------------------------
	// Phase 4: SetOutputLayouts
	// ---------------------------------------------------------------------------

	void RGCSynchroniser::SetOutputLayouts(RGCContext& ctx)
	{
		if (ctx.RenderGraph.m_Outputs.empty())
			return;

		const auto& compiledPasses = ctx.CompiledGraph.CompiledPasses;

		if (ctx.RenderGraph.m_Outputs.size() == 1)
		{
			const auto& output = *ctx.RenderGraph.m_Outputs.begin();

			// Find the compiled pass for this output
			auto lastPassIt = std::find_if(compiledPasses.begin(), compiledPasses.end(),
				[&](const CompiledPass& cp) { return cp.GraphNodeIndex == output.NodeID; });

			if (lastPassIt != compiledPasses.end() && lastPassIt->pPass->GetPassType() == Pass::Type::RENDER)
			{
				auto* pRenderPass = static_cast<RenderPass*>(lastPassIt->pPass.get());
				pRenderPass->SetAttachmentFinal(output.ResourceName, ETextureLayout::PRESENT);
			}

			// Find the first render pass with a COLOR_ATTACHMENT_OPTIMAL attachment and clear it
			bool set = false;
			for (const auto& compiledPass : compiledPasses)
			{
				if (set)
					break;
				if (compiledPass.pPass->GetPassType() != Pass::Type::RENDER)
					continue;

				auto* pRenderPass = static_cast<RenderPass*>(compiledPass.pPass.get());
				for (const auto& attachment : pRenderPass->GetAttachments())
				{
					if (attachment.UsedLayout == ETextureLayout::COLOR_ATTACHMENT_OPTIMAL)
					{
						pRenderPass->SetAttachmentInital(attachment.Name, ETextureLayout::UNDEFINED);
						set = true;
						break;
					}
				}
			}
		}
		else
		{
			// Multiple outputs — find the last one by execution position
			uint32 lastOutputIndex = 0;
			std::string resourceName = "";

			for (const auto& output : ctx.RenderGraph.m_Outputs)
			{
				for (uint32 i = 0; i < compiledPasses.size(); i++)
				{
					if (compiledPasses[i].GraphNodeIndex == output.NodeID && i >= lastOutputIndex)
					{
						lastOutputIndex = i;
						resourceName    = output.ResourceName;
					}
				}
			}

			if (!resourceName.empty() && compiledPasses[lastOutputIndex].pPass->GetPassType() == Pass::Type::RENDER)
			{
				auto* pRenderPass = static_cast<RenderPass*>(compiledPasses[lastOutputIndex].pPass.get());
				pRenderPass->SetAttachmentFinal(resourceName, ETextureLayout::PRESENT);
			}
		}
	}

	// ---------------------------------------------------------------------------
	// Phase 5: InsertSyncPasses
	// ---------------------------------------------------------------------------

	void RGCSynchroniser::InsertSyncPasses(RGCContext& ctx, const std::vector<SyncPassData>& syncPasses)
	{
		for (const auto& syncPassData : syncPasses)
		{
			const std::string& syncPassName = syncPassData.pSyncPass->GetName();
			ctx.RenderGraph.AddPass(syncPassData.pSyncPass, syncPassName);

			for (uint32 edgeID : syncPassData.brokenEdgeIDs)
			{
				const ResourceGUID srcGUID = ctx.RenderGraph.GetEdgeData(edgeID).Src;
				const ResourceGUID dstGUID = ctx.RenderGraph.GetEdgeData(edgeID).Dst;

				ctx.RenderGraph.RemoveLink(srcGUID, dstGUID);
				ctx.RenderGraph.AddLink(srcGUID, { syncPassName, dstGUID.GetResourceName() });
				ctx.RenderGraph.AddLink({ syncPassName, dstGUID.GetResourceName() }, dstGUID);
			}
		}

		if (!syncPasses.empty())
			ctx.IsGraphDirty = true;
	}

	// ---------------------------------------------------------------------------
	// Orchestrator
	// ---------------------------------------------------------------------------

	void RGCSynchroniser::Execute(RGCContext& ctx)
	{
		SyncContext syncCtx;
		InitialiseResourceStates(ctx, syncCtx);
		SetupPassUsages(ctx, syncCtx);
		auto syncPasses = BuildSyncPasses(ctx, syncCtx);
		SetOutputLayouts(ctx);
		InsertSyncPasses(ctx, syncPasses);
	}
}
