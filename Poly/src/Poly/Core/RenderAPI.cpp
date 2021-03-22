#include "polypch.h"
#include "RenderAPI.h"

#include "Platform/Vulkan/PVKInstance.h"
#include "Poly/Core/Window.h"
#include "Platform/API/Sampler.h"
#include "Platform/API/CommandQueue.h"

namespace Poly
{
	void RenderAPI::Init(BackendAPI backendAPI, Window* pWindow)
	{
		m_pWindow = pWindow;
		switch (backendAPI)
		{
		case BackendAPI::VULKAN:
		{
			m_pGraphicsInstance = new PVKInstance();
			m_pGraphicsInstance->Init(pWindow);
			break;
		}
		default:
			POLY_VALIDATE(false, "No valid backendAPI selected!");
		}

		Sampler::InitDefaults();
	}

	void RenderAPI::Release()
	{
		if (m_pGraphicsInstance)
		{
			Sampler::CleanupDefaults();

			delete m_pGraphicsInstance;
		}
	}

	CommandQueue* RenderAPI::GetCommandQueue(FQueueType queue)
	{
		if (queue == FQueueType::GRAPHICS)
		{
			if (!m_pGraphicsQueue)
				m_pGraphicsQueue = CreateCommandQueue(queue, 0);

			return m_pGraphicsQueue.get();
		}

		if (queue == FQueueType::COMPUTE)
		{
			if (!m_pComputeQueue)
				m_pComputeQueue = CreateCommandQueue(queue, 0);

			return m_pComputeQueue.get();
		}

		if (queue == FQueueType::TRANSFER)
		{
			if (!m_pTransferQueue)
				m_pTransferQueue = CreateCommandQueue(queue, 0);

			return m_pTransferQueue.get();
		}

		POLY_CORE_WARN("Tried to get unknown or NONE FQueueType in GetCommandQueue, returning nullptr");
		return nullptr;
	}

	Ref<Buffer> RenderAPI::CreateBuffer(const BufferDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateBuffer(pDesc);
	}

	Ref<Texture> RenderAPI::CreateTexture(const TextureDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateTexture(pDesc);
	}

	Ref<CommandQueue> RenderAPI::CreateCommandQueue(FQueueType queueType, uint32 queueIndex)
	{
		return m_pGraphicsInstance->CreateCommandQueue(queueType, queueIndex);
	}

	Ref<TextureView> RenderAPI::CreateTextureView(const TextureViewDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateTextureView(pDesc);
	}

	Ref<SwapChain> RenderAPI::CreateSwapChain(const SwapChainDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateSwapChain(pDesc);
	}

	Ref<Fence> RenderAPI::CreateFence(FFenceFlag flag)
	{
		return m_pGraphicsInstance->CreateFence(flag);
	}

	Ref<Semaphore> RenderAPI::CreateSemaphore()
	{
		return m_pGraphicsInstance->CreateSemaphoreW();
	}

	Ref<CommandPool> RenderAPI::CreateCommandPool(FQueueType queueType)
	{
		return m_pGraphicsInstance->CreateCommandPool(queueType);
	}

	Ref<Sampler> RenderAPI::CreateSampler(const SamplerDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateSampler(pDesc);
	}

	Ref<Shader> RenderAPI::CreateShader(const ShaderDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateShader(pDesc);
	}

	Ref<GraphicsRenderPass> RenderAPI::CreateGraphicsRenderPass(const GraphicsRenderPassDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateGraphicsRenderPass(pDesc);
	}

	Ref<GraphicsPipeline> RenderAPI::CreateGraphicsPipeline(const GraphicsPipelineDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateGraphicsPipeline(pDesc);
	}

	Ref<PipelineLayout> RenderAPI::CreatePipelineLayout(const PipelineLayoutDesc* pDesc)
	{
		return m_pGraphicsInstance->CreatePipelineLayout(pDesc);
	}

	Ref<Framebuffer> RenderAPI::CreateFramebuffer(const FramebufferDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateFramebuffer(pDesc);
	}

	Ref<DescriptorSet> RenderAPI::CreateDescriptorSet(PipelineLayout* pLayout, uint32 setIndex)
	{
		return m_pGraphicsInstance->CreateDescriptorSet(pLayout, setIndex);
	}


}