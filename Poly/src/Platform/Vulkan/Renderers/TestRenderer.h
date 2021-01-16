#pragma once

#include "Platform/Common/IRenderer.h"

#include "Platform/Vulkan/PVKInstance.h"
#include "Platform/Vulkan/PVKSwapChain.h"
#include "Platform/Vulkan/PVKPipeline.h"
#include "Platform/Vulkan/PVKShader.h"
#include "Platform/Vulkan/PVKRenderPass.h"
#include "Platform/Vulkan/PVKFramebuffer.h"
#include "Platform/Vulkan/PVKCommandPool.h"
#include "Platform/Vulkan/PVKCommandBuffer.h"
#include "Platform/Vulkan/PVKDescriptor.h"
#include "Platform/Vulkan/PVKBuffer.h"
#include "Platform/Vulkan/PVKSampler.h"
#include "Platform/Vulkan/PVKTexture.h"


namespace Poly
{

	class VulkanRenderer;

	class TestRenderer : public IRenderer
	{
	public:
		TestRenderer() = default;
		virtual ~TestRenderer() = default;
		virtual void SetWindow(Window* window) override { m_pWindow = window; }
		virtual void Init(IPlatformRenderer* pRenderer) override;
		virtual void BeginScene(uint32_t imageIndex) override;
		virtual void Record() override;
		virtual void EndScene() override;
		virtual void Shutdown() override;

	private:
		void CreateCommandBuffers();
		void SetupDescriptorSet();
		void SetupTestData();

		VulkanRenderer*	m_pMainRenderer	= nullptr;
		PVKSwapChain*	m_pSwapChain	= nullptr;
		uint32_t		m_ImageIndex	= 0;

		PVKPipeline		m_Pipeline;
		PVKShader		m_Shader;
		PVKRenderPass	m_RenderPass;
		std::vector<PVKFramebuffer> m_Framebuffers;
		PVKCommandPool	m_CommandPool;
		std::vector<PVKCommandBuffer*> m_CommandBuffers;
		PVKDescriptor	m_Descriptor;

		PVKBuffer		m_TestBuffer;
		PVKTexture		m_TestTexture;
		PVKSampler*		m_pTestSampler = nullptr;

		Window*			m_pWindow = nullptr;
	};

}