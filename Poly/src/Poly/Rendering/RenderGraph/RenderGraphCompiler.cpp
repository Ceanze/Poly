#include "polypch.h"
#include "RenderGraphCompiler.h"
#include "SyncPass.h"
#include "Resource.h"
#include "RenderPass.h"
#include "RenderGraph.h"
#include "ResourceCache.h"
#include "RenderGraphProgram.h"
#include "Poly/Core/Utils/DirectedGraphHelper.h"

namespace Poly
{
	Ref<RenderGraphCompiler> RenderGraphCompiler::Create()
	{
		return CreateRef<RenderGraphCompiler>();
	}

	Ref<RenderGraphProgram> RenderGraphCompiler::Compile(RenderGraph* pRenderGraph, RenderGraphDefaultParams defaultParams)
	{
		m_pRenderGraph = pRenderGraph;
		m_DefaultParams = defaultParams;

		SetupExecutionOrder();
		CompilePasses();
		AllocateResources();

		bool newPasses = false;
		AddSync(&newPasses);
		if (newPasses)
		{
			SetupExecutionOrder();
			ValidateGraph();
		}

		Ref<RenderGraphProgram> program = RenderGraphProgram::Create(m_pRenderGraph, m_pResourceCache, m_DefaultParams);
		for (const auto& passData : m_OrderedPasses)
			program->AddPass(passData.pPass);
		return program;
	}

