#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class RenderPass;
	class TextureView;

	struct FramebufferDesc
	{
		uint32			Width			= 0;
		uint32			Height			= 0;
		RenderPass*		pRenderPass		= nullptr;
		TextureView*	pTextureView	= nullptr;
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

	protected:
		FramebufferDesc p_FramebufferDesc;
	};
}