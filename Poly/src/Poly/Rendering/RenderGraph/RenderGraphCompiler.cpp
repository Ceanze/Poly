#include "polypch.h"
#include "RenderGraphCompiler.h"
#include "SyncPass.h"
#include "Resource.h"
#include "RenderPass.h"
#include "RenderGraph.h"
#include "ResourceCache.h"
#include "RenderGraphProgram.h"
#include "Poly/Core/Utils/DirectedGraphHelper.h"
#include <ranges>

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
			const auto& inputs = passData.Reflection.GetIOData(FIOType::INPUT, FResourceBindPoint::ALL_SCENES | FResourceBindPoint::INTERNAL_USE);
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

		// Register external resources
		for (const auto& resPair : m_pRenderGraph->m_ExternalResources)
			m_pResourceCache->RegisterExternalResource("$." + resPair.first, resPair.second);

		for (uint32 passID = 0; passID < m_OrderedPasses.size(); passID++)
		{
			auto& passData = m_OrderedPasses[passID];

			// Register outputs
			const auto& outputs = passData.Reflection.GetIOData(FIOType::OUTPUT, FResourceBindPoint::ALL_SCENES);
			for (auto& output : outputs)
			{
				std::string resourceName = passData.pPass->GetName() + "." + output.Name;

				// PROBLEM: The out (swapchain) image is created as a new image for all passes before the final which is marked as output.
				// Possible solution: Deduce beforehand which resources (outputs in passes) are connected to the output (by going backwards in the graph)
				// and make all of them alias to the actual output which is marked

				// Check if resource is being used, as the graph allows outputs to be non-linked, go to next output if not used
				bool isMarkedOutput = IsResourceGraphOutput(passData.NodeIndex, output.Name);
				bool isUsed = IsResourceUsed(passData.NodeIndex, output.Name);
				bool isDepthStencil = BitsSet(FResourceBindPoint::DEPTH_STENCIL, output.BindPoint);
				if (!isUsed && !isMarkedOutput && !isDepthStencil)
					continue;

				if (isDepthStencil)
					static_cast<RenderPass*>(passData.pPass.get())->SetDepthStenctilUse(true);

				if (isMarkedOutput && !BitsSet(output.IOType, FIOType::INPUT)) // Create alias to backbuffer
				{
					m_pResourceCache->MarkOutput(passData.pPass->GetName() + "." + output.Name, output);
					passData.Reflection.SetFormat(output.Name, EFormat::B8G8R8A8_UNORM);
				}
				else if (!BitsSet(output.IOType, FIOType::INPUT) && !BitsSet(output.BindPoint, FResourceBindPoint::INTERNAL_USE)) // Only register new resource if it wasn't an input (passthrough)
					m_pResourceCache->RegisterResource(passData.pPass->GetName() + "." + output.Name, passID, output);
			}

			// Make aliases of the inputs
			const auto& inputs = passData.Reflection.GetIOData(FIOType::INPUT, FResourceBindPoint::ALL_SCENES | FResourceBindPoint::INTERNAL_USE);
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

				if (IsResourceGraphOutput(passData.NodeIndex, input.Name))
				{
					m_pResourceCache->MarkOutput(passData.pPass->GetName() + "." + input.Name, input);
					passData.Reflection.SetFormat(input.Name, EFormat::B8G8R8A8_UNORM);
				}
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

		struct HalfBarrier
		{
			std::string		Name			= "";
			ETextureLayout	TextureLayout	= ETextureLayout::UNDEFINED;
			FPipelineStage	PipelineStage	= FPipelineStage::NONE;
			FAccessFlag		AccessMask		= FAccessFlag::NONE;
		};

		std::vector<SyncPassData> addedSyncPasses;

		std::unordered_map<uint32, std::vector<HalfBarrier>> m_Invalidates;
		std::unordered_map<uint32, std::vector<HalfBarrier>> m_Flushes;

		for (const auto& passData : m_OrderedPasses)
		{
			// Create invalidates for inputs
			auto inputs = passData.Reflection.GetIOData(FIOType::INPUT, FResourceBindPoint::INTERNAL_USE);
			for (const auto& input : inputs)
			{
				HalfBarrier barrier = {};
				barrier.AccessMask		= GetAccessFlag(input.BindPoint, true);
				barrier.PipelineStage	= GetPipelineStage(input.BindPoint);
				barrier.TextureLayout	= input.TextureLayout;
				barrier.Name			= input.Name;
				m_Invalidates[passData.NodeIndex].push_back(barrier);
			}

			// Create flushes for output. Save as attachments
			auto outputs = passData.Reflection.GetIOData(FIOType::OUTPUT, FResourceBindPoint::INTERNAL_USE);
			uint32 index = 0;
			for (const auto& output : outputs)
			{
				HalfBarrier barrier = {};
				barrier.AccessMask		= GetAccessFlag(output.BindPoint, false);
				barrier.PipelineStage	= GetPipelineStage(output.BindPoint);
				barrier.TextureLayout	= output.TextureLayout;
				barrier.Name			= output.Name;
				m_Flushes[passData.NodeIndex].push_back(barrier);

				// At this stage we can guarantee that the order of attachments are correct (should be given in the correct order at reflect)
				if (passData.pPass->GetPassType() == Pass::Type::RENDER && output.TextureLayout != ETextureLayout::UNDEFINED)
				{
					// Assume the data will be cleared for now
					static_cast<RenderPass*>(passData.pPass.get())->AddAttachment(output.Name, output.TextureLayout, index++, output.Format);
					static_cast<RenderPass*>(passData.pPass.get())->SetAttachmentInital(output.Name, ETextureLayout::UNDEFINED);
				}
			}
		}

		// Create any necessary barriers by combining half barriers
		for (const auto& passData : m_OrderedPasses)
		{
			Ref<SyncPass> syncPass = nullptr;
			std::unordered_set<uint32> brokenEdges;

			const auto& externalResources = passData.pPass->GetExternalResources();
			for (const auto& resPair : externalResources)
			{
				auto& barriers = m_Invalidates[passData.NodeIndex];
				auto itr = std::find_if(barriers.begin(), barriers.end(), [&](const HalfBarrier& b){ return b.Name == resPair.second; });

				// Check if it is a texture or a buffer
				if (itr->TextureLayout != ETextureLayout::UNDEFINED) // Texture
				{
					if (!BitsSet(passData.Reflection.GetIOData(resPair.second).IOType, FIOType::OUTPUT))
						continue;

					if (BitsSet(itr->AccessMask, FAccessFlag::COLOR_ATTACHMENT_WRITE | FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE | FAccessFlag::SHADER_WRITE)) // Write
					{
						// External resource is a texture and will be written to. We can therefore transition from undefined (handled by render pass) and no barrier is needed
						static_cast<RenderPass*>(passData.pPass.get())->SetAttachmentInital(itr->Name, ETextureLayout::UNDEFINED);
					}
					else // Read
					{
						// External resource is a texture and will be read from, a transition might be needed (handled by render pass) but no barrier is needed
						// Current implementation assumes all external textures will be in SHADER_READ_ONLY layout for first time use if being read from
						static_cast<RenderPass*>(passData.pPass.get())->SetAttachmentInital(itr->Name, ETextureLayout::SHADER_READ_ONLY_OPTIMAL);
					}
				}
				else // Buffer
				{
					if (BitsSet(itr->AccessMask, FAccessFlag::SHADER_WRITE)) // Write
					{
						// External resource is a buffer and will be written to, first time use, no barrier needed
					}
					else // Read
					{
						// External resource is a buffer and will be read from, no barrier needed
					}
				}
			}

			// Go through all graph inputs and check if a barrier is needed
			const auto incommingEdges = m_pRenderGraph->m_pGraph->GetNode(passData.NodeIndex)->GetIncommingEdges();
			for (const auto& edgeID : incommingEdges)
			{
				// If external, check if we are reading or writing. If writing and texture, use layout undefined
				const std::string& srcName = m_pRenderGraph->m_Edges[edgeID].Src;
				const std::string& dstName = m_pRenderGraph->m_Edges[edgeID].Dst;
				PassResourcePair srcPair = m_pRenderGraph->GetPassNameResourcePair(srcName);
				PassResourcePair dstPair = m_pRenderGraph->GetPassNameResourcePair(dstName);
				if (srcPair.first == "$") // External
				{
					auto& barriers = m_Invalidates[passData.NodeIndex];
					auto itr = std::find_if(barriers.begin(), barriers.end(), [&](const HalfBarrier& b){ return b.Name == dstPair.second; });

					// Check if it is a texture or a buffer
					if (itr->TextureLayout != ETextureLayout::UNDEFINED) // Texture
					{
						if (BitsSet(itr->AccessMask, FAccessFlag::COLOR_ATTACHMENT_WRITE | FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE | FAccessFlag::SHADER_WRITE)) // Write
						{
							// External resource is a texture and will be written to. We can therefore transition from undefined (handled by render pass) and no barrier is needed
							static_cast<RenderPass*>(passData.pPass.get())->SetAttachmentInital(itr->Name, ETextureLayout::UNDEFINED);
						}
						else // Read
						{
							// External resource is a texture and will be read from, a transition might be needed (handled by render pass) but no barrier is needed
							// Current implementation assumes all external textures will be in SHADER_READ_ONLY layout for first time use if being read from
							static_cast<RenderPass*>(passData.pPass.get())->SetAttachmentInital(itr->Name, ETextureLayout::SHADER_READ_ONLY_OPTIMAL);
						}
					}
					else // Buffer
					{
						if (BitsSet(itr->AccessMask, FAccessFlag::SHADER_WRITE)) // Write
						{
							// External resource is a buffer and will be written to, first time use, no barrier needed
						}
						else // Read
						{
							// External resource is a buffer and will be read from, no barrier needed
						}
					}
				}
				else // From another pass
				{
					auto& flushBarriers = m_Flushes[m_pRenderGraph->m_NameToNodeIndex[srcPair.first]];
					auto srcItr = std::find_if(flushBarriers.begin(), flushBarriers.end(), [&](const HalfBarrier& b){ return b.Name == srcPair.second; });
					auto& invalidateBarriers = m_Invalidates[passData.NodeIndex];
					auto dstItr = std::find_if(invalidateBarriers.begin(), invalidateBarriers.end(), [&](const HalfBarrier& b){ return b.Name == dstPair.second; });

					if (dstItr->TextureLayout != ETextureLayout::UNDEFINED) // Texture
					{
						// Since a barrier is set for both a WAR and RAW - no check is needed for WAW
						if (BitsSet(dstItr->AccessMask, FAccessFlag::COLOR_ATTACHMENT_WRITE | FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE | FAccessFlag::SHADER_WRITE) // Write after read
							|| BitsSet(srcItr->AccessMask, FAccessFlag::COLOR_ATTACHMENT_WRITE | FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE | FAccessFlag::SHADER_WRITE)) // Read after write
						{
							// Write after read - execution barrier needed - for simplicity a normal barrier is added
							if (!syncPass)
								syncPass = SyncPass::Create("Presync for " + dstPair.first);

							SyncPass::SyncData data = {};
							data.Type				= SyncPass::SyncType::TEXTURE;
							data.ResourceName		= dstPair.second;
							data.SrcLayout			= srcItr->TextureLayout;
							data.DstLayout			= dstItr->TextureLayout;
							data.SrcAccessFlag		= srcItr->AccessMask;
							data.DstAccessFlag		= dstItr->AccessMask; // TODO: Check if a READ or WRITE (for i.e. depth or color attachemnt) needs to be added (might be fixed now)
							data.SrcPipelineStage	= srcItr->PipelineStage;
							data.DstPipelineStage	= dstItr->PipelineStage;
							syncPass->AddSyncData(data);

							// m_pResourceCache->RegisterResource(syncPass->GetName() + "." + dstPair.second, 0, {}, srcName);
							m_pResourceCache->RegisterSyncResource(syncPass->GetName() + "." + dstPair.second, srcName);
							brokenEdges.insert(edgeID);

							static_cast<RenderPass*>(passData.pPass.get())->SetAttachmentInital(dstItr->Name, srcItr->TextureLayout);
						}
						else // Read after read
						{
							// A layout transition might be needed
							static_cast<RenderPass*>(passData.pPass.get())->SetAttachmentInital(dstItr->Name, srcItr->TextureLayout);
						}
					}
					else // Buffer
					{
						if (BitsSet(dstItr->AccessMask, FAccessFlag::SHADER_WRITE) || BitsSet(srcItr->AccessMask, FAccessFlag::SHADER_WRITE)) // WAR or RAW
						{
							if (!syncPass)
								syncPass = SyncPass::Create("Presync for " + dstPair.first);

							SyncPass::SyncData data = {};
							data.Type				= SyncPass::SyncType::BUFFER;
							data.ResourceName		= dstPair.second;
							data.SrcLayout			= srcItr->TextureLayout; // These are undefined for this resource (buffer)
							data.DstLayout			= dstItr->TextureLayout;
							data.SrcAccessFlag		= srcItr->AccessMask;
							data.DstAccessFlag		= dstItr->AccessMask;
							data.SrcPipelineStage	= srcItr->PipelineStage;
							data.DstPipelineStage	= dstItr->PipelineStage;
							syncPass->AddSyncData(data);

							// m_pResourceCache->RegisterResource(syncPass->GetName() + "." + dstPair.second, 0, {}, srcName);
							m_pResourceCache->RegisterSyncResource(syncPass->GetName() + "." + dstPair.second, srcName);
							brokenEdges.insert(edgeID);
						}
						else // Read after read
						{
							// Nothing to do in that case
						}
					}
				}
			}

			if (syncPass)
			{
				*pNewPasses = true;
				SyncPassData data = {};
				data.pSyncPass		= syncPass;
				data.brokenEdgeIDs	= brokenEdges;
				addedSyncPasses.push_back(data);
			}
		}

		// Find the last output to transition the backbuffer to present
		if (m_pRenderGraph->m_Outputs.size() == 1)
		{
			auto itr = m_pRenderGraph->m_Outputs.begin();
			auto lastPassItr = std::find_if(m_OrderedPasses.begin(), m_OrderedPasses.end(), [&](const PassData& pd) { return pd.NodeIndex == itr->NodeID; });
			static_cast<RenderPass*>(lastPassItr->pPass.get())->SetAttachmentFinal(itr->ResourceName, ETextureLayout::PRESENT);

			// Find the first use of the backbufer and set that inital attachment to UNDEFINED to clear it
			// TODO: Should probably traverse the graph to find the origin of the backbuffer usage instead
			// of going though attachments with COLOR_ATTACHMENT_OPTIMAL set
			bool set = false;
			for (const auto& passData : m_OrderedPasses)
			{
				if (passData.pPass->GetPassType() == Pass::Type::RENDER)
				{
					RenderPass* pPass = static_cast<RenderPass*>(passData.pPass.get());
					const auto& attachmentMap = pPass->GetAttachments();
					for (const auto& pair : attachmentMap)
					{
						if (pair.second.UsedLayout == ETextureLayout::COLOR_ATTACHMENT_OPTIMAL)
						{
							pPass->SetAttachmentInital(pair.first, ETextureLayout::UNDEFINED);
							set = true;
							break;
						}

					}
					if (set)
						break;
				}
			}
			//static_cast<RenderPass*>(lastPassItr->pPass.get())->SetAttachmentInital(itr->ResourceName, ETextureLayout::UNDEFINED);
		}
		else
		{
			// TODO: If multiple outputs - only set the first to have inital layout to be undefined in a similar way
			// to how only the last output has present.


			uint32 lastOutputIndex = 0;
			std::string resourceName = "";
			for (const auto& output : m_pRenderGraph->m_Outputs)
			{
				for (uint32 i = 0; i < m_OrderedPasses.size(); i++)
				{
					if (output.NodeID == m_OrderedPasses[i].NodeIndex && i > lastOutputIndex)
					{
						lastOutputIndex = i;
						resourceName = output.ResourceName;
					}
				}
			}
			static_cast<RenderPass*>(m_OrderedPasses[lastOutputIndex].pPass.get())->SetAttachmentFinal(resourceName, ETextureLayout::PRESENT);
		}

		// Go through the passes in order of use
		// for (const auto& passData : m_OrderedPasses)
		// {
		// 	// Go though all the inputs of the pass
		// 	Ref<SyncPass> syncPass = nullptr;
		// 	std::unordered_set<uint32> brokenEdges;
		// 	auto incommingEdges = m_pRenderGraph->m_pGraph->GetNode(passData.NodeIndex)->GetIncommingEdges();
		// 	for (const auto& edgeID : incommingEdges)
		// 	{
		// 		bool externalResource = false;
		// 		const std::string& srcName = m_pRenderGraph->m_Edges[edgeID].Src;
		// 		const std::string& dstName = m_pRenderGraph->m_Edges[edgeID].Dst;
		// 		PassResourcePair srcPair = m_pRenderGraph->GetPassNameResourcePair(srcName);
		// 		PassResourcePair dstPair = m_pRenderGraph->GetPassNameResourcePair(dstName);
		// 		if (srcPair.first == "$")
		// 			externalResource = true;

		// 		uint32 srcPassIndex = m_pRenderGraph->m_NameToNodeIndex[srcPair.first];
		// 		auto srcIt = std::find_if(m_OrderedPasses.begin(), m_OrderedPasses.end(), [srcPassIndex](const PassData& passData) { return passData.NodeIndex == srcPassIndex; });
		// 		const auto& srcReflection = srcIt->Reflection.GetIOData(srcPair.second);
		// 		const auto& dstReflection = passData.Reflection.GetIOData(dstPair.second);

		// 		const auto& srcRes = m_pResourceCache->GetResource(srcName);


		// 		// If the current layout of the input resource does not match this pass, add sync
		// 		if (srcReflection.TextureLayout != dstReflection.TextureLayout)
		// 		{
		// 			// add sync [make this render pass transition later]
		// 			if (!syncPass)
		// 				syncPass = SyncPass::Create("Presync for " + dstPair.first);
		// 			SyncPass::SyncData data = {};
		// 			data.Type			= SyncPass::SyncType::TEXTURE;
		// 			data.ResourceName	= dstPair.second;
		// 			data.SrcLayout		= srcReflection.TextureLayout;
		// 			data.DstLayout		= dstReflection.TextureLayout;
		// 			data.SrcBindPoint	= srcReflection.BindPoint;
		// 			data.DstBindPoint	= dstReflection.BindPoint;
		// 			syncPass->AddSyncData(data);

		// 			// Register alias for the syncPass resource so it can be accessed in its execute
		// 			// TODO: Create specific function for syncpass additions
		// 			m_pResourceCache->RegisterResource(syncPass->GetName() + "." + dstPair.second, 0, {}, srcName);
		// 			brokenEdges.insert(edgeID);
		// 		}

		// 		// TODO: Add more sync checks

		// 	}
		// 	// If a sync pass was created, add it to the array
		// 	if (syncPass)
		// 	{
		// 		*pNewPasses = true;
		// 		SyncPassData data = {};
		// 		data.pSyncPass		= syncPass;
		// 		data.brokenEdgeIDs	= brokenEdges;
		// 		addedSyncPasses.push_back(data);
		// 	}
		// }

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

		for (auto edgeID : outgoingEdges)

		{
			if (m_pRenderGraph->m_Edges[edgeID].Src == resourceName)
				return true;
		}

		return false;
	}

	bool RenderGraphCompiler::IsResourceGraphOutput(uint32 nodeIndex, const std::string& outputName)
	{
		std::string resourceName = m_pRenderGraph->m_Passes[nodeIndex]->GetName() + "." + outputName;

		auto& markedOutputs = m_pRenderGraph->m_Outputs;
		for (auto& markedOutput : markedOutputs)
		{
			if (markedOutput.NodeID == nodeIndex)
			{
				if (markedOutput.ResourceName == outputName)
					return true;
			}
		}

		return false;
	}

	FAccessFlag RenderGraphCompiler::GetAccessFlag(FResourceBindPoint bindPoint, bool isInput)
	{
		if (bindPoint == FResourceBindPoint::COLOR_ATTACHMENT)
			return isInput ? FAccessFlag::COLOR_ATTACHMENT_READ : FAccessFlag::COLOR_ATTACHMENT_WRITE;
		if (bindPoint == FResourceBindPoint::DEPTH_STENCIL)
			return isInput ? FAccessFlag::DEPTH_STENCIL_ATTACHMENT_READ : FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE;
		if (bindPoint == FResourceBindPoint::UNIFORM)
			return FAccessFlag::UNIFORM_READ;
		if (bindPoint == FResourceBindPoint::SAMPLER)
			return FAccessFlag::SHADER_READ;
		if (bindPoint == FResourceBindPoint::STORAGE)
			return isInput ? FAccessFlag::SHADER_READ : FAccessFlag::SHADER_WRITE; // TODO: Make this not be both READ and WRITE
		if (bindPoint == FResourceBindPoint::VERTEX)
			return FAccessFlag::VERTEX_ATTRIBUTE_READ;
		if (bindPoint == FResourceBindPoint::INDEX)
			return FAccessFlag::INDEX_READ;
		if (bindPoint == FResourceBindPoint::INDIRECT)
			return FAccessFlag::INDIRECT_COMMAND_READ;
		if (bindPoint == FResourceBindPoint::INPUT_ATTACHMENT)
			return FAccessFlag::INPUT_ATTACHMENT_READ;

		return FAccessFlag::NONE;
	}

	FPipelineStage RenderGraphCompiler::GetPipelineStage(FResourceBindPoint bindPoint)
	{
		if (bindPoint == FResourceBindPoint::COLOR_ATTACHMENT)
			return FPipelineStage::COLOR_ATTACHMENT_OUTPUT;
		if (bindPoint == FResourceBindPoint::DEPTH_STENCIL)
			return FPipelineStage::LATE_FRAGMENT_TEST;
		if (bindPoint == FResourceBindPoint::UNIFORM)
			return FPipelineStage::VERTEX_SHADER;
		if (bindPoint == FResourceBindPoint::SAMPLER)
			return FPipelineStage::FRAGMENT_SHADER;
		if (bindPoint == FResourceBindPoint::STORAGE)
			return FPipelineStage::VERTEX_SHADER;
		if (bindPoint == FResourceBindPoint::VERTEX)
			return FPipelineStage::VERTEX_SHADER;
		if (bindPoint == FResourceBindPoint::INDEX)
			return FPipelineStage::BOTTOM_OF_PIPE;
		if (bindPoint == FResourceBindPoint::INDIRECT)
			return FPipelineStage::DRAW_INDIRECT;
		if (bindPoint == FResourceBindPoint::INPUT_ATTACHMENT)
			return FPipelineStage::VERTEX_INPUT;

		return FPipelineStage::NONE;
	}
}