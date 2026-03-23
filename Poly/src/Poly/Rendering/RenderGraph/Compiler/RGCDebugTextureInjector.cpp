#include "polypch.h"
#include "RGCDebugTextureInjector.h"

#include "RGCContext.h"
#include "RGCSyncTypes.h"

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Rendering/RenderGraph/Pass.h"
#include "Poly/Rendering/RenderGraph/SyncPass.h"
#include "Poly/Rendering/RenderGraph/ResourceGUID.h"
#include "Poly/Rendering/RenderGraph/ResourceCache.h"
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/Reflection/PassField.h"

#include <unordered_set>

namespace Poly
{
	void RGCDebugTextureInjector::Execute(RGCContext& ctx)
	{
		if (!ctx.DefaultParams.EnableDebugTextures)
			return;

		// 1. Find the debug consumer pass (lowest execution index)
		int32 debugConsumerIndex = -1;
		std::string debugConsumerPassName;

		for (size_t i = 0; i < ctx.CompiledGraph.CompiledPasses.size(); ++i)
		{
			if (ctx.CompiledGraph.CompiledPasses[i].pPass->IsDebugConsumer())
			{
				debugConsumerIndex = (int32)i;
				debugConsumerPassName = ctx.CompiledGraph.CompiledPasses[i].pPass->GetName();
				break;
			}
		}

		if (debugConsumerIndex < 0)
			return;

		// Build set of graph output GUIDs so we can skip them (backbuffer etc.)
		// TODO: Support backbuffer? - Requires synchronisation after the ImGui pass to put in PRESENT mode
		std::unordered_set<PassResID> outputGUIDs;
		for (const auto& output : ctx.RenderGraph.m_Outputs)
		{
			const auto& pPass = ctx.RenderGraph.m_Passes.at(output.NodeID);
			PassResID canonicalGUID = ctx.pResourceCache->GetCanonicalGUID(PassResID(pPass->GetName(), output.ResourceID.GetName()));
			outputGUIDs.insert(canonicalGUID);
		}

		// 2. Collect candidate texture resources from passes before the debug consumer
		struct PendingTransition
		{
			PassResID	SourceGUID;
			PassResID	CanonicalGUID;
			ResourceState	State;
			bool			IsDepth;
		};

		std::vector<PendingTransition> pendingTransitions;

		for (uint32 i = 0; i < (uint32)debugConsumerIndex; ++i)
		{
			const auto& compiledPass = ctx.CompiledGraph.CompiledPasses[i];
			const std::string& passName = compiledPass.pPass->GetName();

			auto outputs = compiledPass.Reflection.GetFieldsFiltered(FFieldVisibility::OUTPUT, FResourceBindPoint::INTERNAL_USE);
			for (const PassField* pField : outputs)
			{
				if (pField->GetType() != PassField::EType::Texture)
					continue;

				PassResID sourceGUID(passName, pField->GetName());

				// Skip backbuffer / graph outputs
				if (outputGUIDs.count(sourceGUID) > 0)
					continue;

				PassResID canonicalGUID = ctx.pResourceCache->GetCanonicalGUID(sourceGUID);
				if (!canonicalGUID.HasResource())
					continue;

				auto stateIt = ctx.PostSyncResourceStates.find(canonicalGUID);
				ResourceState state = (stateIt != ctx.PostSyncResourceStates.end()) ? stateIt->second : ResourceState{};

				const bool isDepth = BitsSet(pField->GetBindPoint(), FResourceBindPoint::DEPTH_STENCIL);
				const ETextureLayout targetLayout = isDepth
					? ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL
					: ETextureLayout::SHADER_READ_ONLY_OPTIMAL;

				// Already in the desired read layout — no transition needed
				if (state.Layout == targetLayout)
					continue;

				pendingTransitions.push_back({ sourceGUID, canonicalGUID, state, isDepth });
			}
		}

		if (pendingTransitions.empty())
			return;

		// 4. Build a single "DebugTextureSync" SyncPass for all transitions
		const std::string syncPassName = "DebugTextureSync";
		auto pSyncPass = SyncPass::Create(syncPassName);

		std::unordered_set<std::string> writingPassNames;

		for (const auto& pt : pendingTransitions)
		{
			const ETextureLayout dstLayout = pt.IsDepth
				? ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL
				: ETextureLayout::SHADER_READ_ONLY_OPTIMAL;

			SyncPass::SyncData data = {};
			data.Type             = SyncPass::SyncType::TEXTURE;
			data.ResourceName     = pt.SourceGUID.GetResource().GetName();
			data.SrcLayout        = pt.State.Layout;
			data.DstLayout        = dstLayout;
			data.SrcAccessFlag    = pt.State.AccessMask;
			data.DstAccessFlag    = FAccessFlag::SHADER_READ;
			data.SrcPipelineStage = pt.State.Stage;
			data.DstPipelineStage = FPipelineStage::FRAGMENT_SHADER;
			pSyncPass->AddSyncData(data);

			ctx.pResourceCache->AddBindpoint(pt.SourceGUID, FResourceBindPoint::SHADER_READ);
			ctx.pResourceCache->RegisterSyncResource({ syncPassName, data.ResourceName }, pt.SourceGUID);
			ctx.DebugTextureGUIDs.push_back(pt.SourceGUID);

			writingPassNames.insert(pt.SourceGUID.GetPass().GetName());
		}

		// 5. Add sync pass to the render graph
		PassID syncPassID(syncPassName);
		ctx.RenderGraph.AddPass(pSyncPass, syncPassID);

		// 6. Wire execution-order dependencies (pass-name-only GUIDs = execution-order links)
		for (const auto& writingPassName : writingPassNames)
			ctx.RenderGraph.AddLink(PassID(writingPassName), syncPassID);
		ctx.RenderGraph.AddLink(syncPassID, PassID(debugConsumerPassName));

		// 7. Re-run compiler and validator to topologically re-sort the new pass
		ctx.IsGraphDirty = true;
	}
}
