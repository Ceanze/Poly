#include "polypch.h"
#include "PVKCommandBuffer.h"
#include "Poly/Core/PolyUtils.h"
#include "PVKInstance.h"
#include "PVKPipeline.h"
#include "PVKRenderPass.h"
#include "PVKFramebuffer.h"
#include "PVKDescriptor.h"

namespace Poly
{

	PVKCommandBuffer::PVKCommandBuffer() : 
		buffer(VK_NULL_HANDLE), pool(VK_NULL_HANDLE)
	{
	}

	PVKCommandBuffer::~PVKCommandBuffer()
	{
	}

	void PVKCommandBuffer::init(VkCommandPool pool)
	{
		this->pool = pool;
	}

	void PVKCommandBuffer::cleanup()
	{
		// Commands buffers are cleared automatically by the command pool
	}

	void PVKCommandBuffer::createCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = this->pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.pNext = nullptr;

		PVK_CHECK(vkAllocateCommandBuffers(PVKInstance::getDevice(), &allocInfo, &this->buffer), "Failed to allocate command buffer!");
	}

	void PVKCommandBuffer::begin(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flags; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		PVK_CHECK(vkBeginCommandBuffer(this->buffer, &beginInfo), "Failed to begin recording command buffer!");
	}

	void PVKCommandBuffer::cmdBeginRenderPass(PVKRenderPass& renderPass, PVKFramebuffer& framebuffer, VkExtent2D extent, VkClearValue clearColor)
	{
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass.getNative();
		renderPassInfo.framebuffer = framebuffer.getNative();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		renderPassInfo.pNext = nullptr;

		vkCmdBeginRenderPass(this->buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void PVKCommandBuffer::cmdBindPipeline(PVKPipeline& pipeline)
	{
		vkCmdBindPipeline(this->buffer, pipeline.getType(), pipeline.getNative());
	}

	void PVKCommandBuffer::cmdBindDescriptor(PVKPipeline& pipeline, PVKDescriptor& descriptor, uint32_t setCopyIndex)
	{
		auto& sets = descriptor.getSets(setCopyIndex);
		vkCmdBindDescriptorSets(this->buffer, pipeline.getType(), pipeline.getPipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);
	}

	void PVKCommandBuffer::cmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		vkCmdDraw(this->buffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void PVKCommandBuffer::cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		vkCmdDrawIndexed(this->buffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void PVKCommandBuffer::cmdEndRenderPass()
	{
		vkCmdEndRenderPass(this->buffer);
	}

	void PVKCommandBuffer::end()
	{
		PVK_CHECK(vkEndCommandBuffer(this->buffer), "Failed to record command buffer!")
	}

}