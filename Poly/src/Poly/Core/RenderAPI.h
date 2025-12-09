#pragma once

#include "Core.h"
#include "Platform/API/GraphicsInstance.h"
#include "Platform/API/FramebufferCache.h"

/*
*	TODO: Convert PVKInstance to not be a static class for it to have an interface in which it inherits from.
*			This will allow the instance to have create functions to API specific objects (like GraphicsDeviceVK).
*		For future reference, see this psudo code:

		RenderAPI::GetGraphicsInstance()->CreateBuffer(desc);
		RenderAPI::CreateBuffer(desc);
		RenderAPI holds a pointer to the current instance which is created with the correct API beforehand.
		RenderAPIs function GetInstance returns this pointer, from which the user can call the API agnostic
		functions such as CreateBuffer(desc) which will create API specific objects.
*/

namespace Poly
{
	struct ShaderDesc;
	struct BufferDesc;
	struct SamplerDesc;
	struct TextureDesc;
	struct SwapChainDesc;
	struct FramebufferDesc;
	struct TextureViewDesc;
	struct CommandQueueDesc;
	struct DescriptorSetDesc;
	struct PipelineLayoutDesc;
	struct GraphicsPipelineDesc;
	struct GraphicsRenderPassDesc;

	class Fence;
	class Shader;
	class Buffer;
	class Window;
	class Sampler;
	class Texture;
	class Semaphore;
	class SwapChain;
	class Framebuffer;
	class TextureView;
	class CommandPool;
	class CommandQueue;
	class DescriptorSet;
	class PipelineLayout;
	class GraphicsPipeline;
	class GraphicsRenderPass;

	class RenderAPI
	{
	public:
		enum class BackendAPI { VULKAN };

		CLASS_STATIC(RenderAPI);

		static void Init(BackendAPI backendAPI);
		static void Release();

		static CommandQueue*		GetCommandQueue(FQueueType queue);

		static GraphicsInstance*	GetGraphicsInstance() { return m_pGraphicsInstance; }

		// Create functions
		static Ref<Buffer>				CreateBuffer(const BufferDesc* pDesc);
		static Ref<Texture>				CreateTexture(const TextureDesc* pDesc);
		static Ref<CommandQueue>		CreateCommandQueue(FQueueType queueType, uint32 queueIndex);
		static Ref<TextureView>			CreateTextureView(const TextureViewDesc* pDesc);
		static Ref<SwapChain>			CreateSwapChain(const SwapChainDesc* pDesc);
		static Ref<Fence>				CreateFence(FFenceFlag flag);
		static Ref<Semaphore>			CreateSemaphore();
		static Ref<CommandPool>			CreateCommandPool(FQueueType queueType, FCommandPoolFlags flags);
		static Ref<Sampler>				CreateSampler(const SamplerDesc* pDesc);
		static Ref<Shader>				CreateShader(const ShaderDesc* pDesc);
		static Ref<GraphicsRenderPass>	CreateGraphicsRenderPass(const GraphicsRenderPassDesc* pDesc);
		static Ref<GraphicsPipeline>	CreateGraphicsPipeline(const GraphicsPipelineDesc* pDesc);
		static Ref<PipelineLayout>		CreatePipelineLayout(const PipelineLayoutDesc* pDesc);
		static Ref<Framebuffer>			CreateFramebuffer(const FramebufferDesc* pDesc);
		static Ref<DescriptorSet>		CreateDescriptorSet(PipelineLayout* pLayout, uint32 setIndex);

		static Ref<DescriptorSet>		CreateDescriptorSetCopy(const Ref<DescriptorSet>& pSrcDescriptorSet);

		/**
		 * Gets or creates a framebuffer and returns it
		 * @param attachments - Texture view attachments to be used for the framebuffer
		 * @param pDepthAttachment - Texture view depth attachment to be used - nullptr if no depth attachment
		 * @param pPass - Graphics render pass the framebuffer will use
		 * @param width
		 * @param height
		 * @return Ref<Framebuffer>
		 */
		static Ref<Framebuffer>			GetFramebuffer(const std::vector<TextureView*>& attachments, TextureView* pDepthAttachment, GraphicsRenderPass* pPass, uint32 width, uint32 height);

	private:
		inline static GraphicsInstance*	m_pGraphicsInstance	= nullptr;

		// Queue types [TODO: Support multiple queues per type]
		inline static Ref<CommandQueue>	m_pGraphicsQueue	= nullptr;
		inline static Ref<CommandQueue>	m_pComputeQueue		= nullptr;
		inline static Ref<CommandQueue>	m_pTransferQueue	= nullptr;

		inline static FramebufferCache	m_FramebufferCache;
	};
}