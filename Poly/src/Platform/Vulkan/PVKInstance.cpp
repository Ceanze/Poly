#include "polypch.h"
#include "PVKInstance.h"
#include "Poly/Core/Window.h"

namespace Poly
{

	PVKInstance::PVKInstance(Window* window, unsigned width, unsigned height) :
		window(window)
	{
		createInstance();
		setupDebugMessenger();

		// Create surface
		if (glfwCreateWindowSurface(this->instance, this->window->getNativeWindow(), nullptr, &this->surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}

		pickPhysicalDevice();
		createLogicalDevice();
	}

	PVKInstance::~PVKInstance()
	{
		if (this->enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);

		vkDestroyDevice(this->device, nullptr);
		vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
		vkDestroyInstance(this->instance, nullptr);
	}

	void PVKInstance::init()
	{
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL PVKInstance::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		POLY_CORE_INFO("Validation layer: {}", pCallbackData->pMessage);

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

			// Save the device with the best score and is complete with its queues
			if (score > bestScore && findQueueFamilies(device).isComplete()) {
				bestScore = score;
				this->physicalDevice = device;
			}
		}
	}

	PVKInstance::QueueFamilyIndices PVKInstance::findQueueFamilies(VkPhysicalDevice device)
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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &presentSupport);
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

		// Used for older implementations of vulkan
		createInfo.enabledExtensionCount = 0;
		if (this->enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(this->validationLayers.size());
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