#include "WindowFrameContext.h"

#include "Platform/API/SwapChain.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly/Core/Window.h"
#include "polypch.h"

namespace Poly
{
	WindowFrameContext::WindowFrameContext(Window* pWindow, Ref<SwapChain> pSwapChain)
	    : m_pWindow(pWindow)
	    , m_pSwapChain(std::move(pSwapChain))
	{}

	WindowFrameContext::~WindowFrameContext() = default;

	PresentResult WindowFrameContext::Present(const std::vector<CommandBuffer*>& commandBuffers)
	{
		return m_pSwapChain->Present(commandBuffers);
	}

	Ref<Texture> WindowFrameContext::GetTexture(uint32 bufferIndex) const
	{
		return m_pSwapChain->GetTexture(bufferIndex);
	}

	Ref<TextureView> WindowFrameContext::GetTextureView(uint32 bufferIndex) const
	{
		return m_pSwapChain->GetTextureView(bufferIndex);
	}

	uint32 WindowFrameContext::GetBackbufferIndex() const
	{
		return m_pSwapChain->GetBackbufferIndex();
	}

	uint32 WindowFrameContext::GetBackbufferCount() const
	{
		return m_pSwapChain->GetBackbufferCount();
	}

	void WindowFrameContext::OnWindowResized(int width, int height)
	{
		m_pSwapChain->OnWindowResized(width, height);
	}

	PolyID WindowFrameContext::GetID() const
	{
		return m_pWindow->GetID();
	}

	uint32 WindowFrameContext::GetWidth() const
	{
		return static_cast<uint32>(m_pWindow->GetWidth());
	}

	uint32 WindowFrameContext::GetHeight() const
	{
		return static_cast<uint32>(m_pWindow->GetHeight());
	}
} // namespace Poly
