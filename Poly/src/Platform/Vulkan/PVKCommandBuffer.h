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

		virtual void BeginRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, uint32 width, uint32 height, float* pClearColor, uint32 clearColorCount) override final;

		virtual void BindPipeline(Pipeline* pPipeline) override final;

		virtual void BindDescriptor(Pipeline* pPipeline, Descriptor* pDescriptor) override final;

		virtual void CopyBufferToTexture(Buffer* pBuffer, Texture* pTexture, ETextureLayout layout, const CopyBufferDesc& copyBufferDesc) override final;

		virtual void DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) override final;

		virtual void DrawIndexedInstanced(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override final;

		virtual void EndRenderPass() override final;

		virtual void End() override final;

		/* End of commands */

		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Buffer); }
		VkCommandBuffer GetNativeVK() const { return m_Buffer; }

	private:
		VkCommandBuffer m_Buffer = VK_NULL_HANDLE;
	};

}