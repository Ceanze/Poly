#include "polypch.h"
#include "RenderGraphCompiler.h"
#include "RenderGraph.h"
#include "RenderPass.h"
#include "Poly/Core/Utils/DirectedGraphHelper.h"

namespace Poly
{
	Ref<RenderGraphCompiler> RenderGraphCompiler::Create()
	{
		return CreateRef<RenderGraphCompiler>();
	}

	void RenderGraphCompiler::Compile(RenderGraph* pRenderGraph)
	{
		m_pRenderGraph = pRenderGraph;

		SetupExecutionOrder();
		CompilePasses();
		ValidateGraph();
		AllocateResources();
	}

	void RenderGraphCompiler::SetupExecutionOrder()
	{
		// Determine which passes are actual needed:
		std::unordered_set<uint32> mandatoryPasses;

		// 1. Check all outputs of the graph and traverse backwards
		for (const auto& output : m_pRenderGraph->m_Outputs)
			mandatoryPasses.insert(output.NodeID);

		// 2. Add all passes which has an execution link, since this might affect other mandatory passes
		for (const auto& edge : m_pRenderGraph->m_Edges)
		{
			// Remove dstPair or srcPair check? If one is true the other should also be (check done when link is added)
			auto srcPair = m_pRenderGraph->GetPassNameResourcePair(edge.second.Src);
			auto dstPair = m_pRenderGraph->GetPassNameResourcePair(edge.second.Dst);
			if (srcPair.second.empty() && dstPair.second.empty())
			{
				mandatoryPasses.insert(m_pRenderGraph->m_NameToNodeIndex[srcPair.first]);
				mandatoryPasses.insert(m_pRenderGraph->m_NameToNodeIndex[dstPair.first]);
			}
		}

		// Traverse graph in reverse for all mandatory passes to get all passes that will be used
		std::unordered_set<uint32> usedPasses;
		DirectedGraphHelper graphHelper = DirectedGraphHelper(m_pRenderGraph->m_pGraph, 0, DirectedGraphHelper::CreateFlags::REVERSE | DirectedGraphHelper::CreateFlags::IGNORE_VISITED);
		for (const auto& pass : mandatoryPasses)
		{
			graphHelper.Reset(pass);
			uint32 currNode = graphHelper.Traverse();
			while (currNode != DirectedGraphHelper::INVALID_ID)
			{
				usedPasses.insert(currNode);
				currNode = graphHelper.Traverse();
			}
		}

		// usedPasses should now contain all passes that will be used, now decide the execution order with a sort
		std::vector<uint32> sorted = graphHelper.TopologySort();

		// Go through the sorted passes and save their data in the vector
		for (auto nodeID : sorted)
		{
			PassData data = {};
			data.pResource	= m_pRenderGraph->m_Passes[nodeID];
			data.NodeIndex	= nodeID;
			m_OrderedPasses.push_back(data);
		}
	}

	void RenderGraphCompiler::CompilePasses()
	{

	}

	void RenderGraphCompiler::ValidateGraph()
	{

	}

	void RenderGraphCompiler::AllocateResources()
	{

	}

	void RenderGraphCompiler::AddSync()
	{

	}


}