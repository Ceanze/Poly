#pragma once

#include "IPassDeclarationGraphicsPipeline.h"
#include "PassDeclarationColorBlendAttachment.h"
#include "PassDeclarationVertexInput.h"

namespace Poly
{
	class PassDeclaration;

	class PassDeclarationGraphicsPipeline : public IPassDeclarationGraphicsPipeline
	{
	public:
		explicit PassDeclarationGraphicsPipeline(PassDeclaration& parent);

		// --- Vertex inputs ---

		PassDeclarationVertexInput& AddVertexInput() override;

		// --- Input assembly ---

		IPassDeclarationGraphicsPipeline& Topology(ETopology topology) override;
		IPassDeclarationGraphicsPipeline& RestartPrimitive(bool enable) override;

		// --- Viewport ---

		IPassDeclarationGraphicsPipeline& ViewportDynamic(bool isDynamic) override;
		IPassDeclarationGraphicsPipeline& ViewportOffset(float x, float y) override;
		IPassDeclarationGraphicsPipeline& ViewportSize(float width, float height) override;
		IPassDeclarationGraphicsPipeline& ViewportDepth(float minDepth, float maxDepth) override;

		// --- Scissor ---

		IPassDeclarationGraphicsPipeline& ScissorDynamic(bool isDynamic) override;
		IPassDeclarationGraphicsPipeline& ScissorOffset(int x, int y) override;
		IPassDeclarationGraphicsPipeline& ScissorSize(uint32 width, uint32 height) override;

		// --- Rasterization ---

		IPassDeclarationGraphicsPipeline& DepthClamp(bool enable) override;
		IPassDeclarationGraphicsPipeline& DiscardEnable(bool enable) override;
		IPassDeclarationGraphicsPipeline& PolygonMode(EPolygonMode mode) override;
		IPassDeclarationGraphicsPipeline& LineWidth(float width) override;
		IPassDeclarationGraphicsPipeline& CullMode(ECullMode mode) override;
		IPassDeclarationGraphicsPipeline& ClockwiseFrontFace(bool clockwise) override;
		IPassDeclarationGraphicsPipeline& DepthBiasEnable(bool enable) override;
		IPassDeclarationGraphicsPipeline& DepthBiasFactor(float factor) override;
		IPassDeclarationGraphicsPipeline& DepthBiasClamp(float clamp) override;
		IPassDeclarationGraphicsPipeline& DepthBiasSlopeFactor(float factor) override;

		// --- Depth / stencil ---

		IPassDeclarationGraphicsPipeline& DepthTestEnable(bool enable) override;
		IPassDeclarationGraphicsPipeline& DepthWriteEnable(bool enable) override;
		IPassDeclarationGraphicsPipeline& DepthCompareOp(ECompareOp op) override;
		IPassDeclarationGraphicsPipeline& DepthBoundsTestEnable(bool enable) override;
		IPassDeclarationGraphicsPipeline& DepthBounds(float min, float max) override;
		IPassDeclarationGraphicsPipeline& StencilTestEnable(bool enable) override;
		IPassDeclarationGraphicsPipeline& StencilFront(StencilOpStateDesc desc) override;
		IPassDeclarationGraphicsPipeline& StencilBack(StencilOpStateDesc desc) override;

		// --- Color blend state ---

		PassDeclarationColorBlendAttachment& AddColorBlendAttachment() override;
		IPassDeclarationGraphicsPipeline&    LogicOpEnable(bool enable) override;
		IPassDeclarationGraphicsPipeline&    LogicOp(ELogicOp op) override;
		IPassDeclarationGraphicsPipeline&    BlendConstants(float r, float g, float b, float a) override;

		IPassDeclaration& FinishPipeline() override;

		void CommitVertexInput(const VertexInput& vi);
		void CommitColorBlendAttachment(const ColorBlendAttachmentDesc& desc);

		const GraphicsPipelineDesc& GetDesc() const;

	private:
		PassDeclaration&                    m_Parent;
		GraphicsPipelineDesc                m_Desc = {};
		PassDeclarationVertexInput          m_VertexInputBuilder;
		PassDeclarationColorBlendAttachment m_ColorBlendAttachmentBuilder;
	};
} // namespace Poly
