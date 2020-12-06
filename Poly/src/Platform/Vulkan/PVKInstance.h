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

		static PVKInstance& Get();

		void Init(Window* window);
		void Cleanup();

		// Sets how many queues should be created for that queue type. Must be called before init.
		// Default queue count per queue is 1. If the requested queue count is higher than the available, the
		// most available will be set.
		static void SetQueueCount(QueueType queue, uint32_t count);

		static VkDevice			GetDevice() { return s_Device; }
		static VkPhysicalDevice	GetPhysicalDevice() { return s_PhysicalDevice; }
		static VkInstance		GetInstance() { return s_Instance; }
		static PVKQueue&		GetPresentQueue() { return s_PresentQueue; }
		static PVKQueue&		GetQueue(QueueType queueType, uint32_t index = 0);
		static VkSurfaceKHR		GetSurface() { return s_Surface; }
		static VmaAllocator		GetAllocator() { return s_VmaAllocator; }

	private:
		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> m_DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
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

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void CreateInstance();
		void SetupDebugMessenger();
		bool CheckValidationLayerSupport();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		void PickPhysicalDevice();
		void SetOptimalDevice(const std::vector<VkPhysicalDevice>& devices);
		void CreateLogicalDevice();
		void CreateVmaAllocator();
		std::vector<const char*> GetRequiredExtensions();
		void GetAllQueues();

		VkDebugUtilsMessengerEXT	m_DebugMessenger;
		static VkInstance			s_Instance;
		static VkPhysicalDevice		s_PhysicalDevice;
		static VkDevice				s_Device;
		static PVKQueue				s_PresentQueue; // Should maybe be in the queue map
		static VkSurfaceKHR			s_Surface;
		static uint32_t				s_GraphicsQueueCount;
		static uint32_t				s_ComputeQueueCount;
		static uint32_t				s_TransferQueueCount;
		static std::unordered_map<Poly::QueueType, std::vector<Poly::PVKQueue>> s_Queues;

		static VmaAllocator s_VmaAllocator;

		#ifdef POLY_DEBUG
				const bool m_EnableValidationLayers = true;
		#else
				const bool m_EnableValidationLayers = false;
		#endif
	};
}