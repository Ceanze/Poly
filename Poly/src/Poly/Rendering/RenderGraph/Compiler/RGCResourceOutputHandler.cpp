#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceOutputHandler.h"

#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/ResourceCache.h"
#include "Poly/Rendering/RenderGraph/Pass.h"

namespace Poly
{
	void RGCResourceOutputHandler::Execute(RGCContext& ctx)
	{
		if (!ctx.pResourceCache)
		{
			POLY_VALIDATE(false, "No resource cache has been created/provided for the Render Graph Compiler");
			return;
		}

		const auto& markedOutputs = ctx.RenderGraph.m_Outputs;
		
		for (const auto& markedOutput : markedOutputs)
		{
			std::vector<CompiledPass>& compiledPasses = ctx.CompiledGraph.CompiledPasses;
			auto itr = std::find_if(compiledPasses.begin(), compiledPasses.end(), [nodeIdx = markedOutput.NodeID](const CompiledPass& compiledPass) { return compiledPass.GraphNodeIndex == nodeIdx; });
			if (itr != compiledPasses.end())
			{
				auto& field = itr->Reflection.GetField(markedOutput.ResourceName);
				ResourceGUID resourceGUID(itr->pPass->GetName(), markedOutput.ResourceName);
				ctx.pResourceCache->MarkOutput(resourceGUID, field);
				field.Format(EFormat::B8G8R8A8_UNORM);	
			}
		}
	}
}