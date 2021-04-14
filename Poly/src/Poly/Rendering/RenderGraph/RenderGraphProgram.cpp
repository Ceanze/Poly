#include "polypch.h"
#include "RenderGraphProgram.h"
#include "Pass.h"
#include "Resource.h"
#include "RenderData.h"
#include "RenderPass.h"
#include "RenderGraph.h"
#include "RenderContext.h"
#include "ResourceCache.h"
#include "RenderGraphTypes.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Platform/API/CommandPool.h"
#include "Platform/API/Framebuffer.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/DescriptorSet.h"
#include "Platform/API/CommandBuffer.h"
#include "Platform/API/PipelineLayout.h"
#include "Platform/API/GraphicsPipeline.h"
#include "Platform/API/GraphicsRenderPass.h"

namespace Poly
{
	RenderGraphProgram::RenderGraphProgram(RenderGraph* pRenderGraph, Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams)
	{
		m_pRenderGraph = pRenderGraph;
		m_pResourceCache = pResourceCache;
		m_DefaultParams = defaultParams;
	}

	void RenderGraphProgram::Init()
	{
		for (uint32 i = 0; i < m_Passes.size(); i++)
		{
			if (!m_Reflections.contains(i))
				m_Reflections[i] = m_Passes[i]->Reflect();
		}

		InitCommandBuffers();
		InitPipelineLayouts();

		// Update all resources in the beginning
		for (uint32 passIndex = 0; const auto& pPass : m_Passes)
		{
			if (pPass->GetPassType() == Pass::Type::SYNC)
			{
				passIndex++;
				continue;
			}

			const auto& reflections = m_Reflections[passIndex].GetAllIOs();
			for (const auto& reflection : reflections)
				UpdateGraphResource(pPass->GetName() + "." + reflection.Name, nullptr);
			passIndex++;
		}
	}

	Ref<RenderGraphProgram> RenderGraphProgram::Create(RenderGraph* pRenderGraph, Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams)
	{
		return CreateRef<RenderGraphProgram>(pRenderGraph, pResourceCache, defaultParams);
	}

	void RenderGraphProgram::Execute(uint32 imageIndex)
	{
		// TODO: When adding conditional passes then that should be checked in this loop
		//		For RenderPasses with a GraphicsRenderPass dependency they should execute just
		//		the GraphicsRenderPass to resolve it, but ignore the rest of the pass.
		m_ImageIndex = imageIndex;

		// Remove old resources
		m_DescriptorsToBeDestroyed[imageIndex].clear(); // Since smart pointers - cleanup is handled automatically

		Ref<RenderContext> renderContext = RenderContext::Create();
		RenderData renderData = RenderData(m_pResourceCache, m_DefaultParams);
		for (uint32 passIndex = 0; const auto& pPass : m_Passes)
		{
			// Set inital pass values
			CommandBuffer* currentCommandBuffer = m_CommandBuffers[passIndex][imageIndex];
			renderContext->SetCommandBuffer(currentCommandBuffer);
			renderData.SetRenderPassName(pPass->GetName());

			// Begin recording
			// currentCommandBuffer->Reset();
			currentCommandBuffer->Begin(FCommandBufferFlag::NONE);

			if (pPass->GetPassType() == Pass::Type::RENDER)
			{
				GraphicsRenderPass*	pGraphicsRenderPass = GetGraphicsRenderPass(pPass, passIndex);
				GraphicsPipeline*	pGraphicsPipeline = GetGraphicsPipeline(pPass, passIndex);
				Framebuffer*		pFramebuffer = GetFramebuffer(pPass, passIndex);
				std::vector<ClearValue> clearValues(static_cast<RenderPass*>(pPass.get())->GetAttachments().size());
				memset(clearValues.data(), 0, sizeof(ClearValue) * clearValues.size());
				for (auto& clearValue : clearValues)
					clearValue.Color.Float32[3] = 1.0f;

				currentCommandBuffer->BeginRenderPass(pGraphicsRenderPass, pFramebuffer, pFramebuffer->GetWidth(), pFramebuffer->GetHeight(), clearValues);
				currentCommandBuffer->BindPipeline(pGraphicsPipeline);

				ViewportDesc viewport = {};
				viewport.Width	= pFramebuffer->GetWidth();
				viewport.Height	= pFramebuffer->GetHeight();
				currentCommandBuffer->SetViewport(&viewport);

				ScissorDesc scissor = {};
				scissor.Width	= pFramebuffer->GetWidth();
				scissor.Height	= pFramebuffer->GetHeight();
				currentCommandBuffer->SetScissor(&scissor);

				const std::vector<Ref<DescriptorSet>>& sets = GetDescriptorSets(pPass, passIndex);
				for (const auto& set : sets)
					currentCommandBuffer->BindDescriptor(pGraphicsPipeline, set.get());

				// The pass handles the draw command and any additional data before that
				pPass->Execute(*renderContext, renderData);

				currentCommandBuffer->EndRenderPass();
			}
			else if (pPass->GetPassType() == Pass::Type::COMPUTE)
			{

			}
			else if (pPass->GetPassType() == Pass::Type::SYNC)
			{
				pPass->Execute(*renderContext, renderData);
			}

			currentCommandBuffer->End();
			// Only graphics queue at the moment
			RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Submit(currentCommandBuffer, nullptr, nullptr, nullptr);
			passIndex++;
		}
	}

