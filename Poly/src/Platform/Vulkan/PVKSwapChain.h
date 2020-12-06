#pragma once

#include "VulkanCommon.h"

#include "Poly/Core/Window.h"

namespace Poly
{
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	/*
		PVKSwapChain contains the swapchain and the surface with its creation
	*/
	class PVKSwapChain
	{
	public:

		PVKSwapChain();
		~PVKSwapChain();

		void Init(Window* pWindow);
		void Cleanup();

		void CreateSwapChain();
		SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

		uint32_t AcquireNextImage(VkSemaphore semaphore, VkFence fence);
		VkSwapchainKHR GetNative() const { return m_SwapChain; }
		std::vector<VkImage>& GetImages() { return m_Images; }
		std::vector<VkImageView>& GetImageViews() { return m_ImageViews; }
		VkFormat GetFormat() const { return m_Format; }
		VkExtent2D GetExtent() const { return m_Extent; }
		uint32_t GetNumImages() const { return m_ImageViews.size(); }


	private:
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		void CreateImageViews();

		// Vulkan
		VkSwapchainKHR	m_SwapChain	= VK_NULL_HANDLE;
		VkFormat		m_Format	= VK_FORMAT_UNDEFINED;
		VkExtent2D		m_Extent	= {0, 0};
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		// Custom
		Window* m_pWindow = nullptr;
	};
}