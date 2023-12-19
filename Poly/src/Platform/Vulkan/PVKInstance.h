#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "PVKTypes.h"
#include "Platform/API/GraphicsInstance.h"

#include "VmaInclude.h"

namespace Poly
{
	class Window;

	/*
		PVKInstance contains the vulkan instance, physical device, and logical device
		that are created for that instance.
	*/

	class PVKInstance : public GraphicsInstance
	{
	public:
		PVKInstance();
		~PVKInstance();

		static PVKInstance* Get();

		virtual void Init(Window* pWindow) override final;

		/*
		* GraphicsInstance functions
		*/
		virtual Ref<Buffer>				CreateBuffer(const BufferDesc* pDesc) override final;
		virtual Ref<Texture>			CreateTexture(const TextureDesc* pDesc) override final;
		virtual Ref<CommandQueue>		CreateCommandQueue(FQueueType queueType, uint32 queueIndex) override final;
		virtual Ref<TextureView>		CreateTextureView(const TextureViewDesc* pDesc) override final;
		virtual Ref<SwapChain>			CreateSwapChain(const SwapChainDesc* pDesc) override final;
		virtual Ref<Fence>				CreateFence(FFenceFlag flag) override final;
		virtual Ref<Semaphore>			CreateSemaphore() override final;
		virtual Ref<CommandPool>		CreateCommandPool(FQueueType queueType, FCommandPoolFlags flags) override final;
		virtual Ref<Sampler>			CreateSampler(const SamplerDesc* pDesc) override final;
		virtual Ref<Shader>				CreateShader(const ShaderDesc* pDesc) override final;
		virtual Ref<GraphicsRenderPass>	CreateGraphicsRenderPass(const GraphicsRenderPassDesc* pDesc) override final;
		virtual Ref<GraphicsPipeline>	CreateGraphicsPipeline(const GraphicsPipelineDesc* pDesc) override final;
		virtual Ref<PipelineLayout>		CreatePipelineLayout(const PipelineLayoutDesc* pDesc) override final;
		virtual Ref<Framebuffer>		CreateFramebuffer(const FramebufferDesc* pDesc) override final;
		virtual Ref<DescriptorSet>		CreateDescriptorSet(PipelineLayout* pLayout, uint32 setIndex) override final;

		virtual Ref<DescriptorSet>		CreateDescriptorSetCopy(const Ref<DescriptorSet>& pSrcDescriptorSet) override final;

		// Sets how many queues should be created for that queue type. Must be called before init.
		// Default queue count per queue is 1. If the requested queue count is higher than the available, the
		// most available will be set.
		static void SetQueueCount(FQueueType queue, uint32_t count);

		static VkFormat FindDepthFormat();

		static VkDevice			GetDevice() { return s_Device; }
		static VkPhysicalDevice	GetPhysicalDevice() { return s_PhysicalDevice; }
		static VkInstance		GetInstance() { return s_Instance; }
		static PVKQueue&		GetPresentQueue() { return s_PresentQueue; }
		static PVKQueue&		GetQueue(FQueueType queueType, uint32_t index = 0);
		static VkSurfaceKHR		GetSurface() { return s_Surface; }
		static VmaAllocator		GetAllocator() { return s_VmaAllocator; }

	private:
		inline static PVKInstance* s_PVKInstance = nullptr;

		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char*> m_DeviceExtensions = {
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
		void AddRequiredDeviceExtensions();
		void CreateLogicalDevice();
		void CreateVmaAllocator();
		std::vector<const char*> GetRequiredExtensions();
		void GetAllQueues(std::set<unsigned> queueFamiliesUsed);

		VkDebugUtilsMessengerEXT	m_DebugMessenger;
		static VkInstance			s_Instance;
		static VkPhysicalDevice		s_PhysicalDevice;
		static VkDevice				s_Device;
		static PVKQueue				s_PresentQueue; // TODO: Should maybe be in the queue map
		static VkSurfaceKHR			s_Surface;
		static uint32_t				s_GraphicsQueueCount;
		static uint32_t				s_ComputeQueueCount;
		static uint32_t				s_TransferQueueCount;
		static std::unordered_map<Poly::FQueueType, std::vector<Poly::PVKQueue>> s_Queues;

		static VmaAllocator s_VmaAllocator;

		#ifdef POLY_DEBUG
				const bool m_EnableValidationLayers = true;
		#else
				const bool m_EnableValidationLayers = false;
		#endif
	};
}
