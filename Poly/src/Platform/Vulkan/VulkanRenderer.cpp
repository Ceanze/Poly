#include "polypch.h"
#include "VulkanRenderer.h"

namespace Poly
{

	void VulkanRenderer::initialize(unsigned width, unsigned height)
	{
		this->window = new Window(width, height, "Poly");

		this->instance.init(this->window);
		this->swapChain.init(&this->instance, this->window);
		this->shader.init(&this->instance);
		this->shader.addStage(PVKShader::Type::VERTEX, "vert.spv");
		this->shader.addStage(PVKShader::Type::FRAGMENT, "frag.spv");
		this->renderPass.init(&this->instance, &this->swapChain);
		this->pipeline.init(&this->instance, &this->swapChain, &this->shader, &this->renderPass);
		this->commandPool.init(&this->instance, VK_QUEUE_GRAPHICS_BIT);
		
		this->framebuffers.resize(this->swapChain.getNumImages());
		for (size_t i = 0; i < this->swapChain.getNumImages(); i++)
			this->framebuffers[i].init(&this->instance, &this->swapChain, &this->renderPass, this->swapChain.getImageViews()[i]);
	}

	void VulkanRenderer::setWinTitle(const char* title)
	{
	}

	void VulkanRenderer::shutdown()
	{
		this->commandPool.cleanup();
		PVK_VEC_CLEANUP(this->framebuffers);
		this->pipeline.cleanup();
		this->renderPass.cleanup();
		this->shader.cleanup();
		this->swapChain.cleanup();
		this->instance.cleanup();
		delete this->window;
	}

	void VulkanRenderer::setClearColor(float r, float g, float b, float a)
	{
	}

	void VulkanRenderer::clearBuffer(unsigned int)
	{
	}

}