	void RenderGraphProgram::UpdateGraphResource(const std::string& name, Ref<Resource> pResource)
	{
		PassResourcePair passPair = GetPassResourcePair(name);

		// Go through each pass and find where resource is being used
		for (uint32 passIndex = 0; const auto& pPass : m_Passes)
		{
			if (pPass->GetPassType() == Pass::Type::SYNC || pPass->GetName() != passPair.first)
			{
				passIndex++;
				continue;
			}

			bool external = false;
			if (passPair.first.empty() || passPair.first == "$")
			{
				const auto& externalResources = pPass->GetExternalResources();
				auto itr = std::find_if(externalResources.begin(), externalResources.end(), [passPair](const std::pair<std::string, std::string>& other){
					return other.first == passPair.second;
				});
				if (itr != externalResources.end())
					external = true;
				else
					continue;
			}
			else
			{
				const auto& reflections = m_Reflections[passIndex].GetIOData(FIOType::INPUT);
				auto itr = std::find_if(reflections.begin(), reflections.end(), [passPair](const IOData& data){ return data.Name == passPair.second; });
				if (itr == reflections.end())
					continue;
			}

			// Resource is being used in pass, update corresponding descriptor
			const IOData& inputRes = m_Reflections[passIndex].GetIOData(passPair.second);
			Ref<Resource> pActualRes = pResource;
			if (!pActualRes)
				pActualRes = m_pResourceCache->GetResource(passPair.first.empty() ? ("$." + passPair.second) : name);

			if (!pActualRes)
			{
				POLY_CORE_ERROR("No resource was gotten from the cache!");
				return;
			}

			const auto& sets = GetDescriptorSets(pPass, passIndex); // Creates set if it doesn't exist yet

			Ref<DescriptorSet> pOldSet = sets[inputRes.Set]; // If crash here - check the sets bound to the pass as it should be too few if this happens

			Ref<DescriptorSet> pNewSet = RenderAPI::CreateDescriptorSetCopy(pOldSet);
			m_DescriptorsToBeDestroyed[m_ImageIndex].push_back(pOldSet);
			m_Descriptors[passIndex][inputRes.Set] = pNewSet;

			if (pActualRes->IsBuffer())
				pNewSet->UpdateBufferBinding(inputRes.Binding, pActualRes->GetAsBuffer(), 0, pActualRes->GetAsBuffer()->GetSize());
			else if (pActualRes->IsTexture())
			{
				// Set sampler if it hasn't been set before from the reflection
				if (!pActualRes->GetAsSampler())
					pActualRes->SetSampler(inputRes.pSampler);
				pNewSet->UpdateTextureBinding(inputRes.Binding, inputRes.TextureLayout, pActualRes->GetAsTextureView(), pActualRes->GetAsSampler());
			}

			passIndex++;
		}
	}

	void RenderGraphProgram::SetBackbuffer(Ref<Resource> pResource)
	{
		m_pResourceCache->SetBackbuffer(pResource);
	}

	void RenderGraphProgram::AddPass(Ref<Pass> pPass)
	{
		m_Passes.push_back(pPass);
	}

