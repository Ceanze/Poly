#pragma once

#include "IPassDeclarationColorBlendAttachment.h"
#include "IPassDeclarationVertexInput.h"
#include "Platform/API/GraphicsPipeline.h"

namespace Poly
{
	class IPassDeclaration;

	class IPassDeclarationGraphicsPipeline
	{
	public:
		virtual ~IPassDeclarationGraphicsPipeline() = default;

		// --- Vertex inputs ---

		/*
		 * Begins building a new vertex input attribute. Chain .Binding(), .Location(), .Format(), .Offset() etc.
		 * then .AddVertexInput() to add another, or .FinishVertexInput() to return here.
		 */
		virtual IPassDeclarationVertexInput& AddVertexInput() = 0;

		// --- Input assembly ---

		virtual IPassDeclarationGraphicsPipeline& Topology(ETopology topology)  = 0;
		virtual IPassDeclarationGraphicsPipeline& RestartPrimitive(bool enable) = 0;

		// --- Viewport ---

		virtual IPassDeclarationGraphicsPipeline& ViewportDynamic(bool isDynamic)               = 0;
		virtual IPassDeclarationGraphicsPipeline& ViewportOffset(float x, float y)              = 0;
		virtual IPassDeclarationGraphicsPipeline& ViewportSize(float width, float height)       = 0;
		virtual IPassDeclarationGraphicsPipeline& ViewportDepth(float minDepth, float maxDepth) = 0;

		// --- Scissor ---

		virtual IPassDeclarationGraphicsPipeline& ScissorDynamic(bool isDynamic)           = 0;
		virtual IPassDeclarationGraphicsPipeline& ScissorOffset(int x, int y)              = 0;
		virtual IPassDeclarationGraphicsPipeline& ScissorSize(uint32 width, uint32 height) = 0;

		// --- Rasterization ---

		virtual IPassDeclarationGraphicsPipeline& DepthClamp(bool enable)            = 0;
		virtual IPassDeclarationGraphicsPipeline& DiscardEnable(bool enable)         = 0;
		virtual IPassDeclarationGraphicsPipeline& PolygonMode(EPolygonMode mode)     = 0;
		virtual IPassDeclarationGraphicsPipeline& LineWidth(float width)             = 0;
		virtual IPassDeclarationGraphicsPipeline& CullMode(ECullMode mode)           = 0;
		virtual IPassDeclarationGraphicsPipeline& ClockwiseFrontFace(bool clockwise) = 0;
		virtual IPassDeclarationGraphicsPipeline& DepthBiasEnable(bool enable)       = 0;
		virtual IPassDeclarationGraphicsPipeline& DepthBiasFactor(float factor)      = 0;
		virtual IPassDeclarationGraphicsPipeline& DepthBiasClamp(float clamp)        = 0;
		virtual IPassDeclarationGraphicsPipeline& DepthBiasSlopeFactor(float factor) = 0;

		// --- Depth / stencil ---

		virtual IPassDeclarationGraphicsPipeline& DepthTestEnable(bool enable)          = 0;
		virtual IPassDeclarationGraphicsPipeline& DepthWriteEnable(bool enable)         = 0;
		virtual IPassDeclarationGraphicsPipeline& DepthCompareOp(ECompareOp op)         = 0;
		virtual IPassDeclarationGraphicsPipeline& DepthBoundsTestEnable(bool enable)    = 0;
		virtual IPassDeclarationGraphicsPipeline& DepthBounds(float min, float max)     = 0;
		virtual IPassDeclarationGraphicsPipeline& StencilTestEnable(bool enable)        = 0;
		virtual IPassDeclarationGraphicsPipeline& StencilFront(StencilOpStateDesc desc) = 0;
		virtual IPassDeclarationGraphicsPipeline& StencilBack(StencilOpStateDesc desc)  = 0;

		// --- Color blend state ---

		/*
		 * Begins building a new color blend attachment. Chain blend setters then
		 * .AddColorBlendAttachment() for another, or .FinishColorBlendAttachment() to return here.
		 */
		virtual IPassDeclarationColorBlendAttachment& AddColorBlendAttachment()                          = 0;
		virtual IPassDeclarationGraphicsPipeline&     LogicOpEnable(bool enable)                         = 0;
		virtual IPassDeclarationGraphicsPipeline&     LogicOp(ELogicOp op)                               = 0;
		virtual IPassDeclarationGraphicsPipeline&     BlendConstants(float r, float g, float b, float a) = 0;

		/*
		 * Ends the pipeline override declaration and returns to PassDeclaration.
		 */
		virtual IPassDeclaration& FinishPipeline() = 0;
	};
} // namespace Poly
