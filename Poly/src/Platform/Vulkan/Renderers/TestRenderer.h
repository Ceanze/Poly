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

namespace Poly
{

	class TestRenderer : IRenderer
	{
	public:
		TestRenderer() = default;
		virtual ~TestRenderer() = default;
		virtual void setWindow(Window* window) override { this->window = window; }
		virtual void init() override;
		virtual void beginScene() override;
		virtual void setActiveCamera(Camera* camera) override { this->camera = camera; }
		virtual void record() override;
		virtual void endScene() override;
		virtual void shutdown() override;

	private:
		void createCommandBuffers();
		void createSyncObjects();
		void setupDescriptorSet();
		void setupTestData();

		PVKSwapChain swapChain;
		PVKPipeline pipeline;
		PVKShader shader;
		PVKRenderPass renderPass;
		std::vector<PVKFramebuffer> framebuffers;
		PVKCommandPool commandPool;
		std::vector<PVKCommandBuffer*> commandBuffers;
		PVKDescriptor descriptor;

		// Temp buffers and memories
		PVKBuffer testBuffer;
		PVKMemory testMemory;

		// Sync -- TODO: Move to platform renderer
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;

		const int MAX_FRAMES_IN_FLIGHT = 2;

		Window* window = nullptr;
		Camera* camera = nullptr;
	};

}