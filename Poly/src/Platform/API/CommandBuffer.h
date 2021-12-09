#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	struct ScissorDesc;
	struct ViewportDesc;

	class Buffer;
	class Texture;
	class Pipeline;
	class Framebuffer;
	class CommandPool;
	class CommandQueue;
	class DescriptorSet;
	class PipelineLayout;
	class GraphicsRenderPass;

	// Command structs (for those who want to take in like 50 parameters)

	struct CopyBufferDesc
	{
		// Width, height and aspect mask is presumed to be the same values from the texture image
		uint64			BufferOffset		= 0;
		uint32			BufferRowLength		= 0;
		uint32			BufferImageHeight	= 0;
		uint32			MipLevel			= 0;
		uint32			ArrayLayer			= 0;
		uint32			ArrayCount			= 0;
		int				ImageOffsetX		= 0;
		int				ImageOffsetY		= 0;
		int				ImageOffsetZ		= 0;
		uint32			Width				= 0;
		uint32			Height				= 0;
		uint32			Depth				= 0;
	};

	struct BufferBarrier
	{
		FAccessFlag	SrcAccessFlag	= FAccessFlag::NONE;
		FAccessFlag	DstAccessFlag	= FAccessFlag::NONE;
		uint32		SrcQueueIndex	= 0;
		uint32		DstQueueIndex	= 0;
		Buffer*		pBuffer			= nullptr;
		uint32		Offset			= 0;
	};

	struct TextureBarrier
	{
		FAccessFlag		SrcAccessFlag	= FAccessFlag::NONE;
		FAccessFlag		DstAccessFlag	= FAccessFlag::NONE;
		ETextureLayout	OldLayout		= ETextureLayout::UNDEFINED;
		ETextureLayout	NewLayout		= ETextureLayout::UNDEFINED;
		uint32			SrcQueueIndex	= 0;
		uint32			DstQueueIndex	= 0;
		Texture*		pTexture		= nullptr;
		FImageViewFlag	AspectMask		= FImageViewFlag::NONE;
	};

	struct AccessBarrier
	{
		FAccessFlag		SrcAccessFlag	= FAccessFlag::NONE;
		FAccessFlag		DstAccessFlag	= FAccessFlag::NONE;
	};

	class CommandBuffer
	{
	public:
		CLASS_ABSTRACT(CommandBuffer);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(CommandPool* pCommandPool) = 0;

		/**
		 * Begin the command buffer for recording of commands
		 * @param bufferFlag - Usage of buffer, single time or normal
		 */
		virtual void Begin(FCommandBufferFlag bufferFlag) = 0;

		/**
		 * Begin a render pass
		 * @param pRenderPass - Render pass to begin
		 * @param pFrameBuffer - Framebuffer to use with the render pass
		 * @param width - Width of render area
		 * @param height - Height of render area
		 * @param clearColors - Clear color of the render pass, each color requires a float4
		 * @param clearColorCount - Amount of clear colors to use from clear colors
		 */
		virtual void BeginRenderPass(GraphicsRenderPass* pRenderPass, Framebuffer* pFramebuffer, uint32 width, uint32 height, std::vector<ClearValue> clearValues) = 0;

		/**
		 * Bind a pipeline
		 * @param pPipeline - The pipeline to bind
		 */
		virtual void BindPipeline(Pipeline* pPipeline) = 0;

		/**
		 * Binds a descriptor with the corresponding pipeline
		 * @param pPipeline - Pipeline that contains the layouts
		 * @param pDescriptor - Descriptor containing the sets
		 * @param setIndex - Which set to bind
		 */
		virtual void BindDescriptor(Pipeline* pPipeline, DescriptorSet* pDescriptor, uint32 dynamicOffsetCount = 0, const uint32* pDynamicOffsets = nullptr) = 0;

		/**
		 * Binds a vertex buffer
		 * @param pBuffer - Vertex buffer to bind - must be created with vertex buffer flag
		 * @param firstBinding - First buffer to bind
		 * @param bindingCount - number of buffers to bind
		 * @param offset - Offset in buffer
		 */
		virtual void BindVertexBuffer(const Buffer* pBuffer, uint32 firstBinding, uint32 bindingCount, uint64 offset) = 0;

		/**
		 * Binds index buffer
		 * @param pBuffer - index buffer to bind
		 * @param offset - offset in index buffer
		 * @param indexType - Type of index, either 16 or 32 bit
		 */
		virtual void BindIndexBuffer(const Buffer* pBuffer, uint64 offset, EIndexType indexType) = 0;

		/**
		 * Update push constants
		 * @param pPipelineLayout - Layout which contains the to-be-updated push constant
		 * @param shaderStage - The shader stage in which the push constants are used
		 * @param offset - Starting offset
		 * @param size - Size of the data to update with
		 * @param data - ponter to the new data
		 */
		virtual void UpdatePushConstants(const PipelineLayout* pPipelineLayout, FShaderStage shaderStage, uint64 offset, uint64 size, const void* data) = 0;

		/**
		 * Copy buffer to a texture
		 * @param pBuffer - Buffer to copy from
		 * @param pTexture - Texture to copy to
		 * @param layout - Layout of destination texture
		 * @param copyBufferDesc - Description of the copy details
		 */
		virtual void CopyBufferToTexture(const Buffer* pBuffer, const Texture* pTexture, ETextureLayout layout, const CopyBufferDesc& copyBufferDesc) = 0;

		/**
		 * Copy source buffer to destination buffer
		 * @param pSrcBuffer - source buffer
		 * @param pDstBuffer - destination buffer
		 * @param size - size of copy area
		 * @param srcOffset - offset in bytes of source buffer
		 * @param dstOffset - offset in bytes of destination buffer
		 */
		virtual void CopyBuffer(const Buffer* pSrcBuffer, const Buffer* pDstBuffer, uint64 size, uint64 srcOffset, uint64 dstOffset) = 0;

		/**
		 * Update device buffer with local data from host memory - Limited to 65536
		 * Use staging buffer for larger amount of transfers
		 * @param buffer - destination buffer on GPU
		 * @param size - size of data in bytes - must be a multiple of 4
		 * @param offset - offset of buffer to start updating - must be a multiple of 4
		 * @param pData - data to use for the updating - must be atleast size bytes
		 */
		virtual void UpdateBuffer(const Buffer* pBuffer, uint64 size, uint64 offset, const void* pData) = 0;

		/**
		 * Set dynamic viewport
		 * @param viewport - viewport struct
		 */
		virtual void SetViewport(const ViewportDesc* pViewport) = 0;

		/**
		 * Set dynamic scissor rect
		 * @param scissor - scissor struct
		 */
		virtual void SetScissor(const ScissorDesc* pScissor) = 0;

		/**
		 * Draw the the bound objects instanced
		 * @param vertexCount - Amount of verticies
		 * @param instanceCount - Amount of instances (1 when not drawing instanced)
		 * @param firstVertex - First vertex to start drawing
		 * @param firstInstance - First instance to start drawing
		 */
		virtual void DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) = 0;

		/**
		 * Draw the the bound objects indexed and instanced
		 * @param indexCount - Amount of indicies
		 * @param instanceCount - Amount of instances (1 when not drawing instanced)
		 * @param firstIndex - First index to start drawing
		 * @param vertexOffset - Offset for each vertex in the vertex buffer
		 * @param firstInstance - First instance to start drawing
		 */
		virtual void DrawIndexedInstanced(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) = 0;

		/**
		 * Acquire buffer ownership - NOTE: Must match with a ReleaseBuffer with the necessary parameters
		 * @param pBuffer - Buffer to acquire (must match ReleaseBuffer)
		 * @param srcStage - Stage where the release happens (must match ReleaseBuffer)
		 * @param dstStage - Stage where the acquire happens (must match ReleaseBuffer)
		 * @param accessMask - Where the buffer will be accessed (must match ReleaseBuffer)
		 * @param srcQueueIndex - Queue that released the buffer (must match ReleaseBuffer)
		 * @param dstQueueIndex - Queue that will acuiqre the buffer (must match ReleaseBuffer)
		 */
		virtual void AcquireBuffer(const Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag dstAccessMask, uint32 srcQueueIndex, uint32 dstQueueIndex) = 0;

		/**
		 * Release buffer ownership - NOTE: Must match with a AcquireBuffer with the necessary parameters
		 * @param pBuffer - Buffer to release (must match AcquireBuffer)
		 * @param srcStage - Stage where the release happens (must match AcquireBuffer)
		 * @param dstStage - Stage where the acquire happens (must match AcquireBuffer)
		 * @param accessMask - Where the buffer will be accessed (must match AcquireBuffer)
		 * @param srcQueueIndex - Queue that will release the buffer (must match AcquireBuffer)
		 * @param dstQueueIndex - Queue that will acuiqre the buffer (must match AcquireBuffer)
		 */
		virtual void ReleaseBuffer(const Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessMask, uint32 srcQueueIndex, uint32 dstQueueIndex) = 0;

		/**
		 * Acquire texture ownership - NOTE: Must match with a ReleaseTexture with the necessary parameters
		 * @param pTexture - Texture to acquire (must match ReleaseTexture)
		 * @param srcStage - Stage where the release happens (must match ReleaseTexture)
		 * @param dstStage - Stage where the acquire happens (must match ReleaseTexture)
		 * @param accessMask - Where the texture will be accessed (must match ReleaseTexture)
		 * @param oldLayout - Old layout of the texture - keep same if no layout is required (must match ReleaseTexture)
		 * @param newLayout - New layout of the texture - keep same if no layout is required (must match ReleaseTexture)
		 * @param srcQueueIndex - Queue that released the texture (must match ReleaseTexture)
		 * @param dstQueueIndex - Queue that will acuiqre the texture (must match ReleaseTexture)
		 */
		virtual void AcquireTexture(const Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag dstAccessMask, ETextureLayout oldLayout, ETextureLayout newLayout, uint32 srcQueueIndex, uint32 dstQueueIndex) = 0;

		/**
		 * Release texture ownership - NOTE: Must match with a AcquireTexture with the necessary parameters
		 * @param pTexture - Texture to release (must match AcquireTexture)
		 * @param srcStage - Stage where the release happens (must match AcquireTexture)
		 * @param dstStage - Stage where the acquire happens (must match AcquireTexture)
		 * @param accessMask - Where the texture will be accessed (must match AcquireTexture)
		 * @param oldLayout - Old layout of the texture - keep same if no layout is required (must match AcquireTexture)
		 * @param newLayout - New layout of the texture - keep same if no layout is required (must match AcquireTexture)
		 * @param srcQueueIndex - Queue that will release the texture (must match AcquireTexture)
		 * @param dstQueueIndex - Queue that will acuiqre the texture (must match AcquireTexture)
		 */
		virtual void ReleaseTexture(const Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessMask, ETextureLayout oldLayout, ETextureLayout newLayout, uint32 srcQueueIndex, uint32 dstQueueIndex) = 0;

		/**
		 * Add a pipeline texture memory barrier. This function does not transfer between queues, see AcquireTexture and ReleaseTexture for that
		 * @param pTexture - Texture to apply barrier for
		 * @param srcStage - Source stage of the barrier
		 * @param dstStage - Destination stage of the barrier
		 * @param srcAccessMask - Last point before barrier the texture will be accessed
		 * @param dstAccessMask - First point after barrier the texture will be accessed
		 * @param oldLayout - Old layout of the texture - keep same if no layout is required
		 * @param newLayout - New layout of the texture - keep same if no layout is required
		 */
		virtual void PipelineTextureBarrier(const Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessFlag, FAccessFlag dstAccessFlag, ETextureLayout oldLayout, ETextureLayout newLayout) = 0;

		/**
		 * Add a pipeline buffer memory barrier. This function does not transfer between queues, see AcquireBuffer and ReleaseBuffer for that
		 * @param pBuffer - Buffer to apply barrier for
		 * @param srcStage - Source stage of the barrier
		 * @param dstStage - Destination stage of the barrier
		 * @param srcAccessMask - Last point before barrier the buffer will be accessed
		 * @param dstAccessMask - First point after barrier the buffer will be accessed
		 */
		virtual void PipelineBufferBarrier(const Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessFlag, FAccessFlag dstAccessFlag) = 0;

		/**
		 * Add multiple barriers of different types, for only one barrier of either buffer or texture see their respective functions
		 * @param accessBarriers
		 * @param bufferBarriers
		 * @param textureBarriers
		 */
		virtual void PipelineBarrier(FPipelineStage srcStage, FPipelineStage dstStage, const std::vector<AccessBarrier>& accessBarriers, const std::vector<BufferBarrier>& bufferBarriers, const std::vector<TextureBarrier>& textureBarriers) = 0;

		/**
		 * End the render pass
		 */
		virtual void EndRenderPass() = 0;

		/**
		 * End the recording of commands
		 */
		virtual void End() = 0;

		/**
		 * Reset the command buffer - clears all commands
		 * Note: Mustn't be in pending state
		 */
		virtual void Reset() = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

		/**
		 * @return Command pool command buffer was created with
		 */
		inline CommandPool* GetPool() const
		{
			return p_pCommandPool;
		}

	protected:
		CommandPool* p_pCommandPool;
	};
}