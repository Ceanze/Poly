#pragma once

#include <vulkan/vulkan.h>
#include <optional>

namespace Poly
{
	// Struct to keep track of the different queue families
	struct QueueFamilyIndices {
		std::optional<unsigned> graphicsFamily;
		std::optional<unsigned> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct PVKSwapChainCreateInfo {
		VkPhysicalDevice physicalDevice;
		VkSurfaceKHR surface;
		QueueFamilyIndices indices;
		VkDevice device;
		unsigned width;
		unsigned height;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class PVKSwapChain
	{
	public:

		PVKSwapChain(PVKSwapChainCreateInfo &info);
		~PVKSwapChain();

		void createSwapChain();
		SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

		VkSwapchainKHR getSwapChain() const;

	private:
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkSwapchainKHR swapChain;
		PVKSwapChainCreateInfo info;
	};
}