#include "polypch.h"
#include "PVKRenderPass.h"
#include "PVKInstance.h"
#include "PVKSwapChain.h"

namespace Poly
{

	PVKRenderPass::PVKRenderPass() :
		device(VK_NULL_HANDLE), imageFormat(VK_FORMAT_UNDEFINED), renderPass(VK_NULL_HANDLE)
	{
	}

	PVKRenderPass::~PVKRenderPass()
	{
	}

	void PVKRenderPass::init(PVKInstance* instance, PVKSwapChain* swapChain)
	{
		this->device = instance->getDevice();
		this->imageFormat = swapChain->getFormat();

		// Color attachment
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = imageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Color attachment reference
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Subpasses (always atleast 1)
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = this->subpassDependencies.size();
		renderPassInfo.pDependencies = this->subpassDependencies.data();

		PVK_CHECK(vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &this->renderPass), "Failed to create render pass!");
	}

	void PVKRenderPass::cleanup()
	{
		vkDestroyRenderPass(this->device, this->renderPass, nullptr);
	}

	void PVKRenderPass::addSubpassDependency(VkSubpassDependency dep)
	{
		this->subpassDependencies.push_back(dep);
	}

}