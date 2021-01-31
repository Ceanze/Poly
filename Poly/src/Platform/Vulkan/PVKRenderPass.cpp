#include "polypch.h"
#include "PVKRenderPass.h"
#include "PVKInstance.h"
#include "PVKSwapChain.h"

namespace Poly
{
	PVKRenderPass::~PVKRenderPass()
	{
		vkDestroyRenderPass(PVKInstance::GetDevice(), m_RenderPass, nullptr);
	}

	void PVKRenderPass::Init(const RenderPassDesc* pDesc)
	{
		// AttachmentDesc
		std::vector<VkAttachmentDescription> attachments;
		attachments.reserve(pDesc->Attachments.size());

		for (auto& attachDesc : pDesc->Attachments)
		{
			VkAttachmentDescription descVK = {};
			descVK.format			= ConvertFormatVK(attachDesc.Format);
			descVK.samples			= ConvertSampleCountVK(attachDesc.SampleCount);
			descVK.loadOp			= ConvertLoadOpVK(attachDesc.LoadOp);
			descVK.storeOp			= ConvertStoreOpVK(attachDesc.StoreOp);
			descVK.stencilLoadOp	= ConvertLoadOpVK(attachDesc.StencilLoadOp);
			descVK.stencilStoreOp	= ConvertStoreOpVK(attachDesc.StencilStoreOp);
			descVK.initialLayout	= ConvertTextureLayoutVK(attachDesc.InitialLayout);
			descVK.finalLayout		= ConvertTextureLayoutVK(attachDesc.FinalLayout);
			descVK.flags			= 0;
			attachments.push_back(descVK);
		}

		// Subpasses and Color attachment references
		std::vector<VkSubpassDescription> subpasses;
		subpasses.reserve(pDesc->Subpasses.size());
		for (auto& subpass : pDesc->Subpasses)
		{
			VkSubpassDescription subpassVK = {};
			std::vector<VkAttachmentReference>	inputRefs;
			std::vector<VkAttachmentReference>	colorRefs;
			std::vector<VkAttachmentReference>	resolveRefs;
			VkAttachmentReference				depthRef = {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED};
			inputRefs.reserve(subpass.InputAttachmentsLayouts.size());
			colorRefs.reserve(subpass.ColorAttachmentsLayouts.size());
			resolveRefs.reserve(subpass.ResolveAttachmentsLayouts.size());


			// Go through each different type of attachment, check if they are used.
			for (auto& inputAttachment : subpass.InputAttachmentsLayouts)
			{
				VkAttachmentReference ref = {};
				ref.attachment	= inputAttachment.Index;
				ref.layout		= ConvertTextureLayoutVK(inputAttachment.Layout);
				inputRefs.push_back(ref);
			}

			for (auto& colorAttachment : subpass.ColorAttachmentsLayouts)
			{
				VkAttachmentReference ref = {};
				ref.attachment	= colorAttachment.Index;
				ref.layout		= ConvertTextureLayoutVK(colorAttachment.Layout);
				colorRefs.push_back(ref);
			}

			for (auto& resolveAttachment : subpass.ResolveAttachmentsLayouts)
			{
				VkAttachmentReference ref = {};
				ref.attachment	= resolveAttachment.Index;
				ref.layout		= ConvertTextureLayoutVK(resolveAttachment.Layout);
				resolveRefs.push_back(ref);
			}

			if (subpass.DepthStencilAttachmentLayout.Layout != ETextureLayout::UNDEFINED)
			{
				depthRef.attachment	= subpass.DepthStencilAttachmentLayout.Index;
				depthRef.layout		= ConvertTextureLayoutVK(subpass.DepthStencilAttachmentLayout.Layout);
			}

			VkSubpassDescription subpassDescVK = {};
			subpassDescVK.pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS; // Only graphics is currently supported
			subpassDescVK.flags						= 0;
			subpassDescVK.pInputAttachments			= inputRefs.data();
			subpassDescVK.inputAttachmentCount		= inputRefs.size();
			subpassDescVK.pColorAttachments			= colorRefs.data();
			subpassDescVK.colorAttachmentCount		= colorRefs.size();
			subpassDescVK.pResolveAttachments		= resolveRefs.data();
			subpassDescVK.preserveAttachmentCount	= 0;
			subpassDescVK.pPreserveAttachments		= nullptr;
			subpassDescVK.pDepthStencilAttachment	= depthRef.attachment == VK_ATTACHMENT_UNUSED ? nullptr : &depthRef;
			subpasses.push_back(subpassDescVK);
		}

		// Dependencies
		std::vector<VkSubpassDependency> dependencies;
		dependencies.reserve(pDesc->SubpassDependencies.size());
		for (auto& dep : pDesc->SubpassDependencies)
		{
			VkSubpassDependency depVK = {};
			depVK.dependencyFlags	= 0;
			depVK.srcSubpass		= dep.SrcSubpass;
			depVK.dstSubpass		= dep.DstSubpass;
			depVK.srcStageMask		= ConvertPipelineStageFlagsVK(dep.SrcStageMask);
			depVK.dstStageMask		= ConvertPipelineStageFlagsVK(dep.DstStageMask);
			depVK.srcAccessMask		= ConvertAccessFlagVK(dep.SrcAccessMask);
			depVK.dstAccessMask		= ConvertAccessFlagVK(dep.DstAccessMask);
			dependencies.push_back(depVK);
		}

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount	= attachments.size();
		renderPassInfo.pAttachments		= attachments.data();
		renderPassInfo.subpassCount		= subpasses.size();
		renderPassInfo.pSubpasses		= subpasses.data();
		renderPassInfo.dependencyCount	= dependencies.size();
		renderPassInfo.pDependencies	= dependencies.data();
		renderPassInfo.flags			= 0;
		renderPassInfo.pNext			= nullptr;

		PVK_CHECK(vkCreateRenderPass(PVKInstance::GetDevice(), &renderPassInfo, nullptr, &m_RenderPass), "Failed to create render pass!");
	}
}

// Code left as a guideline if it goes to shit

// Color attachment
// VkAttachmentDescription colorAttachment = {};
// colorAttachment.format = m_ImageFormat;
// colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
// colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
// colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
// colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
// colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
// colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

// Color attachment reference
// VkAttachmentReference colorAttachmentRef = {};
// colorAttachmentRef.attachment = 0;
// colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

// Subpasses (always atleast 1)
// VkSubpassDescription subpass = {};
// subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
// subpass.colorAttachmentCount = 1;
// subpass.pColorAttachments = &colorAttachmentRef;