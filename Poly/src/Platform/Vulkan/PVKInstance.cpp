#include "polypch.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"
#include "Poly/Core/Window.h"

#include <cstdint>


namespace Poly
{
	VkInstance PVKInstance::instance = VK_NULL_HANDLE;
	VkDevice PVKInstance::device = VK_NULL_HANDLE;
	VkPhysicalDevice PVKInstance::physicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR PVKInstance::surface = VK_NULL_HANDLE;
	std::unordered_map<Poly::QueueType, std::vector<Poly::PVKQueue>> PVKInstance::queues;
	PVKQueue PVKInstance::presentQueue = {};
	uint32_t PVKInstance::graphicsQueueCount = 1;
	uint32_t PVKInstance::computeQueueCount = 1;
	uint32_t PVKInstance::transferQueueCount = 1;
	VmaAllocator PVKInstance::vmaAllocator = VK_NULL_HANDLE;

	PVKInstance::PVKInstance()
		: debugMessenger(VK_NULL_HANDLE)
	{

	}

	PVKInstance::~PVKInstance()
	{
	}

	PVKInstance& PVKInstance::get()
	{
		static PVKInstance i;
		return i;
	}

	void PVKInstance::init(Window* window)
	{
		createInstance();
		setupDebugMessenger();

		PVK_CHECK(glfwCreateWindowSurface(instance, window->getNative(), nullptr, &surface), "Failed to create window surface!");

		pickPhysicalDevice();
		createLogicalDevice();

		createVmaAllocator();
	}

