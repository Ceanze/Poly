#include "polypch.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace Poly
{
	IPlatformRenderer* RendererAPI::s_pRenderer = nullptr;
	BackendAPI RendererAPI::s_API = BackendAPI::NONE;

	void RendererAPI::Create(BackendAPI backend)
	{
		s_API = backend;
		switch (backend)
		{
		case BackendAPI::VULKAN:
			s_pRenderer = new VulkanRenderer();
			break;
		default:
			POLY_CORE_FATAL("Only Vulkan renderer is currently supported!");
		}
	}

	void RendererAPI::CreateRenderer(Renderer subRenderer)
	{
		s_pRenderer->CreateRenderer(subRenderer);
	}

	void RendererAPI::SetActiveCamera(Camera* pCamera)
	{
		s_pRenderer->SetActiveCamera(pCamera);
	}

	void RendererAPI::Init(uint32_t width, uint32_t height)
	{
		s_pRenderer->Init(width, height);
	}

	void RendererAPI::BeginScene()
	{
		s_pRenderer->BeginScene();
	}

	void RendererAPI::EndScene()
	{
		s_pRenderer->EndScene();
	}

	void RendererAPI::SetWinTitle(const char* title)
	{
		//s_pRenderer->setWinTitle(title);
	}

	void RendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		//s_pRenderer->setClearColor(r, g, b, a);
	}

	void RendererAPI::Shutdown()
	{
		delete s_pRenderer;
	}

	BackendAPI RendererAPI::GetAPI()
	{
		return s_API;
	}

}