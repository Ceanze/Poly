#include "polypch.h"
#include "PVKSwapChain.h"

#include "PVKFence.h"
#include "PVKTexture.h"
#include "PVKInstance.h"
#include "PVKSemaphore.h"
#include "PVKTextureView.h"
#include "PVKCommandQueue.h"
#include "PVKCommandBuffer.h"

#include "Poly/Core/Window.h"

namespace Poly
{
	PVKSwapChain::~PVKSwapChain()
	{
		for (uint32 i = 0; i < p_SwapchainDesc.BufferCount; i++)
		{
			delete m_TextureViews[i];
			delete m_Textures[i];

			delete m_ImagesInFlight[i];
			delete m_RenderSemaphores[i];
			delete m_AcquireSemaphores[i];
		}
		m_TextureViews.clear();
		m_Textures.clear();
		m_ImagesInFlight.clear();
		m_RenderSemaphores.clear();
		m_AcquireSemaphores.clear();

		vkDestroySwapchainKHR(PVKInstance::GetDevice(), m_SwapChain, nullptr);
	}

	void PVKSwapChain::Init(const SwapChainDesc* pDesc)
	{
		p_SwapchainDesc = *pDesc;

		CreateSyncObjects();
		CreateSwapChain();
		CreateImageViews();
		AcquireNextImage();
	}

	void PVKSwapChain::Resize(uint32 width, uint32 height)
	{

	}

	void PVKSwapChain::Present(std::vector<CommandBuffer*> commandBuffers, Semaphore* pWaitSemaphore)
	{
		m_ImagesInFlight[m_FrameIndex]->Reset();

		p_SwapchainDesc.pQueue->Submit(commandBuffers, pWaitSemaphore, m_RenderSemaphores[m_FrameIndex], m_ImagesInFlight[m_FrameIndex]);

		VkSemaphore waitSemaphore = m_RenderSemaphores[m_FrameIndex]->GetNativeVK();
		VkSwapchainKHR swapChains[] = { m_SwapChain };

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr; // Optional
		PVK_CHECK(vkQueuePresentKHR(PVKInstance::GetPresentQueue().queue, &presentInfo), "Failed to present image!");

		AcquireNextImage();
	}

	void PVKSwapChain::CreateSwapChain()
	{
		// Use the predefined functions to choose and query everything
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(PVKInstance::GetSurface(), PVKInstance::GetPhysicalDevice());
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		m_Extent = ChooseSwapExtent(swapChainSupport.capabilities);
		m_FormatVK = surfaceFormat.format;

		// One more image than the minimum to avoid unneccesary waiting (if possible)
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// Info for swap chain constructions
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = PVKInstance::GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1; // Used for 3D images (steroscopic)
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Render directly to swap chain, if post-processing then TRANSER_DST_BIT is recommended
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // Ignore the obscured pixles (another window infront or similar)
		createInfo.oldSwapchain = VK_NULL_HANDLE; // If swap chain is recreated during runtime, having the previous swap chain can help

		// If we have several queues (rare) then specify on how to use them
		QueueFamilyIndices indices = findQueueFamilies(PVKInstance::GetPhysicalDevice(), PVKInstance::GetSurface());
		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		// Create the swap chain
		PVK_CHECK(vkCreateSwapchainKHR(PVKInstance::GetDevice(), &createInfo, nullptr, &m_SwapChain), "Failed to create swap chain!");
	}

