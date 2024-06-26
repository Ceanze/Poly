#include "polypch.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"
#include "Poly/Core/Window.h"

#include <cstdint>
// Required for VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#include <vulkan/vulkan_beta.h>

// API Specific objects
#include <cstring>

#include "PVKFence.h"
#include "PVKShader.h"
#include "PVKBuffer.h"
#include "PVKTexture.h"
#include "PVKSampler.h"
#include "PVKSwapChain.h"
#include "PVKSemaphore.h"
#include "PVKRenderPass.h"
#include "PVKFramebuffer.h"
#include "PVKTextureView.h"
#include "PVKCommandPool.h"
#include "PVKCommandQueue.h"
#include "PVKDescriptorSet.h"
#include "PVKCommandBuffer.h"
#include "PVKPipelineLayout.h"
#include "PVKGraphicsPipeline.h"

namespace
{
	VkFormat FindSupportedFormat(
	const std::vector<VkFormat>& candidates,
	VkImageTiling tiling,
	VkFormatFeatureFlags features,
	VkPhysicalDevice physicalDevice)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if ((tiling == VK_IMAGE_TILING_LINEAR) &&
				((props.linearTilingFeatures & features) == features))
			{
				return format;
			}
			else if ((tiling == VK_IMAGE_TILING_OPTIMAL) &&
				((props.optimalTilingFeatures & features) == features))
			{
				return format;
			}
		}
		return VK_FORMAT_UNDEFINED;
	}
}

namespace Poly
{
	VkInstance			PVKInstance::s_Instance				= VK_NULL_HANDLE;
	VkDevice			PVKInstance::s_Device				= VK_NULL_HANDLE;
	VkPhysicalDevice	PVKInstance::s_PhysicalDevice		= VK_NULL_HANDLE;
	VkSurfaceKHR		PVKInstance::s_Surface				= VK_NULL_HANDLE;
	PVKQueue			PVKInstance::s_PresentQueue			= {};
	uint32_t			PVKInstance::s_GraphicsQueueCount	= 1;
	uint32_t 			PVKInstance::s_ComputeQueueCount	= 1;
	uint32_t 			PVKInstance::s_TransferQueueCount	= 1;
	std::unordered_map<Poly::FQueueType, std::vector<Poly::PVKQueue>> PVKInstance::s_Queues;
	VmaAllocator PVKInstance::s_VmaAllocator = VK_NULL_HANDLE;

	PVKInstance::PVKInstance()
		: m_DebugMessenger(VK_NULL_HANDLE)
	{

	}

	PVKInstance::~PVKInstance()
	{
		vkDeviceWaitIdle(s_Device);

		vkDestroySurfaceKHR(s_Instance, s_Surface, nullptr);

		if (m_EnableValidationLayers)
			DestroyDebugUtilsMessengerEXT(s_Instance, m_DebugMessenger, nullptr);

		vmaDestroyAllocator(s_VmaAllocator);

		vkDestroyDevice(s_Device, nullptr);
		vkDestroyInstance(s_Instance, nullptr);
	}

	PVKInstance* PVKInstance::Get()
	{
		POLY_VALIDATE(s_PVKInstance, "Cannot get PVKInstance before init has been called! Has RenderAPI Init been called?");
		return s_PVKInstance;
	}

	void PVKInstance::Init(Window* pWindow)
	{
		CreateInstance();
		SetupDebugMessenger();

		PVK_CHECK(glfwCreateWindowSurface(s_Instance, pWindow->GetNative(), nullptr, &s_Surface), "Failed to create window surface!");

		PickPhysicalDevice();
		CreateLogicalDevice();

		CreateVmaAllocator();

		s_PVKInstance = this;
	}

	/*
	* GraphicsInstance functions
	*/
	Ref<Buffer> PVKInstance::CreateBuffer(const BufferDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "BufferDesc cannot be nullptr!");

