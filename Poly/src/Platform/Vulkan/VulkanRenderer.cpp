#include "polypch.h"
#include "VulkanRenderer.h"

#include "Poly/Core/Window.h"
#include "Platform/Vulkan/Renderers/TestRenderer.h"

#include "Poly/Core/RenderAPI.h"

#include "Platform/API/SwapChain.h"
#include "Platform/API/CommandQueue.h"

namespace Poly
{
	VulkanRenderer::~VulkanRenderer()
	{
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();

		// Cleanup sync objects
		// for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); i++) {
		// 	vkDestroySemaphore(PVKInstance::GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
		// 	vkDestroySemaphore(PVKInstance::GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
		// 	vkDestroyFence(PVKInstance::GetDevice(), m_InFlightFences[i], nullptr);
		// }

		// Cleanup the rest
		// m_SwapChain.Cleanup();
		// TODO: Should be done in RenderAPI
		//PVKInstance::Get()->Cleanup();

		// delete m_pWindow;
		for (uint32_t i = 0; i < m_SubRenderers.size(); i++)
			delete m_SubRenderers[i];

		// RenderAPI::Release();
	}

	void VulkanRenderer::Init(uint32_t width, uint32_t height)
	{
		// Call init on the sub renderers and create window
		// m_pWindow = new Window(width, height, "Vulkan renderer window");
		// RenderAPI::Init(RenderAPI::BackendAPI::VULKAN, m_pWindow);
		m_pWindow = RenderAPI::GetWindow();

		SwapChainDesc swapChainDesc = {
			.pWindow		= m_pWindow,
			.pQueue			= RenderAPI::GetCommandQueue(FQueueType::GRAPHICS),
			.Width			= width,
			.Height			= height,
			.BufferCount	= 3,
			.Format			= EFormat::B8G8R8A8_UNORM
		};
		m_SwapChain = RenderAPI::CreateSwapChain(&swapChainDesc);

		// Test renderer
		for (auto& subRenderer : m_SubRenderers) {
			subRenderer->SetWindow(m_pWindow);
			subRenderer->SetActiveCamera(m_pCamera);
			subRenderer->Init(this);
		}
	}

	void VulkanRenderer::BeginScene()
	{
		// Call all subrenderers beginScene
		for (auto& subRenderer : m_SubRenderers)
			subRenderer->BeginScene(m_SwapChain->GetBackbufferIndex());
	}

	void VulkanRenderer::Draw(Model* model)
	{
		// Call the subRenderers record function
		for (auto& subRenderer : m_SubRenderers)
			subRenderer->Record();
	}

	void VulkanRenderer::EndScene()
	{
		for (auto& subRenderer : m_SubRenderers)
			subRenderer->EndScene();

		m_SwapChain->Present(m_GraphicsBuffers, nullptr);

		// Clear buffers for next frame
		m_GraphicsBuffers.clear();
	}

	void VulkanRenderer::CreateRenderer(Renderer subRenderer)
	{
		switch (subRenderer)
		{
		case Poly::Renderer::TEST:
			m_SubRenderers.push_back(new TestRenderer);
			break;
		case Poly::Renderer::MESH:
		default:
			POLY_CORE_ERROR("Only test renderer is currently supported for VulkanRenderer");
			break;
		}
	}

	void VulkanRenderer::AddCommandBuffer(FQueueType queueType, CommandBuffer* pBuffer)
	{
		std::vector<CommandBuffer*>& buffers = m_GraphicsBuffers;
		switch (queueType)
		{
		case Poly::FQueueType::GRAPHICS:
			break;
		case Poly::FQueueType::COMPUTE:
			// buffers = this->computeBuffers;
			break;
		case Poly::FQueueType::TRANSFER:
			// buffers = this->transferBuffers;
			break;
		default:
			POLY_CORE_ERROR("Only graphics, compute and transfer command buffers are allowed!");
			break;
		}

		buffers.push_back(pBuffer);
	}

}