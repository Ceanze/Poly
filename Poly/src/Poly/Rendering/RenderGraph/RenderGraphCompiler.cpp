#include "polypch.h"
#include "RenderGraphCompiler.h"
#include "RenderGraph.h"
#include "RenderPass.h"
#include "ResourceCache.h"
#include "RenderGraphTypes.h"
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
		AddSync();
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
			data.pPass	= m_pRenderGraph->m_Passes[nodeID];
			data.NodeIndex	= nodeID;
			data.Reflection	= data.pPass->Reflect();
			m_OrderedPasses.push_back(data);
		}
	}

	void RenderGraphCompiler::CompilePasses()
	{
		// This function for now only calls the render passes compile function
		// but is here for possible future expansion

		for (const auto& pass : m_OrderedPasses)
		{
			pass.pPass->Compile();
		}
	}

	void RenderGraphCompiler::ValidateGraph()
	{
		// Checks if the current graph is valid to render to a swap chain for.
		// To do this check if there are any resources marked for output and
		// that all inputs are linked

		if (m_pRenderGraph->m_Outputs.empty())
		{
			POLY_CORE_WARN("Rendergraph must have atleast one resource marked as output");
			return;
		}

		for (auto passData : m_OrderedPasses)
		{
			const auto& inputs = passData.Reflection.GetInputs();
			auto incommingEdges = m_pRenderGraph->m_pGraph->GetNode(passData.NodeIndex)->GetIncommingEdges();
			auto externalResources = passData.pPass->GetExternalResources();
			for (auto& input : inputs)
			{
				std::string dst = passData.pPass->GetName() + "." + input.Name;
				bool valid = false;
				for (auto edgeID : incommingEdges)
				{
					auto& edgeData = m_pRenderGraph->m_Edges[edgeID];
					if (edgeData.Dst == dst)
					{
						valid = true;
						break;
					}
				}
				if (!valid)
				{
					for (auto& externalResource : externalResources)
					{
						std::string externalDst = passData.pPass->GetName() + "." + externalResource.second;
						if (externalDst == dst)
						{
							valid = true;
							break;
						}
					}
				}
				// Let the user know, continue the loop to find if more are missing
				if (!valid)
					POLY_CORE_WARN("Input resource {} did not have a link to it", dst);
			}
		}
	}

	void RenderGraphCompiler::AllocateResources()
	{
		RenderGraphDefaultParams defaults = {};
		defaults.TextureHeight	= 720;
		defaults.TextureWidth	= 1080;

		m_pResourceCache = ResourceCache::Create(defaults);

		for (uint32 passID = 0; passID < m_OrderedPasses.size(); passID++)
		{
			auto& passData = m_OrderedPasses[passID];

			// Register outputs
			const auto& outputs = passData.Reflection.GetOutputs();
			for (auto& output : outputs)
			{
				std::string resourceName = passData.pPass->GetName() + "." + output.Name;

				// Check if resource is being used, as the graph allows outputs to be non-linked, go to next output if not used
				bool isUsed = IsResourceUsed(passData.NodeIndex, output.Name);
				if (!isUsed)
					continue;

				m_pResourceCache->RegisterResource(passData.pPass->GetName() + "." + output.Name, passID, output);
			}

			// Make aliases of the passthroughs
			const auto& passThroughs = passData.Reflection.GetPassThroughs();
			for (auto& passThough : passThroughs)
			{
				std::string resourceName = passData.pPass->GetName() + "." + passThough.Name;
				std::string alias = "";

				// Find incomming edge to alias to
				const auto& incommingEdges = m_pRenderGraph->m_pGraph->GetNode(passData.NodeIndex)->GetIncommingEdges();
				for (auto edgeID : incommingEdges)
				{
					auto& edgeData = m_pRenderGraph->m_Edges[edgeID];
					if (edgeData.Dst == resourceName)
						alias = edgeData.Src;
				}

				// If incomming didn't get any match, check for externals
				if (alias.empty())
				{
					const auto& externals = passData.pPass->GetExternalResources();
					for (auto& external : externals)
					{
						if (external.second == passThough.Name)
							alias = "$." + external.first;
					}
				}

				if (alias.empty())
				{
					POLY_CORE_ERROR("No resource linkage was found for {}, this should now happen and should have been found earlier", resourceName);
					return;
				}

				m_pResourceCache->RegisterResource(passData.pPass->GetName() + "." + passThough.Name, passID, passThough, alias);
			}

			// Make aliases of the inputs
			const auto& inputs = passData.Reflection.GetInputs();
			for (auto& input : inputs)
			{
				std::string resourceName = passData.pPass->GetName() + "." + input.Name;
				std::string alias = "";

				const auto& incommingEdges = m_pRenderGraph->m_pGraph->GetNode(passData.NodeIndex)->GetIncommingEdges();
				for (auto edgeID : incommingEdges)
				{
					auto& edgeData = m_pRenderGraph->m_Edges[edgeID];
					if (edgeData.Dst == resourceName)
						alias = edgeData.Src;
				}

				// If incomming didn't get any match, check for externals
				if (alias.empty())
				{
					const auto& externals = passData.pPass->GetExternalResources();
					for (auto& external : externals)
					{
						if (external.second == input.Name)
							alias = "$." + external.first;
					}
				}

				if (alias.empty())
				{
					POLY_CORE_ERROR("No resource linkage was found for {}, this should now happen and should have been found earlier", resourceName);
					return;
				}

				m_pResourceCache->RegisterResource(passData.pPass->GetName() + "." + input.Name, passID, input, alias);
			}
		}

		// Now allocate the resources
		m_pResourceCache->AllocateResources();
	}

	void RenderGraphCompiler::AddSync()
	{
		/**
		 * Syncronization will be done by adding special SyncPasses between passes whose resources
		 * need to be synced. To do this, go though the passes in order and for each pass check what
		 * state the resources it is using are in (gotten from reflection). If the resources current
		 * state doesn't match what the previous state it was in then add a barrier
		 * [or renderpass sync for some textures].
		 */
	}

	bool RenderGraphCompiler::IsResourceUsed(uint32 nodeIndex, const std::string& outputName)
	{
		std::string resourceName = m_pRenderGraph->m_Passes[nodeIndex]->GetName() + "." + outputName;
		const auto& outgoingEdges = m_pRenderGraph->m_pGraph->GetNode(nodeIndex)->GetOutgoingEdges();

		auto& markedOutputs = m_pRenderGraph->m_Outputs;
		for (auto& markedOutput : markedOutputs)
		{
			if (markedOutput.NodeID == nodeIndex)
			{
				if (markedOutput.ResourceName == outputName)
					return true;
			}
		}

		for (auto edgeID : outgoingEdges)

		{
			if (m_pRenderGraph->m_Edges[edgeID].Src == resourceName)
				return true;
		}

		return false;
	}
}