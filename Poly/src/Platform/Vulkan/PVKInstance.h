#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "PVKTypes.h"
#include "Platform/API/GraphicsInstance.h"
#include "VulkanCommon.h"

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

		static VkFormat FindDepthFormat();

		static VkDevice			GetDevice() { return s_Device; }
		static VkPhysicalDevice	GetPhysicalDevice() { return s_PhysicalDevice; }
		static VkInstance		GetInstance() { return s_Instance; }
		static PVKQueue&		GetQueue(FQueueType queueType, uint32_t index = 0);
		static const std::vector<PVKQueue>& GetAllQueues();
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
		void PopulateQueues(const std::vector<QueueSpec>& queueSpecs);

		VkDebugUtilsMessengerEXT			m_DebugMessenger;
		inline static VkInstance			s_Instance				= VK_NULL_HANDLE;
		inline static VkPhysicalDevice		s_PhysicalDevice		= VK_NULL_HANDLE;
		inline static VkDevice				s_Device				= VK_NULL_HANDLE;
		inline static VkSurfaceKHR			s_Surface				= VK_NULL_HANDLE;
		inline static std::vector<PVKQueue> s_Queues;
		inline static std::unordered_map<FQueueType, std::vector<uint32_t>> s_QueueMappings;

		inline static VmaAllocator s_VmaAllocator = VK_NULL_HANDLE;

		#ifdef POLY_DEBUG
				const bool m_EnableValidationLayers = true;
		#else
				const bool m_EnableValidationLayers = false;
		#endif
	};
}
