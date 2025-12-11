#include "polypch.h"

#include "Renderer.h"
#include "Poly/Core/Window.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/SwapChain.h"
#include "Platform/API/CommandQueue.h"
#include "RenderGraph/RenderGraphProgram.h"
#include "RenderGraph/Resource.h"

namespace Poly
{
	constexpr const uint32 BUFFER_COUNT = 3;

	Renderer::Renderer() {}

	Renderer::~Renderer()
	{
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
	}

	Ref<Renderer> Renderer::Create()
	{
		return CreateRef<Renderer>();
	}

	void Renderer::SetRenderGraph(Ref<RenderGraphProgram> pRenderGraphProgram)
	{
		m_pRenderGraphProgram = pRenderGraphProgram;

		for (const WindowContext& windowCtx : m_Windows)
		{
			CreateBackbufferResources(windowCtx);
		}
	}

	void Renderer::AddWindow(Window* pWindow)
	{
		SwapChainDesc swapChainDesc = {
			.pWindow		= pWindow,
			.pQueue			= RenderAPI::GetCommandQueue(FQueueType::GRAPHICS),
			.Width			= pWindow->GetWidth(),
			.Height			= pWindow->GetHeight(),
			.BufferCount	= BUFFER_COUNT,
			.Format			= EFormat::B8G8R8A8_UNORM
		};
		Ref<SwapChain> pSwapChain = RenderAPI::CreateSwapChain(&swapChainDesc);

		WindowContext context{ pWindow, pSwapChain };
		m_Windows.emplace_back(context);
	}

	void Renderer::RemoveWindow(Window* pWindow)
	{
		std::erase_if(m_Windows, [pWindow](const WindowContext& windowCtx) { return windowCtx.pWindow == pWindow; });
	}

	void Renderer::Render()
	{
		for (const WindowContext& windowCtx : m_Windows)
		{
			m_pRenderGraphProgram->Execute(windowCtx.pWindow->GetID(), windowCtx.pSwapChain->GetBackbufferIndex());

			std::vector<CommandBuffer*> emptyCommandbuffers;
			PresentResult res = windowCtx.pSwapChain->Present(emptyCommandbuffers, nullptr);
			if (res == PresentResult::RECREATED_SWAPCHAIN)
				CreateBackbufferResources(windowCtx);
		}
	}

	void Renderer::CreateBackbufferResources(const WindowContext& windowCtx)
	{
		for (uint32 i = 0; i < BUFFER_COUNT; i++)
		{
			std::string name = "Backbuffer " + std::to_string(i);
			m_pRenderGraphProgram->SetBackbuffer(windowCtx.pWindow->GetID(), i, Resource::Create(windowCtx.pSwapChain->GetTexture(i), windowCtx.pSwapChain->GetTextureView(i), name));
		}

		m_pRenderGraphProgram->RecreateResources(windowCtx.pWindow->GetWidth(), windowCtx.pWindow->GetHeight());
	}
}