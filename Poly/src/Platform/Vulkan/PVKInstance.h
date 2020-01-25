#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

namespace Poly
{
	/**
		PVKInstance contains the vulkan instance, physical device, and logical device
		that are created for that instance.
	**/
	class PVKInstance
	{
	public:
		PVKInstance(unsigned width, unsigned height);
		~PVKInstance();

		void init();

	private:
		// Struct to keep track of the different queue families
		struct QueueFamilyIndices {
			std::optional<unsigned> graphicsFamily;

			bool isComplete() {
				return graphicsFamily.has_value();
			}
		};

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		void createInstance();
		bool checkValidationLayerSupport();
		void pickPhysicalDevice();
		void setOptimalDevice(const std::vector<VkPhysicalDevice>& devices);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		void createLogicalDevice();

		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;

		#ifdef POLY_DEBUG
				const bool enableValidationLayers = true;
		#else
				const bool enableValidationLayers = false;
		#endif
	};
}