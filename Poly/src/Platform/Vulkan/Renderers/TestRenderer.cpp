#include "polypch.h"
#include "TestRenderer.h"

#include "Poly/Core/Camera.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include "Poly/Core/RenderAPI.h"

namespace Poly
{

	void TestRenderer::Init(IPlatformRenderer* renderer)
	{
		m_pMainRenderer = static_cast<VulkanRenderer*>(renderer);
		m_pSwapChain = m_pMainRenderer->GetSwapChain();

		m_Shader.AddStage(FShaderStage::VERTEX, "vert.glsl");
		m_Shader.AddStage(FShaderStage::FRAGMENT, "frag.glsl");
		m_Shader.Init();

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		m_RenderPass.AddSubpassDependency(dependency);
		m_RenderPass.Init(*m_pSwapChain);

		SetupDescriptorSet();
		m_Pipeline.SetDescriptor(m_Descriptor);
		m_Pipeline.Init(*m_pSwapChain, m_Shader, m_RenderPass);

		// Until this line
		m_CommandPool.Init(FQueueType::GRAPHICS);

		m_Framebuffers.resize(m_pSwapChain->GetNumImages());
		for (size_t i = 0; i < m_pSwapChain->GetNumImages(); i++)
			m_Framebuffers[i].Init(*m_pSwapChain, m_RenderPass, m_pSwapChain->GetImageViews()[i]);

		SetupTestData();
		CreateCommandBuffers();
	}

	void TestRenderer::BeginScene(uint32_t imageIndex)
	{
		m_ImageIndex = imageIndex;
	}

	void TestRenderer::Record()
	{
	}

	void TestRenderer::EndScene()
	{
		m_pMainRenderer->AddCommandBuffer(FQueueType::GRAPHICS, m_CommandBuffers[m_ImageIndex]);
		//m_TestMemory.directTransfer(m_TestBuffer, &m_pCamera->GetMatrix(), sizeof(glm::mat4), 0);
		m_TestBuffer.TransferData(&m_pCamera->GetMatrix(), sizeof(glm::mat4));
	}

	void TestRenderer::Shutdown()
	{
		PVK_CHECK(vkDeviceWaitIdle(PVKInstance::GetDevice()), "Failed to wait for device!");

		delete m_pTestSampler;
		m_CommandPool.Cleanup();
		PVK_VEC_CLEANUP(m_Framebuffers);
		m_Pipeline.Cleanup();
		m_Descriptor.Cleanup();
		m_TestBuffer.Cleanup();
		m_RenderPass.Cleanup();
		m_Shader.Cleanup();
	}

	void TestRenderer::CreateCommandBuffers()
	{
		m_CommandBuffers = m_CommandPool.CreateCommandBuffers(3);

		for (uint32_t i = 0; i < m_CommandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			m_CommandBuffers[i]->Begin(0);
			m_CommandBuffers[i]->BeginRenderPass(&m_RenderPass, &m_Framebuffers[i], m_pSwapChain->GetExtent(), { 0.0f, 0.0f, 0.0f, 1.0f });
			m_CommandBuffers[i]->BindPipeline(&m_Pipeline);
			m_CommandBuffers[i]->BindDescriptor(&m_Pipeline, &m_Descriptor, i);
			m_CommandBuffers[i]->Draw(3, 1, 0, 0);
			m_CommandBuffers[i]->EndRenderPass();
			m_CommandBuffers[i]->End();
		}
	}

	void TestRenderer::SetupDescriptorSet()
	{
		m_Descriptor.AddBinding(0, 0, BufferType::UNIFORM, FShaderStage::VERTEX);
		m_Descriptor.AddBinding(0, 1, BufferType::COMBINED_IMAGE_SAMPLER, FShaderStage::FRAGMENT);
		m_Descriptor.FinalizeSet(0);
		m_Descriptor.Init(m_pSwapChain->GetNumImages());
	}

	void TestRenderer::SetupTestData()
	{
		m_pTestSampler = new PVKSampler();
		//m_TestTexture.Init(1, 1, ColorFormat::R8G8B8A8_SRGB, ImageUsage::SAMPLED, ImageCreate::NONE, 1, PVKInstance::GetQueue(QueueType::GRAPHICS).queueIndex, VMA_MEMORY_USAGE_GPU_ONLY);
		//m_TestTexture.InitView(ImageViewType::DIM_2, ImageAspect::COLOR_BIT);

		TextureDesc textureDesc = {
			.Width			= 1,
			.Height			= 1,
			.Depth			= 1,
			.ArrayLayers	= 1,
			.MipLevels		= 1,
			.MemoryUsage	= EMemoryUsage::GPU_ONLY,
			.Format			= EFormat::R8G8B8A8_UNORM,
			.TextureUsage	= FTextureUsage::SAMPLED,
			.TextureDim		= ETextureDim::DIM_2D,
			.ImageViewType	= EImageViewType::TYPE_2D,
			.ImageViewFlags	= FImageViewFlag::SHADER_RESOURCE
		};
		m_TestTexture.Init(&textureDesc);

		BufferDesc bufferDesc = {
			.Size			= sizeof(glm::mat4),
			.MemUsage		= EMemoryUsage::CPU_GPU_MAPPABLE,
			.BufferUsage	= FBufferUsage::UNIFORM_BUFFER
		};
		m_TestBuffer.Init(&bufferDesc);

		m_Descriptor.UpdateBufferBinding(0, 0, &m_TestBuffer);
		m_Descriptor.UpdateTextureBinding(0, 1, ImageLayout::SHADER_READ_ONLY_OPTIMAL, &m_TestTexture, m_pTestSampler);
	}

}