	void RenderGraphCompiler::SetupExecutionOrder()
	{
		m_OrderedPasses.clear();

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
			data.Reflection	= data.pPass.get()->Reflect();
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

		for (const auto& passData : m_OrderedPasses)
		{
			const auto& inputs = passData.Reflection.GetIOData(FIOType::INPUT);
			const auto& incommingEdges = m_pRenderGraph->m_pGraph->GetNode(passData.NodeIndex)->GetIncommingEdges();
			const auto& externalResources = passData.pPass->GetExternalResources();
			for (uint32 i = 0; i < inputs.size(); i++)
			{
				std::string dst = passData.pPass->GetName() + "." + inputs[i].Name;
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
		m_pResourceCache = ResourceCache::Create(m_DefaultParams);

		for (uint32 passID = 0; passID < m_OrderedPasses.size(); passID++)
		{
			auto& passData = m_OrderedPasses[passID];

			// Register outputs
			const auto& outputs = passData.Reflection.GetIOData(FIOType::OUTPUT);
			for (auto& output : outputs)
			{
				std::string resourceName = passData.pPass->GetName() + "." + output.Name;

				// Check if resource is being used, as the graph allows outputs to be non-linked, go to next output if not used
				bool isUsed = IsResourceUsed(passData.NodeIndex, output.Name);
				if (!isUsed)
					continue;

				m_pResourceCache->RegisterResource(passData.pPass->GetName() + "." + output.Name, passID, output);
			}

			// Make aliases of the inputs
			const auto& inputs = passData.Reflection.GetIOData(FIOType::INPUT);
			for (auto& input : inputs)
			{
				std::string resourceName = passData.pPass->GetName() + "." + input.Name;
				std::string alias = "";

				const auto& incommingEdges = m_pRenderGraph->m_pGraph->GetNode(passData.NodeIndex)->GetIncommingEdges();
				for (auto edgeID : incommingEdges)
				{
					auto& edgeData = m_pRenderGraph->m_Edges[edgeID];
					if (edgeData.Dst == resourceName)
					{
						alias = edgeData.Src;
						break;
					}
				}

				// If incomming didn't get any match, check for externals
				if (alias.empty())
				{
					const auto& externals = passData.pPass->GetExternalResources();
					for (auto& external : externals)
					{
						if (external.second == input.Name)
						{
							alias = "$." + external.first;
							break;
						}
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

	void RenderGraphCompiler::AddSync(bool* pNewPasses)
	{
		/**
		 * Syncronization will be done by adding special SyncPasses between passes whose resources
		 * need to be synced. To do this, go though the passes in order and for each pass check what
		 * state the resources it is using are in (gotten from reflection). If the resources current
		 * state doesn't match what the previous state it was in then add a barrier
		 * [or renderpass sync for some textures].
		 */

		// Resources are created before this stage

		struct SyncPassData
		{
			Ref<SyncPass>				pSyncPass;
			std::unordered_set<uint32>	brokenEdgeIDs;
		};

		std::vector<SyncPassData> addedSyncPasses;

		// Go through the passes in order of use
		for (const auto& passData : m_OrderedPasses)
		{
			// Go though all the inputs of the pass
			Ref<SyncPass> syncPass = nullptr;
			std::unordered_set<uint32> brokenEdges;
			auto incommingEdges = m_pRenderGraph->m_pGraph->GetNode(passData.NodeIndex)->GetIncommingEdges();
			for (const auto& edgeID : incommingEdges)
			{
				const std::string& srcName = m_pRenderGraph->m_Edges[edgeID].Src;
				const std::string& dstName = m_pRenderGraph->m_Edges[edgeID].Dst;
				PassResourcePair srcPair = m_pRenderGraph->GetPassNameResourcePair(srcName);
				PassResourcePair dstPair = m_pRenderGraph->GetPassNameResourcePair(dstName);
				uint32 srcPassIndex = m_pRenderGraph->m_NameToNodeIndex[srcPair.first];
				auto srcIt = std::find_if(m_OrderedPasses.begin(), m_OrderedPasses.end(), [srcPassIndex](const PassData& passData) { return passData.NodeIndex == srcPassIndex; });
				const auto& srcReflection = srcIt->Reflection.GetIOData(srcPair.second);
				const auto& dstReflection = passData.Reflection.GetIOData(dstPair.second);

				const auto& srcRes = m_pResourceCache->GetResource(srcName);


				// If the current layout of the input resource does not match this pass, add sync
				if (srcReflection.TextureLayout != dstReflection.TextureLayout)
				{
					// add sync [make this render pass transition later]
					if (!syncPass)
						syncPass = SyncPass::Create("[SyncPass] " + srcPair.first + " to " + dstPair.first); // TODO: Make a cheaper way to combine strings
					SyncPass::SyncData data = {};
					data.Type			= SyncPass::SyncType::TEXTURE;
					data.ResourceName	= dstPair.second;
					data.SrcLayout		= srcReflection.TextureLayout;
					data.DstLayout		= dstReflection.TextureLayout;
					data.SrcBindPoint	= srcReflection.BindPoint;
					data.DstBindPoint	= dstReflection.BindPoint;
					syncPass->AddSyncData(data);

					// Register alias for the syncPass resource so it can be accessed in its execute
					// TODO: Create specific function for syncpass additions
					m_pResourceCache->RegisterResource(syncPass->GetName() + "." + dstPair.second, 0, {}, srcName);
					brokenEdges.insert(edgeID);
				}

				// TODO: Add more sync checks

			}
			// If a sync pass was created, add it to the array
			if (syncPass)
			{
				*pNewPasses = true;
				SyncPassData data = {};
				data.pSyncPass		= syncPass;
				data.brokenEdgeIDs	= brokenEdges;
				addedSyncPasses.push_back(data);
			}
		}

		// Add sync passes to the graph
		for (const auto& syncPassData : addedSyncPasses)
		{
			// Create pass
			m_pRenderGraph->AddPass(syncPassData.pSyncPass, syncPassData.pSyncPass->GetName());

			for (const uint32 brokenEdgeID : syncPassData.brokenEdgeIDs)
			{
				RenderGraph::EdgeData edge = m_pRenderGraph->GetEdgeData(brokenEdgeID);
				// Current implementation assumes that the resource name is set to dst resource name
				PassResourcePair dstPair = m_pRenderGraph->GetPassNameResourcePair(edge.Dst);

				// Add links for new pass
				m_pRenderGraph->RemoveLink(edge.Src, edge.Dst);
				m_pRenderGraph->AddLink(edge.Src, syncPassData.pSyncPass->GetName() + "." + dstPair.second);
				m_pRenderGraph->AddLink(syncPassData.pSyncPass->GetName() + "." + dstPair.second, edge.Dst);
			}
		}
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