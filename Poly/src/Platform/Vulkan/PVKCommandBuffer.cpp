#include "polypch.h"
#include "PVKCommandBuffer.h"
#include "Poly/Core/PolyUtils.h"

#include "PVKBuffer.h"
#include "PVKTexture.h"
#include "PVKInstance.h"
#include "PVKRenderPass.h"
#include "PVKFramebuffer.h"
#include "PVKCommandPool.h"
#include "PVKDescriptorSet.h"
#include "PVKPipelineLayout.h"
#include "PVKGraphicsPipeline.h"

namespace Poly
{

	PVKCommandBuffer::~PVKCommandBuffer()
	{
		// Destruction of command buffers happens when command pool is destroyed
	}

	void PVKCommandBuffer::Init(CommandPool* pCommandPool)
	{
		p_pCommandPool = pCommandPool;

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool			= reinterpret_cast<PVKCommandPool*>(pCommandPool)->GetNativeVK();
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Current implementation only supports primary
		allocInfo.commandBufferCount	= 1;

		PVK_CHECK(vkAllocateCommandBuffers(PVKInstance::GetDevice(), &allocInfo, &m_Buffer), "Failed to allocate command buffers!")
	}


	void PVKCommandBuffer::Begin(FCommandBufferFlag bufferFlag)
	{
		// TODO: Add support for secondary buffers

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= ConvertCommandBufferUsage(bufferFlag);
		beginInfo.pInheritanceInfo	= nullptr;

		PVK_CHECK(vkBeginCommandBuffer(m_Buffer, &beginInfo), "Failed to begin recording of command buffer!");
	}

