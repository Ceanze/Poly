#include "polypch.h"
#include "VulkanRenderer.h"

namespace Poly
{

	// Init should always be called last

	void VulkanRenderer::initialize(unsigned width, unsigned height)
	{
		this->window = new Window(width, height, "Poly");

		this->instance.init(this->window);
		this->swapChain.init(this->window);

		// Everything under this line should be made in Sandbox
		this->shader.addStage(PVKShader::Type::VERTEX, "vert.spv");
		this->shader.addStage(PVKShader::Type::FRAGMENT, "frag.spv");
		this->shader.init();

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		this->renderPass.addSubpassDependency(dependency);
		this->renderPass.init(this->swapChain);

		this->pipeline.init(this->swapChain, this->shader, this->renderPass);

		// Until this line
		this->commandPool.init(VK_QUEUE_GRAPHICS_BIT);
		
		this->framebuffers.resize(this->swapChain.getNumImages());
		for (size_t i = 0; i < this->swapChain.getNumImages(); i++)
			this->framebuffers[i].init(this->swapChain, this->renderPass, this->swapChain.getImageViews()[i]);

		this->createCommandBuffers();
		this->createSyncObjects();
	}

	void VulkanRenderer::setWinTitle(const char* title)
	{
	}

	void VulkanRenderer::render()
	{
		vkWaitForFences(this->instance.getDevice(), 1, &this->inFlightFences[this->currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex = this->swapChain.acquireNextImage(this->imageAvailableSemaphores[this->currentFrame], VK_NULL_HANDLE);

		if (this->imagesInFlight[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(this->instance.getDevice(), 1, &this->imagesInFlight[imageIndex], VK_TRUE, UINT16_MAX);
		this->imagesInFlight[imageIndex] = inFlightFences[this->currentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[this->currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		VkCommandBuffer b = this->commandBuffers[imageIndex]->getCommandBuffer();
		submitInfo.pCommandBuffers = &b;
		VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[this->currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		PVK_CHECK(vkResetFences(this->instance.getDevice(), 1, &this->inFlightFences[this->currentFrame]), "Failed to reset fences!");

		PVK_CHECK(vkQueueSubmit(this->instance.getGraphicsQueue().queue, 1, &submitInfo, this->inFlightFences[this->currentFrame]), "Failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { this->swapChain.getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional
		PVK_CHECK(vkQueuePresentKHR(this->instance.getPresentQueue().queue, &presentInfo), "Failed to present image!");

		this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::shutdown()
	{
		PVK_CHECK(vkDeviceWaitIdle(this->instance.getDevice()), "Failed to wait for device!");

		for (size_t i = 0; i < this->renderFinishedSemaphores.size(); i++) {
			vkDestroySemaphore(this->instance.getDevice(), this->renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(this->instance.getDevice(), this->imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(this->instance.getDevice(), this->inFlightFences[i], nullptr);
		}

		this->commandPool.cleanup();
		PVK_VEC_CLEANUP(this->framebuffers);
		this->pipeline.cleanup();
		this->renderPass.cleanup();
		this->shader.cleanup();
		this->swapChain.cleanup();
		this->instance.cleanup();
		delete this->window;
	}

	void VulkanRenderer::setClearColor(float r, float g, float b, float a)
	{
	}

	void VulkanRenderer::clearBuffer(unsigned int)
	{
	}

	void VulkanRenderer::createCommandBuffers()
	{
		this->commandBuffers = this->commandPool.createCommandBuffers(3);

		for (size_t i = 0; i < this->commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			this->commandBuffers[i]->begin(0);
			this->commandBuffers[i]->cmdBeginRenderPass(&this->renderPass, &this->framebuffers[i], this->swapChain.getExtent(), { 0.0f, 0.0f, 0.0f, 1.0f });
			this->commandBuffers[i]->cmdBindPipeline(&this->pipeline);
			this->commandBuffers[i]->cmdDraw(3, 1, 0, 0);
			this->commandBuffers[i]->cmdEndRenderPass();
			this->commandBuffers[i]->end();
		}
	}

	void VulkanRenderer::createSyncObjects()
	{
		this->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		this->imagesInFlight.resize(this->swapChain.getNumImages(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			PVK_CHECK(vkCreateSemaphore(this->instance.getDevice(), &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]), "Failed to create semaphores!");
			PVK_CHECK(vkCreateSemaphore(this->instance.getDevice(), &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]), "Failed to create semaphores!");
			PVK_CHECK(vkCreateFence(this->instance.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]), "Failed to create fences!");
		}
	}

}