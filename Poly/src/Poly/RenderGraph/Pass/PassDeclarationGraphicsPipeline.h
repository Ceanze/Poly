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

		PassDeclarationGraphicsPipeline& Topology(ETopology topology) override;
		PassDeclarationGraphicsPipeline& RestartPrimitive(bool enable) override;

		// --- Viewport ---

		PassDeclarationGraphicsPipeline& ViewportDynamic(bool isDynamic) override;
		PassDeclarationGraphicsPipeline& ViewportOffset(float x, float y) override;
		PassDeclarationGraphicsPipeline& ViewportSize(float width, float height) override;
		PassDeclarationGraphicsPipeline& ViewportDepth(float minDepth, float maxDepth) override;

		// --- Scissor ---

		PassDeclarationGraphicsPipeline& ScissorDynamic(bool isDynamic) override;
		PassDeclarationGraphicsPipeline& ScissorOffset(int x, int y) override;
		PassDeclarationGraphicsPipeline& ScissorSize(uint32 width, uint32 height) override;

		// --- Rasterization ---

		PassDeclarationGraphicsPipeline& DepthClamp(bool enable) override;
		PassDeclarationGraphicsPipeline& DiscardEnable(bool enable) override;
		PassDeclarationGraphicsPipeline& PolygonMode(EPolygonMode mode) override;
		PassDeclarationGraphicsPipeline& LineWidth(float width) override;
		PassDeclarationGraphicsPipeline& CullMode(ECullMode mode) override;
		PassDeclarationGraphicsPipeline& ClockwiseFrontFace(bool clockwise) override;
		PassDeclarationGraphicsPipeline& DepthBiasEnable(bool enable) override;
		PassDeclarationGraphicsPipeline& DepthBiasFactor(float factor) override;
		PassDeclarationGraphicsPipeline& DepthBiasClamp(float clamp) override;
		PassDeclarationGraphicsPipeline& DepthBiasSlopeFactor(float factor) override;

		// --- Depth / stencil ---

		PassDeclarationGraphicsPipeline& DepthTestEnable(bool enable) override;
		PassDeclarationGraphicsPipeline& DepthWriteEnable(bool enable) override;
		PassDeclarationGraphicsPipeline& DepthCompareOp(ECompareOp op) override;
		PassDeclarationGraphicsPipeline& DepthBoundsTestEnable(bool enable) override;
		PassDeclarationGraphicsPipeline& DepthBounds(float min, float max) override;
		PassDeclarationGraphicsPipeline& StencilTestEnable(bool enable) override;
		PassDeclarationGraphicsPipeline& StencilFront(StencilOpStateDesc desc) override;
		PassDeclarationGraphicsPipeline& StencilBack(StencilOpStateDesc desc) override;

		// --- Color blend state ---

		PassDeclarationColorBlendAttachment& AddColorBlendAttachment() override;
		PassDeclarationGraphicsPipeline&     LogicOpEnable(bool enable) override;
		PassDeclarationGraphicsPipeline&     LogicOp(ELogicOp op) override;
		PassDeclarationGraphicsPipeline&     BlendConstants(float r, float g, float b, float a) override;

		PassDeclaration& FinishPipeline() override;

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
