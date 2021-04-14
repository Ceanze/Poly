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

		std::vector<VkImageView> imageViews;
		imageViews.reserve(pDesc->Attachments.size());
		for (const auto& attachment : pDesc->Attachments)
			imageViews.push_back(reinterpret_cast<PVKTextureView*>(attachment)->GetNativeVK());

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= reinterpret_cast<PVKRenderPass*>(pDesc->pRenderPass)->GetNativeVK();
		framebufferInfo.attachmentCount	= imageViews.size();
		framebufferInfo.pAttachments	= imageViews.data();
		framebufferInfo.width			= pDesc->Width;
		framebufferInfo.height			= pDesc->Height;
		framebufferInfo.layers			= 1;

		PVK_CHECK(vkCreateFramebuffer(PVKInstance::GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer), "Failed to create framebuffer!");
	}
}