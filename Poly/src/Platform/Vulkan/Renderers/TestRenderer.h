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
#include "Platform/Vulkan/PVKMemory.h"
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
		virtual void setWindow(Window* window) override { this->window = window; }
		virtual void init(IPlatformRenderer* renderer) override;
		virtual void beginScene(uint32_t imageIndex) override;
		virtual void record() override;
		virtual void endScene() override;
		virtual void shutdown() override;

	private:
		void createCommandBuffers();
		void setupDescriptorSet();
		void setupTestData();

		VulkanRenderer* mainRenderer = nullptr;
		PVKSwapChain* swapChain = nullptr;
		uint32_t imageIndex = 0;

		PVKPipeline pipeline;
		PVKShader shader;
		PVKRenderPass renderPass;
		std::vector<PVKFramebuffer> framebuffers;
		PVKCommandPool commandPool;
		std::vector<PVKCommandBuffer*> commandBuffers;
		PVKDescriptor descriptor;

		PVKBuffer testBuffer;
		PVKMemory testMemory;
		PVKMemory testTextureMemory;
		PVKTexture testTexture;
		PVKSampler* testSampler;

		Window* window = nullptr;
	};

}