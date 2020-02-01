#pragma once

#include "VulkanCommon.h"

#include "PVKInstance.h"
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

		void init(PVKInstance* instance, Window* window);
		void cleanup();

		void createSwapChain();
		SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

		VkSwapchainKHR getSwapChain() const;
		std::vector<VkImage>& getImages();
		std::vector<VkImageView>& getImageViews();
		VkFormat getFormat() const;
		VkExtent2D getExtent() const;

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
		VkSurfaceKHR surface;
		VkDevice device;
		VkPhysicalDevice physicalDevice;

		// Custom
		PVKInstance* instance;
		Window* window;
	};
}