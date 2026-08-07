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

	Unique<Renderer> Renderer::Create()
	{
		return CreateUnique<Renderer>();
	}

	void Renderer::SetScene(Ref<Scene> pScene)
	{
		m_pScene = pScene;
	}

	void Renderer::SetRenderGraph(Ref<RenderGraphProgram> pRenderGraphProgram)
	{
		m_pRenderGraphProgram = pRenderGraphProgram;

		for (const WindowContext& windowCtx : m_Windows)
		{
			CreateBackbufferResources(windowCtx);
		}
	}

	void Renderer::SetRenderProgram(Ref<RenderProgram> pRenderProgram)
	{
		// TODO: Handle render program init in a clearer way
		// This is done so now so that UpdateResource can be called in the program instance instead of after a Render call
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
			if (m_pRenderGraphProgram)
				m_pRenderGraphProgram->Execute(windowCtx.pWindow->GetID(), windowCtx.pSwapChain->GetBackbufferIndex());

			if (windowCtx.pRenderProgramInstance)
			{
				RenderView view{.pScene  = m_pScene.get(),
				                .pTarget = windowCtx.pSwapChain.get()->GetTextureView(windowCtx.pSwapChain->GetBackbufferIndex()).get()};
				windowCtx.pRenderProgramInstance->Execute(view);
			}

			std::vector<CommandBuffer*> emptyCommandbuffers;
			PresentResult               res = windowCtx.pSwapChain->Present(emptyCommandbuffers);
			if (res == PresentResult::RECREATED_SWAPCHAIN)
				CreateBackbufferResources(windowCtx);
		}
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

	void Renderer::CreateBackbufferResources(const WindowContext& windowCtx)
	{
		// The old (RG1) render graph program is optional - an app using only the new RenderProgram
		// pipeline (see RenderProgram/RenderProgramInstance) never calls SetRenderGraph().
		if (!m_pRenderGraphProgram)
			return;

		for (uint32 i = 0; i < BUFFER_COUNT; i++)
		{
			std::string name = "Backbuffer " + std::to_string(i);
			m_pRenderGraphProgram->SetBackbuffer(windowCtx.pWindow->GetID(), i, Resource::Create(windowCtx.pSwapChain->GetTexture(i), windowCtx.pSwapChain->GetTextureView(i), name));
		}

		m_pRenderGraphProgram->RecreateResources(windowCtx.pWindow->GetWidth(), windowCtx.pWindow->GetHeight());
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