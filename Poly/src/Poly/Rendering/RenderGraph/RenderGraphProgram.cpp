#include "polypch.h"
#include "RenderGraphProgram.h"
#include "Pass.h"
#include "Resource.h"
#include "RenderData.h"
#include "RenderPass.h"
#include "RenderGraph.h"
#include "RenderContext.h"
#include "ResourceCache.h"
#include "Poly/Model/Mesh.h"
#include "Poly/Scene/Scene.h"
#include "RenderGraphTypes.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Rendering/RenderScene.h"
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
#include "Poly/Resources/Shader/ShaderManager.h"
#include "Poly/Rendering/Utilities/StagingBufferCache.h"

namespace Poly
{
	RenderGraphProgram::RenderGraphProgram(Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams, std::vector<PassData> passes)
		: m_pResourceCache(pResourceCache)
		, m_DefaultParams(defaultParams)
		, m_Passes(passes) {}

	void RenderGraphProgram::Init()
	{
		for (uint32 passIndex = 0; auto& passData : m_Passes)
			passData.PassIndex = passIndex++;

		InitCommandBuffers();
		InitPipelineLayouts();

		// Update all resources in the beginning
		for (const auto& [pPass, reflection, _, passIndex] : m_Passes)
		{
			if (pPass->GetPassType() == Pass::Type::SYNC)
			{
				continue;
			}

			const auto& reflections = reflection.GetFields(FFieldVisibility::IN_OUT);
			if (!reflections.empty())
			{
				m_DescriptorCaches[passIndex].SetPipelineLayout(m_PipelineLayouts[passIndex].get());
				for (const auto& reflection : reflections)
				{
					// Only update graph resource if a resource is already valid, otherwise skip
					if (m_pResourceCache->HasResource({ pPass->GetName(), reflection->GetName()}))
						UpdateGraphResource({ pPass->GetName(), reflection->GetName()}, nullptr);
				}
			}
		}

		m_pStagingBufferCache = StagingBufferCache::Create();
	}

	Ref<RenderGraphProgram> RenderGraphProgram::Create(Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams, std::vector<PassData> passes)
	{
		return CreateRef<RenderGraphProgram>(pResourceCache, defaultParams, passes);
	}