	void RenderGraphProgram::InitCommandBuffers()
	{
		m_CommandPools[FQueueType::GRAPHICS] = RenderAPI::CreateCommandPool(FQueueType::GRAPHICS, FCommandPoolFlags::RESET_COMMAND_BUFFERS);

		m_CommandBuffers.resize(m_Passes.size());
		for (uint32 passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			m_CommandBuffers[passIndex].resize(m_DefaultParams.MaxBackbufferCount);
			for (uint32 imageIndex = 0; imageIndex < m_DefaultParams.MaxBackbufferCount; imageIndex++)
			{
				m_CommandBuffers[passIndex][imageIndex] = m_CommandPools[FQueueType::GRAPHICS]->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);
			}
		}
	}

	void RenderGraphProgram::InitPipelineLayouts()
	{
		for (uint32 i = 0; i < m_Reflections.size(); i++)
		{
			if (m_Passes[i]->GetPassType() != Pass::Type::SYNC)
			{
				const auto inputs = m_Reflections[i].GetIOData(FIOType::INPUT);
				std::unordered_map<uint32, DescriptorSetLayout> sets;
				for (const auto& input : inputs)
				{
					DescriptorSetBinding binding = {};
					binding.Binding			= input.Binding;
					binding.DescriptorCount	= 1;
					binding.DescriptorType	= ConvertBindpointToDescriptorType(input.BindPoint);
					binding.ShaderStage		= FShaderStage::VERTEX | FShaderStage::FRAGMENT;
					binding.pSampler		= input.pSampler.get();
					sets[input.Set].DescriptorSetBindings.push_back(binding);
				}

				PipelineLayoutDesc desc = {};
				for (const auto& set : sets)
					desc.DescriptorSetLayouts.push_back(set.second);

				m_PipelineLayouts[i] = RenderAPI::CreatePipelineLayout(&desc);
			}
		}
	}

	GraphicsRenderPass* RenderGraphProgram::GetGraphicsRenderPass(const Ref<Pass>& pPass, uint32 passIndex)
	{
		if (pPass->GetPassType() != Pass::Type::RENDER)
		{
			POLY_CORE_ERROR("Only a render pass can have a graphics render pass!");
			return nullptr;
		}

		if (m_GraphicsRenderPasses.contains(passIndex))
			return m_GraphicsRenderPasses[passIndex].get();

		// If it does not exist yet, create it
		RenderPass* pRenderPass = static_cast<RenderPass*>(pPass.get());
		const auto& attachments = pRenderPass->GetAttachments();
		std::vector<GraphicsRenderPassAttachmentDesc> attachmentDescs;
		std::vector<GraphicsRenderPassSubpassAttachmentReference> colorRefs;
		attachmentDescs.reserve(attachments.size());
		colorRefs.reserve(attachments.size());
		for (auto& attachmentPair : attachments)
		{
			GraphicsRenderPassAttachmentDesc attachmentDesc = {};
			attachmentDesc.Format			= attachmentPair.second.Format;
			attachmentDesc.SampleCount		= 1;
			attachmentDesc.LoadOp			= attachmentPair.second.InitalLayout == ETextureLayout::UNDEFINED ? ELoadOp::CLEAR : ELoadOp::LOAD;
			attachmentDesc.StoreOp			= EStoreOp::STORE;
			attachmentDesc.StencilLoadOp	= ELoadOp::DONT_CARE;
			attachmentDesc.StencilStoreOp	= EStoreOp::DONT_CARE;
			attachmentDesc.InitialLayout	= attachmentPair.second.InitalLayout;
			attachmentDesc.FinalLayout		= attachmentPair.second.FinalLayout;
			attachmentDescs.push_back(attachmentDesc);

			GraphicsRenderPassSubpassAttachmentReference attachmentRef = {};
			attachmentRef.Index		= attachmentPair.second.Index;
			attachmentRef.Layout	= attachmentPair.second.UsedLayout;
			colorRefs.push_back(attachmentRef);
		}

		GraphicsRenderPassSubpassDesc subpassDesc = {};
		subpassDesc.ColorAttachmentsLayouts = colorRefs;

		GraphicsRenderPassSubpassDependencyDesc depDesc = {};
		depDesc.SrcSubpass		= EXTERNAL_SUBPASS;
		depDesc.DstSubpass		= 0;
		depDesc.SrcStageMask	= FPipelineStage::COLOR_ATTACHMENT_OUTPUT;
		depDesc.SrcAccessMask	= FAccessFlag::NONE;
		depDesc.DstStageMask	= FPipelineStage::COLOR_ATTACHMENT_OUTPUT;
		depDesc.DstAccessMask	= FAccessFlag::COLOR_ATTACHMENT_READ | FAccessFlag::COLOR_ATTACHMENT_WRITE;

		GraphicsRenderPassDesc renderPassDesc = {};
		renderPassDesc.Attachments			= attachmentDescs;
		renderPassDesc.Subpasses			= { subpassDesc };
		renderPassDesc.SubpassDependencies	= { depDesc };

		m_GraphicsRenderPasses[passIndex] = RenderAPI::CreateGraphicsRenderPass(&renderPassDesc);

		return m_GraphicsRenderPasses[passIndex].get();
	}

	Framebuffer* RenderGraphProgram::GetFramebuffer(const Ref<Pass>& pPass, uint32 passIndex)
	{
		// If we have already created it, return it
		if (m_Framebuffers.contains(passIndex))
		{
			if (m_Framebuffers[passIndex][m_ImageIndex]) // Check if current image index has a created framebuffer
				return m_Framebuffers[passIndex][m_ImageIndex].get();
		}

		// Allocate array for future each image index - only first time pass does not have a framebuffer
		if (!m_Framebuffers.contains(passIndex))
			m_Framebuffers[passIndex].resize(m_DefaultParams.MaxBackbufferCount);

		// Create it (get it from the cache)
		RenderPass* renderPass = static_cast<RenderPass*>(pPass.get());
		std::vector<TextureView*> attachments;
		TextureView* pDepthAttachment	= nullptr;
		uint32 width					= 0;
		uint32 height					= 0;

		const auto& attachmentInfos = renderPass->GetAttachments();
		attachments.reserve(attachmentInfos.size());
		for (const auto& a : attachmentInfos)
		{
			Ref<Resource> res = m_pResourceCache->GetResource(renderPass->GetName() + "." + a.first);

			if (!width || !height)
			{
				width = res->GetAsTexture()->GetWidth();
				height = res->GetAsTexture()->GetHeight();
			}

			if (a.second.UsedLayout == ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				pDepthAttachment == res->GetAsTextureView();
			else
				attachments.push_back(res->GetAsTextureView());
		}

		Ref<Framebuffer> framebuffer = RenderAPI::GetFramebuffer(attachments, pDepthAttachment, GetGraphicsRenderPass(pPass, passIndex), width, height);
		m_Framebuffers[passIndex][m_ImageIndex] = framebuffer;

		return m_Framebuffers[passIndex][m_ImageIndex].get();
	}

	GraphicsPipeline* RenderGraphProgram::GetGraphicsPipeline(const Ref<Pass>& pPass, uint32 passIndex)
	{
		// Return if we already have created it
		if (m_GraphicsPipelines.contains(passIndex))
			return m_GraphicsPipelines[passIndex].get();

		// Create it if it didn't exist TODO: Make this more programmable
		InputAssemblyDesc assembly = {};
		assembly.RestartPrimitive	= false;
		assembly.Topology			= ETopology::TRIANGLE_LIST;

		ViewportDesc viewport = {};
		viewport.IsDynamic = true;
		// viewport.Width		= static_cast<float>(m_Framebuffers[passIndex]->GetWidth());
		// viewport.Height		= static_cast<float>(m_Framebuffers[passIndex]->GetHeight());

		ScissorDesc scissor = {};
		scissor.IsDynamic = true;
		// scissor.Width		= m_Framebuffers[passIndex]->GetWidth();
		// scissor.Height		= m_Framebuffers[passIndex]->GetHeight();

		RasterizationDesc raster = {};
		raster.DepthClampEnable		= false;
		raster.DiscardEnable		= false;
		raster.PolygonMode			= EPolygonMode::FILL;
		raster.CullMode				= ECullMode::NONE;
		raster.ClockwiseFrontFace	= false;
		raster.DepthBiasEnable		= false;

		ColorBlendAttachmentDesc colorBlendAttachment = {};
		colorBlendAttachment.ColorWriteMask		= FColorComponentFlag::RED | FColorComponentFlag::GREEN | FColorComponentFlag::BLUE | FColorComponentFlag::ALPHA;
		colorBlendAttachment.BlendEnable		= false;

		ColorBlendStateDesc colorBlend = {};
		colorBlend.LogicOpEnable			= false;
		colorBlend.ColorBlendAttachments	= { colorBlendAttachment };

		GraphicsPipelineDesc desc = {};
		desc.InputAssembly		= assembly;
		desc.Viewport			= viewport;
		desc.Scissor			= scissor;
		desc.Rasterization		= raster;
		desc.ColorBlendState	= colorBlend;
		desc.pPipelineLayout	= m_PipelineLayouts[passIndex].get();
		desc.pRenderPass		= m_GraphicsRenderPasses[passIndex].get();
		desc.pVertexShader		= pPass->GetShader(FShaderStage::VERTEX).get();
		desc.pFragmentShader	= pPass->GetShader(FShaderStage::FRAGMENT).get();

		m_GraphicsPipelines[passIndex] = RenderAPI::CreateGraphicsPipeline(&desc);

		return m_GraphicsPipelines[passIndex].get();
	}

	const std::vector<Ref<DescriptorSet>>& RenderGraphProgram::GetDescriptorSets(const Ref<Pass>& pPass, uint32 passIndex)
	{
		if (m_Descriptors.contains(passIndex))
			return m_Descriptors[passIndex];

		// Collect the sets from the reflection
		const auto inputs = m_Reflections[passIndex].GetIOData(FIOType::INPUT);
		std::unordered_set<uint32> setIndicies;
		for (const auto& input : inputs)
			setIndicies.insert(input.Set);

		//for (uint32 imageIndex = 0; imageIndex < m_DefaultParams.MaxBackbufferCount; imageIndex++)
		//{
		m_Descriptors[passIndex].resize(setIndicies.size());
		for (uint32 setIndex : setIndicies)
			m_Descriptors[passIndex][setIndex] = RenderAPI::CreateDescriptorSet(m_PipelineLayouts[passIndex].get(), setIndex);
		//}

		return m_Descriptors[passIndex];
	}
}