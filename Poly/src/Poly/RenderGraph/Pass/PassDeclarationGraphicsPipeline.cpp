#include "PassDeclarationGraphicsPipeline.h"

#include "PassDeclaration.h"

namespace Poly
{
	PassDeclarationGraphicsPipeline::PassDeclarationGraphicsPipeline(PassDeclaration& parent)
	    : m_Parent(parent)
	    , m_VertexInputBuilder(*this)
	    , m_ColorBlendAttachmentBuilder(*this)
	{}

	PassDeclarationVertexInput& PassDeclarationGraphicsPipeline::AddVertexInput()
	{
		return m_VertexInputBuilder;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::Topology(ETopology topology)
	{
		m_Desc.InputAssembly.Topology = topology;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::RestartPrimitive(bool enable)
	{
		m_Desc.InputAssembly.RestartPrimitive = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::ViewportDynamic(bool isDynamic)
	{
		m_Desc.Viewport.IsDynamic = isDynamic;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::ViewportOffset(float x, float y)
	{
		m_Desc.Viewport.PosX = x;
		m_Desc.Viewport.PosY = y;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::ViewportSize(float width, float height)
	{
		m_Desc.Viewport.Width  = width;
		m_Desc.Viewport.Height = height;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::ViewportDepth(float minDepth, float maxDepth)
	{
		m_Desc.Viewport.MinDepth = minDepth;
		m_Desc.Viewport.MaxDepth = maxDepth;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::ScissorDynamic(bool isDynamic)
	{
		m_Desc.Scissor.IsDynamic = isDynamic;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::ScissorOffset(int x, int y)
	{
		m_Desc.Scissor.OffsetX = x;
		m_Desc.Scissor.OffsetY = y;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::ScissorSize(uint32 width, uint32 height)
	{
		m_Desc.Scissor.Width  = width;
		m_Desc.Scissor.Height = height;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthClamp(bool enable)
	{
		m_Desc.Rasterization.DepthClampEnable = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DiscardEnable(bool enable)
	{
		m_Desc.Rasterization.DiscardEnable = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::PolygonMode(EPolygonMode mode)
	{
		m_Desc.Rasterization.PolygonMode = mode;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::LineWidth(float width)
	{
		m_Desc.Rasterization.LineWidth = width;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::CullMode(ECullMode mode)
	{
		m_Desc.Rasterization.CullMode = mode;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::ClockwiseFrontFace(bool clockwise)
	{
		m_Desc.Rasterization.ClockwiseFrontFace = clockwise;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthBiasEnable(bool enable)
	{
		m_Desc.Rasterization.DepthBiasEnable = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthBiasFactor(float factor)
	{
		m_Desc.Rasterization.DepthBiasFactor = factor;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthBiasClamp(float clamp)
	{
		m_Desc.Rasterization.DepthBiasClamp = clamp;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthBiasSlopeFactor(float factor)
	{
		m_Desc.Rasterization.DepthBiasSlopeFactor = factor;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthTestEnable(bool enable)
	{
		m_Desc.DepthStencil.DepthTestEnable = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthWriteEnable(bool enable)
	{
		m_Desc.DepthStencil.DepthWriteEnable = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthCompareOp(ECompareOp op)
	{
		m_Desc.DepthStencil.DepthCompareOp = op;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthBoundsTestEnable(bool enable)
	{
		m_Desc.DepthStencil.DepthBoundsTestEnable = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::DepthBounds(float min, float max)
	{
		m_Desc.DepthStencil.MinDepthBounds = min;
		m_Desc.DepthStencil.MaxDepthBounds = max;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::StencilTestEnable(bool enable)
	{
		m_Desc.DepthStencil.StencilTestEnable = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::StencilFront(StencilOpStateDesc desc)
	{
		m_Desc.DepthStencil.Front = desc;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::StencilBack(StencilOpStateDesc desc)
	{
		m_Desc.DepthStencil.Back = desc;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationGraphicsPipeline::AddColorBlendAttachment()
	{
		return m_ColorBlendAttachmentBuilder;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::LogicOpEnable(bool enable)
	{
		m_Desc.ColorBlendState.LogicOpEnable = enable;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::LogicOp(ELogicOp op)
	{
		m_Desc.ColorBlendState.LogicOp = op;
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationGraphicsPipeline::BlendConstants(float r, float g, float b, float a)
	{
		m_Desc.ColorBlendState.BlendConstants[0] = r;
		m_Desc.ColorBlendState.BlendConstants[1] = g;
		m_Desc.ColorBlendState.BlendConstants[2] = b;
		m_Desc.ColorBlendState.BlendConstants[3] = a;
		return *this;
	}

	IPassDeclaration& PassDeclarationGraphicsPipeline::FinishPipeline()
	{
		return m_Parent;
	}

	void PassDeclarationGraphicsPipeline::CommitVertexInput(const VertexInput& vi)
	{
		m_Desc.VertexInputs.push_back(vi);
	}

	void PassDeclarationGraphicsPipeline::CommitColorBlendAttachment(const ColorBlendAttachmentDesc& desc)
	{
		m_Desc.ColorBlendState.ColorBlendAttachments.push_back(desc);
	}

	const GraphicsPipelineDesc& PassDeclarationGraphicsPipeline::GetDesc() const
	{
		return m_Desc;
	}
} // namespace Poly