	void RenderGraphProgram::Execute(uint32 imageIndex)
	{
		// TODO: When adding conditional passes then that should be checked in this loop
		//		For RenderPasses with a GraphicsRenderPass dependency they should execute just
		//		the GraphicsRenderPass to resolve it, but ignore the rest of the pass.
		m_ImageIndex = imageIndex;

		RenderContext renderContext = RenderContext();
		RenderData renderData = RenderData(m_pResourceCache, m_DefaultParams);
		renderContext.SetImageIndex(m_ImageIndex);
		renderData.SetScene(m_pScene.get());
		for (const auto& [pPass, reflection, _, passIndex] : m_Passes)
		{
			// Clear old descriptors
			if (m_DescriptorCaches.contains(passIndex))
				m_DescriptorCaches[passIndex].Update();

			// Set inital pass values
			CommandBuffer* currentCommandBuffer = m_CommandBuffers[passIndex][imageIndex];
			renderContext.SetCommandBuffer(currentCommandBuffer);
			renderContext.SetActivePassIndex(passIndex);
			renderContext.SetActivePipelineLayout(m_PipelineLayouts[passIndex].get());
			renderContext.SetRenderGraphProgram(this);
			renderContext.SetDescriptorCache(&(m_DescriptorCaches[passIndex]));
			renderData.SetRenderPassName(pPass->GetName());

			// Begin recording
			currentCommandBuffer->Begin(FCommandBufferFlag::NONE);

			// Update current pass before executing it
			pPass->Update(renderContext);

			// Transfer staging buffer data
			// TODO: This should probably be done on the Transfer queue asyncronously
			m_pStagingBufferCache->SubmitQueuedBuffers(currentCommandBuffer);

			if (pPass->GetPassType() == Pass::Type::RENDER)
			{
				RenderPass*			pRenderPass = static_cast<RenderPass*>(pPass.get());
				GraphicsRenderPass*	pGraphicsRenderPass = GetGraphicsRenderPass(pPass, passIndex);
				GraphicsPipeline*	pGraphicsPipeline = GetGraphicsPipeline(pPass, passIndex);
				Framebuffer*		pFramebuffer = GetFramebuffer(pPass, passIndex);

				std::vector<ClearValue> clearValues(pRenderPass->GetAttachments().size());
				for (uint32 i = 0; i < clearValues.size() - pRenderPass->GetDepthStenctilUse() ? 1 : 0; i++)
					clearValues[i].Color.Float32[3] = 1.0f;
				if (pRenderPass->GetDepthStenctilUse())
					clearValues.back().DepthStencil.Depth = 1.0f;

				currentCommandBuffer->BeginRenderPass(pGraphicsRenderPass, pFramebuffer, pFramebuffer->GetWidth(), pFramebuffer->GetHeight(), clearValues);
				currentCommandBuffer->BindPipeline(pGraphicsPipeline);

				ViewportDesc viewport = {};
				viewport.Width	= static_cast<float>(pFramebuffer->GetWidth());
				viewport.Height	= static_cast<float>(pFramebuffer->GetHeight());
				currentCommandBuffer->SetViewport(&viewport);

				ScissorDesc scissor = {};
				scissor.Width	= pFramebuffer->GetWidth();
				scissor.Height	= pFramebuffer->GetHeight();
				currentCommandBuffer->SetScissor(&scissor);

				renderContext.SetActivePipeline(pGraphicsPipeline);

				const std::vector<SceneBatch>& batches = m_pScene->GetRenderScene()->GetBatches();
				uint32 batchSize = 1;
				if (pPass->IsInstancedSceneRenderingEnabled())
					batchSize = static_cast<uint32>(batches.size());

				for (uint32 batchIndex = 0; batchIndex < batchSize; batchIndex++) {
					renderContext.SetSceneBatch(&batches[batchIndex]);
					renderContext.SetBatchIndex(batchIndex);

					const std::set<uint32>& setIndices = reflection.GetAutoBindedSets();
					for (uint32 setIndex : setIndices)
					{
						const DescriptorSet* pSet = m_DescriptorCaches[passIndex].GetDescriptorSet(setIndex, batchIndex, DescriptorCache::EAction::GET);
						if (pSet)
							currentCommandBuffer->BindDescriptor(pGraphicsPipeline, pSet);
					}

					// The pass handles the draw command and any additional data before that
					pPass->Execute(renderContext, renderData);

				}

				currentCommandBuffer->EndRenderPass();
			}
			else if (pPass->GetPassType() == Pass::Type::COMPUTE)
			{

			}
			else if (pPass->GetPassType() == Pass::Type::SYNC)
			{
				pPass->Execute(renderContext, renderData);
			}

			currentCommandBuffer->End();

			// Only graphics queue at the moment
			RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Submit(currentCommandBuffer, nullptr, nullptr, nullptr);
		}

		m_pStagingBufferCache->Update(m_ImageIndex);
	}

	void RenderGraphProgram::CreateResource(ResourceGUID resourceGUID, uint64 size, const void* data, FBufferUsage bufferUsage, uint64 offset, uint32 index)
	{
		if (!resourceGUID.IsExternal())
		{
			POLY_CORE_WARN("Resource {} is not external. Only external resource can use CreateResource()", resourceGUID.GetFullName());
			return;
		}

		if (!m_pResourceCache->IsResourceRegistered(resourceGUID))
		{
			POLY_CORE_WARN("External resource {} has not been registered. A resource must be registered before being created", resourceGUID.GetFullName());
			return;
		}

		BufferDesc desc = {};
		desc.Size = size;
		desc.MemUsage = EMemoryUsage::GPU_ONLY;
		desc.BufferUsage = bufferUsage | FBufferUsage::COPY_DST;
		Ref<Buffer> pBuffer = RenderAPI::CreateBuffer(&desc);

		Ref<Resource> pResource = Resource::Create(pBuffer, resourceGUID.GetResourceName());

		m_pResourceCache->RegisterExternalResource(resourceGUID, { pResource, true }); // TODO: Check autoBindDescriptor ("true")

		if (data)
			m_pStagingBufferCache->QueueTransfer(pBuffer.get(), size, offset, data);

	}

	bool RenderGraphProgram::HasResource(ResourceGUID resourceGUID) const
	{
		return m_pResourceCache->HasResource(resourceGUID);
	}

	void RenderGraphProgram::UpdateGraphResource(ResourceGUID resourceGUID, const Resource* pResource, uint32 index)
	{
		// Note: This function always updates a whole span of a resource - therefore no size or offset is supplied, and views for
		// that whole resource are automatically created with default span (resource size) and default offset (0)

		if (!pResource)
		{
			UpdateGraphResource(resourceGUID, ResourceView::Empty(), index);
		}
		else if (pResource->IsBuffer())
		{
			const Buffer* pBuffer = pResource->GetAsBuffer();
			UpdateGraphResource(resourceGUID, {pBuffer, pBuffer->GetSize(), 0}, index);
		}
		else if (pResource->IsTexture())
		{
			const TextureView* pTextureView = pResource->GetAsTextureView();
			const Sampler* pSampler = pResource->GetAsSampler();
			UpdateGraphResource(resourceGUID, {pTextureView, pSampler}, index);
		}
	}

