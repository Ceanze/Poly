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
	PVKInstance::PVKInstance()
		: m_DebugMessenger(VK_NULL_HANDLE) {}

	PVKInstance::~PVKInstance()
	{
		vkDeviceWaitIdle(s_Device);

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

	void PVKInstance::Init()
	{
		CreateInstance();
		SetupDebugMessenger();

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
		if (!s_QueueMappings.contains(queueType))
		{
			POLY_CORE_ERROR("Requested queue with index {} could not be gotten! Returned standard graphics queue instead", index);
			POLY_VALIDATE(s_QueueMappings.contains(FQueueType::GRAPHICS), "Graphics queue does not exist");
			POLY_VALIDATE(!s_QueueMappings[FQueueType::GRAPHICS].empty(), "Graphics queue type does not have any queues created");
			return s_Queues[s_QueueMappings[FQueueType::GRAPHICS].front()];
		}

		index = std::min(index, static_cast<uint32_t>(s_QueueMappings[queueType].size()));

		POLY_VALIDATE(index >= 0, "Graphics queue type does not have any queues created");

		uint32_t mappedIndex = s_QueueMappings[queueType][index];

		return s_Queues[mappedIndex];
	}

	const std::vector<PVKQueue>& PVKInstance::GetAllQueues()
	{
		return s_Queues;
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
		appInfo.apiVersion = VK_API_VERSION_1_3;

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
			//POLY_VALIDATE(extensionsSupported, "Required extensions are not supported!");

			// Save the device with the best score and is complete with its queues
			if (extensionsSupported && score > bestScore && deviceFeatures.samplerAnisotropy) {
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
		std::vector<QueueSpec> queueSpecs = FindAllQueues(s_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		float queuePriority = 1.0f;
		std::vector<std::vector<float>> allPriorities;
		for (const QueueSpec& queueSpec : queueSpecs) {
			allPriorities.emplace_back(queueSpec.QueueCount, 1.0f);

			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueSpec.QueueFamily;
			queueCreateInfo.queueCount = queueSpec.QueueCount;
			queueCreateInfo.pQueuePriorities = allPriorities.back().data();;
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

		PopulateQueues(queueSpecs);
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

	void PVKInstance::PopulateQueues(const std::vector<QueueSpec>& queueSpecs)
	{
		for (uint32_t fIndex = 0; fIndex < queueSpecs.size(); fIndex++)
		{
			for (uint32_t qIndex = 0; qIndex < queueSpecs[fIndex].QueueCount; qIndex++)
			{
				const QueueSpec& queueSpec = queueSpecs[fIndex];

				// Get queue from device
				VkQueue queue;
				VkDeviceQueueInfo2 desc = {};
				desc.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
				desc.queueFamilyIndex	= fIndex;
				desc.queueIndex			= qIndex;
				vkGetDeviceQueue2(s_Device, &desc, &queue);
				s_Queues.push_back(PVKQueue{ queue, qIndex, fIndex });

				// Map queue
				uint32_t index = static_cast<uint32_t>(s_Queues.size() - 1);
				if (VK_QUEUE_GRAPHICS_BIT & queueSpec.QueueFlags)
					s_QueueMappings[FQueueType::GRAPHICS].push_back(index);
				else if (VK_QUEUE_COMPUTE_BIT & queueSpec.QueueFlags)
					s_QueueMappings[FQueueType::COMPUTE].push_back(index);
				else if (VK_QUEUE_VIDEO_DECODE_BIT_KHR & queueSpec.QueueFlags)
					s_QueueMappings[FQueueType::VIDEO_DECODE].push_back(index);
				else if (VK_QUEUE_VIDEO_ENCODE_BIT_KHR & queueSpec.QueueFlags)
					s_QueueMappings[FQueueType::VIDEO_ENCODE].push_back(index);
				else if (VK_QUEUE_OPTICAL_FLOW_BIT_NV & queueSpec.QueueFlags)
					s_QueueMappings[FQueueType::OPTICAL_FLOW].push_back(index);
				else if (VK_QUEUE_DATA_GRAPH_BIT_ARM & queueSpec.QueueFlags)
					s_QueueMappings[FQueueType::DATA_GRAPH].push_back(index);
				else if (VK_QUEUE_TRANSFER_BIT & queueSpec.QueueFlags)
					s_QueueMappings[FQueueType::TRANSFER].push_back(index);
				else if (VK_QUEUE_SPARSE_BINDING_BIT & queueSpec.QueueFlags)
					s_QueueMappings[FQueueType::SPARSE_BINDING].push_back(index);
			}
		}
	}

}