	void PVKInstance::cleanup()
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);

		if (this->enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(instance, this->debugMessenger, nullptr);

		vmaDestroyAllocator(vmaAllocator);

		vkDestroyDevice(device, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	void PVKInstance::setQueueCount(QueueType queue, uint32_t count)
	{
		switch (queue) {
		case QueueType::GRAPHICS:
			graphicsQueueCount = count;
			break;
		case QueueType::COMPUTE:
			computeQueueCount = count;
			break;
		case QueueType::TRANSFER:
			transferQueueCount = count;
			break;
		}
	}

	PVKQueue& PVKInstance::getQueue(QueueType queueType, uint32_t index)
	{
		// Check if valid queue and index, return if successful
		auto it = queues.find(queueType);
		if (it != queues.end()) {
			auto& vec = queues[queueType];
			if (index < vec.size())
				return queues[queueType][index];
		}

		// Return default graphics queue if the requested is not supported
		return queues[QueueType::GRAPHICS][0];
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL PVKInstance::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		{
			POLY_CORE_TRACE("{}", pCallbackData->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		{
			POLY_CORE_WARN("{}", pCallbackData->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		{
			POLY_CORE_ERROR("{}", pCallbackData->pMessage);
			break;
		}
		// default:
		// 	POLY_CORE_TRACE("{}", pCallbackData->pMessage);
		}

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
		PVK_CHECK(this->enableValidationLayers && !checkValidationLayerSupport(), "Validation layers requested, but not available!");

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
		PVK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance), "Failed to create instance!");
	}

	void PVKInstance::setupDebugMessenger()
	{
		if (!this->enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		populateDebugMessengerCreateInfo(createInfo);

		PVK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &this->debugMessenger), "Failed to set up debug messenger!");
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

	bool PVKInstance::checkDeviceExtensionSupport(VkPhysicalDevice checkDevice)
	{
		// Get all the extensions for the device
		unsigned extensionCount;
		vkEnumerateDeviceExtensionProperties(checkDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(checkDevice, nullptr, &extensionCount, availableExtensions.data());

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
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		// If no were found, exit
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		
		// Get the physical devices
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

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
		for (auto d : devices)
		{
			unsigned score = 0;
			// Query the device
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(d, &deviceProperties);
			vkGetPhysicalDeviceFeatures(d, &deviceFeatures);

			// Favor dGPUs
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				score += 1000;
			}

			POLY_CORE_INFO("Number of devices: {}", devices.size());
			POLY_CORE_INFO("Name: {}", deviceProperties.deviceName);

			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;

			// Make sure the device has support for the requested extensions
			bool extensionsSupported = checkDeviceExtensionSupport(d);

			// Save the device with the best score and is complete with its queues
			if (score > bestScore && findQueueFamilies(d, surface).isComplete() && deviceFeatures.samplerAnisotropy) {
				bestScore = score;
				this->physicalDevice = d;
			}
		}
	}

	void PVKInstance::createLogicalDevice()
	{
		// Create info for queues on the device
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
		std::pair<uint32_t, uint32_t> graphicsQueueIndex = findQueueIndex(VK_QUEUE_GRAPHICS_BIT, physicalDevice);
		std::pair<uint32_t, uint32_t> computeQueueIndex = findQueueIndex(VK_QUEUE_COMPUTE_BIT, physicalDevice);
		std::pair<uint32_t, uint32_t> transferQueueIndex = findQueueIndex(VK_QUEUE_TRANSFER_BIT, physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<unsigned> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), computeQueueIndex.first, transferQueueIndex.first };

		// Update queue count to match what the program can create
		graphicsQueueCount	= std::min(graphicsQueueCount, graphicsQueueIndex.second);
		computeQueueCount	= std::min(computeQueueCount, graphicsQueueIndex.second);
		transferQueueCount	= std::min(transferQueueCount, graphicsQueueIndex.second);

		std::unordered_map<uint32_t, uint32_t> queueCounts;
		queueCounts[indices.presentFamily.value()]	= 1;
		queueCounts[indices.graphicsFamily.value()] = graphicsQueueCount;
		queueCounts[computeQueueIndex.first]		= computeQueueCount;
		queueCounts[transferQueueIndex.first]		= transferQueueCount;

		float queuePriority = 1.0f;
		for (unsigned queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = queueCounts[queueFamily];
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Enable or disable features for the device
		// TODO: Move this to an easier place for editing
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		// Create info for the logical device
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<unsigned>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
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
		PVK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device), "Failed to create logical device!");

		//graphicsQueue.queueIndex = indices.graphicsFamily.value();
		presentQueue.queueIndex = indices.presentFamily.value();

		//vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue.queue);
		getAllQueues(); // This function does not check for present support, hence seperate functions
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue.queue);
	}

	void PVKInstance::createVmaAllocator()
	{
		// Create info
		VmaAllocatorCreateInfo createInfo = {};
		createInfo.physicalDevice = physicalDevice;
		createInfo.device = device;
		createInfo.instance = instance;
		createInfo.vulkanApiVersion = VK_API_VERSION_1_1;
		
		// All three are enabled by default in vulkan 1.1
		if (VK_KHR_dedicated_allocation)
		{
			createInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
		}
		if (VK_KHR_bind_memory2)
		{
			createInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
		}
		if (VK_AMD_device_coherent_memory)
		{
			createInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
		}

		// Uncomment to enable recording to CSV file.
		/*
		static VmaRecordSettings recordSettings = {};
		recordSettings.pFilePath = "VulkanSample.csv";
		outInfo.pRecordSettings = &recordSettings;
		*/

		// Creation
		//VmaAllocator vmaAllocator;
		vmaCreateAllocator(&createInfo, &vmaAllocator);
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

	void PVKInstance::getAllQueues()
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		for (uint32_t fIndex = 0; fIndex < queueFamilyCount; fIndex++) {
			// Graphics queue
			if (VK_QUEUE_GRAPHICS_BIT & queueFamilies[fIndex].queueFlags) {
				queues[QueueType::GRAPHICS].resize(graphicsQueueCount);
				for (uint32_t qIndex = 0; qIndex < graphicsQueueCount; qIndex++) {
					vkGetDeviceQueue(device, fIndex, qIndex, &queues[QueueType::GRAPHICS][qIndex].queue);
				}
			}

			// Compute queue
			if (VK_QUEUE_COMPUTE_BIT & queueFamilies[fIndex].queueFlags) {
				queues[QueueType::COMPUTE].resize(computeQueueCount);
				for (uint32_t qIndex = 0; qIndex < computeQueueCount; qIndex++) {
					vkGetDeviceQueue(device, fIndex, qIndex, &queues[QueueType::COMPUTE][qIndex].queue);
				}
			}

			// Transfer queue
			if (VK_QUEUE_TRANSFER_BIT & queueFamilies[fIndex].queueFlags) {
				queues[QueueType::TRANSFER].resize(transferQueueCount);
				for (uint32_t qIndex = 0; qIndex < transferQueueCount; qIndex++) {
					vkGetDeviceQueue(device, fIndex, qIndex, &queues[QueueType::TRANSFER][qIndex].queue);
				}
			}
		}
	}

}