#pragma once

#include "Platform/API/CommandBuffer.h" // CommandBuffer is important for the interface of RenderContext, included here for ease of use

namespace Poly
{
	class Pipeline;
	class CommandBuffer;
	class PipelineLayout;
	class RenderGraphProgram;

	class RenderContext
	{
	public:
		RenderContext() = default;
		~RenderContext() = default;

		CommandBuffer* GetCommandBuffer() const;

		Pipeline* GetActivePipeline() const { return m_pPipeline; };

		PipelineLayout* GetActivePipelineLayout() const { return m_pPipelineLayout; }

		uint32 GetInstanceSetIndex() const { return m_InstanceSetIndex; }

		uint32 GetPassIndex() const { return m_PassIndex; }

		uint32 GetImageIndex() const { return m_ImageIndex; }

		RenderGraphProgram* GetRenderGraphProgram() const { return m_pRenderGraphProgram; }

		static Ref<RenderContext> Create();

	private:
		friend class RenderGraphProgram;
		void SetCommandBuffer(CommandBuffer* pCommandBuffer);
		void SetActivePipeline(Pipeline* pPipeline) { m_pPipeline = pPipeline; }
		void SetInstanceData(uint32 instanceSetIndex, PipelineLayout* pPipelineLayout) { m_InstanceSetIndex = instanceSetIndex, m_pPipelineLayout = pPipelineLayout; }
		void SetActivePassIndex(uint32 passIndex) { m_PassIndex = passIndex; }
		void SetImageIndex(uint32 imageIndex) { m_ImageIndex = imageIndex; }
		void SetRenderGraphProgram(RenderGraphProgram* pRenderGraphProgram) { m_pRenderGraphProgram = pRenderGraphProgram; }

		CommandBuffer*		m_pCommandBuffer		= nullptr;
		Pipeline*			m_pPipeline				= nullptr;
		PipelineLayout*		m_pPipelineLayout		= nullptr;
		uint32				m_InstanceSetIndex		= 0;
		uint32				m_PassIndex				= 0;
		uint32				m_ImageIndex			= 0;
		RenderGraphProgram*	m_pRenderGraphProgram	= nullptr;
	};
}