#pragma once
#include <vulkan/vulkan.h>
#include <string>

namespace Poly
{
	class PVKInstance;
	class PVKSwapChain;
	class PVKShader;
	class PVKRenderPass;

	class PVKPipeline
	{
	public:
		PVKPipeline();
		~PVKPipeline();

		void init(PVKInstance* instance, PVKSwapChain* swapChain, PVKShader* shader, PVKRenderPass* renderPass);
		void cleanup();

		void addVertexDescriptions(uint32_t binding, uint32_t location, uint32_t stride, VkFormat format);
		VkPipelineBindPoint getType() const { return this->pipelineType; }
		VkPipeline getPipeline() const { return this->pipeline; }
		VkPipelineLayout getPipelineLayout() const { return this->pipelineLayout; }

	private:
		void createPipeline();

		VkPipelineBindPoint pipelineType;
		VkDevice device;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		PVKShader* shader;
		std::vector<VkVertexInputBindingDescription> vertexBinding;
		std::vector<VkVertexInputAttributeDescription> vertexAttributes;

		PVKRenderPass* renderPass;
		PVKSwapChain* swapChain;
	};

}