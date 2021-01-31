#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Buffer;
	class Texture;
	class Pipeline;
	class RenderPass;
	class Descriptor;
	class Framebuffer;
	class CommandPool;
	class CommandQueue;

	// Command structs (for those who want to take in like 50 parameters)

	struct CopyBufferDesc
	{
		// Width, height and aspect mask is presumed to be the same values from the texture image
		uint64			BufferOffset		= 0;
		uint32			BufferRowLengh		= 0;
		uint32			BufferImageHeight	= 0;
		uint32			MipLevel			= 0;
		uint32			ArrayLayer			= 0;
		uint32			ArrayCount			= 0;
		uint32			ImageOffsetX		= 0;
		uint32			ImageOffsetY		= 0;
		uint32			ImageOffsetZ		= 0;
		uint32			Width				= 0;
		uint32			Height				= 0;
		uint32			Depth				= 0;
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
		virtual void BeginRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, uint32 width, uint32 height, float* pClearColor, uint32 clearColorCount) = 0;

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
		virtual void BindDescriptor(Pipeline* pPipeline, Descriptor* pDescriptor) = 0;

		/**
		 * Copy buffer to a texture
		 * @param pBuffer - Buffer to copy from
		 * @param pTexture - Texture to copy to
		 * @param layout - Layout of destination texture
		 * @param copyBufferDesc - Description of the copy details
		 */
		virtual void CopyBufferToTexture(Buffer* pBuffer, Texture* pTexture, ETextureLayout layout, const CopyBufferDesc& copyBufferDesc) = 0;

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
		 * End the render pass
		 */
		virtual void EndRenderPass() = 0;

		/**
		 * End the recording of commands
		 */
		virtual void End() = 0;

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