		Ref<PVKBuffer> pBuffer = CreateRef<PVKBuffer>();
		pBuffer->Init(pDesc);
		return pBuffer;
	}

	Ref<Texture> PVKInstance::CreateTexture(const TextureDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "TextureDesc cannot be nullptr!");

		Ref<PVKTexture> pTexture = CreateRef<PVKTexture>();
		pTexture->Init(pDesc);
		return pTexture;
	}

	Ref<CommandQueue> PVKInstance::CreateCommandQueue(FQueueType queueType, uint32 queueIndex)
	{
		POLY_VALIDATE(queueType != FQueueType::NONE, "QueueType cannot be NONE!");

		Ref<PVKCommandQueue> pCommandQueue = CreateRef<PVKCommandQueue>();
		pCommandQueue->Init(queueType, queueIndex);
		return pCommandQueue;
	}

	Ref<TextureView> PVKInstance::CreateTextureView(const TextureViewDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "TextureViewDesc cannot be nullptr!");

		Ref<PVKTextureView> pTextureView = CreateRef<PVKTextureView>();
		pTextureView->Init(pDesc);
		return pTextureView;
	}

	Ref<SwapChain> PVKInstance::CreateSwapChain(const SwapChainDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "SwapChainDesc cannot be nullptr!");

		Ref<PVKSwapChain> pSwapChain = CreateRef<PVKSwapChain>();
		pSwapChain->Init(pDesc);
		return pSwapChain;
	}

	Ref<Fence> PVKInstance::CreateFence(FFenceFlag flag)
	{
		Ref<PVKFence> pFence = CreateRef<PVKFence>();
		pFence->Init(flag);
		return pFence;
	}

	Ref<Semaphore> PVKInstance::CreateSemaphore()
	{
		Ref<PVKSemaphore> pSemaphore = CreateRef<PVKSemaphore>();
		pSemaphore->Init();
		return pSemaphore;
	}

	Ref<CommandPool> PVKInstance::CreateCommandPool(FQueueType queueType, FCommandPoolFlags flags)
	{
		POLY_VALIDATE(queueType != FQueueType::NONE, "FQueueType cannot be NONE!");

		Ref<PVKCommandPool> pCommandPool = CreateRef<PVKCommandPool>();
		pCommandPool->Init(queueType, flags);
		return pCommandPool;
	}

	Ref<Sampler> PVKInstance::CreateSampler(const SamplerDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "SamplerDesc cannot be nullptr!");

		Ref<PVKSampler> pSampler = CreateRef<PVKSampler>();
		pSampler->Init(pDesc);
		return pSampler;
	}

	Ref<Shader> PVKInstance::CreateShader(const ShaderDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "ShaderDesc cannot be nullptr!");

		Ref<PVKShader> pShader = CreateRef<PVKShader>();
		pShader->Init(pDesc);
		return pShader;
	}

	Ref<GraphicsRenderPass> PVKInstance::CreateGraphicsRenderPass(const GraphicsRenderPassDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "RenderPassDesc cannot be nullptr!");

		Ref<PVKRenderPass> pShader = CreateRef<PVKRenderPass>();
		pShader->Init(pDesc);
		return pShader;
	}

	Ref<GraphicsPipeline> PVKInstance::CreateGraphicsPipeline(const GraphicsPipelineDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "GraphicsPipelineDesc cannot be nullptr!");

		Ref<PVKGraphicsPipeline> pShader = CreateRef<PVKGraphicsPipeline>();
		pShader->Init(pDesc);
		return pShader;
	}

	Ref<PipelineLayout> PVKInstance::CreatePipelineLayout(const PipelineLayoutDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "PipelineLayoutDesc cannot be nullptr!");

		Ref<PVKPipelineLayout> pShader = CreateRef<PVKPipelineLayout>();
		pShader->Init(pDesc);
		return pShader;
	}

	Ref<Framebuffer> PVKInstance::CreateFramebuffer(const FramebufferDesc* pDesc)
	{
		POLY_VALIDATE(pDesc, "FramebufferDesc cannot be nullptr!");

		Ref<PVKFramebuffer> pShader = CreateRef<PVKFramebuffer>();
		pShader->Init(pDesc);
		return pShader;
	}

	Ref<DescriptorSet> PVKInstance::CreateDescriptorSet(PipelineLayout* pLayout, uint32 setIndex)
	{
		POLY_VALIDATE(pLayout, "PipelineLayout cannot be nullptr!");

		Ref<PVKDescriptorSet> pShader = CreateRef<PVKDescriptorSet>();
		pShader->Init(pLayout, setIndex);
		return pShader;
	}

	Ref<DescriptorSet> PVKInstance::CreateDescriptorSetCopy(const Ref<DescriptorSet>& pSrcDescriptorSet)
	{
		Ref<PVKDescriptorSet> pNewSet = CreateRef<PVKDescriptorSet>();
		PipelineLayout* pPipelineLayout = pSrcDescriptorSet->GetLayout();
		uint32 setIndex = pSrcDescriptorSet->GetSetIndex();
		pNewSet->Init(pPipelineLayout, setIndex);

		VkCopyDescriptorSet copySetDesc = {};
		copySetDesc.sType			= VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
		copySetDesc.pNext			= nullptr;
		copySetDesc.srcSet			= static_cast<PVKDescriptorSet*>(pSrcDescriptorSet.get())->GetNativeVK();
		copySetDesc.srcArrayElement	= 0;
		copySetDesc.dstSet			= pNewSet->GetNativeVK();
		copySetDesc.dstArrayElement	= 0;
		copySetDesc.descriptorCount	= 1; // TODO: Change this to allow for multiple counts

		const auto& bindings = static_cast<PVKPipelineLayout*>(pPipelineLayout)->GetBindings(setIndex);
		std::vector<VkCopyDescriptorSet> copies;
		copies.reserve(bindings.size());
		for (uint32 i = 0; i < bindings.size(); i++)
		{
			copySetDesc.srcBinding	= bindings[i].Binding;
			copySetDesc.dstBinding	= bindings[i].Binding;
			copies.push_back(copySetDesc);
		}

		vkUpdateDescriptorSets(s_Device, 0, nullptr, static_cast<uint32>(copies.size()), copies.data());

		return pNewSet;
	}


	void PVKInstance::SetQueueCount(FQueueType queue, uint32_t count)
	{
		switch (queue) {
		case FQueueType::GRAPHICS:
			s_GraphicsQueueCount = count;
			break;
		case FQueueType::COMPUTE:
			s_ComputeQueueCount = count;
			break;
		case FQueueType::TRANSFER:
			s_TransferQueueCount = count;
			break;
		}
	}

	VkFormat PVKInstance::FindDepthFormat()
	{
		std::vector<VkFormat> formats;
		formats.push_back(VK_FORMAT_D32_SFLOAT);
		formats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
		formats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);

		return FindSupportedFormat(
			formats,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
			s_PhysicalDevice);
	}

	PVKQueue& PVKInstance::GetQueue(FQueueType queueType, uint32_t index)
	{
		// Check if valid queue and index, return if successful
		auto it = s_Queues.find(queueType);
		if (it != s_Queues.end()) {
			auto& vec = s_Queues[queueType];
			if (index < vec.size())
				return s_Queues[queueType][index];
		}

		// Return default graphics queue if the requested is not supported
		POLY_CORE_WARN("Requested queue with index {} could not be gotten! Returned standard graphics queue instead", index);
		return s_Queues[FQueueType::GRAPHICS][0];
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL PVKInstance::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		{
			POLY_CORE_TRACE("Info: {}\n", pCallbackData->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		{
			POLY_CORE_WARN("Warn: {}\n", pCallbackData->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		{
			POLY_CORE_ERROR("Error: {}\n", pCallbackData->pMessage);
			break;
		}
		// default:
		// 	POLY_CORE_TRACE("{}", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	VkResult PVKInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void PVKInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void PVKInstance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	void PVKInstance::CreateInstance()
	{
		PVK_CHECK(m_EnableValidationLayers && !CheckValidationLayerSupport(), "Validation layers requested, but not available!");

		// App info (Optional but can improve performance)
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Poly Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Poly";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		// Create info for instance creation
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
        createInfo.flags = 0;

		// Add the validation layers if they are enabled
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (m_EnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<unsigned>(m_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

#ifdef POLY_PLATFORM_MACOS
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

		// Add GLFW as an extension to the instance
		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Create instance and check if it succeded
		PVK_CHECK(vkCreateInstance(&createInfo, nullptr, &s_Instance), "Failed to create instance!");
	}

	void PVKInstance::SetupDebugMessenger()
	{
		if (!m_EnableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		PopulateDebugMessengerCreateInfo(createInfo);

		PVK_CHECK(CreateDebugUtilsMessengerEXT(s_Instance, &createInfo, nullptr, &m_DebugMessenger), "Failed to set up debug messenger!");
	}

	bool PVKInstance::CheckValidationLayerSupport()
	{
		// Get available layers
		unsigned layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Go through the selected validation layers and check if they are available
		for (const char* layerName : m_ValidationLayers) {
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

	bool PVKInstance::CheckDeviceExtensionSupport(VkPhysicalDevice checkDevice)
	{
		// Get all the extensions for the device
		unsigned extensionCount;
		vkEnumerateDeviceExtensionProperties(checkDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(checkDevice, nullptr, &extensionCount, availableExtensions.data());

		// Only get the unique extensions in our requested list
		std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

		// Check if the extensions we requested is supported by the device
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void PVKInstance::PickPhysicalDevice()
	{
		// Get number of physical devices
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(s_Instance, &deviceCount, nullptr);

		// If no were found, exit
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		// Get the physical devices
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(s_Instance, &deviceCount, devices.data());

		// Set optimal device
		SetOptimalDevice(devices);

		// If no suitable device was found, exit
		if (s_PhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		AddRequiredDeviceExtensions();
	}

	void PVKInstance::SetOptimalDevice(const std::vector<VkPhysicalDevice>& devices)
	{
		std::string pickedDeviceName;
		unsigned bestScore = 0;
		for (auto& d : devices)
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
			const bool extensionsSupported = CheckDeviceExtensionSupport(d);
			POLY_VALIDATE(extensionsSupported, "Required extensions are not supported!");

			// Save the device with the best score and is complete with its queues
			if (score > bestScore && findQueueFamilies(d, s_Surface).isComplete() && deviceFeatures.samplerAnisotropy) {
				bestScore = score;
				s_PhysicalDevice = d;
				pickedDeviceName = deviceProperties.deviceName;
			}
		}

		POLY_CORE_INFO("Picked device: {}", pickedDeviceName);
	}

	// TODO: Improve extension handling (Better extension adding, extension checking, etc.)
	void PVKInstance::AddRequiredDeviceExtensions()
	{
		unsigned extensionCount;
		vkEnumerateDeviceExtensionProperties(s_PhysicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(s_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

		const bool foundPortabilitySubset = std::find_if(availableExtensions.begin(), availableExtensions.end(), [](const VkExtensionProperties& properties)
		{
			return std::strcmp(properties.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0;
		}) != availableExtensions.end();

		if (foundPortabilitySubset)
		{
			m_DeviceExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
		}
	}

	void PVKInstance::CreateLogicalDevice()
	{
		// Create info for queues on the device
		QueueFamilyIndices indices = findQueueFamilies(s_PhysicalDevice, s_Surface);
		std::pair<uint32_t, uint32_t> graphicsQueueIndex = findQueueIndex(VK_QUEUE_GRAPHICS_BIT, s_PhysicalDevice);
		std::pair<uint32_t, uint32_t> computeQueueIndex = findQueueIndex(VK_QUEUE_COMPUTE_BIT, s_PhysicalDevice);
		std::pair<uint32_t, uint32_t> transferQueueIndex = findQueueIndex(VK_QUEUE_TRANSFER_BIT, s_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<unsigned> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), computeQueueIndex.first, transferQueueIndex.first };

		// Update queue count to match what the program can create
		s_GraphicsQueueCount	= std::min(s_GraphicsQueueCount, graphicsQueueIndex.second);
		s_ComputeQueueCount		= std::min(s_ComputeQueueCount, graphicsQueueIndex.second);
		s_TransferQueueCount	= std::min(s_TransferQueueCount, graphicsQueueIndex.second);

		std::unordered_map<uint32_t, uint32_t> queueCounts;
		queueCounts[indices.presentFamily.value()]	= 1;
		queueCounts[indices.graphicsFamily.value()] = s_GraphicsQueueCount;
		queueCounts[computeQueueIndex.first]		= s_ComputeQueueCount;
		queueCounts[transferQueueIndex.first]		= s_TransferQueueCount;

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
		createInfo.enabledExtensionCount = static_cast<unsigned>(m_DeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

		// Used for older implementations of vulkan
		if (m_EnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<unsigned>(m_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		// If extensions are to be added (which they will be) then it is here it will be

		// Create the logical device, bound to the physical device
		PVK_CHECK(vkCreateDevice(s_PhysicalDevice, &createInfo, nullptr, &s_Device), "Failed to create logical device!");

		//graphicsQueue.queueIndex = indices.graphicsFamily.value();
		s_PresentQueue.queueIndex = indices.presentFamily.value();

		//vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue.queue);
		GetAllQueues(uniqueQueueFamilies); // This function does not check for present support, hence seperate functions
		vkGetDeviceQueue(s_Device, indices.presentFamily.value(), 0, &s_PresentQueue.queue);
	}

	void PVKInstance::CreateVmaAllocator()
	{
		// Create info
		VmaAllocatorCreateInfo createInfo = {};
		createInfo.physicalDevice = s_PhysicalDevice;
		createInfo.device = s_Device;
		createInfo.instance = s_Instance;
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
		vmaCreateAllocator(&createInfo, &s_VmaAllocator);
	}

	std::vector<const char*> PVKInstance::GetRequiredExtensions()
	{
		unsigned glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_EnableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

#ifdef POLY_PLATFORM_MACOS
      extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

		return extensions;
	}

	void PVKInstance::GetAllQueues(std::set<unsigned> queueFamiliesUsed)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(s_PhysicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(s_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		for (uint32_t fIndex = 0; fIndex < queueFamilyCount; fIndex++) {
			// Only get the queues that we have assigned to the logical device
			if (!queueFamiliesUsed.contains(fIndex)) {
				continue;
			}

			// Graphics queue
			if (VK_QUEUE_GRAPHICS_BIT & queueFamilies[fIndex].queueFlags) {
				s_Queues[FQueueType::GRAPHICS].resize(s_GraphicsQueueCount);
				for (uint32_t qIndex = 0; qIndex < s_GraphicsQueueCount; qIndex++) {
					vkGetDeviceQueue(s_Device, fIndex, qIndex, &s_Queues[FQueueType::GRAPHICS][qIndex].queue);
					s_Queues[FQueueType::GRAPHICS][qIndex].queueFamilyIndex = fIndex;
				}
			}

			// Compute queue
			if (VK_QUEUE_COMPUTE_BIT & queueFamilies[fIndex].queueFlags) {
				s_Queues[FQueueType::COMPUTE].resize(s_ComputeQueueCount);
				for (uint32_t qIndex = 0; qIndex < s_ComputeQueueCount; qIndex++) {
					vkGetDeviceQueue(s_Device, fIndex, qIndex, &s_Queues[FQueueType::COMPUTE][qIndex].queue);
					s_Queues[FQueueType::COMPUTE][qIndex].queueFamilyIndex = fIndex;
				}
			}

			// Transfer queue
			if (VK_QUEUE_TRANSFER_BIT & queueFamilies[fIndex].queueFlags) {
				s_Queues[FQueueType::TRANSFER].resize(s_TransferQueueCount);
				for (uint32_t qIndex = 0; qIndex < s_TransferQueueCount; qIndex++) {
					vkGetDeviceQueue(s_Device, fIndex, qIndex, &s_Queues[FQueueType::TRANSFER][qIndex].queue);
					s_Queues[FQueueType::TRANSFER][qIndex].queueFamilyIndex = fIndex;
				}
			}
		}
	}

}
