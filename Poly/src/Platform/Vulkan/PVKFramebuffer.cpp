#include "polypch.h"
#include "PVKFramebuffer.h"
#include "PVKSwapChain.h"
#include "PVKInstance.h"
#include "PVKRenderPass.h"

namespace Poly
{

	PVKFramebuffer::PVKFramebuffer() : framebuffer(VK_NULL_HANDLE)
	{
	}

	PVKFramebuffer::~PVKFramebuffer()
	{
	}

	void PVKFramebuffer::init(PVKSwapChain& swapChain, PVKRenderPass& renderPass, VkImageView attachment)
	{
		VkExtent2D extent = swapChain.getExtent();

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass.getNative();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &attachment;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		PVK_CHECK(vkCreateFramebuffer(PVKInstance::getDevice(), &framebufferInfo, nullptr, &this->framebuffer), "Failed to create framebuffer!");
	}

	void PVKFramebuffer::cleanup()
	{
		vkDestroyFramebuffer(PVKInstance::getDevice(), this->framebuffer, nullptr);
	}

}