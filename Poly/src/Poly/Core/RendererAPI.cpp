#include "polypch.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace Poly
{
	IPlatformRenderer* RendererAPI::renderer = nullptr;

	void RendererAPI::create(BACKEND backend)
	{
		switch (backend)
		{
		case BACKEND::VULKAN:
			renderer =  new VulkanRenderer();
			break;
		default:
			POLY_CORE_FATAL("Only Vulkan renderer is currently supported!");
		}
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