#pragma once
#include <vulkan/vulkan.h>
#include <string>

namespace Poly
{
	class PVKSwapChain;
	class PVKShader;
	class PVKRenderPass;
	class PVKDescriptor;

	class PVKPipeline
	{
	public:
		PVKPipeline();
		~PVKPipeline();

		void init(PVKSwapChain& swapChain, PVKShader& shader, PVKRenderPass& renderPass);
		void cleanup();

		void addVertexDescriptions(uint32_t binding, uint32_t location, uint32_t stride, VkFormat format);
		void setDescriptor(PVKDescriptor& descriptor);
		VkPipelineBindPoint getType() const { return this->pipelineType; }
		VkPipeline getNative() const { return this->pipeline; }
		VkPipelineLayout getPipelineLayout() const { return this->pipelineLayout; }

	private:
		void createPipeline();

		VkPipelineBindPoint pipelineType;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		PVKShader* shader;
		PVKDescriptor* descriptor;
		std::vector<VkVertexInputBindingDescription> vertexBinding;
		std::vector<VkVertexInputAttributeDescription> vertexAttributes;

		PVKRenderPass* renderPass;
		PVKSwapChain* swapChain;
	};

}