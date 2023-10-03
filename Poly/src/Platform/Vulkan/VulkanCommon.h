#pragma once

#include "PVKTypes.h"

#include <optional>
#include <vulkan/vulkan.h>
#include <vector>
#include <fstream>

namespace Poly {

	// Struct to keep track of the different queue families
	struct QueueFamilyIndices {
		std::optional<unsigned> graphicsFamily;
		std::optional<unsigned> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	// Find the desired queues index in the list of families. Returns a pair containing the queueIndex and queueCount
	static std::pair<uint32_t, uint32_t> findQueueIndex(VkQueueFlagBits queueFamily, VkPhysicalDevice device)
	{
		// Get the queue families.
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// Backup queue if no unique desired queue was found
		std::pair<uint32_t, uint32_t> backupQueue;
		bool backupQueueSet = false;

		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) {

			VkQueueFlags desiredQueue = queueFamilies[i].queueFlags & queueFamily;
			VkQueueFlags graphicsCheck = queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

			if (((queueFamily & VK_QUEUE_GRAPHICS_BIT) == 1) && (queueFamilies[i].queueFlags & queueFamily))
				return { i, queueFamilies[i].queueCount };
			// If not requesting graphics queue, should return a queue without graphics support (to avoid non-unique queue)
			else if ((desiredQueue) && (graphicsCheck == 0))
				return { i, queueFamilies[i].queueCount };
			// If no queue meets the critera of no-graphics and other desired queue - use the first available
			else if (desiredQueue && !backupQueueSet)
				backupQueue = { i, queueFamilies[i].queueCount };
		}

		if (backupQueueSet)
			return backupQueue;

		POLY_VALIDATE(false, "Failed to find queue index for queue family {}!", static_cast<uint32>(queueFamily));
		return {0, 0};
	}

	// Reads the given file in a binary format
	static std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		// Get size
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		// Read file
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		POLY_VALIDATE(false, "Failed to find suitable memory type!");
		return 0;
	}
}