	SwapChainSupportDetails PVKSwapChain::QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// Get capabilities of the device and surface
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Check if the current physical device supports presentation for the surface
		QueueFamilyIndices families = findQueueFamilies(PVKInstance::GetPhysicalDevice(), PVKInstance::GetSurface());
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, families.presentFamily.value(), surface, &presentSupport);

		// Get the formats of the device and surface
		unsigned formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		// Get all the supported presentation modes
		unsigned presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkExtent2D PVKSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = { p_SwapchainDesc.pWindow->GetWidth(), p_SwapchainDesc.pWindow->GetHeight() };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	VkPresentModeKHR PVKSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		// Use mailbox if found, if not then settle on FIFO (which always exists)
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkSurfaceFormatKHR PVKSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		// Vulkan Tutorial uses format = VK_FORMAT_B8G8R8A8_UNORM, colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		VkFormat requestedFormat = ConvertFormatVK(p_SwapchainDesc.Format);
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == requestedFormat && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		// If requested format fails, check for VK_FORMAT_B8G8R8A8_UNORM if that wasn't the requested to begin with
		if (requestedFormat != VK_FORMAT_B8G8R8A8_UNORM)
		{
			requestedFormat = VK_FORMAT_B8G8R8A8_UNORM;
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == requestedFormat && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					POLY_CORE_ERROR("Failed to find requested format for swapchain, returns VK_FORMAT_B8G8R8A8_UNORM instead!");
					return availableFormat;
				}
			}
		}

		// If both alternatives fail, message the user and return the first found
		POLY_CORE_ERROR("Failed to find requested and B8G8R8A8_UNORM format for swapchain, returns first available instead!");
		return availableFormats[0];
	}

	void PVKSwapChain::CreateImageViews()
	{
		// Get images from swapchain
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(PVKInstance::GetDevice(), m_SwapChain, &imageCount, nullptr);
		std::vector<VkImage> images(imageCount);
		vkGetSwapchainImagesKHR(PVKInstance::GetDevice(), m_SwapChain, &imageCount, images.data());

		// Create swapchain image views
		m_TextureViews.resize(imageCount);
		m_Textures.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++) {
			// Texture
			TextureDesc textureDesc		= {};
			textureDesc.Width			= m_Extent.width;
			textureDesc.Height			= m_Extent.height;
			textureDesc.Depth			= 1;
			textureDesc.ArrayLayers		= 1;
			textureDesc.MipLevels		= 1;
			textureDesc.SampleCount		= 1;
			textureDesc.Format			= p_SwapchainDesc.Format;
			textureDesc.MemoryUsage		= EMemoryUsage::GPU_ONLY;
			textureDesc.TextureDim		= ETextureDim::DIM_2D;
			textureDesc.TextureUsage	= FTextureUsage::COLOR_ATTACHMENT;
			m_Textures[i] = new PVKTexture();
			m_Textures[i]->InitWithImage(&textureDesc, images[i]);

			// Texture view
			TextureViewDesc textureViewDesc = {};
			textureViewDesc.Format			= p_SwapchainDesc.Format;
			textureViewDesc.pTexture		= m_Textures[i];
			textureViewDesc.MipLevel		= 0;
			textureViewDesc.MipLevelCount	= 1;
			textureViewDesc.ArrayLayer		= 0;
			textureViewDesc.ArrayLayerCount	= 1;
			textureViewDesc.ImageViewFlag	= FImageViewFlag::RENDER_TARGET;
			textureViewDesc.ImageViewType	= EImageViewType::TYPE_2D;
			m_TextureViews[i] = new PVKTextureView();
			m_TextureViews[i]->Init(&textureViewDesc);
		}
	}

	void PVKSwapChain::CreateSyncObjects()
	{
		m_RenderSemaphores.resize(p_SwapchainDesc.BufferCount);
		m_AcquireSemaphores.resize(p_SwapchainDesc.BufferCount);
		m_ImagesInFlight.resize(p_SwapchainDesc.BufferCount);

		for (uint32 i = 0; i < p_SwapchainDesc.BufferCount; i++)
		{
			m_RenderSemaphores[i] = new PVKSemaphore();
			m_RenderSemaphores[i]->Init();
			m_AcquireSemaphores[i] = new PVKSemaphore();
			m_AcquireSemaphores[i]->Init();

			m_ImagesInFlight[i] = new PVKFence();
			m_ImagesInFlight[i]->Init(FFenceFlag::SIGNALED);
		}
	}

	void PVKSwapChain::AcquireNextImage()
	{
		m_FrameIndex = (m_FrameIndex + 1) % p_SwapchainDesc.BufferCount;

		m_ImagesInFlight[m_FrameIndex]->Wait(UINT64_MAX);

		PVK_CHECK(vkAcquireNextImageKHR(PVKInstance::GetDevice(), m_SwapChain, UINT64_MAX, m_AcquireSemaphores[m_FrameIndex]->GetNativeVK(), VK_NULL_HANDLE, &m_ImageIndex), "Failed to acquire image!");

		m_AcquireSemaphores[m_FrameIndex]->AddWaitStageMask(FPipelineStage::TOP_OF_PIPE);
		reinterpret_cast<PVKCommandQueue*>(p_SwapchainDesc.pQueue)->AddWaitSemaphore(m_AcquireSemaphores[m_FrameIndex]);
	}

}