	void PVKCommandBuffer::BeginRenderPass(RenderPass* pRenderPass, Framebuffer* pFramebuffer, uint32 width, uint32 height, float* pClearColor, uint32 clearColorCount)
	{
		VkExtent2D extent = { width, height };
		VkRenderPassBeginInfo renderPassInfo = {};

		// TODO: Make clear color thingy magic better
		VkClearColorValue clearColorValue = {};
		memcpy(clearColorValue.float32, pClearColor, sizeof(float) * 4);
		VkClearValue clearValue = {};
		clearValue.color = clearColorValue;

		renderPassInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass			= reinterpret_cast<PVKRenderPass*>(pRenderPass)->GetNativeVK();
		renderPassInfo.framebuffer			= reinterpret_cast<PVKFramebuffer*>(pFramebuffer)->GetNativeVK();
		renderPassInfo.renderArea.offset	= { 0, 0 };
		renderPassInfo.renderArea.extent	= extent;
		renderPassInfo.clearValueCount		= clearColorCount;
		renderPassInfo.pClearValues			= &clearValue;
		renderPassInfo.pNext				= nullptr;

		vkCmdBeginRenderPass(m_Buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void PVKCommandBuffer::BindPipeline(Pipeline* pPipeline)
	{
		if (pPipeline->GetPipelineType() == EPipelineType::GRAPHICS)
		{
			vkCmdBindPipeline(
				m_Buffer,
				ConvertPipelineTypeVK(pPipeline->GetPipelineType()),
				reinterpret_cast<PVKGraphicsPipeline*>(pPipeline)->GetNativeVK());
		}
		else
		{
			POLY_VALIDATE(false, "BindPipeline failed: Given pipeline did not have any type associated with it!");
		}
	}

	void PVKCommandBuffer::BindDescriptor(Pipeline* pPipeline, DescriptorSet* pDescriptor)
	{
		if (pPipeline->GetPipelineType() == EPipelineType::GRAPHICS)
		{
			VkDescriptorSet descSet = reinterpret_cast<PVKDescriptorSet*>(pDescriptor)->GetNativeVK();
			vkCmdBindDescriptorSets(
				m_Buffer,
				ConvertPipelineTypeVK(pPipeline->GetPipelineType()),
				reinterpret_cast<PVKPipelineLayout*>(reinterpret_cast<PVKGraphicsPipeline*>(pPipeline)->GetPipelineLayout())->GetNativeVK(),
				0,
				1,
				&descSet,
				0,
				nullptr);
		}
	}

	void PVKCommandBuffer::CopyBufferToTexture(Buffer* pBuffer, Texture* pTexture, ETextureLayout layout, const CopyBufferDesc& copyBufferDesc)
	{
		VkImageSubresourceLayers subresource = {};
		subresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.baseArrayLayer	= copyBufferDesc.ArrayLayer;
		subresource.layerCount		= copyBufferDesc.ArrayCount;
		subresource.mipLevel		= copyBufferDesc.MipLevel;

		VkBufferImageCopy copyDesc = {};
		copyDesc.bufferImageHeight	= copyBufferDesc.BufferImageHeight;
		copyDesc.bufferOffset		= copyBufferDesc.BufferOffset;
		copyDesc.bufferRowLength	= copyBufferDesc.BufferRowLengh;
		copyDesc.imageExtent		= { copyBufferDesc.Width, copyBufferDesc.Height, copyBufferDesc.Depth };
		copyDesc.imageOffset		= { copyBufferDesc.ImageOffsetX, copyBufferDesc.ImageOffsetY, copyBufferDesc.ImageOffsetZ };
		copyDesc.imageSubresource	= subresource;

		vkCmdCopyBufferToImage(
			m_Buffer,
			reinterpret_cast<PVKBuffer*>(pBuffer)->GetNativeVK(),
			reinterpret_cast<PVKTexture*>(pTexture)->GetNativeVK(),
			ConvertTextureLayoutVK(layout),
			1,
			&copyDesc);
	}

	void PVKCommandBuffer::DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
	{
		vkCmdDraw(m_Buffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void PVKCommandBuffer::DrawIndexedInstanced(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance)
	{
		vkCmdDrawIndexed(m_Buffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void PVKCommandBuffer::AcquireBuffer(Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag dstAccessMask, uint32 srcQueueIndex, uint32 dstQueueIndex)
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= 0;
		barrier.dstAccessMask		= ConvertAccessFlagVK(dstAccessMask);
		barrier.srcQueueFamilyIndex	= srcQueueIndex;
		barrier.dstQueueFamilyIndex = dstQueueIndex;
		barrier.buffer				= reinterpret_cast<PVKBuffer*>(pBuffer)->GetNativeVK();
		barrier.offset				= 0;
		barrier.size				= pBuffer->GetSize();

		vkCmdPipelineBarrier(m_Buffer, ConvertPipelineStageFlagsVK(srcStage), ConvertPipelineStageFlagsVK(dstStage), 0, 0, nullptr, 1, &barrier, 0, nullptr);
	}

	void PVKCommandBuffer::ReleaseBuffer(Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessMask, uint32 srcQueueIndex, uint32 dstQueueIndex)
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= ConvertAccessFlagVK(srcAccessMask);
		barrier.dstAccessMask		= 0;
		barrier.srcQueueFamilyIndex	= srcQueueIndex;
		barrier.dstQueueFamilyIndex = dstQueueIndex;
		barrier.buffer				= reinterpret_cast<PVKBuffer*>(pBuffer)->GetNativeVK();
		barrier.offset				= 0;
		barrier.size				= pBuffer->GetSize();

		vkCmdPipelineBarrier(m_Buffer, ConvertPipelineStageFlagsVK(srcStage), ConvertPipelineStageFlagsVK(dstStage), 0, 0, nullptr, 1, &barrier, 0, nullptr);
	}

	void PVKCommandBuffer::AcquireTexture(Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag dstAccessMask, ETextureLayout oldLayout, ETextureLayout newLayout, uint32 srcQueueIndex, uint32 dstQueueIndex)
	{
		VkImageSubresourceRange range = {};
		range.aspectMask		= VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel		= 0;
		range.levelCount		= 1;
		range.baseArrayLayer	= 0;
		range.layerCount		= 1;

		VkImageMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= 0;
		barrier.dstAccessMask		= ConvertAccessFlagVK(dstAccessMask);
		barrier.oldLayout			= ConvertTextureLayoutVK(oldLayout);
		barrier.newLayout			= ConvertTextureLayoutVK(newLayout);
		barrier.srcQueueFamilyIndex	= srcQueueIndex;
		barrier.dstQueueFamilyIndex	= dstQueueIndex;
		barrier.image				= reinterpret_cast<PVKTexture*>(pTexture)->GetNativeVK();
		barrier.subresourceRange	= range;

		vkCmdPipelineBarrier(m_Buffer, ConvertPipelineStageFlagsVK(srcStage), ConvertPipelineStageFlagsVK(dstStage), 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void PVKCommandBuffer::ReleaseTexture(Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessMask, ETextureLayout oldLayout, ETextureLayout newLayout, uint32 srcQueueIndex, uint32 dstQueueIndex)
	{
		VkImageSubresourceRange range = {};
		range.aspectMask		= VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel		= 0;
		range.levelCount		= 1;
		range.baseArrayLayer	= 0;
		range.layerCount		= 1;

		VkImageMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= ConvertAccessFlagVK(srcAccessMask);
		barrier.dstAccessMask		= 0;
		barrier.oldLayout			= ConvertTextureLayoutVK(oldLayout);
		barrier.newLayout			= ConvertTextureLayoutVK(newLayout);
		barrier.srcQueueFamilyIndex	= srcQueueIndex;
		barrier.dstQueueFamilyIndex	= dstQueueIndex;
		barrier.image				= reinterpret_cast<PVKTexture*>(pTexture)->GetNativeVK();
		barrier.subresourceRange	= range;

		vkCmdPipelineBarrier(m_Buffer, ConvertPipelineStageFlagsVK(srcStage), ConvertPipelineStageFlagsVK(dstStage), 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void PVKCommandBuffer::PipelineTextureBarrier(Texture* pTexture, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessFlag, FAccessFlag dstAccessFlag, ETextureLayout oldLayout, ETextureLayout newLayout)
	{
		// TODO: Allow for a dynamic range
		VkImageSubresourceRange range = {};
		range.aspectMask		= VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel		= 0;
		range.levelCount		= 1;
		range.baseArrayLayer	= 0;
		range.layerCount		= 1;

		VkImageMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= ConvertAccessFlagVK(srcAccessFlag);
		barrier.dstAccessMask		= ConvertAccessFlagVK(dstAccessFlag);
		barrier.oldLayout			= ConvertTextureLayoutVK(oldLayout);
		barrier.newLayout			= ConvertTextureLayoutVK(newLayout);
		barrier.srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		barrier.image				= reinterpret_cast<PVKTexture*>(pTexture)->GetNativeVK();
		barrier.subresourceRange	= range;

		vkCmdPipelineBarrier(m_Buffer, ConvertPipelineStageFlagsVK(srcStage), ConvertPipelineStageFlagsVK(dstStage), 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void PVKCommandBuffer::PipelineBufferBarrier(Buffer* pBuffer, FPipelineStage srcStage, FPipelineStage dstStage, FAccessFlag srcAccessFlag, FAccessFlag dstAccessFlag)
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= ConvertAccessFlagVK(srcAccessFlag);
		barrier.dstAccessMask		= ConvertAccessFlagVK(dstAccessFlag);
		barrier.srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer				= reinterpret_cast<PVKBuffer*>(pBuffer)->GetNativeVK();
		barrier.offset				= 0;
		barrier.size				= pBuffer->GetSize();

		vkCmdPipelineBarrier(m_Buffer, ConvertPipelineStageFlagsVK(srcStage), ConvertPipelineStageFlagsVK(dstStage), 0, 0, nullptr, 1, &barrier, 0, nullptr);
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