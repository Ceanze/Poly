#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Poly/Core/Renderer.h"
#include "Poly/Core/Window.h"
#include "PVKInstance.h"
#include "PVKSwapChain.h"
#include "PVKPipeline.h"
#include "PVKShader.h"
#include "PVKRenderPass.h"
#include "PVKFramebuffer.h"
#include "PVKCommandPool.h"
#include "PVKCommandBuffer.h"
#include "PVKDescriptor.h"
#include "PVKBuffer.h"
#include "PVKMemory.h"


namespace Poly
{

	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer() = default;
		virtual ~VulkanRenderer() = default;
		virtual void initialize(unsigned width = 1280, unsigned height = 720);
		virtual void setWinTitle(const char* title);
		virtual void render();
		//virtual void present() = 0;
		virtual void shutdown();

		virtual void setClearColor(float r, float g, float b, float a);
		virtual void clearBuffer(unsigned int);

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

		// Sync -- TODO: Relocate sync objects?
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;

		const int MAX_FRAMES_IN_FLIGHT = 2;

		Window* window;
	};

}