#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceRegister.h"

#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/ResourceCache.h"
#include "Poly/Rendering/RenderGraph/Pass.h"
#include "Poly/Core/Utils/DirectedGraph.h"

namespace Poly
{
	void RGCResourceRegister::Execute(RGCContext& ctx)
	{
		if (!ctx.pResourceCache)
		{
			POLY_CORE_ERROR("No resource cache has been created/provided for the Render Graph Compiler");
			return;
		}

		RegisterExternalResources(ctx);
		RegisterResources(ctx);

	}

	void RGCResourceRegister::RegisterExternalResources(RGCContext& ctx)
	{
		for (const auto& [resGUID, resInfo] : ctx.RenderGraph.m_ExternalResources)
			ctx.pResourceCache->RegisterExternalResource(resGUID, resInfo);
	}

	void RGCResourceRegister::RegisterResources(RGCContext& ctx)
	{
		// 1. Go through passes from first to last
		//  a. For each pass, alias inputs (in order to handle first pass external inputs)
		//  b. Register outputs which are not aliased
		// Order must be from first to last, so that outputs are registered before aliases are checked for connected passes.
		for (auto& passData : ctx.CompiledGraph.CompiledPasses)
		{
			AliasInputs(ctx, passData);
			RegisterOutputs(ctx, passData);
		}
	}

	void RGCResourceRegister::AliasInputs(RGCContext& ctx, CompiledPass& compiledPass)
	{
		auto inputs = compiledPass.Reflection.GetFields(FFieldVisibility::INPUT);
		for (auto& input : inputs)
		{
			ResourceGUID resourceGUID(compiledPass.pPass->GetName(), input->GetName());
			ResourceGUID aliasGUID = GetAliasedResourceGUID(ctx, compiledPass, resourceGUID);

			// Passthroughs (INPUT + OUTPUT) with no provided input are not aliased - they create the resource (and are registed as an Output)
			if (aliasGUID == ResourceGUID::Invalid() && BitsSet(input->GetVisibility(), FFieldVisibility::OUTPUT))
				continue;

			if (aliasGUID == ResourceGUID::Invalid() && !BitsSet(input->GetBindPoint(), FResourceBindPoint::INTERNAL_USE))
			{
				POLY_CORE_ERROR("Tried to alias resource '{}', but no connection has been made. If a resource is not marked as INTERNAL_USE,"
								"then it is expected that there is a connection made in the render graph.", resourceGUID.GetFullName());
				return;
			}

			// TODO: Remove timepoint from registration step - is not valid after sync is added
			ctx.pResourceCache->RegisterResource(resourceGUID, 0, *input, aliasGUID);
		}
	}

	void RGCResourceRegister::RegisterOutputs(RGCContext & ctx, CompiledPass & compiledPass)
	{
		auto outputs = compiledPass.Reflection.GetFields(FFieldVisibility::OUTPUT);
		for (auto& output : outputs)
		{
			// Check if aliased, register if it isn't
			ResourceGUID resourceGUID(compiledPass.pPass->GetName(), output->GetName());
			if (!ctx.pResourceCache->IsResourceRegistered(resourceGUID))
			{
				// TODO: Remove timepoint from registration step - is not valid after sync is added
				ctx.pResourceCache->RegisterResource(resourceGUID, 0, *output);
			}
		}
	}

	ResourceGUID RGCResourceRegister::GetAliasedResourceGUID(RGCContext& ctx, CompiledPass& compiledPass, ResourceGUID resourceGUID)
	{
		const auto& incommingEdges = ctx.RenderGraph.m_pGraph->GetNode(compiledPass.GraphNodeIndex)->GetIncommingEdges();
		for (auto edgeID : incommingEdges)
		{
			auto& edgeData = ctx.RenderGraph.m_Edges[edgeID];
			if (edgeData.Dst == resourceGUID)
			{
				return edgeData.Src;
			}
		}

		// If incomming didn't get any match, check for externals
		const auto& externals = compiledPass.pPass->GetExternalResources();
		for (auto& external : externals)
		{
			if (external.DstGUID == resourceGUID)
			{
				return ResourceGUID(external.SrcGUID);
			}
		}

		return ResourceGUID::Invalid();
	}
}