#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class TextureView;
	class GraphicsRenderPass;

	struct FramebufferDesc
	{
		uint32				Width			= 0;
		uint32				Height			= 0;
		GraphicsRenderPass*	pRenderPass		= nullptr;
		TextureView*		pTextureView	= nullptr;
	};

	class Framebuffer
	{
	public:
		CLASS_ABSTRACT(Framebuffer);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const FramebufferDesc* pDesc) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

		/**
		 * @return const BufferDesc
		 */
		inline const FramebufferDesc& GetDesc() const
		{
			return p_FramebufferDesc;
		}

		/**
		 * @return width of framebuffer
		 */
		uint32 GetWidth() const { p_FramebufferDesc.Width; }

		/**
		 * @return height of framebuffer
		 */
		uint32 GetHeight() const { p_FramebufferDesc.Height; }

	protected:
		FramebufferDesc p_FramebufferDesc;
	};
}