#pragma once

#include <optional>
#include <vulkan/vulkan.h>
#include <vector>

namespace Poly {

	// Struct to keep track of the different queue families
	struct QueueFamilyIndices {
		std::optional<unsigned> graphicsFamily;
		std::optional<unsigned> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	// Current implementation assumes that the graphics queue and presentation queue is the same.
	// If this is to change then the SurfaceKHR is needed in this function
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		// Get queue families from the device
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// Find graphics queue and save its index from the queue
		unsigned i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
				indices.presentFamily = i;
			}

			//VkBool32 presentSupport = false;
			//vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			//if (presentSupport) {
			//	indices.presentFamily = i;
			//}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

}