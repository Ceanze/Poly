#include "polypch.h"
#include "VulkanRenderer.h"

#include "Poly/Core/Window.h"
#include "Platform/Vulkan/Renderers/TestRenderer.h"

namespace Poly
{

	void VulkanRenderer::init(uint32_t width, uint32_t height)
	{
		// Call init on the sub renderers and create window
		this->window = new Window(width, height, "Vulkan renderer window");
		PVKInstance::get().init(this->window);
		this->swapChain.init(this->window);

		// Test renderer
		for (auto& subRenderer : this->subRenderers) {
			subRenderer->setWindow(this->window);
			subRenderer->setActiveCamera(this->camera);
			subRenderer->init(this);
		}

		createSyncObjects();
	}

	void VulkanRenderer::beginScene()
	{
		vkWaitForFences(PVKInstance::getDevice(), 1, &this->inFlightFences[this->currentFrame], VK_TRUE, UINT64_MAX);

		// Checks minimization
		int width = 0, height = 0;
		glfwGetFramebufferSize(this->window->getNativeWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(this->window->getNativeWindow(), &width, &height);
			glfwWaitEvents();
		}

		this->imageIndex = this->swapChain.acquireNextImage(this->imageAvailableSemaphores[this->currentFrame], VK_NULL_HANDLE);

		if (this->imagesInFlight[this->imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(PVKInstance::getDevice(), 1, &this->imagesInFlight[this->imageIndex], VK_TRUE, UINT16_MAX);
		this->imagesInFlight[this->imageIndex] = inFlightFences[this->currentFrame];

		// Call all subrenderers beginScene
		for (auto& subRenderer : this->subRenderers)
			subRenderer->beginScene(this->imageIndex);
	}

	void VulkanRenderer::draw(Model* model)
	{
		// Call the subRenderers record function
		for (auto& subRenderer : this->subRenderers)
			subRenderer->record();
	}

	void VulkanRenderer::endScene()
	{
		for (auto& subRenderer : this->subRenderers)
			subRenderer->endScene();

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[this->currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = this->graphicsBuffers.size();
		std::vector<VkCommandBuffer> buffers;
		for (uint32_t i = 0; i < this->graphicsBuffers.size(); i++)
			buffers.push_back(this->graphicsBuffers[i]->getCommandBuffer());
		submitInfo.pCommandBuffers = buffers.data();
		VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[this->currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		PVK_CHECK(vkResetFences(PVKInstance::getDevice(), 1, &this->inFlightFences[this->currentFrame]), "Failed to reset fences!");

		PVK_CHECK(vkQueueSubmit(PVKInstance::getQueue(QueueType::GRAPHICS).queue, 1, &submitInfo, this->inFlightFences[this->currentFrame]), "Failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { this->swapChain.getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &this->imageIndex;
		presentInfo.pResults = nullptr; // Optional
		PVK_CHECK(vkQueuePresentKHR(PVKInstance::getPresentQueue().queue, &presentInfo), "Failed to present image!");

		this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		// Clear buffers for next frame
		this->graphicsBuffers.clear();
	}

	void VulkanRenderer::shutdown()
	{
		vkDeviceWaitIdle(PVKInstance::getDevice());

		// Cleanup all the sub renderer and own sync objects
		for (auto& subRenderer : this->subRenderers)
			subRenderer->shutdown();

		// Cleanup sync objects
		for (size_t i = 0; i < this->renderFinishedSemaphores.size(); i++) {
			vkDestroySemaphore(PVKInstance::getDevice(), this->renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(PVKInstance::getDevice(), this->imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(PVKInstance::getDevice(), this->inFlightFences[i], nullptr);
		}

		// Cleanup the rest
		this->swapChain.cleanup();
		PVKInstance::get().cleanup();

		delete this->window;
	}

	void VulkanRenderer::createRenderer(Renderer subRenderer)
	{
		switch (subRenderer)
		{
		case Poly::Renderer::TEST:
			this->subRenderers.push_back(new TestRenderer);
			break;
		case Poly::Renderer::MESH:
		default:
			POLY_CORE_ERROR("Only test renderer is currently supported for VulkanRenderer");
			break;
		}
	}

	void VulkanRenderer::addCommandBuffer(QueueType queueType, PVKCommandBuffer* buffer)
	{
		std::vector<PVKCommandBuffer*>& buffers = this->graphicsBuffers;
		switch (queueType)
		{
		case Poly::QueueType::GRAPHICS:
			break;
		case Poly::QueueType::COMPUTE:
			// buffers = this->computeBuffers;
			break;
		case Poly::QueueType::TRANSFER:
			// buffers = this->transferBuffers;
			break;
		default:
			POLY_CORE_ERROR("Only graphics, compute and transfer command buffers are allowed!");
			break;
		}

		buffers.push_back(buffer);
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
			PVK_CHECK(vkCreateSemaphore(PVKInstance::getDevice(), &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]), "Failed to create semaphores!");
			PVK_CHECK(vkCreateSemaphore(PVKInstance::getDevice(), &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]), "Failed to create semaphores!");
			PVK_CHECK(vkCreateFence(PVKInstance::getDevice(), &fenceInfo, nullptr, &inFlightFences[i]), "Failed to create fences!");
		}
	}

}