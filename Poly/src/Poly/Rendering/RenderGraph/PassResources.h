#pragma once

#include <unordered_map>

#include "Poly/Core/PolyID.h"

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
		std::unordered_map<PolyID, PassWindowResources>	PassWindowResources;
	};
}