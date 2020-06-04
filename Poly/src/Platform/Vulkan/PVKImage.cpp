#include "polypch.h"
#include "PVKImage.h"

#include "PVKInstance.h"
#include "PVKCommandPool.h"
#include "PVKCommandBuffer.h"
#include "PVKBuffer.h"

namespace Poly
{
	PVKImage::PVKImage() : image(VK_NULL_HANDLE), width(0), height(0), layout(VK_IMAGE_LAYOUT_UNDEFINED)
	{
	}

	void PVKImage::init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueFamilyIndex)
	{
		init(width, height, format, usage, flags, arrayLayers, { queueFamilyIndex });
	}

	void PVKImage::init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueFamilyIndices)
	{
		this->width = width;
		this->height = height;

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = (VkFormat)format;
		//If you want to be able to directly access texels in the memory of the image, then you must use VK_IMAGE_TILING_LINEAR
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = (VkImageUsageFlags)usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		if (queueFamilyIndices.size() > 1)
			imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		else
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
		imageInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		imageInfo.flags = (VkImageCreateFlags)flags;


		PVK_CHECK(vkCreateImage(PVKInstance::getDevice(), &imageInfo, nullptr, &this->image), "Failed to create image!");
	}

	void PVKImage::cleanup()
	{
		PVK_CLEANUP(this->image, vkDestroyImage(PVKInstance::getDevice(), this->image, nullptr));
	}

	void PVKImage::copyBufferToImage(PVKBuffer& buffer, PVKCommandPool* pool)
	{
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { this->width, this->height, 1 };
		std::vector<VkBufferImageCopy> regions = { region };
		copyBufferToImage(buffer, pool, regions);
	}

	void PVKImage::copyBufferToImage(PVKBuffer& buffer, PVKCommandPool* pool, const std::vector<VkBufferImageCopy>& regions)
	{
		PVKCommandBuffer* cBuffer = pool->beginSingleTimeCommand();
		cBuffer->cmdCopyBufferToImage(buffer, *this, regions);
		pool->endSingleTimeCommand(cBuffer);
	}

	void PVKImage::transitionLayout(ColorFormat format, ImageLayout oldLayout, ImageLayout newLayout, PVKCommandPool* pool, uint32_t layerCount)
	{
		this->layout = (VkImageLayout)newLayout;

		PVKCommandBuffer* buffer = pool->beginSingleTimeCommand();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = (VkImageLayout)oldLayout;
		barrier.newLayout = (VkImageLayout)newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = this->image;

		if (newLayout == ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			// Check if format has a stencil component
			if ((VkFormat)format == VK_FORMAT_D32_SFLOAT_S8_UINT || (VkFormat)format == VK_FORMAT_D24_UNORM_S8_UINT) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == ImageLayout::UNDEFINED && newLayout == ImageLayout::TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == ImageLayout::TRANSFER_DST_OPTIMAL && newLayout == ImageLayout::SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == ImageLayout::UNDEFINED && newLayout == ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else {
			POLY_ASSERT(false, "Unsupported layout transistion!");
		}

		vkCmdPipelineBarrier(buffer->getNative(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		pool->endSingleTimeCommand(buffer);
	}

}