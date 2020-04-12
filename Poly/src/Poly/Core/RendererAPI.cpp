#include "polypch.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace Poly
{
	IPlatformRenderer* RendererAPI::renderer = nullptr;

	void RendererAPI::create(BackendAPI backend)
	{
		switch (backend)
		{
		case BackendAPI::VULKAN:
			renderer = new VulkanRenderer();
			break;
		default:
			POLY_CORE_FATAL("Only Vulkan renderer is currently supported!");
		}
	}

	void RendererAPI::createRenderer(Renderer subRenderer)
	{
		renderer->createRenderer(subRenderer);
	}

	void RendererAPI::setActiveCamera(Camera* camera)
	{
		renderer->setActiveCamera(camera);
	}

	void RendererAPI::init(uint32_t width, uint32_t height)
	{
		renderer->init(width, height);
	}

	void RendererAPI::beginScene()
	{
		renderer->beginScene();
	}

	void RendererAPI::endScene()
	{
		renderer->endScene();
	}

	void RendererAPI::setWinTitle(const char* title)
	{
		//renderer->setWinTitle(title);
	}

	void RendererAPI::setClearColor(float r, float g, float b, float a)
	{
		//renderer->setClearColor(r, g, b, a);
	}

	void RendererAPI::shutdown()
	{
		renderer->shutdown();
		delete renderer;
	}

}