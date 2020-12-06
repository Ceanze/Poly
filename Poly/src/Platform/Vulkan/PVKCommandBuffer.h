#pragma once

#include <vulkan/vulkan.h>

/*	
	The creation of the command buffer is delayed and not in the init in order to gain
	small optimizations when creating several buffers at the same time (which is done in 
	PVKCommandPool createCommandBufferS function 
*/

namespace Poly
{
	class PVKRenderPass;
	class PVKPipeline;
	class PVKFramebuffer;
	class PVKDescriptor;
	class PVKBuffer;
	class PVKImage;

	class PVKCommandBuffer
	{
	public:
		PVKCommandBuffer();
		~PVKCommandBuffer();

		void Init(VkCommandPool pool);
		void Cleanup();

		VkCommandBuffer GetNative() const { return m_Buffer; }
		void SetCommandBuffer(VkCommandBuffer buffer) { m_Buffer = buffer; }
		void CreateCommandBuffer();

		// Commands for recording
		void Begin(VkCommandBufferUsageFlags flags);
		void BeginRenderPass(PVKRenderPass& renderPass, PVKFramebuffer& framebuffer, VkExtent2D extent, VkClearValue clearColor);
		void BindPipeline(PVKPipeline& pipeline);
		void BindDescriptor(PVKPipeline& pipeline, PVKDescriptor& descriptor, uint32_t setCopyIndex);
		void CopyBufferToImage(PVKBuffer& buffer, VkImage image, VkImageLayout layout, const std::vector<VkBufferImageCopy>& regions);
		void CopyBufferToImage(PVKBuffer& buffer, PVKImage& image, const std::vector<VkBufferImageCopy>& regions);
		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
		void EndRenderPass();
		void End();

	private:
		VkCommandBuffer	m_Buffer	= VK_NULL_HANDLE;
		VkCommandPool	m_Pool		= VK_NULL_HANDLE;
	};

}