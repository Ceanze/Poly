#pragma once

namespace Poly
{
	class CommandBuffer;
	class Framebuffer;
	class GraphicsRenderPass;
	class PipelineLayout;
	class GraphicsPipeline;

	struct PassWindowResources
	{
		std::vector<CommandBuffer*>		CommandBuffers;
		std::vector<Ref<Framebuffer>>	Framebuffers;
	};

	struct PassResources
	{
		Ref<GraphicsRenderPass>				GraphicsRenderPass;
		Ref<PipelineLayout>					PipelineLayout;
		Ref<GraphicsPipeline>				GraphicsPipeline;
		std::vector<PassWindowResources>	PassWindowResources;
	};
}