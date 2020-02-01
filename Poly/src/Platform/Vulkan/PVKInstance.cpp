#include "polypch.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"

#include <cstdint>

namespace Poly
{

	PVKInstance::PVKInstance()
	{

	}

	PVKInstance::~PVKInstance()
	{
	}

	void PVKInstance::init()
	{
		createInstance();
		setupDebugMessenger();

		pickPhysicalDevice();
		createLogicalDevice();
	}

	void PVKInstance::cleanup()
	{
		if (this->enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);

		vkDestroyDevice(this->device, nullptr);
		vkDestroyInstance(this->instance, nullptr);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL PVKInstance::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		POLY_CORE_TRACE("Validation layer: {}", pCallbackData->pMessage);

		return VK_FALSE;
	}

	VkResult PVKInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void PVKInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void PVKInstance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void PVKInstance::createInstance()
	{
		if (this->enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		// App info (Optional but can improve performance)
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Poly Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Poly";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Create info for instance creation
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Add the validation layers if they are enabled
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (this->enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<unsigned>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		// Add GLFW as an extension to the instance
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Create instance and check if it succeded
		if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	void PVKInstance::setupDebugMessenger()
	{
		if (!this->enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(this->instance, &createInfo, nullptr, &this->debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	bool PVKInstance::checkValidationLayerSupport()
	{
		// Get available layers
		unsigned layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Go through the selected validation layers and check if they are available
		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	bool PVKInstance::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		// Get all the extensions for the device
		unsigned extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// Only get the unique extensions in our requested list
		std::set<std::string> requiredExtensions(this->deviceExtensions.begin(), this->deviceExtensions.end());

		// Check if the extensions we requested is supported by the device
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void PVKInstance::pickPhysicalDevice()
	{
		// Get number of physical devices
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);

		// If no were found, exit
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		
		// Get the physical devices
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

		// Set optimal device
		setOptimalDevice(devices);

		// If no suitable device was found, exit
		if (this->physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void PVKInstance::setOptimalDevice(const std::vector<VkPhysicalDevice>& devices)
	{
		unsigned bestScore = 0;
		for (auto device : devices)
		{
			unsigned score = 0;
			// Query the device
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			// Favor dGPUs
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				score += 1000;
			}

			POLY_CORE_INFO("Number of devices: {}", devices.size());
			POLY_CORE_INFO("Name: {}", deviceProperties.deviceName);

			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;

			// Make sure the device has support for the requested extensions
			bool extensionsSupported = checkDeviceExtensionSupport(device);

			// Save the device with the best score and is complete with its queues
			if (score > bestScore && findQueueFamilies(device).isComplete()) {
				bestScore = score;
				this->physicalDevice = device;
			}
		}
	}

	void PVKInstance::createLogicalDevice()
	{
		// Create info for queues on the device
		QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<unsigned> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (unsigned queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Enable or disable features for the device
		// Currently no features are to be enabled
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// Create info for the logical device
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<unsigned>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<unsigned>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// Used for older implementations of vulkan
		if (this->enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<unsigned>(this->validationLayers.size());
			createInfo.ppEnabledLayerNames = this->validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		// If extensions are to be added (which they will be) then it is here it will be

		// Create the logical device, bound to the physical device
		if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(this->device, indices.graphicsFamily.value(), 0, &this->graphicsQueue);
		vkGetDeviceQueue(this->device, indices.presentFamily.value(), 0, &this->presentQueue);
	}

	std::vector<const char*> PVKInstance::getRequiredExtensions()
	{
		unsigned glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (this->enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

}