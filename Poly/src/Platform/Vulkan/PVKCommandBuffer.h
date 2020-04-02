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

	class PVKCommandBuffer
	{
	public:
		PVKCommandBuffer();
		~PVKCommandBuffer();

		void init(VkCommandPool pool);
		void cleanup();

		VkCommandBuffer getCommandBuffer() const { return this->buffer; }
		void setCommandBuffer(VkCommandBuffer buffer) { this->buffer = buffer; }
		void createCommandBuffer();

		// Commands for recording
		void begin(VkCommandBufferUsageFlags flags);
		void cmdBeginRenderPass(PVKRenderPass& renderPass, PVKFramebuffer& framebuffer, VkExtent2D extent, VkClearValue clearColor);
		void cmdBindPipeline(PVKPipeline& pipeline);
		void cmdBindDescriptor(PVKPipeline& pipeline, PVKDescriptor& descriptor, uint32_t setCopyIndex);
		void cmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
		void cmdEndRenderPass();
		void end();

	private:
		VkCommandBuffer buffer;
		VkCommandPool pool;
	};

}