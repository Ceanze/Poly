#pragma once

#include "FrameContext.h"

namespace Poly
{
	class Window;
	class SwapChain;

	class WindowFrameContext : public FrameContext
	{
	public:
		CLASS_REMOVE_COPY(WindowFrameContext);
		CLASS_REMOVE_MOVE(WindowFrameContext);

		WindowFrameContext(Window* pWindow, Ref<SwapChain> pSwapChain);
		~WindowFrameContext();

		PresentResult    Present(const std::vector<CommandBuffer*>& commandBuffers) override;
		Ref<Texture>     GetTexture(uint32 bufferIndex) const override;
		Ref<TextureView> GetTextureView(uint32 bufferIndex) const override;
		uint32           GetBackbufferIndex() const override;
		uint32           GetBackbufferCount() const override;
		void             OnWindowResized(int width, int height) override;
		PolyID           GetID() const override;
		uint32           GetWidth() const override;
		uint32           GetHeight() const override;

	private:
		Window*        m_pWindow;
		Ref<SwapChain> m_pSwapChain;
	};
} // namespace Poly
