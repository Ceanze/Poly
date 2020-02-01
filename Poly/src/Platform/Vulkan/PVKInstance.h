#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

namespace Poly
{
	/*
		PVKInstance contains the vulkan instance, physical device, and logical device
		that are created for that instance.
	*/
	class PVKInstance
	{
	public:
		PVKInstance();
		~PVKInstance();

		void init();
		void cleanup();

		VkDevice getDevice() { return this->device; };
		VkPhysicalDevice getPhysicalDevice() { return this->physicalDevice; };
		VkInstance getInstance() { return this->instance; };

	private:
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		void pickPhysicalDevice();
		void setOptimalDevice(const std::vector<VkPhysicalDevice>& devices);
		void createLogicalDevice();
		std::vector<const char*> getRequiredExtensions();

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;

		#ifdef POLY_DEBUG
				const bool enableValidationLayers = true;
		#else
				const bool enableValidationLayers = false;
		#endif
	};
}