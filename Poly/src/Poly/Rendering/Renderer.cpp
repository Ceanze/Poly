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

	Renderer::Renderer(Window* pWindow)
		: m_pWindow(pWindow)
	{
		SwapChainDesc swapChainDesc = {
			.pWindow		= pWindow,
			.pQueue			= RenderAPI::GetCommandQueue(FQueueType::GRAPHICS),
			.Width			= pWindow->GetWidth(),
			.Height			= pWindow->GetHeight(),
			.BufferCount	= BUFFER_COUNT,
			.Format			= EFormat::B8G8R8A8_UNORM
		};
		m_pSwapChain = RenderAPI::CreateSwapChain(&swapChainDesc);

		CreateBackbufferResources();
	}

	Renderer::~Renderer()
	{
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
	}

	Ref<Renderer> Renderer::Create(Window* pWindow)
	{
		return CreateRef<Renderer>(pWindow);
	}

	void Renderer::SetRenderGraph(Ref<RenderGraphProgram> pRenderGraphProgram)
	{
		m_pRenderGraphProgram = pRenderGraphProgram;
		m_pRenderGraphProgram->RecreateResources(m_pWindow->GetWidth(), m_pWindow->GetHeight());
	}

	void Renderer::Render()
	{
		m_pRenderGraphProgram->SetBackbuffer(m_BackbufferResources[m_pSwapChain->GetBackbufferIndex()]);
		m_pRenderGraphProgram->Execute(m_pSwapChain->GetBackbufferIndex());

		std::vector<CommandBuffer*> emptyCommandbuffers;
		PresentResult res = m_pSwapChain->Present(emptyCommandbuffers, nullptr);
		if (res == PresentResult::RECREATED_SWAPCHAIN)
			CreateBackbufferResources();
	}

	void Renderer::CreateBackbufferResources()
	{
		m_BackbufferResources.clear();
		m_BackbufferResources.reserve(BUFFER_COUNT);
		for (uint32 i = 0; i < BUFFER_COUNT; i++)
		{
			std::string name = "Backbuffer " + std::to_string(i);
			m_BackbufferResources.push_back(Resource::Create(m_pSwapChain->GetTexture(i), m_pSwapChain->GetTextureView(i), name));
		}

		if (m_pRenderGraphProgram)
			m_pRenderGraphProgram->RecreateResources(m_pWindow->GetWidth(), m_pWindow->GetHeight());
	}
}