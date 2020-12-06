#include "polypch.h"
#include "PVKCommandBuffer.h"
#include "Poly/Core/PolyUtils.h"
#include "PVKInstance.h"
#include "PVKPipeline.h"
#include "PVKRenderPass.h"
#include "PVKFramebuffer.h"
#include "PVKDescriptor.h"
#include "PVKBuffer.h"
#include "PVKImage.h"

namespace Poly
{

	PVKCommandBuffer::PVKCommandBuffer()
	{
	}

	PVKCommandBuffer::~PVKCommandBuffer()
	{
	}

	void PVKCommandBuffer::Init(VkCommandPool pool)
	{
		m_Pool = pool;
	}

	void PVKCommandBuffer::Cleanup()
	{
		// Commands buffers are cleared automatically by the command pool
	}

	void PVKCommandBuffer::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_Pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.pNext = nullptr;

		PVK_CHECK(vkAllocateCommandBuffers(PVKInstance::GetDevice(), &allocInfo, &m_Buffer), "Failed to allocate command buffer!");
	}

	void PVKCommandBuffer::Begin(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flags; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		PVK_CHECK(vkBeginCommandBuffer(m_Buffer, &beginInfo), "Failed to begin recording command buffer!");
	}

	void PVKCommandBuffer::BeginRenderPass(PVKRenderPass& renderPass, PVKFramebuffer& framebuffer, VkExtent2D extent, VkClearValue clearColor)
	{
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass.GetNative();
		renderPassInfo.framebuffer = framebuffer.GetNative();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		renderPassInfo.pNext = nullptr;

		vkCmdBeginRenderPass(m_Buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void PVKCommandBuffer::BindPipeline(PVKPipeline& pipeline)
	{
		vkCmdBindPipeline(m_Buffer, pipeline.GetType(), pipeline.GetNative());
	}

	void PVKCommandBuffer::BindDescriptor(PVKPipeline& pipeline, PVKDescriptor& descriptor, uint32_t setCopyIndex)
	{
		auto sets = descriptor.GetSets(setCopyIndex);
		vkCmdBindDescriptorSets(m_Buffer, pipeline.GetType(), pipeline.GetPipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);
	}

	void PVKCommandBuffer::CopyBufferToImage(PVKBuffer& buffer, VkImage image, VkImageLayout layout, const std::vector<VkBufferImageCopy>& regions)
	{
		vkCmdCopyBufferToImage(m_Buffer, buffer.GetNative(), image, layout, regions.size(), regions.data());
	}

	void PVKCommandBuffer::CopyBufferToImage(PVKBuffer& buffer, PVKImage& image, const std::vector<VkBufferImageCopy>& regions)
	{
		vkCmdCopyBufferToImage(m_Buffer, buffer.GetNative(), image.GetNative(), image.GetLayout(), regions.size(), regions.data());
	}

	void PVKCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		vkCmdDraw(m_Buffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void PVKCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		vkCmdDrawIndexed(m_Buffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void PVKCommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(m_Buffer);
	}

	void PVKCommandBuffer::End()
	{
		PVK_CHECK(vkEndCommandBuffer(m_Buffer), "Failed to record command buffer!")
	}

}