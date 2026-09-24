#include "Renderer.h"

#include "Platform/API/CommandQueue.h"
#include "Platform/API/SwapChain.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/RenderGraph/RenderProgramInstance.h"
#include "Poly/RenderGraph/RenderView.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "polypch.h"

namespace Poly
{
	constexpr const uint32 BUFFER_COUNT = 3;

	Renderer::Renderer() {}

	Renderer::~Renderer()
	{
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
	}

	Unique<Renderer> Renderer::Create()
	{
		return CreateUnique<Renderer>();
	}

	void Renderer::Submit(const RenderRequest& request)
	{
		m_FrameRequests.push_back(request);
	}

	void Renderer::SetRenderProgram(Ref<RenderProgram> pRenderProgram)
	{
		m_pQueuedRenderProgram = std::move(pRenderProgram);
		SwapRenderProgramIfQueued();
	}

	RenderProgramInstance* Renderer::GetRenderProgramInstance(Window* pWindow) const
	{
		for (const WindowContext& windowCtx : m_Windows)
		{
			if (!pWindow || windowCtx.pWindow == pWindow)
				return windowCtx.pRenderProgramInstance.get();
		}

		return nullptr;
	}

	void Renderer::AddWindow(Window* pWindow)
	{
		SwapChainDesc swapChainDesc = {
		    .pWindow     = pWindow,
		    .pQueue      = RenderAPI::GetCommandQueue(FQueueType::GRAPHICS),
		    .Width       = pWindow->GetWidth(),
		    .Height      = pWindow->GetHeight(),
		    .BufferCount = BUFFER_COUNT,
		    .Format      = EFormat::B8G8R8A8_UNORM};
		Ref<SwapChain> pSwapChain = RenderAPI::CreateSwapChain(&swapChainDesc);

		WindowContext context{pWindow, pSwapChain};
		if (m_pActiveRenderProgram)
			context.pRenderProgramInstance = CreateUnique<RenderProgramInstance>(m_pActiveRenderProgram);

		m_Windows.emplace_back(std::move(context));
	}

	void Renderer::RemoveWindow(Window* pWindow)
	{
		std::erase_if(m_Windows, [pWindow](const WindowContext& windowCtx) { return windowCtx.pWindow == pWindow; });
	}

	void Renderer::Render()
	{
		ResourceManager::Update();

		for (const WindowContext& windowCtx : m_Windows)
		{
			if (windowCtx.pRenderProgramInstance)
			{
				const RenderRequest* pRequest = FindRequest(windowCtx.pWindow);
				RenderView           view{.pGlobalResources = &m_GlobalResources,
				                          .pWorld           = pRequest ? pRequest->pWorld : nullptr,
				                          .pViewResources   = pRequest ? pRequest->pViewResources : nullptr,
				                          .pTarget          = windowCtx.pSwapChain.get()->GetTextureView(windowCtx.pSwapChain->GetBackbufferIndex()).get()};
				windowCtx.pRenderProgramInstance->Execute(view, windowCtx.pSwapChain->GetAcquireSemaphore());
			}

			std::vector<CommandBuffer*> emptyCommandbuffers;
			windowCtx.pSwapChain->Present(emptyCommandbuffers);
		}

		m_FrameRequests.clear();
	}

	void Renderer::OnEvent(Event& event)
	{
		EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Events::WindowResized>([this](Events::WindowResized& event) {
			for (auto& context : m_Windows)
			{
				context.pSwapChain->OnWindowResized(event.GetWidth(), event.GetHeight());
			}

			return false;
		});
	}

	const RenderRequest* Renderer::FindRequest(const Window* pWindow) const
	{
		const Window* pDefaultWindow = m_Windows.empty() ? nullptr : m_Windows.front().pWindow;

		for (auto it = m_FrameRequests.rbegin(); it != m_FrameRequests.rend(); ++it)
		{
			const Window* pTargetWindow = it->pWindow ? it->pWindow : pDefaultWindow;
			if (pTargetWindow == pWindow)
				return &(*it);
		}

		return nullptr;
	}

	void Renderer::SwapRenderProgramIfQueued()
	{
		if (!m_pQueuedRenderProgram)
			return;

		m_pActiveRenderProgram = std::move(m_pQueuedRenderProgram);
		m_pQueuedRenderProgram.reset();

		for (WindowContext& windowCtx : m_Windows)
			windowCtx.pRenderProgramInstance = CreateUnique<RenderProgramInstance>(m_pActiveRenderProgram);
	}
} // namespace Poly