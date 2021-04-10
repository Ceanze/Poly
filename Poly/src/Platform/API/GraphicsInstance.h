#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	// Structs
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

	// Classes
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

	class GraphicsInstance
	{
	public:
		CLASS_ABSTRACT(GraphicsInstance);

		virtual void Init(Window* pWindow) = 0;

		virtual Ref<Buffer>				CreateBuffer(const BufferDesc* pDesc) = 0;
		virtual Ref<Texture>			CreateTexture(const TextureDesc* pDesc) = 0;
		virtual Ref<CommandQueue>		CreateCommandQueue(FQueueType queueType, uint32 queueIndex) = 0;
		virtual Ref<TextureView>		CreateTextureView(const TextureViewDesc* pDesc) = 0;
		virtual Ref<SwapChain>			CreateSwapChain(const SwapChainDesc* pDesc) = 0;
		virtual Ref<Fence>				CreateFence(FFenceFlag flag) = 0;
		virtual Ref<Semaphore>			CreateSemaphore() = 0;
		virtual Ref<CommandPool>		CreateCommandPool(FQueueType queueType) = 0;
		virtual Ref<Sampler>			CreateSampler(const SamplerDesc* pDesc) = 0;
		virtual Ref<Shader>				CreateShader(const ShaderDesc* pDesc) = 0;
		virtual Ref<GraphicsRenderPass>	CreateGraphicsRenderPass(const GraphicsRenderPassDesc* pDesc) = 0;
		virtual Ref<GraphicsPipeline>	CreateGraphicsPipeline(const GraphicsPipelineDesc* pDesc) = 0;
		virtual Ref<PipelineLayout>		CreatePipelineLayout(const PipelineLayoutDesc* pDesc) = 0;
		virtual Ref<Framebuffer>		CreateFramebuffer(const FramebufferDesc* pDesc) = 0;
		virtual Ref<DescriptorSet>		CreateDescriptorSet(PipelineLayout* pLayout, uint32 setIndex) = 0;

		virtual Ref<DescriptorSet>		CreateDescriptorSetCopy(const Ref<DescriptorSet>& pSrcDescriptorSet) = 0;
	};
}