#include "polypch.h"
#include "TestRenderer.h"

#include "Poly/Core/Camera.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace Poly
{

	void TestRenderer::init(IPlatformRenderer* renderer)
	{
		this->mainRenderer = static_cast<VulkanRenderer*>(renderer);
		this->swapChain = this->mainRenderer->getSwapChain();

		this->shader.addStage(ShaderType::VERTEX, "vert.spv");
		this->shader.addStage(ShaderType::FRAGMENT, "frag.spv");
		this->shader.init();

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		this->renderPass.addSubpassDependency(dependency);
		this->renderPass.init(*this->swapChain);

		setupDescriptorSet();
		this->pipeline.setDescriptor(this->descriptor);
		this->pipeline.init(*this->swapChain, this->shader, this->renderPass);

		// Until this line
		this->commandPool.init(QueueType::GRAPHICS);

		this->framebuffers.resize(this->swapChain->getNumImages());
		for (size_t i = 0; i < this->swapChain->getNumImages(); i++)
			this->framebuffers[i].init(*this->swapChain, this->renderPass, this->swapChain->getImageViews()[i]);

		this->setupTestData();
		this->createCommandBuffers();
	}

	void TestRenderer::beginScene(uint32_t imageIndex)
	{
		this->imageIndex = imageIndex;
	}

	void TestRenderer::record()
	{
	}

	void TestRenderer::endScene()
	{
		this->mainRenderer->addCommandBuffer(QueueType::GRAPHICS, this->commandBuffers[this->imageIndex]);
		this->testMemory.directTransfer(this->testBuffer, &this->camera->getMatrix(), sizeof(glm::mat4), 0);
	}

	void TestRenderer::shutdown()
	{
		PVK_CHECK(vkDeviceWaitIdle(PVKInstance::getDevice()), "Failed to wait for device!");

		delete this->testSampler;
		this->commandPool.cleanup();
		PVK_VEC_CLEANUP(this->framebuffers);
		this->pipeline.cleanup();
		this->descriptor.cleanup();
		this->testBuffer.cleanup();
		this->testMemory.cleanup();
		this->testTexture.cleanup();
		this->testTextureMemory.cleanup();
		this->renderPass.cleanup();
		this->shader.cleanup();
	}

	void TestRenderer::createCommandBuffers()
	{
		this->commandBuffers = this->commandPool.createCommandBuffers(3);

		for (uint32_t i = 0; i < this->commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			this->commandBuffers[i]->begin(0);
			this->commandBuffers[i]->cmdBeginRenderPass(this->renderPass, this->framebuffers[i], this->swapChain->getExtent(), { 0.0f, 0.0f, 0.0f, 1.0f });
			this->commandBuffers[i]->cmdBindPipeline(this->pipeline);
			this->commandBuffers[i]->cmdBindDescriptor(this->pipeline, this->descriptor, i);
			this->commandBuffers[i]->cmdDraw(3, 1, 0, 0);
			this->commandBuffers[i]->cmdEndRenderPass();
			this->commandBuffers[i]->end();
		}
	}

	void TestRenderer::setupDescriptorSet()
	{
		this->descriptor.addBinding(0, 0, BufferType::UNIFORM, ShaderStage::VERTEX);
		this->descriptor.addBinding(0, 1, BufferType::COMBINED_IMAGE_SAMPLER, ShaderStage::FRAGMENT);
		this->descriptor.finalizeSet(0);
		this->descriptor.init(this->swapChain->getNumImages());
	}

	void TestRenderer::setupTestData()
	{
		this->testSampler = new PVKSampler();
		this->testTexture.init(1, 1, ColorFormat::R8G8B8A8_SRGB, ImageUsage::SAMPLED, ImageCreate::NONE, 1, PVKInstance::getQueue(QueueType::GRAPHICS).queueIndex);
		this->testTextureMemory.bindTexture(this->testTexture);
		this->testTextureMemory.init(MemoryPropery::DEVICE_LOCAL);
		this->testTexture.initView(ImageViewType::DIM_2, ImageAspect::COLOR_BIT);

		this->testBuffer.init(sizeof(glm::mat4), BufferUsage::UNIFORM_BUFFER, { PVKInstance::getQueue(QueueType::GRAPHICS).queueIndex });
		this->testMemory.bindBuffer(this->testBuffer);
		this->testMemory.init(MemoryPropery::HOST_VISIBLE_COHERENT);

		this->descriptor.updateBufferBinding(0, 0, this->testBuffer);
		this->descriptor.updateTextureBinding(0, 1, ImageLayout::SHADER_READ_ONLY_OPTIMAL, this->testTexture, *this->testSampler);
	}

}