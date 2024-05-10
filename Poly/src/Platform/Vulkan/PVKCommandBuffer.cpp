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

	void PVKCommandBuffer::BeginRenderPass(GraphicsRenderPass* pRenderPass, Framebuffer* pFramebuffer, uint32 width, uint32 height, std::vector<ClearValue> clearValues)
	{
		VkExtent2D extent = { width, height };
		VkRenderPassBeginInfo renderPassInfo = {};

		std::vector<VkClearValue> vkClearValues(clearValues.size());
		memcpy(vkClearValues.data(), clearValues.data(), sizeof(ClearValue) * clearValues.size());

		renderPassInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass			= reinterpret_cast<PVKRenderPass*>(pRenderPass)->GetNativeVK();
		renderPassInfo.framebuffer			= reinterpret_cast<PVKFramebuffer*>(pFramebuffer)->GetNativeVK();
		renderPassInfo.renderArea.offset	= { 0, 0 };
		renderPassInfo.renderArea.extent	= extent;
		renderPassInfo.clearValueCount		= static_cast<uint32>(vkClearValues.size());
		renderPassInfo.pClearValues			= vkClearValues.data();
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

	void PVKCommandBuffer::BindDescriptor(const Pipeline* pPipeline, const DescriptorSet* pDescriptor, uint32 dynamicOffsetCount, const uint32* pDynamicOffsets)
	{
		if (pPipeline->GetPipelineType() == EPipelineType::GRAPHICS)
		{
			VkDescriptorSet descSet = reinterpret_cast<const PVKDescriptorSet*>(pDescriptor)->GetNativeVK();
			vkCmdBindDescriptorSets(
				m_Buffer,
				ConvertPipelineTypeVK(pPipeline->GetPipelineType()),
				reinterpret_cast<PVKPipelineLayout*>(reinterpret_cast<const PVKGraphicsPipeline*>(pPipeline)->GetPipelineLayout())->GetNativeVK(),
				pDescriptor->GetSetIndex(),
				1,
				&descSet,
				dynamicOffsetCount,
				pDynamicOffsets);
		}
	}

	void PVKCommandBuffer::BindVertexBuffer(const Buffer* pBuffer, uint32 firstBinding, uint32 bindingCount, uint64 offset)
	{
		const PVKBuffer* pPVKBuffer = static_cast<const PVKBuffer*>(pBuffer);
		VkBuffer vkBuffer = pPVKBuffer->GetNativeVK();
		vkCmdBindVertexBuffers(m_Buffer, firstBinding, bindingCount, &vkBuffer, &offset);
	}

	void PVKCommandBuffer::BindIndexBuffer(const Buffer* pBuffer, uint64 offset, EIndexType indexType)
	{
		const PVKBuffer* pPVKBuffer = static_cast<const PVKBuffer*>(pBuffer);
		VkBuffer vkBuffer = pPVKBuffer->GetNativeVK();
		vkCmdBindIndexBuffer(m_Buffer, vkBuffer, offset, ConvertIndexTypeVK(indexType));
	}

	void PVKCommandBuffer::UpdatePushConstants(const PipelineLayout* pPipelineLayout, FShaderStage shaderStage, uint32 offset, uint32 size, const void* data)
	{
		VkPipelineLayout layout = static_cast<const PVKPipelineLayout*>(pPipelineLayout)->GetNativeVK();
		vkCmdPushConstants(m_Buffer, layout, ConvertShaderStageVK(shaderStage), offset, size, data);
	}

	void PVKCommandBuffer::CopyBufferToTexture(const Buffer* pBuffer, const Texture* pTexture, ETextureLayout layout, const CopyBufferDesc& copyBufferDesc)
	{
		VkImageSubresourceLayers subresource = {};
		subresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.baseArrayLayer	= copyBufferDesc.ArrayLayer;
		subresource.layerCount		= copyBufferDesc.ArrayCount;
		subresource.mipLevel		= copyBufferDesc.MipLevel;

		VkBufferImageCopy copyDesc = {};
		copyDesc.bufferImageHeight	= copyBufferDesc.BufferImageHeight;
		copyDesc.bufferOffset		= copyBufferDesc.BufferOffset;
		copyDesc.bufferRowLength	= copyBufferDesc.BufferRowLength;
		copyDesc.imageExtent		= { copyBufferDesc.Width, copyBufferDesc.Height, copyBufferDesc.Depth };
		copyDesc.imageOffset		= { copyBufferDesc.ImageOffsetX, copyBufferDesc.ImageOffsetY, copyBufferDesc.ImageOffsetZ };
		copyDesc.imageSubresource	= subresource;

		vkCmdCopyBufferToImage(
			m_Buffer,
			reinterpret_cast<const PVKBuffer*>(pBuffer)->GetNativeVK(),
			reinterpret_cast<const PVKTexture*>(pTexture)->GetNativeVK(),
			ConvertTextureLayoutVK(layout),
			1,
			&copyDesc);
	}

	void PVKCommandBuffer::CopyBuffer(const Buffer* pSrcBuffer, const Buffer* pDstBuffer, uint64 size, uint64 srcOffset, uint64 dstOffset)
	{
		VkBufferCopy copyDesc = {};
		copyDesc.size		= size;
		copyDesc.srcOffset	= srcOffset;
		copyDesc.dstOffset	= dstOffset;

		vkCmdCopyBuffer(
			m_Buffer,
			reinterpret_cast<const PVKBuffer*>(pSrcBuffer)->GetNativeVK(),
			reinterpret_cast<const PVKBuffer*>(pDstBuffer)->GetNativeVK(),
			1,
			&copyDesc
		);
	}

	void PVKCommandBuffer::UpdateBuffer(const Buffer* pBuffer, uint64 size, uint64 offset, const void* pData)
	{
		const PVKBuffer* pPVKBuffer = static_cast<const PVKBuffer*>(pBuffer);
		VkBuffer vkBuffer = pPVKBuffer->GetNativeVK();

		vkCmdUpdateBuffer(m_Buffer, vkBuffer, offset, size, pData);
	}

	void PVKCommandBuffer::SetViewport(const ViewportDesc* pViewport)
	{
		// TODO: Allow for multiple viewports

		VkViewport viewport = {};
		viewport.height		= pViewport->Height;
		viewport.width		= pViewport->Width;
		viewport.x			= pViewport->PosX;
		viewport.y			= pViewport->PosY;
		viewport.minDepth	= pViewport->MinDepth;
		viewport.maxDepth	= pViewport->MaxDepth;

		vkCmdSetViewport(m_Buffer, 0, 1, &viewport);
	}

	void PVKCommandBuffer::SetScissor(const ScissorDesc* pScissor)
	{
		// TODO: Allow for multiple scissors
		VkRect2D scissor = {};
		scissor.extent.height	= pScissor->Height;
		scissor.extent.width	= pScissor->Width;
		scissor.offset.x		= pScissor->OffsetX;
		scissor.offset.y		= pScissor->OffsetY;

		vkCmdSetScissor(m_Buffer, 0, 1, &scissor);
	}

	void PVKCommandBuffer::DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
	{
		vkCmdDraw(m_Buffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void PVKCommandBuffer::DrawIndexedInstanced(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance)
	{
		vkCmdDrawIndexed(m_Buffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void PVKCommandBuffer::AcquireBuffer(
		const Buffer* pBuffer,
		FPipelineStage srcStage,
		FPipelineStage dstStage,
		FAccessFlag dstAccessMask,
		uint32 srcQueueIndex,
		uint32 dstQueueIndex)
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= 0;
		barrier.dstAccessMask		= ConvertAccessFlagVK(dstAccessMask);
		barrier.srcQueueFamilyIndex	= srcQueueIndex;
		barrier.dstQueueFamilyIndex = dstQueueIndex;
		barrier.buffer				= reinterpret_cast<const PVKBuffer*>(pBuffer)->GetNativeVK();
		barrier.offset				= 0;
		barrier.size				= pBuffer->GetSize();

		vkCmdPipelineBarrier(
			m_Buffer,
			ConvertPipelineStageFlagsVK(srcStage),
			ConvertPipelineStageFlagsVK(dstStage),
			0,
			0,
			nullptr,
			1,
			&barrier,
			0,
			nullptr);
	}

	void PVKCommandBuffer::ReleaseBuffer(
		const Buffer* pBuffer,
		FPipelineStage srcStage,
		FPipelineStage dstStage,
		FAccessFlag srcAccessMask,
		uint32 srcQueueIndex,
		uint32 dstQueueIndex)
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= ConvertAccessFlagVK(srcAccessMask);
		barrier.dstAccessMask		= 0;
		barrier.srcQueueFamilyIndex	= srcQueueIndex;
		barrier.dstQueueFamilyIndex = dstQueueIndex;
		barrier.buffer				= reinterpret_cast<const PVKBuffer*>(pBuffer)->GetNativeVK();
		barrier.offset				= 0;
		barrier.size				= pBuffer->GetSize();

		vkCmdPipelineBarrier(
			m_Buffer,
			ConvertPipelineStageFlagsVK(srcStage),
			ConvertPipelineStageFlagsVK(dstStage),
			0,
			0,
			nullptr,
			1,
			&barrier,
			0,
			nullptr);
	}

	void PVKCommandBuffer::AcquireTexture(
		const Texture* pTexture,
		FPipelineStage srcStage,
		FPipelineStage dstStage,
		FAccessFlag dstAccessMask,
		ETextureLayout oldLayout,
		ETextureLayout newLayout,
		uint32 srcQueueIndex,
		uint32 dstQueueIndex)
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
		barrier.image				= reinterpret_cast<const PVKTexture*>(pTexture)->GetNativeVK();
		barrier.subresourceRange	= range;

		vkCmdPipelineBarrier(
			m_Buffer,
			ConvertPipelineStageFlagsVK(srcStage),
			ConvertPipelineStageFlagsVK(dstStage),
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);
	}

	void PVKCommandBuffer::ReleaseTexture(
		const Texture* pTexture,
		FPipelineStage srcStage,
		FPipelineStage dstStage,
		FAccessFlag srcAccessMask,
		ETextureLayout oldLayout,
		ETextureLayout newLayout,
		uint32 srcQueueIndex,
		uint32 dstQueueIndex)
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
		barrier.image				= reinterpret_cast<const PVKTexture*>(pTexture)->GetNativeVK();
		barrier.subresourceRange	= range;

		vkCmdPipelineBarrier(
			m_Buffer,
			ConvertPipelineStageFlagsVK(srcStage),
			ConvertPipelineStageFlagsVK(dstStage),
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);
	}

	void PVKCommandBuffer::PipelineTextureBarrier(
		const Texture* pTexture,
		FPipelineStage srcStage,
		FPipelineStage dstStage,
		FAccessFlag srcAccessFlag,
		FAccessFlag dstAccessFlag,
		ETextureLayout oldLayout,
		ETextureLayout newLayout)
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
		barrier.image				= reinterpret_cast<const PVKTexture*>(pTexture)->GetNativeVK();
		barrier.subresourceRange	= range;

		vkCmdPipelineBarrier(
			m_Buffer,
			ConvertPipelineStageFlagsVK(srcStage),
			ConvertPipelineStageFlagsVK(dstStage),
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);
	}

	void PVKCommandBuffer::PipelineBufferBarrier(
		const Buffer* pBuffer,
		FPipelineStage srcStage,
		FPipelineStage dstStage,
		FAccessFlag srcAccessFlag,
		FAccessFlag dstAccessFlag)
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType				= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext				= nullptr;
		barrier.srcAccessMask		= ConvertAccessFlagVK(srcAccessFlag);
		barrier.dstAccessMask		= ConvertAccessFlagVK(dstAccessFlag);
		barrier.srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer				= reinterpret_cast<const PVKBuffer*>(pBuffer)->GetNativeVK();
		barrier.offset				= 0;
		barrier.size				= pBuffer->GetSize();

		vkCmdPipelineBarrier(
			m_Buffer,
			ConvertPipelineStageFlagsVK(srcStage),
			ConvertPipelineStageFlagsVK(dstStage),
			0,
			0,
			nullptr,
			1,
			&barrier,
			0,
			nullptr);
	}

	void PVKCommandBuffer::PipelineBarrier(
		FPipelineStage srcStage,
		FPipelineStage dstStage,
		const std::vector<AccessBarrier>& accessBarriers,
		const std::vector<BufferBarrier>& bufferBarriers,
		const std::vector<TextureBarrier>& textureBarriers)
	{
		// Memory barriers
		std::vector<VkMemoryBarrier> vkMemoryBarriers;
		vkMemoryBarriers.reserve(accessBarriers.size());
		for (const auto& b : accessBarriers)
		{
			VkMemoryBarrier vkBarrier = {};
			vkBarrier.sType			= VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			vkBarrier.pNext			= nullptr;
			vkBarrier.srcAccessMask	= ConvertAccessFlagVK(b.SrcAccessFlag);
			vkBarrier.dstAccessMask	= ConvertAccessFlagVK(b.DstAccessFlag);

			vkMemoryBarriers.push_back(vkBarrier);
		}

		// Buffer barriers
		std::vector<VkBufferMemoryBarrier> vkBufferBarriers;
		vkBufferBarriers.reserve(bufferBarriers.size());
		for (const auto& b : bufferBarriers)
		{
			uint32 srcQueueIndex = b.SrcQueueIndex;
			uint32 dstQueueIndex = b.DstQueueIndex;
			if (srcQueueIndex == dstQueueIndex)
			{
				srcQueueIndex = VK_QUEUE_FAMILY_IGNORED;
				dstQueueIndex = VK_QUEUE_FAMILY_IGNORED;
			}

			VkBufferMemoryBarrier vkBarrier = {};
			vkBarrier.sType					= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			vkBarrier.pNext					= nullptr;
			vkBarrier.srcAccessMask			= ConvertAccessFlagVK(b.SrcAccessFlag);
			vkBarrier.dstAccessMask			= ConvertAccessFlagVK(b.DstAccessFlag);
			vkBarrier.srcQueueFamilyIndex	= srcQueueIndex;
			vkBarrier.dstQueueFamilyIndex	= dstQueueIndex;
			vkBarrier.buffer				= reinterpret_cast<PVKBuffer*>(b.pBuffer)->GetNativeVK();
			vkBarrier.offset				= b.Offset;
			vkBarrier.size					= b.pBuffer->GetSize();

			vkBufferBarriers.push_back(vkBarrier);
		}

		// Texture/image barriers
		std::vector<VkImageMemoryBarrier> vkImageBarriers;
		vkImageBarriers.reserve(textureBarriers.size());
		for (const auto& b : textureBarriers)
		{
			uint32 srcQueueIndex = b.SrcQueueIndex;
			uint32 dstQueueIndex = b.DstQueueIndex;
			if (srcQueueIndex == dstQueueIndex)
			{
				srcQueueIndex = VK_QUEUE_FAMILY_IGNORED;
				dstQueueIndex = VK_QUEUE_FAMILY_IGNORED;
			}

			VkImageSubresourceRange range = {};
			range.aspectMask		= ConvertImageViewFlagsVK(b.AspectMask);
			range.baseMipLevel		= 0;
			range.levelCount		= 1;
			range.baseArrayLayer	= 0;
			range.layerCount		= 1;

			VkImageMemoryBarrier vkBarrier = {};
			vkBarrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			vkBarrier.pNext				= nullptr;
			vkBarrier.srcAccessMask		= ConvertAccessFlagVK(b.SrcAccessFlag);
			vkBarrier.dstAccessMask		= ConvertAccessFlagVK(b.DstAccessFlag);
			vkBarrier.oldLayout			= ConvertTextureLayoutVK(b.OldLayout);
			vkBarrier.newLayout			= ConvertTextureLayoutVK(b.NewLayout);
			vkBarrier.srcQueueFamilyIndex	= srcQueueIndex;
			vkBarrier.dstQueueFamilyIndex	= dstQueueIndex;
			vkBarrier.image				= reinterpret_cast<PVKTexture*>(b.pTexture)->GetNativeVK();
			vkBarrier.subresourceRange	= range;

			vkImageBarriers.push_back(vkBarrier);
		}

		vkCmdPipelineBarrier(
			m_Buffer,
			ConvertPipelineStageFlagsVK(srcStage),
			ConvertPipelineStageFlagsVK(dstStage),
			0,
			static_cast<uint32>(vkMemoryBarriers.size()),
			vkMemoryBarriers.data(),
			static_cast<uint32>(vkBufferBarriers.size()),
			vkBufferBarriers.data(),
			static_cast<uint32>(vkImageBarriers.size()),
			vkImageBarriers.data());
	}


	void PVKCommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(m_Buffer);
	}

	void PVKCommandBuffer::End()
	{
		PVK_CHECK(vkEndCommandBuffer(m_Buffer), "Failed to record command buffer!")
	}

	void PVKCommandBuffer::Reset()
	{
		vkResetCommandBuffer(m_Buffer, 0);
	}

}