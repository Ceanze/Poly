#pragma once

#include "Platform/API/GraphicsPipeline.h"
#include "PVKTypes.h"

#include <string>

namespace Poly
{
	class PVKPipelineLayout;

	class PVKGraphicsPipeline : public GraphicsPipeline
	{
	public:
		PVKGraphicsPipeline() = default;
		~PVKGraphicsPipeline();

		virtual void Init(const GraphicsPipelineDesc* pDesc) override final;

		VkPipeline GetNativeVK() const { return m_Pipeline; }
		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Pipeline); }
		virtual PipelineLayout* GetPipelineLayout() const override final { return reinterpret_cast<PipelineLayout*>(m_pPipelineLayout); }

	private:
		void CreatePipeline();

		VkPipeline			m_Pipeline			= VK_NULL_HANDLE;
		PVKPipelineLayout*	m_pPipelineLayout	= nullptr;
	};

}