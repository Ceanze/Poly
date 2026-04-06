#include "Poly/Rendering/RenderGraph/Compiler/RGCCompiler.h"

#include "Poly/Core/Utils/DirectedGraphHelper.h"
#include "Poly/Rendering/RenderGraph/Compiler/CompiledGraph.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/Pass.h"

namespace Poly
{
	void RGCCompiler::Execute(RGCContext& ctx)
	{
		std::unordered_set<uint32> mandatoryNodes = GetMandatoryNodes(ctx);
		std::vector<uint32>        sortedNodes    = PruneAndSortGraph(ctx, mandatoryNodes);
		ctx.CompiledGraph                         = Compile(ctx, sortedNodes);
	}

	std::unordered_set<uint32> RGCCompiler::GetMandatoryNodes(RGCContext& ctx)
	{
		std::unordered_set<uint32> mandatoryNodes;

		// Seed with graph output nodes. PruneAndSortGraph will then do a backward traversal
		// from these seeds following all edge types (data and execution), which naturally includes
		// any pass connected, directly or transitively, to an output. Island connections are excluded.
		for (const auto& output : ctx.RenderGraph.m_Outputs)
			mandatoryNodes.insert(output.NodeID);

		// Also seed with passes explicitly marked as mandatory (e.g. mandatory passes
		// that have no swapchain output but must still be compiled and executed).
		for (uint32 nodeID : ctx.RenderGraph.m_MandatoryPasses)
			mandatoryNodes.insert(nodeID);

		return mandatoryNodes;
	}

	std::vector<uint32> RGCCompiler::PruneAndSortGraph(RGCContext& ctx, const std::unordered_set<uint32>& mandatoryPasses)
	{
		// Traverse graph in reverse for all mandatory passes to get all passes that will be used
		std::unordered_set<uint32> usedNodes;
		DirectedGraphHelper        graphHelper = DirectedGraphHelper(ctx.RenderGraph.m_pGraph, 0, DirectedGraphHelper::CreateFlags::REVERSE | DirectedGraphHelper::CreateFlags::IGNORE_VISITED);
		for (const auto& pass : mandatoryPasses)
		{
			graphHelper.Reset(pass);
			uint32 currNode = graphHelper.Traverse();
			while (currNode != DirectedGraphHelper::INVALID_ID)
			{
				usedNodes.insert(currNode);
				currNode = graphHelper.Traverse();
			}
		}

		std::vector<uint32>                                    nodesToRemove;
		const std::unordered_map<uint32, DirectedGraph::Node>& allNodes = ctx.RenderGraph.m_pGraph->GetAllNodes();
		for (const auto& [nodeIndex, _] : allNodes)
			if (!usedNodes.contains(nodeIndex))
				nodesToRemove.push_back(nodeIndex);

		for (const auto& node : nodesToRemove)
			ctx.RenderGraph.m_pGraph->RemoveNode(node);

		// Graph should now contain all passes that will be used, now decide the execution order with a sort
		return graphHelper.TopologySort();
	}

	CompiledGraph RGCCompiler::Compile(RGCContext& ctx, std::vector<uint32> sortedNodes)
	{
		CompiledGraph compiledGraph;

		// Go through the sorted passes and save their data in the vector
		// TODO: Cache reflection
		for (auto nodeID : sortedNodes)
		{
			CompiledPass data   = {};
			data.pPass          = ctx.RenderGraph.m_Passes[nodeID];
			data.GraphNodeIndex = nodeID;
			data.Reflection     = data.pPass.get()->Reflect();
			compiledGraph.CompiledPasses.push_back(data);
		}

		return compiledGraph;
	}
} // namespace Poly