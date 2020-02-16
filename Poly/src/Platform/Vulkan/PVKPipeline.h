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
		enum class ShaderType {Vertex = 0, Geometry, Tesselation, Fragment, Compute};

		PVKPipeline();
		~PVKPipeline();

		void init(PVKInstance* instance, PVKSwapChain* swapChain, PVKShader* shader, PVKRenderPass* renderPass);
		void cleanup();

		void addVertexDescriptions(uint32_t binding, uint32_t location, uint32_t stride, VkFormat format);

	private:
		void createPipeline();

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