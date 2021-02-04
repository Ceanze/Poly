#include "polypch.h"
#include "PVKFramebuffer.h"

#include "PVKInstance.h"
#include "PVKSwapChain.h"
#include "PVKRenderPass.h"
#include "PVKTextureView.h"

namespace Poly
{
	PVKFramebuffer::~PVKFramebuffer()
	{
		vkDestroyFramebuffer(PVKInstance::GetDevice(), m_Framebuffer, nullptr);
	}

	void PVKFramebuffer::Init(const FramebufferDesc* pDesc)
	{
		p_FramebufferDesc = *pDesc;

		VkImageView attachment = reinterpret_cast<PVKTextureView*>(pDesc->pTextureView)->GetNativeVK();

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= reinterpret_cast<PVKRenderPass*>(pDesc->pRenderPass)->GetNativeVK();
		framebufferInfo.attachmentCount	= 1;
		framebufferInfo.pAttachments	= &attachment;
		framebufferInfo.width			= pDesc->Width;
		framebufferInfo.height			= pDesc->Height;
		framebufferInfo.layers			= 1;

		PVK_CHECK(vkCreateFramebuffer(PVKInstance::GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer), "Failed to create framebuffer!");
	}
}