	void RenderGraphProgram::UpdateGraphResource(ResourceGUID resourceGUID, ResourceView view, uint32 index)
	{
		// Go through each pass and find where resource is being used
		for (const auto& [pPass, reflection, _, passIndex] : m_Passes)
		{
			if (pPass->GetPassType() == Pass::Type::SYNC)
				continue;

			// Check if pass has the requested resource, continue if not
			const ResourceGUID mappedResource = GetMappedResourceGUID(resourceGUID, pPass, passIndex);
			if (!mappedResource.HasResource())
				continue;

			// Resource is being used in pass, update corresponding descriptor
			const PassField& inputRes = reflection.GetField(mappedResource.GetResourceName());
			if (BitsSet(inputRes.GetBindPoint(), FResourceBindPoint::COLOR_ATTACHMENT))
				continue;

			Resource* pResource = nullptr;
			bool hasProvidedResource = view.HasBuffer() || view.HasTextureView();
			if (!hasProvidedResource && BitsSet(inputRes.GetBindPoint(), FResourceBindPoint::INTERNAL_USE))
				return;

			if (!hasProvidedResource)
				pResource = m_pResourceCache->HasResource(mappedResource.GetFullName()) ? m_pResourceCache->GetResource(mappedResource.GetFullName()) : nullptr;

			if (!hasProvidedResource && !pResource)
			{
				POLY_CORE_ERROR("No resource was gotten from the cache!");
				return;
			}

			if (!m_DescriptorCaches.contains(passIndex))
				m_DescriptorCaches[passIndex].SetPipelineLayout(m_PipelineLayouts[passIndex].get());

			DescriptorSet* pNewSet = m_DescriptorCaches[passIndex].GetDescriptorSetCopy(inputRes.GetSet(), index, static_cast<uint32>(view.GetOffset()), static_cast<uint32>(view.GetSpan()));

			if ((pResource && pResource->IsBuffer()) || view.HasBuffer())
			{
				const Buffer* pBuffer = (pResource && pResource->IsBuffer()) ? pResource->GetAsBuffer() : view.GetBuffer();
				const uint64 span = view.GetSpan() > 0 ? view.GetSpan() : pBuffer->GetSize();
				pNewSet->UpdateBufferBinding(inputRes.GetBinding(), pBuffer, 0, span);
			}
			else if ((pResource && pResource->IsTexture()) || view.HasTextureView())
			{
				const TextureView* pTextureView = (pResource && pResource->IsTexture()) ? pResource->GetAsTextureView() : view.GetTextureView();
				Sampler* pSampler = pResource ? pResource->GetAsSampler() : (inputRes.GetSampler() ? inputRes.GetSampler().get() : m_DefaultParams.pSampler.get());

				// Set sampler if it hasn't been set before from the reflection
				if (pResource && !pResource->GetAsSampler())
					pResource->SetSampler(inputRes.GetSampler() ? inputRes.GetSampler() : m_DefaultParams.pSampler);
				pNewSet->UpdateTextureBinding(inputRes.GetBinding(), inputRes.GetTextureLayout(), pTextureView, pSampler);
			}
		}
	}

	void RenderGraphProgram::UpdateGraphResource(ResourceGUID resourceGUID, uint64 size, const void* data, uint64 offset, uint32 index)
	{
		const Resource* pRes = m_pResourceCache->GetResource(resourceGUID);

		if (!pRes)
		{
			POLY_CORE_ERROR("UpdateGraphResource - cannot update resource {} as it was not in the resource cache", resourceGUID.GetResourceName());
			return;
		}

		if (!pRes->IsBuffer())
		{
			POLY_CORE_WARN("UpdateGraphResource(data*) can only update buffers, but a non-buffer resource was provided.");
			return;
		}

		const Buffer* pBuffer = pRes->GetAsBuffer();
		uint64 oldSize = pBuffer->GetSize();
		bool hasSizeChanged = oldSize != size;
		if (oldSize < size)
		{
			pRes = m_pResourceCache->UpdateResourceSize(resourceGUID, size);
		}

		m_pStagingBufferCache->QueueTransfer(pRes->GetAsBuffer(), size, offset, data);

		//if (hasSizeChanged)
		//{
			UpdateGraphResource(resourceGUID, { pBuffer, size, offset }, index);
			//UpdateGraphResource(resourceGUID, pRes, index);
		//}
		//else
		//{
		//	UpdateGraphResource(resourceGUID, { pBuffer, size, offset }, offset, index);
		//	UpdateGraphResource(resourceGUID, nullptr, index);
		//}
	}

