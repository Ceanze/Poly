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

		void init(Window* window);
		void cleanup();

		void createSwapChain();
		SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

		uint32_t acquireNextImage(VkSemaphore semaphore, VkFence fence);
		VkSwapchainKHR getSwapChain() const { return this->swapChain; }
		std::vector<VkImage>& getImages() { return this->images; }
		std::vector<VkImageView>& getImageViews() { return this->imageViews; }
		VkFormat getFormat() const { return this->format; }
		VkExtent2D getExtent() const { return this->extent; }
		size_t getNumImages() const { return this->imageViews.size(); }


	private:
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		void createImageViews();

		// Vulkan
		VkSwapchainKHR swapChain;
		VkFormat format;
		VkExtent2D extent;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;

		// Custom
		Window* window;
	};
}