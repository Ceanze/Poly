#pragma once

#include "PVKTypes.h"

#include <optional>
#include <vulkan/vulkan.h>
#include <vector>

namespace Poly {

	// Struct to keep track of the different queue families
	struct QueueFamilyIndices {
		std::optional<unsigned> GraphicsFamily;
		std::optional<unsigned> PresentFamily;

		bool isComplete() {
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	struct QueueSpec {
		uint32_t QueueFamily;
		uint32_t QueueCount;
		uint32_t QueueFlags;
	};

	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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
				indices.GraphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.PresentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	// Find the desired queues index in the list of families. Returns a pair containing the queueIndex and queueCount
	static QueueSpec FindQueueIndex(VkQueueFlagBits queueFamily, VkPhysicalDevice device)
	{
		// Get the queue families.
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// Backup queue if no unique desired queue was found
		QueueSpec backupQueue;
		bool backupQueueSet = false;

		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) {
			VkQueueFlags desiredQueue = queueFamilies[i].queueFlags & queueFamily;
			VkQueueFlags graphicsCheck = queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

			if (((queueFamily & VK_QUEUE_GRAPHICS_BIT) == 1) && desiredQueue)
				return { i, queueFamilies[i].queueCount };
			// If not requesting graphics queue, should return a queue without graphics support (to avoid non-unique queue)
			else if ((desiredQueue) && (graphicsCheck == 0))
				return { i, queueFamilies[i].queueCount };
			// If no queue meets the criteria of no-graphics and other desired queue - use the first available
			else if (desiredQueue && !backupQueueSet)
            {
                backupQueue = {i, queueFamilies[i].queueCount};
                backupQueueSet = true;
            }
		}

		if (backupQueueSet)
			return backupQueue;

		POLY_VALIDATE(false, "Failed to find queue index for queue family {}!", static_cast<uint32>(queueFamily));
		return {0, 0};
	}

	static std::vector<QueueSpec> FindAllQueues(VkPhysicalDevice device)
	{
		// Get the queue families.
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		std::vector<QueueSpec> queueSpecs;
		for (size_t i = 0; i < queueFamilies.size(); i++)
			queueSpecs.push_back(QueueSpec{ static_cast<uint32_t>(i), queueFamilies[i].queueCount, queueFamilies[i].queueFlags});

		return queueSpecs;
	}
}
