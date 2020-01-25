#include "polypch.h"
#include "PVKInstance.h"

namespace Poly
{

	PVKInstance::PVKInstance(unsigned width, unsigned height)
	{
		createInstance();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	PVKInstance::~PVKInstance()
	{
		vkDestroyDevice(this->device, nullptr);

		vkDestroyInstance(this->instance, nullptr);
	}

	void PVKInstance::init()
	{
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
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<unsigned>(this->validationLayers.size());
			createInfo.ppEnabledLayerNames = this->validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		// Add GLFW as an extension to the instance
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		// Validation layers
		createInfo.enabledLayerCount = 0;

		// Create instance and check if it succeded
		if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
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

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		// Enable or disable features for the device
		// Currently no features are to be enabled
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// Create info for the logical device
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
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
	}

}