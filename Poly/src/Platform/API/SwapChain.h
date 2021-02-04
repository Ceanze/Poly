#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Window;
	class Texture;
	class Semaphore;
	class TextureView;
	class CommandQueue;
	class CommandBuffer;

	struct SwapChainDesc
	{
		Window*			pWindow		= nullptr;
		CommandQueue*	pQueue		= nullptr;
		uint32			Width		= 0;
		uint32			Height		= 0;
		uint32			BufferCount	= 0;
		EFormat			Format		= EFormat::UNDEFINED;

	};

	class SwapChain
	{
	public:
		CLASS_ABSTRACT(SwapChain);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const SwapChainDesc* pDesc) = 0;

		/**
		 * Resize the buffers of the swapchain.
		 * @param width		- new width of buffers
		 * @param height	- new height of buffers
		 */
		virtual void Resize(uint32 width, uint32 height) = 0;

		/**
		 * Present the current buffer to the surface
		 * @param commandBufers - (optional) Additional buffers to submit before presentation
		 * @param pWaitSemaphore - (optional) Semaphore to wait on before present
		 */
		virtual void Present(std::vector<CommandBuffer*> commandBuffers, Semaphore* pWaitSemaphore) = 0;

		/**
		 * Get the texture
		 * @return Texture pointer of image buffers
		 */
		virtual Texture* GetTexture(uint32 bufferIndex) const = 0;

		/**
		 * Get the texture view
		 * @return Texture pointer of image view buffers
		 */
		virtual TextureView* GetTextureView(uint32 bufferIndex) const = 0;

		/**
		 * Get the current backbuffer index
		 * @return backbuffer index
		 */
		virtual uint32 GetBackbufferIndex() const = 0;

		/**
		 * Get the count of backbuffers
		 * @return backbuffer count
		 */
		virtual uint32 GetBackbufferCount() const = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

		/**
		 * @return const TextureDesc
		 */
		inline const SwapChainDesc& GetDesc() const
		{
			return p_SwapchainDesc;
		}

	protected:
		SwapChainDesc p_SwapchainDesc;
	};
}