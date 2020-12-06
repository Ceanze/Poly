#include "polypch.h"
#include "PVKSwapChain.h"
#include "PVKInstance.h"

namespace Poly
{

	PVKSwapChain::PVKSwapChain()
	{
	}

	PVKSwapChain::~PVKSwapChain()
	{
	}

	void PVKSwapChain::Init(Window* pWindow)
	{
		m_pWindow = pWindow;

		CreateSwapChain();
		CreateImageViews();
	}

	void PVKSwapChain::Cleanup()
	{
		for (auto imageView : m_ImageViews) {
			vkDestroyImageView(PVKInstance::GetDevice(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(PVKInstance::GetDevice(), m_SwapChain, nullptr);
	}

	void PVKSwapChain::CreateSwapChain()
	{
		// Use the predefined functions to choose and query everything
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(PVKInstance::GetSurface(), PVKInstance::GetPhysicalDevice());
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

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
		createInfo.imageExtent = extent;
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

		// Save extent and format for future use
		m_Extent = extent;
		m_Format = surfaceFormat.format;

		// Create the swap chain
		PVK_CHECK(vkCreateSwapchainKHR(PVKInstance::GetDevice(), &createInfo, nullptr, &m_SwapChain), "Failed to create swap chain!");

		// VkImages created automatically by the swapchain, just need to retrive them
		vkGetSwapchainImagesKHR(PVKInstance::GetDevice(), m_SwapChain, &imageCount, nullptr);
		m_Images.resize(imageCount);
		vkGetSwapchainImagesKHR(PVKInstance::GetDevice(), m_SwapChain, &imageCount, m_Images.data());
	}

	VkExtent2D PVKSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = { m_pWindow->GetWidth(), m_pWindow->GetHeight() };

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
		// Try to use R8G8B8 and SRGB, if that fails, use the first one found
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	void PVKSwapChain::CreateImageViews()
	{
		// Get images from swapchain
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(PVKInstance::GetDevice(), m_SwapChain, &imageCount, nullptr);
		m_Images.resize(imageCount);
		vkGetSwapchainImagesKHR(PVKInstance::GetDevice(), m_SwapChain, &imageCount, m_Images.data());

		// Create swapchain image views
		m_ImageViews.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++) {
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_Images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_Format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			PVK_CHECK(vkCreateImageView(PVKInstance::GetDevice(), &createInfo, nullptr, &m_ImageViews[i]), "Failed to create image views!");
		}
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

	uint32_t PVKSwapChain::AcquireNextImage(VkSemaphore semaphore, VkFence fence)
	{
		uint32_t imageIndex;
		PVK_CHECK(vkAcquireNextImageKHR(PVKInstance::GetDevice(), m_SwapChain, UINT64_MAX, semaphore, fence, &imageIndex), "Failed to acquire image!");
		return imageIndex;
	}

}