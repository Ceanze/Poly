#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceRegister.h"

#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/ResourceCache.h"
#include "Poly/Rendering/RenderGraph/Pass.h"
#include "Poly/Rendering/RenderGraph/ExternalPass.h"
#include "Poly/Core/Utils/DirectedGraph.h"
#include "Poly/Rendering/RenderGraph/EdgeData.h"

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
		const auto* pExtPass = static_cast<const ExternalPass*>(
			ctx.RenderGraph.m_Passes[ctx.RenderGraph.m_ExternalPassNodeID].get());

		for (const auto& [guid, info] : pExtPass->GetResources())
			ctx.pResourceCache->RegisterExternalResource(guid, info);
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
			PassResID inputID(compiledPass.pPass->GetName(), input->GetName());
			PassResID aliasID = GetAliasedResourceGUID(ctx, compiledPass, inputID);

			// Passthroughs (INPUT + OUTPUT) with no provided input are not aliased - they create the resource (and are registed as an Output)
			if (aliasID == PassResID::Invalid() && BitsSet(input->GetVisibility(), FFieldVisibility::OUTPUT))
				continue;

			if (aliasID == PassResID::Invalid() && !BitsSet(input->GetBindPoint(), FResourceBindPoint::INTERNAL_USE))
			{
				POLY_CORE_ERROR("Tried to alias resource '{}', but no connection has been made. If a resource is not marked as INTERNAL_USE,"
								"then it is expected that there is a connection made in the render graph.", inputID.GetFullName());
				return;
			}

			// TODO: Remove timepoint from registration step - is not valid after sync is added
			ctx.pResourceCache->RegisterResource(inputID, 0, *input, aliasID);
		}
	}

	void RGCResourceRegister::RegisterOutputs(RGCContext & ctx, CompiledPass & compiledPass)
	{
		auto outputs = compiledPass.Reflection.GetFields(FFieldVisibility::OUTPUT);
		for (auto& output : outputs)
		{
			// Check if aliased, register if it isn't
			PassResID outputID(compiledPass.pPass->GetName(), output->GetName());
			if (!ctx.pResourceCache->IsResourceRegistered(outputID))
				ctx.pResourceCache->RegisterResource(outputID, 0, *output);
		}
	}

	PassResID RGCResourceRegister::GetAliasedResourceGUID(RGCContext& ctx, CompiledPass& compiledPass, const PassResID& inputID)
	{
		const auto& incommingEdges = ctx.RenderGraph.m_pGraph->GetNode(compiledPass.GraphNodeIndex)->GetIncommingEdges();
		for (auto edgeID : incommingEdges)
		{
			const EdgeData& edgeData = ctx.RenderGraph.m_Edges.at(edgeID);
			if (!edgeData.IsDataDependency())
				continue;

			if (edgeData.GetDstPassRes() == inputID)
			{
				return edgeData.GetSrcPassResOrExternal();
			}
		}

		return PassResID::Invalid();
	}
}