	void RenderGraphProgram::SetBackbuffer(Ref<Resource> pResource)
	{
		m_pResourceCache->SetBackbuffer(pResource);
	}

	void RenderGraphProgram::RecreateResources(uint32 width, uint32 height)
	{
		m_Framebuffers.clear();
		m_pResourceCache->ReallocateBackbufferBoundResources(width, height);
	}

	void RenderGraphProgram::SetScene(const Ref<Scene>& pScene)
	{
		m_pScene = pScene;
		pScene->CreateRenderScene(*this);
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
		for (const auto& [pPass, reflection, _, passIndex] : m_Passes)
		{
			if (pPass->GetPassType() != Pass::Type::SYNC)
			{
				// Note that the layout creates the bindings for internal types as well for ease of use
				const std::vector<const PassField*> inputs = reflection.GetFieldsFiltered(FFieldVisibility::INPUT, FResourceBindPoint::VERTEX | FResourceBindPoint::INDEX);
				const auto& pushConstants = reflection.GetPushConstants();
				uint32 maxSet = 0;
				for (const PassField* input : inputs)
				{
					if (input->GetSet() > maxSet)
						maxSet = input->GetSet();
				}

				std::vector<DescriptorSetLayout> sets(maxSet + 1);
				for (const PassField* input : inputs)
				{
					if (BitsSet(input->GetBindPoint(), FResourceBindPoint::COLOR_ATTACHMENT))
						continue;
					DescriptorSetBinding binding = {};
					binding.Binding			= input->GetBinding();
					binding.DescriptorCount	= 1;
					binding.DescriptorType	= ConvertBindpointToDescriptorType(input->GetBindPoint());
					binding.ShaderStage		= FShaderStage::VERTEX | FShaderStage::FRAGMENT;
					binding.pSampler		= input->GetSampler().get();
					sets[input->GetSet()].DescriptorSetBindings.push_back(binding);
				}

				std::vector<PushConstantRange> ranges;
				ranges.reserve(pushConstants.size());
				for (const auto& pushConstant : pushConstants)
				{
					PushConstantRange range;
					range.Size			= static_cast<uint32>(pushConstant.Size);
					range.Offset		= static_cast<uint32>(pushConstant.Offset);
					range.ShaderStage	= pushConstant.ShaderStage;
					ranges.push_back(range);
				}

				PipelineLayoutDesc desc = {};
				desc.PushConstantRanges = ranges;
				for (const auto& set : sets)
					desc.DescriptorSetLayouts.push_back(set);

				m_PipelineLayouts[passIndex] = RenderAPI::CreatePipelineLayout(&desc);
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
		GraphicsRenderPassSubpassAttachmentReference depthStencilAttachment = {};
		for (const auto& attachment : attachments)
		{
			GraphicsRenderPassAttachmentDesc attachmentDesc = {};
			attachmentDesc.Format			= attachment.Format;
			attachmentDesc.SampleCount		= 1;
			attachmentDesc.LoadOp			= attachment.InitalLayout == ETextureLayout::UNDEFINED ? ELoadOp::CLEAR : ELoadOp::LOAD;
			attachmentDesc.StoreOp			= attachment.UsedLayout == ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL ? EStoreOp::DONT_CARE : EStoreOp::STORE;
			attachmentDesc.StencilLoadOp	= ELoadOp::DONT_CARE;
			attachmentDesc.StencilStoreOp	= EStoreOp::DONT_CARE;
			attachmentDesc.InitialLayout	= attachment.InitalLayout;
			attachmentDesc.FinalLayout		= attachment.FinalLayout;
			attachmentDescs.push_back(attachmentDesc);

			if (attachment.UsedLayout == ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				depthStencilAttachment.Index	= attachment.Index;
				depthStencilAttachment.Layout	= attachment.UsedLayout;
			}
			else
			{
				GraphicsRenderPassSubpassAttachmentReference attachmentRef = {};
				attachmentRef.Index		= attachment.Index;
				attachmentRef.Layout	= attachment.UsedLayout;
				colorRefs.push_back(attachmentRef);
			}
		}

		GraphicsRenderPassSubpassDesc subpassDesc = {};
		subpassDesc.ColorAttachmentsLayouts = colorRefs;
		subpassDesc.DepthStencilAttachmentLayout = depthStencilAttachment;

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
		if (pPass->GetPassType() != Pass::Type::RENDER)
		{
			POLY_CORE_ERROR("Cannot get/create a framebuffer for non-render passes. Pass {} with passIndex {} is not a render pass", pPass->GetName(), passIndex);
			return nullptr;
		}

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
		for (const auto& attachmentInfo : attachmentInfos)
		{
			Resource* pRes = m_pResourceCache->GetResource({ renderPass->GetName(), attachmentInfo.Name });

			if (!width || !height)
			{
				width = pRes->GetAsTexture()->GetWidth();
				height = pRes->GetAsTexture()->GetHeight();
			}

			if (attachmentInfo.UsedLayout == ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				pDepthAttachment = pRes->GetAsTextureView();
			else
				attachments.push_back(pRes->GetAsTextureView());
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
		raster.CullMode				= ECullMode::BACK;
		raster.ClockwiseFrontFace	= false;
		raster.DepthBiasEnable		= false;

		ColorBlendAttachmentDesc colorBlendAttachment = {};
		colorBlendAttachment.ColorWriteMask		= FColorComponentFlag::RED | FColorComponentFlag::GREEN | FColorComponentFlag::BLUE | FColorComponentFlag::ALPHA;
		colorBlendAttachment.BlendEnable		= false;

		ColorBlendStateDesc colorBlend = {};
		colorBlend.LogicOpEnable			= false;
		colorBlend.ColorBlendAttachments	= { colorBlendAttachment };

		DepthStencilDesc depthStencil = {};
		if (static_cast<RenderPass*>(pPass.get())->GetDepthStenctilUse())
		{
			depthStencil.DepthTestEnable		= true;
			depthStencil.DepthWriteEnable		= true;
			depthStencil.DepthCompareOp			= ECompareOp::LESS_OR_EQUAL;
			// depthStencil.DepthBoundsTestEnable	= false;
			// depthStencil.MinDepthBounds			= 0.0f;
			// depthStencil.MaxDepthBounds			= 1.0f;
			depthStencil.StencilTestEnable		= false; // TODO: Allow stencil part to be used by the user in future
			// depthStencil.Front;
			// depthStencil.Back;
		}

		GraphicsPipelineDesc desc = {};
		GraphicsPipelineDesc* pPipelineDesc = static_cast<GraphicsPipelineDesc*>(pPass->GetCustomPipelineDesc());
		if (pPipelineDesc)
		{
			desc.VertexInputs		= pPipelineDesc->VertexInputs;
			desc.InputAssembly		= pPipelineDesc->InputAssembly;
			desc.Viewport			= pPipelineDesc->Viewport;
			desc.Scissor			= pPipelineDesc->Scissor;
			desc.Rasterization		= pPipelineDesc->Rasterization;
			desc.ColorBlendState	= pPipelineDesc->ColorBlendState;
			desc.DepthStencil		= pPipelineDesc->DepthStencil;
		}
		else
		{
			desc.InputAssembly		= assembly;
			desc.Viewport			= viewport;
			desc.Scissor			= scissor;
			desc.Rasterization		= raster;
			desc.ColorBlendState	= colorBlend;
			desc.DepthStencil		= depthStencil;
		}

		desc.pPipelineLayout	= m_PipelineLayouts[passIndex].get();
		desc.pRenderPass		= m_GraphicsRenderPasses[passIndex].get();
		desc.pVertexShader		= ShaderManager::GetShader(pPass->GetShaderID(FShaderStage::VERTEX)).pShader.get();
		desc.pFragmentShader	= ShaderManager::GetShader(pPass->GetShaderID(FShaderStage::FRAGMENT)).pShader.get();

		m_GraphicsPipelines[passIndex] = RenderAPI::CreateGraphicsPipeline(&desc);

		return m_GraphicsPipelines[passIndex].get();
	}

	ResourceGUID RenderGraphProgram::GetMappedResourceGUID(const ResourceGUID& resourceGUID, const Ref<Pass>& pPass, uint32 passIndex)
	{
		ResourceGUID mappedResource = m_pResourceCache->GetMappedResourceName(resourceGUID, pPass->GetName());
		if (!mappedResource.HasResource())
			return ResourceGUID::Invalid();

		const PassReflection& reflection = m_Passes[passIndex].Reflection;
		const std::vector<const PassField*> reflections = reflection.GetFields(FFieldVisibility::INPUT);
		auto itr = std::find_if(reflections.begin(), reflections.end(), [&mappedResource](const PassField* data){ return data->GetName() == mappedResource.GetResourceName(); });
		if (itr == reflections.end())
			return ResourceGUID::Invalid();

		return mappedResource;
	}
}
