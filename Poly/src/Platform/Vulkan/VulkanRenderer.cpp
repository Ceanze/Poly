#include "polypch.h"
#include "VulkanRenderer.h"

namespace Poly
{

	void VulkanRenderer::initialize(unsigned width, unsigned height)
	{
		this->window = new Window(width, height, "Poly");

		this->instance.init();
		this->swapChain.init(&this->instance, this->window);
	}

	void VulkanRenderer::setWinTitle(const char* title)
	{
	}

	void VulkanRenderer::shutdown()
	{
		this->swapChain.cleanup();
		this->instance.cleanup();
	}

	void VulkanRenderer::setClearColor(float r, float g, float b, float a)
	{
	}

	void VulkanRenderer::clearBuffer(unsigned int)
	{
	}

}