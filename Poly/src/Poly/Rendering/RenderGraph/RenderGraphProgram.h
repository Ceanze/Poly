#pragma once

#include "RenderGraphTypes.h"
#include "PassReflection.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Pass;
	class Resource;
	class Framebuffer;
	class RenderGraph;
	class CommandPool;
	class DescriptorSet;
	class CommandBuffer;
	class ResourceCache;
	class PipelineLayout;
	class GraphicsPipeline;
	class GraphicsRenderPass;

	class RenderGraphProgram
	{
	public:
		RenderGraphProgram(RenderGraph* pRenderGraph, Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams);
		~RenderGraphProgram() = default;

		void Init();

		static Ref<RenderGraphProgram> Create(RenderGraph* pRenderGraph, Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams);

		void Execute(uint32 imageIndex);

		void UpdateExternalResource(const std::string& name, Ref<Resource> pResource);

		void SetBackbuffer(Ref<Resource> pResource);

	private:
		friend class RenderGraphCompiler;

		void AddPass(Ref<Pass> pPass); // Used by render graph compiler
		void InitCommandBuffers();
		void InitPipelineLayouts();
		void UpdateBindings(const Ref<Pass>& pPass, uint32 passIndex);
		GraphicsRenderPass* GetGraphicsRenderPass(const Ref<Pass>& pPass, uint32 passIndex);
		Framebuffer* GetFramebuffer(const Ref<Pass>& pPass, uint32 passIndex);
		GraphicsPipeline* GetGraphicsPipeline(const Ref<Pass>& pPass, uint32 passIndex);
		const std::vector<Ref<DescriptorSet>>& GetDescriptorSets(const Ref<Pass>& pPass, uint32 passIndex);

		// Render Graph specific types
		std::vector<Ref<Pass>>		m_Passes;
		Ref<ResourceCache>			m_pResourceCache;
		RenderGraphDefaultParams	m_DefaultParams;
		RenderGraph*				m_pRenderGraph;
		std::unordered_map<uint32, PassReflection>	m_Reflections;

		// Rendering specific types
		uint32														m_ImageIndex = 0;
		std::vector<std::vector<CommandBuffer*>>					m_CommandBuffers; // buffs[passIndex][imageIndex]
		std::unordered_map<FQueueType, Ref<CommandPool>>			m_CommandPools;
		std::unordered_map<uint32, Ref<GraphicsRenderPass>>			m_GraphicsRenderPasses; // key: passIndex
		std::unordered_map<uint32, std::vector<Ref<Framebuffer>>>	m_Framebuffers; // key: passIndex, index: imageIndex
		std::unordered_map<uint32, Ref<PipelineLayout>>				m_PipelineLayouts; // key: passIndex
		std::unordered_map<uint32, Ref<GraphicsPipeline>>			m_GraphicsPipelines; // key: passIndex
		std::unordered_map<uint32, std::vector<Ref<DescriptorSet>>>	m_Descriptors; // key: passIndex, index: set
		std::unordered_map<uint32, std::vector<Ref<DescriptorSet>>>	m_DescriptorsToBeDestroyed; // key: frameIndex
	};
}