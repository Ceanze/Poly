#include "polypch.h"
#include "FramebufferCache.h"
#include "GraphicsRenderPass.h"
#include "TextureView.h"
#include "Framebuffer.h"
#include "Poly/Core/RenderAPI.h"

namespace Poly
{
	void FramebufferCache::Key::CreateHash()
	{
		Hash = std::hash<size_t>()(pPass->GetNative());
		if (pDepthAttachment)
			Hash ^= std::hash<size_t>()(pDepthAttachment->GetNative());
		for (const auto& attachment : Attachments)
			Hash ^= (Hash << 5) + (Hash >> 2) + std::hash<size_t>()(attachment->GetNative());
	}

	Ref<Framebuffer> FramebufferCache::GetFramebuffer(const std::vector<TextureView*>& attachments, TextureView* pDepthAttachment, GraphicsRenderPass* pPass, uint32 width, uint32 height)
	{
		Key newKey = {};
		newKey.Attachments		= attachments;
		newKey.pDepthAttachment	= pDepthAttachment;
		newKey.pPass			= pPass;
		newKey.CreateHash();

		// If the framebuffer is already created, return it
		if (m_Framebuffers.contains(newKey))
			return m_Framebuffers[newKey];

		// Create new framebuffer

		// Place depth attachment at the end if it was given
		std::vector<TextureView*> allAttachments;
		allAttachments.reserve(attachments.size());
		allAttachments.insert(allAttachments.begin(), attachments.begin(), attachments.end());
		if (pDepthAttachment)
			allAttachments.push_back(pDepthAttachment);

		FramebufferDesc desc = {};
		desc.Height			= height;
		desc.Width			= width;
		desc.Attachments	= allAttachments;
		desc.pRenderPass	= pPass;
		Ref<Framebuffer> framebuffer = RenderAPI::CreateFramebuffer(&desc);

		m_Framebuffers[newKey] = framebuffer;
		return framebuffer;
	}


}