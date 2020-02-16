#include "polypch.h"
#include "PVKFramebuffer.h"
#include "PVKSwapChain.h"
#include "PVKInstance.h"
#include "PVKRenderPass.h"

namespace Poly
{

	PVKFramebuffer::PVKFramebuffer() : framebuffer(VK_NULL_HANDLE), instance(nullptr)
	{
	}

	PVKFramebuffer::~PVKFramebuffer()
	{
	}

	void PVKFramebuffer::init(PVKInstance* instance, PVKSwapChain* swapChain, PVKRenderPass* renderPass, VkImageView attachment)
	{
		VkExtent2D extent = swapChain->getExtent();
		this->instance = instance;

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass->getRenderPass();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &attachment;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		PVK_CHECK(vkCreateFramebuffer(this->instance->getDevice(), &framebufferInfo, nullptr, &this->framebuffer), "Failed to create framebuffer!");
	}

	void PVKFramebuffer::cleanup()
	{
		vkDestroyFramebuffer(this->instance->getDevice(), this->framebuffer, nullptr);
	}

}