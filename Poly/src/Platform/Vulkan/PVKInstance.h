#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "PVKTypes.h"

#include "VmaInclude.h"

namespace Poly
{
	class Window;

	/*
		PVKInstance contains the vulkan instance, physical device, and logical device
		that are created for that instance.
	*/

	class PVKInstance
	{
	public:
		PVKInstance();
		~PVKInstance();

		static PVKInstance& get();

		void init(Window* window);
		void cleanup();

		// Sets how many queues should be created for that queue type. Must be called before init.
		// Default queue count per queue is 1. If the requested queue count is higher than the available, the
		// most available will be set.
		static void setQueueCount(QueueType queue, uint32_t count);

		static VkDevice getDevice() { return device; }
		static VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
		static VkInstance getInstance() { return instance; }
		static PVKQueue& getPresentQueue() { return presentQueue; }
		static PVKQueue& getQueue(QueueType queueType, uint32_t index = 0);
		static VkSurfaceKHR getSurface() { return surface; }
		static VmaAllocator getVmaAllocator() { return vmaAllocator; }

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
		void createVmaAllocator();
		std::vector<const char*> getRequiredExtensions();
		void getAllQueues();

		static VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		static VkPhysicalDevice physicalDevice;
		static VkDevice device;
		//static PVKQueue graphicsQueue;
		static std::unordered_map<Poly::QueueType, std::vector<Poly::PVKQueue>> queues;
		static PVKQueue presentQueue;
		static VkSurfaceKHR surface;
		static uint32_t graphicsQueueCount;
		static uint32_t computeQueueCount;
		static uint32_t transferQueueCount;

		static VmaAllocator vmaAllocator;

		#ifdef POLY_DEBUG
				const bool enableValidationLayers = true;
		#else
				const bool enableValidationLayers = false;
		#endif
	};
}