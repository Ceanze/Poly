#include "polypch.h"
#include "RendererAPI.h"
#include "Renderer.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace Poly
{
	Renderer* RendererAPI::renderer = nullptr;

	void RendererAPI::create(BACKEND backend)
	{
		renderer = Renderer::create((Renderer::BACKEND)backend);
	}

	void RendererAPI::finalize()
	{
		// Add finalize here
	}

	void RendererAPI::initialize(unsigned int width, unsigned int height)
	{
		renderer->initialize(width, height);
	}

	void RendererAPI::render()
	{
		renderer->render();
	}

	void RendererAPI::setWinTitle(const char* title)
	{
		renderer->setWinTitle(title);
	}

	void RendererAPI::setClearColor(float r, float g, float b, float a)
	{
		renderer->setClearColor(r, g, b, a);
	}

	void RendererAPI::shutdown()
	{
		renderer->shutdown();
		delete renderer;
	}

}