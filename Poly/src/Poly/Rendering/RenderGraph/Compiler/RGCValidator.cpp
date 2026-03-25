#include "Poly/Rendering/RenderGraph/Compiler/RGCValidator.h"

#include "Poly/Core/Utils/DirectedGraph.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/EdgeData.h"
#include "Poly/Rendering/RenderGraph/Pass.h"

namespace Poly
{
	bool RGCValidator::Execute(RGCContext& ctx)
	{
		// Checks if the current graph is valid to render to a swap chain for.
		// To do this check if there are any resources marked for output and
		// that all inputs are linked

		bool validGraph = true;

		if (ctx.RenderGraph.m_Outputs.empty())
		{
			// TODO: Validate if an output always need to exist. For Compute only or render to texture it wouldn't be needed
			POLY_CORE_ERROR("Rendergraph must have atleast one resource marked as output");
			return false;
		}

		for (const CompiledPass& passData : ctx.CompiledGraph.CompiledPasses)
		{
			const std::vector<const PassField*> inputs         = passData.Reflection.GetFieldsFiltered(FFieldVisibility::INPUT, FResourceBindPoint::INTERNAL_USE);
			const auto&                         incommingEdges = ctx.RenderGraph.m_pGraph->GetNode(passData.GraphNodeIndex)->GetIncommingEdges();

			for (const PassField* input : inputs)
			{
				PassResID dstGUID(passData.pPass->GetName(), input->GetName());
				bool      valid = false;

				// Current implementation of ArrayInputs does not require a resource to be set
				if (input->IsArray())
					continue;

				// If field is passthrough (INPUT + OUTPUT), then it is valid to not have incomming edges
				if (BitsSet(input->GetVisibility(), FFieldVisibility::OUTPUT))
					continue;

				for (uint32 edgeID : incommingEdges)
				{
					const EdgeData& edgeData = ctx.RenderGraph.m_Edges.at(edgeID);

					if (!edgeData.IsDataDependency())
						continue;

					if (edgeData.GetDstPassRes() == dstGUID)
					{
						valid = true;
						break;
					}
				}

				if (!valid)
				{
					validGraph = false;
					POLY_CORE_WARN("Input resource {} did not have a link to it", dstGUID.GetFullName());
				}
			}
		}

		return validGraph;
	}
} // namespace Poly