#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

namespace Poly
{
	class Window;

	/**
		PVKInstance contains the vulkan instance, physical device, and logical device
		that are created for that instance.
	**/
	class PVKInstance
	{
	public:
		PVKInstance(Window* window, unsigned width, unsigned height);
		~PVKInstance();

		void init();

	private:
		// Struct to keep track of the different queue families
		struct QueueFamilyIndices {
			std::optional<unsigned> graphicsFamily;
			std::optional<unsigned> presentFamily;

			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);

		static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator);

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void createInstance();
		void setupDebugMessenger();
		bool checkValidationLayerSupport();
		void pickPhysicalDevice();
		void setOptimalDevice(const std::vector<VkPhysicalDevice>& devices);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		void createLogicalDevice();
		std::vector<const char*> getRequiredExtensions();

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkSurfaceKHR surface;

		Window* window;

		#ifdef POLY_DEBUG
				const bool enableValidationLayers = true;
		#else
				const bool enableValidationLayers = false;
		#endif
	};
}