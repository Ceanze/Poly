#pragma once

#include "PVKRenderPass.h"

#include <vulkan/vulkan.h>
#include <string>

namespace Poly
{
	class PVKInstance;
	class PVKSwapChain;
	class PVKShader;

	class PVKPipeline
	{
	public:
		enum class ShaderType {Vertex = 0, Geometry, Tesselation, Fragment, Compute};

		PVKPipeline();
		~PVKPipeline();

		void init(PVKInstance* instance, PVKSwapChain* swapChain, PVKShader* shader);
		void cleanup();

		void addShader(ShaderType shader, const std::string& path);
		void addVertexDescriptions(uint32_t binding, uint32_t location, uint32_t stride, VkFormat format);

	private:
		void createShaderModule(ShaderType shader, const std::vector<char>& code);
		void createPipeline();
		void createFramebuffers();

		VkDevice device;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		PVKShader* shader;
		std::vector<VkVertexInputBindingDescription> vertexBinding;
		std::vector<VkVertexInputAttributeDescription> vertexAttributes;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		PVKRenderPass renderPass;
		PVKSwapChain* swapChain;
	};

}