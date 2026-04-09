#include "Renderer.h"

#include "Platform/API/CommandQueue.h"
#include "Platform/API/SwapChain.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/Rendering/HeadlessFrameContext.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Rendering/RenderGraph/Resource.h"
#include "Poly/Rendering/WindowFrameContext.h"
#include "polypch.h"

namespace Poly
{
	constexpr uint32 BUFFER_COUNT          = 3;
	constexpr uint32 HEADLESS_BUFFER_COUNT = 2;

	Renderer::Renderer() {}

	Renderer::~Renderer()
	{
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
	}

	Unique<Renderer> Renderer::Create()
	{
		return CreateUnique<Renderer>();
	}

	void Renderer::SetRenderGraph(Ref<RenderGraphProgram> pRenderGraphProgram)
	{
		m_pRenderGraphProgram = pRenderGraphProgram;

		for (const Ref<FrameContext>& ctx : m_FrameContexts)
			CreateBackbufferResources(*ctx);
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

		m_FrameContexts.emplace_back(CreateRef<WindowFrameContext>(pWindow, pSwapChain));
	}

	void Renderer::RemoveWindow(Window* pWindow)
	{
		std::erase_if(m_FrameContexts, [pWindow](const Ref<FrameContext>& ctx) {
			return ctx->GetID() == pWindow->GetID();
		});
	}

	void Renderer::Render()
	{
		if (!m_pRenderGraphProgram)
			return;

		for (const Ref<FrameContext>& ctx : m_FrameContexts)
		{
			m_pRenderGraphProgram->Execute(ctx->GetID(), ctx->GetBackbufferIndex());

			std::vector<CommandBuffer*> emptyCommandBuffers;
			PresentResult               res = ctx->Present(emptyCommandBuffers);
			if (res == PresentResult::RECREATED_SWAPCHAIN)
				CreateBackbufferResources(*ctx);
		}
	}

	void Renderer::OnEvent(Event& event)
	{
		EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Events::WindowResized>([this](Events::WindowResized& event) {
			for (auto& ctx : m_FrameContexts)
				ctx->OnWindowResized(event.GetWidth(), event.GetHeight());

			return false;
		});
	}

	// TODO: Move to be enabled in construction, and have it be a application setting - it cannot change during runtime
	void Renderer::EnableHeadless(uint32 width, uint32 height)
	{
		HeadlessFrameContextDesc desc = {
		    .pQueue      = RenderAPI::GetCommandQueue(FQueueType::GRAPHICS),
		    .Width       = width,
		    .Height      = height,
		    .BufferCount = HEADLESS_BUFFER_COUNT,
		    .Format      = EFormat::B8G8R8A8_UNORM};

		auto pCtx = CreateRef<HeadlessFrameContext>(desc);
		m_FrameContexts.emplace_back(pCtx);

		if (m_pRenderGraphProgram)
			CreateBackbufferResources(*pCtx);
	}

	// TODO: Move to be enabled in construction, and have it be a application setting - it cannot change during runtime
	void Renderer::DisableHeadless()
	{
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
		std::erase_if(m_FrameContexts, [](const Ref<FrameContext>& ctx) {
			return dynamic_cast<HeadlessFrameContext*>(ctx.get()) != nullptr;
		});
	}

	Ref<Texture> Renderer::GetHeadlessTexture(uint32 index) const
	{
		for (const Ref<FrameContext>& ctx : m_FrameContexts)
		{
			if (auto* pHeadless = dynamic_cast<HeadlessFrameContext*>(ctx.get()))
				return pHeadless->GetTexture(index);
		}
		return nullptr;
	}

	void Renderer::CreateBackbufferResources(FrameContext& ctx)
	{
		for (uint32 i = 0; i < ctx.GetBackbufferCount(); i++)
		{
			std::string name = "Backbuffer " + std::to_string(i);
			m_pRenderGraphProgram->SetBackbuffer(ctx.GetID(), i, Resource::Create(ctx.GetTexture(i), ctx.GetTextureView(i), name));
		}

		m_pRenderGraphProgram->RecreateResources(ctx.GetWidth(), ctx.GetHeight());
	}
} // namespace Poly
