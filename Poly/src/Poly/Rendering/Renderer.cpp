#include "polypch.h"
#include "Renderer.h"
#include "Poly/Core/Window.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/SwapChain.h"
#include "Platform/API/CommandQueue.h"
#include "RenderGraph/RenderGraphProgram.h"

namespace Poly
{
	constexpr const uint32 BUFFER_COUNT = 3;

	Renderer::Renderer()
	{
		Window* pWindow = RenderAPI::GetWindow();

		SwapChainDesc swapChainDesc = {
			.pWindow		= pWindow,
			.pQueue			= RenderAPI::GetCommandQueue(FQueueType::GRAPHICS),
			.Width			= pWindow->GetWidth(),
			.Height			= pWindow->GetHeight(),
			.BufferCount	= BUFFER_COUNT,
			.Format			= EFormat::B8G8R8A8_UNORM
		};
		m_pSwapChain = RenderAPI::CreateSwapChain(&swapChainDesc);
	}

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
	}

	void Renderer::Render()
	{
		m_pRenderGraphProgram->Execute(m_pSwapChain->GetBackbufferIndex());

		std::vector<CommandBuffer*> emptyCommandbuffers;
		m_pSwapChain->Present(emptyCommandbuffers, nullptr);
	}
}