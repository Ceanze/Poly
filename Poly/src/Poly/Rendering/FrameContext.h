#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Core/PolyID.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Texture;
	class TextureView;
	class CommandBuffer;

	class FrameContext
	{
	public:
		CLASS_ABSTRACT(FrameContext);

		virtual PresentResult    Present(const std::vector<CommandBuffer*>& commandBuffers) = 0;
		virtual Ref<Texture>     GetTexture(uint32 bufferIndex) const                       = 0;
		virtual Ref<TextureView> GetTextureView(uint32 bufferIndex) const                   = 0;
		virtual uint32           GetBackbufferIndex() const                                 = 0;
		virtual uint32           GetBackbufferCount() const                                 = 0;
		virtual void             OnWindowResized(int width, int height)                     = 0;
		virtual PolyID           GetID() const                                              = 0;
		virtual uint32           GetWidth() const                                           = 0;
		virtual uint32           GetHeight() const                                          = 0;
	};
} // namespace Poly
