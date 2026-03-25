#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Window;
	class Texture;
	class TextureView;
	class CommandQueue;
	class CommandBuffer;

	struct SwapChainDesc
	{
		Window*       pWindow     = nullptr;
		CommandQueue* pQueue      = nullptr;
		uint32        Width       = 0;
		uint32        Height      = 0;
		uint32        BufferCount = 0;
		EFormat       Format      = EFormat::UNDEFINED;
	};

	enum class PresentResult
	{
		// Normal, expected result
		SUCCESS,

		// Swapchain failed to present for unknown reason (validaton layer error should clarify)
		FAIL,

		// Swapchain has been recreated, probably due to a resize of the window
		RECREATED_SWAPCHAIN
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
		 * Present the current buffer to the surface
		 * @param commandBufers - (optional) Additional buffers to submit before presentation
		 */
		virtual PresentResult Present(const std::vector<CommandBuffer*>& commandBuffers) = 0;

		/**
		 * Get the texture
		 * @return Texture pointer of image buffers
		 */
		virtual Ref<Texture> GetTexture(uint32 bufferIndex) const = 0;

		/**
		 * Get the texture view
		 * @return Texture pointer of image view buffers
		 */
		virtual Ref<TextureView> GetTextureView(uint32 bufferIndex) const = 0;

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
		 * Called when the window has been resized and requires updated backbuffer images
		 * @param width - new width
		 * @param height - new height
		 */
		virtual void OnWindowResized(int width, int height) = 0;

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
} // namespace Poly