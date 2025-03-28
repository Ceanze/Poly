#pragma once

#include "Platform/API/CommandBuffer.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKCommandBuffer : public CommandBuffer
	{
	public:
		PVKCommandBuffer() = default;
		~PVKCommandBuffer();

		virtual void Init(CommandPool* pCommandPool) override final;

		/* Commands */

		virtual void Begin(FCommandBufferFlag bufferFlag) override final;

		virtual void BeginRenderPass(GraphicsRenderPass* pRenderPass, Framebuffer* pFramebuffer, uint32 width, uint32 height, std::vector<ClearValue> clearValues) override final;

		virtual void BindPipeline(Pipeline* pPipeline) override final;

		virtual void BindDescriptor(const Pipeline* pPipeline, const DescriptorSet* pDescriptor, uint32 dynamicOffsetCount = 0, const uint32* pDynamicOffsets = nullptr) override final;

		virtual void BindVertexBuffer(const Buffer* pBuffer, uint32 firstBinding, uint32 bindingCount, uint64 offset) override final;

		virtual void BindIndexBuffer(const Buffer* pBuffer, uint64 offset, EIndexType indexType) override final;

		virtual void UpdatePushConstants(const PipelineLayout* pPipelineLayout, FShaderStage shaderStage, uint32 offset, uint32 size, const void* data) override final;

		virtual void CopyBufferToTexture(const Buffer* pBuffer, const Texture* pTexture, ETextureLayout layout, const CopyBufferDesc& copyBufferDesc) override final;

		virtual void CopyBuffer(const Buffer* pSrcBuffer, const Buffer* pDstBuffer, uint64 size, uint64 srcOffset, uint64 dstOffset) override final;

		virtual void CopyBufferRegions(const Buffer* pSrcBuffer, const Buffer* pDstBuffer, const std::vector<BufferRegion>& regions) override final;

		virtual void UpdateBuffer(const Buffer* pBuffer, uint64 size, uint64 offset, const void* pData) override final;

		virtual void SetViewport(const ViewportDesc* pViewport) override final;

		virtual void SetScissor(const ScissorDesc* pScissor) override final;

		virtual void DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) override final;

		virtual void DrawIndexedInstanced(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override final;

		virtual void AcquireBuffer(const Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag dstAccessMask, uint32 srcQueueIndex, uint32 dstQueueIndex) override final;

		virtual void ReleaseBuffer(const Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessMask, uint32 srcQueueIndex, uint32 dstQueueIndex) override final;

		virtual void AcquireTexture(const Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag dstAccessMask, ETextureLayout oldLayout, ETextureLayout newLayout, uint32 srcQueueIndex, uint32 dstQueueIndex) override final;

		virtual void ReleaseTexture(const Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessMask, ETextureLayout oldLayout, ETextureLayout newLayout, uint32 srcQueueIndex, uint32 dstQueueIndex) override final;

		virtual void PipelineTextureBarrier(const Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessFlag, FAccessFlag dstAccessFlag, ETextureLayout oldLayout, ETextureLayout newLayout) override final;

		virtual void PipelineBufferBarrier(const Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessFlag, FAccessFlag dstAccessFlag) override final;

		virtual void PipelineBarrier(FPipelineStage srcStage, FPipelineStage dstStage, const std::vector<AccessBarrier>& accessBarriers, const std::vector<BufferBarrier>& bufferBarriers, const std::vector<TextureBarrier>& textureBarriers) override final;

		virtual void EndRenderPass() override final;

		virtual void End() override final;

		virtual void Reset() override final;

		/* End of commands */

		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Buffer); }
		VkCommandBuffer GetNativeVK() const { return m_Buffer; }

	private:
		VkCommandBuffer m_Buffer = VK_NULL_HANDLE;
	};

}