#include "polypch.h"
#include "VulkanRenderer.h"

#include "Poly/Core/Window.h"
#include "Platform/Vulkan/Renderers/TestRenderer.h"

#include "Poly/Core/RenderAPI.h"

namespace Poly
{

	void VulkanRenderer::Init(uint32_t width, uint32_t height)
	{
		// Call init on the sub renderers and create window
		m_pWindow = new Window(width, height, "Vulkan renderer window");
		// TODO: Should be done in RenderAPI
		//PVKInstance::Get()->Init(m_pWindow);
		RenderAPI::Init(RenderAPI::BackendAPI::VULKAN, m_pWindow);
		m_SwapChain.Init(m_pWindow);

		// Test renderer
		for (auto& subRenderer : m_SubRenderers) {
			subRenderer->SetWindow(m_pWindow);
			subRenderer->SetActiveCamera(m_pCamera);
			subRenderer->Init(this);
		}

		CreateSyncObjects();
	}

	void VulkanRenderer::BeginScene()
	{
		vkWaitForFences(PVKInstance::GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		// Checks minimization
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_pWindow->GetNative(), &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(m_pWindow->GetNative(), &width, &height);
			glfwWaitEvents();
		}

		m_ImageIndex = m_SwapChain.AcquireNextImage(m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE);

		// Wait for fence to make sure the image we will use is availabe and not in flight (use)
		if (m_ImagesInFlight[m_ImageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(PVKInstance::GetDevice(), 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT16_MAX);
		m_ImagesInFlight[m_ImageIndex] = m_InFlightFences[m_CurrentFrame];

		// Call all subrenderers beginScene
		for (auto& subRenderer : m_SubRenderers)
			subRenderer->BeginScene(m_ImageIndex);
	}

	void VulkanRenderer::Draw(Model* model)
	{
		// Call the subRenderers record function
		for (auto& subRenderer : m_SubRenderers)
			subRenderer->Record();
	}

	void VulkanRenderer::EndScene()
	{
		for (auto& subRenderer : m_SubRenderers)
			subRenderer->EndScene();

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = m_GraphicsBuffers.size();
		std::vector<VkCommandBuffer> buffers;
		for (uint32_t i = 0; i < m_GraphicsBuffers.size(); i++)
			buffers.push_back(m_GraphicsBuffers[i]->GetNative());
		submitInfo.pCommandBuffers = buffers.data();
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		PVK_CHECK(vkResetFences(PVKInstance::GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]), "Failed to reset fences!");

		PVK_CHECK(vkQueueSubmit(PVKInstance::GetQueue(FQueueType::GRAPHICS).queue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]), "Failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { m_SwapChain.GetNative() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr; // Optional
		PVK_CHECK(vkQueuePresentKHR(PVKInstance::GetPresentQueue().queue, &presentInfo), "Failed to present image!");

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		// Clear buffers for next frame
		m_GraphicsBuffers.clear();
	}

	void VulkanRenderer::Shutdown()
	{
		vkDeviceWaitIdle(PVKInstance::GetDevice());

		// Cleanup all the sub renderer and own sync objects
		for (auto& subRenderer : m_SubRenderers)
			subRenderer->Shutdown();

		// Cleanup sync objects
		for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); i++) {
			vkDestroySemaphore(PVKInstance::GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(PVKInstance::GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(PVKInstance::GetDevice(), m_InFlightFences[i], nullptr);
		}

		// Cleanup the rest
		m_SwapChain.Cleanup();
		// TODO: Should be done in RenderAPI
		//PVKInstance::Get()->Cleanup();

		delete m_pWindow;
		for (uint32_t i = 0; i < m_SubRenderers.size(); i++)
			delete m_SubRenderers[i];

		RenderAPI::Release();
	}

	void VulkanRenderer::CreateRenderer(Renderer subRenderer)
	{
		switch (subRenderer)
		{
		case Poly::Renderer::TEST:
			m_SubRenderers.push_back(new TestRenderer);
			break;
		case Poly::Renderer::MESH:
		default:
			POLY_CORE_ERROR("Only test renderer is currently supported for VulkanRenderer");
			break;
		}
	}

	void VulkanRenderer::AddCommandBuffer(FQueueType queueType, PVKCommandBuffer* pBuffer)
	{
		std::vector<PVKCommandBuffer*>& buffers = m_GraphicsBuffers;
		switch (queueType)
		{
		case Poly::FQueueType::GRAPHICS:
			break;
		case Poly::FQueueType::COMPUTE:
			// buffers = this->computeBuffers;
			break;
		case Poly::FQueueType::TRANSFER:
			// buffers = this->transferBuffers;
			break;
		default:
			POLY_CORE_ERROR("Only graphics, compute and transfer command buffers are allowed!");
			break;
		}

		buffers.push_back(pBuffer);
	}

	void VulkanRenderer::CreateSyncObjects()
	{
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(m_SwapChain.GetNumImages(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			PVK_CHECK(vkCreateSemaphore(PVKInstance::GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]), "Failed to create semaphores!");
			PVK_CHECK(vkCreateSemaphore(PVKInstance::GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]), "Failed to create semaphores!");
			PVK_CHECK(vkCreateFence(PVKInstance::GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]), "Failed to create fences!");
		}
	}

}