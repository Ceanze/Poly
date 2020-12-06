#include "polypch.h"
#include "PVKFramebuffer.h"
#include "PVKSwapChain.h"
#include "PVKInstance.h"
#include "PVKRenderPass.h"

namespace Poly
{

	PVKFramebuffer::PVKFramebuffer()
	{
	}

	PVKFramebuffer::~PVKFramebuffer()
	{
	}

	void PVKFramebuffer::Init(PVKSwapChain& swapChain, PVKRenderPass& renderPass, VkImageView attachment)
	{
		VkExtent2D extent = swapChain.GetExtent();

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass.GetNative();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &attachment;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		PVK_CHECK(vkCreateFramebuffer(PVKInstance::GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer), "Failed to create framebuffer!");
	}

	void PVKFramebuffer::Cleanup()
	{
		vkDestroyFramebuffer(PVKInstance::GetDevice(), m_Framebuffer, nullptr);
	}

}